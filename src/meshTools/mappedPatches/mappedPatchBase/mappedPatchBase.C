/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2022 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "mappedPatchBase.H"
#include "SubField.H"
#include "Time.H"
#include "triPointRef.H"
#include "treeDataFace.H"
#include "indexedOctree.H"
#include "globalIndex.H"
#include "RemoteData.H"
#include "OBJstream.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(mappedPatchBase, 0);

    const scalar mappedPatchBase::defaultMatchTol_ = 1e-4;
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

Foam::tmp<Foam::vectorField> Foam::mappedPatchBase::patchFaceAreas() const
{
    return patch_.primitivePatch::faceAreas();
}


Foam::tmp<Foam::pointField> Foam::mappedPatchBase::patchFaceCentres() const
{
    return patch_.primitivePatch::faceCentres();
}


Foam::tmp<Foam::pointField> Foam::mappedPatchBase::patchLocalPoints() const
{
    return patch_.localPoints();
}


void Foam::mappedPatchBase::calcMapping() const
{
    if (mapPtr_.valid())
    {
        FatalErrorInFunction
            << "Mapping already calculated" << exit(FatalError);
    }

    // Calculate the transform as necessary
    transform_ =
        cyclicTransform
        (
            patch_.name(),
            patchFaceCentres(),
            patchFaceAreas(),
            transform_,
            nbrPolyPatch().name(),
            nbrPolyPatch().faceCentres(),
            nbrPolyPatch().faceAreas(),
            nbrPatchIsMapped()
          ? nbrMappedPatch().transform_
          : cyclicTransform(false),
            matchTol_,
            true
        );

    // Do a sanity check. Am I sampling my own patch? This only makes sense if
    // the position is transformed.
    if
    (
        nbrRegionName() == patch_.boundaryMesh().mesh().name()
     && nbrPatchName() == patch_.name()
     && !transform_.transform().transformsPosition()
    )
    {
        FatalErrorInFunction
            << "Patch " << patch_.name() << " is sampling itself with no "
            << "transform. The patch face values are undefined."
            << exit(FatalError);

    }

    // Build the mapping...
    //
    // This octree based solution is deprecated. The "matching" patch-to-patch
    // method is equivalent, less code, and should be more efficiently
    // parallelised.
    if (!patchToPatchIsUsed_)
    {
        const globalIndex patchGlobalIndex(patch_.size());

        // Find processor and cell/face indices of samples
        labelList sampleGlobalPatchFaces, sampleIndices;
        {
            // Lookup the correct region
            const polyMesh& mesh = nbrMesh();

            // Gather the sample points into a single globally indexed list
            List<point> allPoints(patchGlobalIndex.size());
            {
                List<pointField> procSamplePoints(Pstream::nProcs());
                procSamplePoints[Pstream::myProcNo()] =
                    transform_.transform().invTransformPosition
                    (
                        patchFaceCentres()
                    );
                Pstream::gatherList(procSamplePoints);
                Pstream::scatterList(procSamplePoints);

                forAll(procSamplePoints, proci)
                {
                    forAll(procSamplePoints[proci], procSamplei)
                    {
                        allPoints
                        [
                            patchGlobalIndex.toGlobal(proci, procSamplei)
                        ] = procSamplePoints[proci][procSamplei];
                    }
                }
            }

            // List of possibly remote mapped faces
            List<RemoteData<scalar>> allNearest(patchGlobalIndex.size());

            // Find nearest face opposite every face
            if (nbrPolyPatch().empty())
            {
                forAll(allPoints, alli)
                {
                    allNearest[alli].proci = -1;
                    allNearest[alli].elementi = -1;
                    allNearest[alli].data = Foam::sqr(great);
                }
            }
            else
            {
                const polyPatch& pp = nbrPolyPatch();

                const labelList patchFaces(identity(pp.size()) + pp.start());

                const treeBoundBox patchBb
                (
                    treeBoundBox(pp.points(), pp.meshPoints()).extend(1e-4)
                );

                const indexedOctree<treeDataFace> boundaryTree
                (
                    treeDataFace    // all information needed to search faces
                    (
                        false,      // do not cache bb
                        mesh,
                        patchFaces  // boundary faces only
                    ),
                    patchBb,        // overall search domain
                    8,              // maxLevel
                    10,             // leafsize
                    3.0             // duplicity
                );

                forAll(allPoints, alli)
                {
                    const point& p = allPoints[alli];

                    const pointIndexHit pih =
                        boundaryTree.findNearest(p, magSqr(patchBb.span()));

                    if (pih.hit())
                    {
                        const point fc = pp[pih.index()].centre(pp.points());

                        allNearest[alli].proci = Pstream::myProcNo();
                        allNearest[alli].elementi = pih.index();
                        allNearest[alli].data = magSqr(fc - p);
                    }
                }
            }

            // Find nearest. Combine on master.
            Pstream::listCombineGather
            (
                allNearest,
                RemoteData<scalar>::smallestEqOp()
            );
            Pstream::listCombineScatter(allNearest);

            // Determine the number of faces for which a nearest neighbouring
            // face was not found (no reduction necessary as this is computed
            // from synchronised data)
            label nNotFound = 0;
            forAll(allPoints, alli)
            {
                if (allNearest[alli].proci == -1)
                {
                    nNotFound ++;
                }
            }

            // If any points were not found within cells then re-search for
            // them using a nearest test, which should not fail. Warn that this
            // is happening. If any points were not found for some other
            // method, then fail.
            if (nNotFound)
            {
                FatalErrorInFunction
                    << "Mapping failed for " << nl
                    << "    patch: " << patch_.name() << nl
                    << "    neighbourRegion: " << nbrRegionName() << nl
                    << "    neighbourPatch: " << nbrPatchName() << nl
                    << exit(FatalError);
            }

            // Build lists of samples
            DynamicList<label> samplePatchGlobalFacesDyn;
            DynamicList<label> sampleIndicesDyn;
            forAll(allNearest, alli)
            {
                if (allNearest[alli].proci == Pstream::myProcNo())
                {
                    samplePatchGlobalFacesDyn.append(alli);
                    sampleIndicesDyn.append(allNearest[alli].elementi);
                }
            }
            sampleGlobalPatchFaces.transfer(samplePatchGlobalFacesDyn);
            sampleIndices.transfer(sampleIndicesDyn);
        }

        // Construct distribution schedule
        List<Map<label>> compactMap;
        mapPtr_.reset
        (
            new distributionMap
            (
                patchGlobalIndex,
                sampleGlobalPatchFaces,
                compactMap
            )
        );
        const labelList oldToNew(move(sampleGlobalPatchFaces));
        const labelList oldSampleIndices(move(sampleIndices));

        // Construct input mapping for data to be distributed
        nbrPatchFaceIndices_ = labelList(mapPtr_->constructSize(), -1);
        UIndirectList<label>(nbrPatchFaceIndices_, oldToNew) = oldSampleIndices;

        // Reverse the map. This means the map is "forward" when going from
        // the neighbour patch to this patch, which is logical.
        mapPtr_.reset
        (
            new distributionMap
            (
                patch_.size(),
                move(mapPtr_->constructMap()),
                move(mapPtr_->subMap())
            )
        );
    }

    // This (much simpler) patch-to-patch solution supersedes the above
    else
    {
        if (patchToPatchIsValid_)
        {
            FatalErrorInFunction
                << "Patch-to-patch already calculated" << exit(FatalError);
        }

        const pointField patchLocalPoints(this->patchLocalPoints());

        const primitivePatch patch
        (
            SubList<face>(patch_.localFaces(), patch_.size()),
            patchLocalPoints
        );

        patchToPatchPtr_->update
        (
            patch,
            patch.pointNormals(),
            nbrPolyPatch(),
            transform_.transform()
        );

        patchToPatchIsValid_ = true;
    }
}


// * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * * * * //

Foam::mappedPatchBase::mappedPatchBase
(
    const polyPatch& pp
)
:
    patch_(pp),
    nbrRegionName_(patch_.boundaryMesh().mesh().name()),
    nbrPatchName_(word::null),
    coupleGroup_(),
    transform_(true),
    mapPtr_(nullptr),
    nbrPatchFaceIndices_(),
    patchToPatchIsUsed_(false),
    patchToPatchIsValid_(false),
    patchToPatchPtr_(nullptr),
    matchTol_(defaultMatchTol_)
{}


Foam::mappedPatchBase::mappedPatchBase
(
    const polyPatch& pp,
    const word& nbrRegionName,
    const word& nbrPatchName,
    const cyclicTransform& transform
)
:
    patch_(pp),
    nbrRegionName_(nbrRegionName),
    nbrPatchName_(nbrPatchName),
    coupleGroup_(),
    transform_(transform),
    mapPtr_(nullptr),
    nbrPatchFaceIndices_(),
    patchToPatchIsUsed_(false),
    patchToPatchIsValid_(false),
    patchToPatchPtr_(nullptr),
    matchTol_(defaultMatchTol_)
{}


Foam::mappedPatchBase::mappedPatchBase
(
    const polyPatch& pp,
    const dictionary& dict,
    const bool transformIsNone
)
:
    patch_(pp),
    nbrRegionName_
    (
        dict.lookupOrDefaultBackwardsCompatible<word>
        (
            {"neighbourRegion", "sampleRegion"},
            word::null
        )
    ),
    nbrPatchName_
    (
        dict.lookupOrDefaultBackwardsCompatible<word>
        (
            {"neighbourPatch", "samplePatch"},
            word::null
        )
    ),
    coupleGroup_(dict),
    transform_
    (
        transformIsNone
      ? cyclicTransform(true)
      : cyclicTransform(dict, false)
    ),
    mapPtr_(nullptr),
    nbrPatchFaceIndices_(),
    patchToPatchIsUsed_(dict.found("method") || dict.found("sampleMode")),
    patchToPatchIsValid_(false),
    patchToPatchPtr_
    (
        patchToPatchIsUsed_
      ? patchToPatch::New
        (
            dict.lookupBackwardsCompatible<word>({"method", "sampleMode"}),
            false
        ).ptr()
      : nullptr
    ),
    matchTol_(dict.lookupOrDefault("matchTolerance", defaultMatchTol_))
{
    if (!coupleGroup_.valid() && nbrRegionName_.empty())
    {
        nbrRegionName_ = patch_.boundaryMesh().mesh().name();
    }
}


Foam::mappedPatchBase::mappedPatchBase
(
    const polyPatch& pp,
    const mappedPatchBase& mpb
)
:
    patch_(pp),
    nbrRegionName_(mpb.nbrRegionName_),
    nbrPatchName_(mpb.nbrPatchName_),
    coupleGroup_(mpb.coupleGroup_),
    transform_(mpb.transform_),
    mapPtr_(nullptr),
    nbrPatchFaceIndices_(),
    patchToPatchIsUsed_(mpb.patchToPatchIsUsed_),
    patchToPatchIsValid_(false),
    patchToPatchPtr_
    (
        patchToPatchIsUsed_
      ? patchToPatch::New(mpb.patchToPatchPtr_->type(), false).ptr()
      : nullptr
    ),
    matchTol_(mpb.matchTol_)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::mappedPatchBase::~mappedPatchBase()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const Foam::polyMesh& Foam::mappedPatchBase::nbrMesh() const
{
    return patch_.boundaryMesh().mesh().time().lookupObject<polyMesh>
    (
        nbrRegionName()
    );
}


const Foam::polyPatch& Foam::mappedPatchBase::nbrPolyPatch() const
{
    const polyMesh& nbrMesh = this->nbrMesh();

    const label patchi = nbrMesh.boundaryMesh().findPatchID(nbrPatchName());

    if (patchi == -1)
    {
        FatalErrorInFunction
            << "Cannot find patch " << nbrPatchName()
            << " in region " << nbrRegionName() << endl
            << "Valid patches are " << nbrMesh.boundaryMesh().names()
            << exit(FatalError);
    }

    return nbrMesh.boundaryMesh()[patchi];
}


void Foam::mappedPatchBase::clearOut()
{
    mapPtr_.clear();
    nbrPatchFaceIndices_.clear();
    patchToPatchIsValid_ = false;
}


bool Foam::mappedPatchBase::specified(const dictionary& dict)
{
    return
        dict.found("neighbourRegion")
     || dict.found("neighbourPatch")
     || dict.found("coupleGroup");
}


void Foam::mappedPatchBase::write(Ostream& os) const
{
    writeEntryIfDifferent(os, "neighbourRegion", word::null, nbrRegionName_);
    writeEntryIfDifferent(os, "neighbourPatch", word::null, nbrPatchName_);

    coupleGroup_.write(os);

    transform_.write(os);

    if (patchToPatchIsUsed_)
    {
        writeEntry(os, "method", patchToPatchPtr_->type());
    }

    writeEntryIfDifferent(os, "matchTolerance", defaultMatchTol_, matchTol_);
}


// ************************************************************************* //
