/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2022 OpenFOAM Foundation
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

Description
    Perform mapping of finite volume fields required by stitching.

\*---------------------------------------------------------------------------*/

#include "volFields.H"
#include "surfaceFields.H"
#include "fvPatchFieldMapper.H"
#include "fvMeshStitcher.H"
#include "setSizeFieldMapper.H"
#include "nonConformalBoundary.H"
#include "nonConformalCyclicFvPatch.H"
#include "nonConformalProcessorCyclicFvPatch.H"
#include "nonConformalErrorFvPatch.H"
#include "processorFvPatch.H"
#include "surfaceToVolVelocity.H"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class Type, template<class> class GeoField>
void Foam::fvMeshStitcher::resizePatchFields()
{
    struct fvPatchFieldSetSizer
    :
        public fvPatchFieldMapper,
        public setSizeFieldMapper
    {
        fvPatchFieldSetSizer(const label size)
        :
            setSizeFieldMapper(size)
        {}
    };

    HashTable<GeoField<Type>*> fields(mesh_.lookupClass<GeoField<Type>>());
    forAllIter(typename HashTable<GeoField<Type>*>, fields, iter)
    {
        forAll(mesh_.boundary(), patchi)
        {
            typename GeoField<Type>::Patch& pf =
                iter()->boundaryFieldRef()[patchi];

            if (isA<nonConformalFvPatch>(pf.patch()))
            {
                pf.autoMap(fvPatchFieldSetSizer(pf.patch().size()));
            }
        }
    }
}


template<template<class> class GeoField>
void Foam::fvMeshStitcher::resizePatchFields()
{
    #define ResizePatchFields(Type, nullArg) \
        resizePatchFields<Type, GeoField>();
    FOR_ALL_FIELD_TYPES(ResizePatchFields);
    #undef ResizePatchFields
}


template<class Type>
void Foam::fvMeshStitcher::preConformSurfaceFields()
{
    HashTable<SurfaceField<Type>*> fields
    (
        mesh_.lookupClass<SurfaceField<Type>>()
    );

    const surfaceScalarField* phiPtr =
        mesh_.foundObject<surfaceScalarField>("meshPhi")
      ? &mesh_.lookupObject<surfaceScalarField>("meshPhi")
      : nullptr;

    forAllIter(typename HashTable<SurfaceField<Type>*>, fields, iter)
    {
        SurfaceField<Type>& field = *iter();

        if
        (
            &field == static_cast<const regIOobject*>(phiPtr)
         || field.isOldTime()
        ) continue;

        autoPtr<SurfaceField<Type>> nccFieldPtr
        (
            new SurfaceField<Type>
            (
                IOobject
                (
                    nccFieldPrefix_ + field.name(),
                    mesh_.time().timeName(),
                    mesh_
                ),
                field
            )
        );

        for (label i = 0; i <= field.nOldTimes(); ++ i)
        {
            SurfaceField<Type>& field0 = field.oldTime(i);

            nccFieldPtr->oldTime(i).boundaryFieldRef() =
                conformalNccBoundaryField<Type>(field0.boundaryField());

            field0.boundaryFieldRef() =
                conformalOrigBoundaryField<Type>(field0.boundaryField());
        }

        nccFieldPtr.ptr()->store();
    }
}


inline void Foam::fvMeshStitcher::preConformSurfaceFields()
{
    #define PreConformSurfaceFields(Type, nullArg) \
        preConformSurfaceFields<Type>();
    FOR_ALL_FIELD_TYPES(PreConformSurfaceFields);
    #undef PreConformSurfaceFields
}


template<class Type>
void Foam::fvMeshStitcher::postNonConformSurfaceFields()
{
    HashTable<SurfaceField<Type>*> fields
    (
        mesh_.lookupClass<SurfaceField<Type>>()
    );

    const surfaceScalarField* phiPtr =
        mesh_.foundObject<surfaceScalarField>("meshPhi")
      ? &mesh_.lookupObject<surfaceScalarField>("meshPhi")
      : nullptr;

    forAllIter(typename HashTable<SurfaceField<Type>*>, fields, iter)
    {
        if (iter.key()(nccFieldPrefix_.size()) == nccFieldPrefix_) continue;

        SurfaceField<Type>& field = *iter();

        if
        (
            &field == static_cast<const regIOobject*>(phiPtr)
         || field.isOldTime()
         || mesh_.topoChanged()
        ) continue;

        const word nccFieldName = nccFieldPrefix_ + field.name();

        const SurfaceField<Type>& nccField =
            mesh_.lookupObject<SurfaceField<Type>>(nccFieldName);

        for (label i = 0; i <= field.nOldTimes(); ++ i)
        {
            SurfaceField<Type>& field0 = field.oldTime(i);

            field0.boundaryFieldRef() =
                nonConformalBoundaryField<Type>
                (
                    nccField.oldTime(i).boundaryField(),
                    field0.boundaryField()
                );

            field0.boundaryFieldRef() =
                synchronisedBoundaryField<Type>(field0.boundaryField());
        }
    }

    // Remove NCC fields after all fields have been mapped. This is so that
    // old-time fields aren't removed by current-time fields in advance of the
    // old-time field being mapped.
    forAllIter(typename HashTable<SurfaceField<Type>*>, fields, iter)
    {
        if (iter.key()(nccFieldPrefix_.size()) == nccFieldPrefix_) continue;

        SurfaceField<Type>& field = *iter();

        if
        (
            &field == static_cast<const regIOobject*>(phiPtr)
         || field.isOldTime()
        ) continue;

        const word nccFieldName = nccFieldPrefix_ + field.name();

        SurfaceField<Type>& nccField =
            mesh_.lookupObjectRef<SurfaceField<Type>>(nccFieldName);

        nccField.checkOut();
    }

    // Check there are no NCC fields left over
    fields = mesh_.lookupClass<SurfaceField<Type>>();
    forAllIter(typename HashTable<SurfaceField<Type>*>, fields, iter)
    {
        if (iter.key()(nccFieldPrefix_.size()) != nccFieldPrefix_) continue;

        FatalErrorInFunction
            << "Stitching mapping field \"" << iter()->name()
            << "\" found, but the field it corresponds to no longer exists"
            << exit(FatalError);
    }
}


inline void Foam::fvMeshStitcher::postNonConformSurfaceFields()
{
    #define PostNonConformSurfaceFields(Type, nullArg) \
        postNonConformSurfaceFields<Type>();
    FOR_ALL_FIELD_TYPES(PostNonConformSurfaceFields);
    #undef PostNonConformSurfaceFields
}


template<class Type>
void Foam::fvMeshStitcher::evaluateVolFields()
{
    HashTable<VolField<Type>*> fields(mesh_.lookupClass<VolField<Type>>());
    forAllIter(typename HashTable<VolField<Type>*>, fields, iter)
    {
        const label nReq = Pstream::nRequests();

        forAll(mesh_.boundary(), patchi)
        {
            typename VolField<Type>::Patch& pf =
                iter()->boundaryFieldRef()[patchi];

            if (isA<nonConformalFvPatch>(pf.patch()))
            {
                pf.initEvaluate(Pstream::defaultCommsType);
            }
        }

        if
        (
            Pstream::parRun()
         && Pstream::defaultCommsType == Pstream::commsTypes::nonBlocking
        )
        {
            Pstream::waitRequests(nReq);
        }

        forAll(mesh_.boundary(), patchi)
        {
            typename VolField<Type>::Patch& pf =
                iter()->boundaryFieldRef()[patchi];

            if (isA<nonConformalFvPatch>(pf.patch()))
            {
                pf.evaluate(Pstream::defaultCommsType);
            }
        }
    }
}


inline void Foam::fvMeshStitcher::evaluateVolFields()
{
    #define EvaluateVolFields(Type, nullArg) \
        evaluateVolFields<Type>();
    FOR_ALL_FIELD_TYPES(EvaluateVolFields);
    #undef EvaluateVolFields
}


inline void Foam::fvMeshStitcher::postNonConformSurfaceVelocities()
{
    if (mesh_.topoChanged())
    {
        HashTable<surfaceVectorField*> Ufs
        (
            mesh_.lookupClass<surfaceVectorField>()
        );

        forAllIter(HashTable<surfaceVectorField*>, Ufs, iter)
        {
            surfaceVectorField& Uf = *iter();

            const volVectorField& U = surfaceToVolVelocity(Uf);

            if (!isNull(U))
            {
                forAll(Uf.boundaryField(), patchi)
                {
                    if (isA<nonConformalFvPatch>(mesh_.boundary()[patchi]))
                    {
                        Uf.boundaryFieldRef()[patchi] ==
                            U.boundaryField()[patchi];
                    }
                }
            }
        }
    }
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

template<class GeoBoundaryField>
void Foam::fvMeshStitcher::resizeBoundaryFieldPatchFields
(
    const SurfaceFieldBoundary<label>& polyFacesBf,
    GeoBoundaryField& fieldBf
)
{
    struct fvPatchFieldSetSizer
    :
        public fvPatchFieldMapper,
        public setSizeFieldMapper
    {
        fvPatchFieldSetSizer(const label size)
        :
            setSizeFieldMapper(size)
        {}
    };

    forAll(polyFacesBf, nccPatchi)
    {
        if (isA<nonConformalFvPatch>(polyFacesBf[nccPatchi].patch()))
        {
            fieldBf[nccPatchi].autoMap
            (
                fvPatchFieldSetSizer(polyFacesBf[nccPatchi].size())
            );
        }
    }
}

template<class GeoField>
void Foam::fvMeshStitcher::resizeFieldPatchFields
(
    const SurfaceFieldBoundary<label>& polyFacesBf,
    GeoField& field
)
{
    for (label i = 0; i <= field.nOldTimes(); ++ i)
    {
        resizeBoundaryFieldPatchFields
        (
            polyFacesBf,
            field.oldTime(i).boundaryFieldRef()
        );
    }
}


template<class Type>
Foam::tmp<Foam::Field<Type>> Foam::fvMeshStitcher::fieldRMapSum
(
    const Field<Type>& f,
    const label size,
    const labelUList& addr
)
{
    tmp<Field<Type>> tresult(new Field<Type>(size, Zero));
    forAll(addr, i)
    {
        tresult.ref()[addr[i]] += f[i];
    }
    return tresult;
}


template<class Type>
Foam::tmp<Foam::Field<Type>> Foam::fvMeshStitcher::fieldRMapSum
(
    const tmp<Field<Type>>& f,
    const label size,
    const labelUList& addr
)
{
    tmp<Field<Type>> tresult = fieldRMapSum(f(), size, addr);
    f.clear();
    return tresult;
}


inline Foam::tmp<Foam::surfaceScalarField::Boundary>
Foam::fvMeshStitcher::getOrigNccMagSfb() const
{
    const fvBoundaryMesh& fvbm = mesh_.boundary();

    const surfaceScalarField::Boundary& magSfb =
        fvbm.mesh().magSf().boundaryField();

    tmp<surfaceScalarField::Boundary> tOrigNccMagSfb
    (
        new surfaceScalarField::Boundary
        (
            fvbm,
            surfaceScalarField::Internal::null(),
            calculatedFvPatchField<scalar>::typeName
        )
    );

    surfaceScalarField::Boundary& origNccMagSfb = tOrigNccMagSfb.ref();

    origNccMagSfb == 0;

    forAll(fvbm, nccPatchi)
    {
        const fvPatch& fvp = fvbm[nccPatchi];

        if (isA<nonConformalCoupledFvPatch>(fvp))
        {
            const nonConformalCoupledFvPatch& nccFvp =
                refCast<const nonConformalCoupledFvPatch>(fvp);

            const label origPatchi = nccFvp.origPatchID();
            const fvPatch& origFvp = nccFvp.origPatch();

            const labelList nccOrigPatchFace =
                nccFvp.polyFaces() - origFvp.start();

            origNccMagSfb[origPatchi] +=
                fieldRMapSum
                (
                    magSfb[nccPatchi],
                    origFvp.size(),
                    nccOrigPatchFace
                );
        }
    }

    return tOrigNccMagSfb;
}


template<class Type>
Foam::tmp<Foam::fvMeshStitcher::SurfaceFieldBoundary<Type>>
Foam::fvMeshStitcher::conformalNccBoundaryField
(
    const SurfaceFieldBoundary<Type>& fieldb
) const
{
    const bool isFluxField = isFlux(fieldb[0].internalField());

    const fvBoundaryMesh& fvbm = fieldb[0].patch().boundaryMesh();

    const surfaceScalarField::Boundary& magSfb =
        fvbm.mesh().magSf().boundaryField();

    tmp<SurfaceFieldBoundary<Type>> tnccFieldb
    (
        new SurfaceFieldBoundary<Type>
        (
            SurfaceField<Type>::Internal::null(),
            fieldb
        )
    );

    SurfaceFieldBoundary<Type>& nccFieldb = tnccFieldb.ref();

    nccFieldb == pTraits<Type>::zero;

    const surfaceScalarField::Boundary origNccMagSfb
    (
        surfaceScalarField::Internal::null(),
        getOrigNccMagSfb()
    );

    // Accumulate the non-conformal parts of the field into the original faces
    forAll(fvbm, nccPatchi)
    {
        const fvPatch& fvp = fvbm[nccPatchi];

        if (isA<nonConformalCoupledFvPatch>(fvp))
        {
            const nonConformalCoupledFvPatch& nccFvp =
                refCast<const nonConformalCoupledFvPatch>(fvbm[nccPatchi]);

            const label origPatchi = nccFvp.origPatchID();
            const fvPatch& origFvp = nccFvp.origPatch();

            const labelList nccOrigPatchFace =
                nccFvp.polyFaces() - origFvp.start();

            // If this is a flux then sum
            if (isFluxField)
            {
                nccFieldb[origPatchi] +=
                    fieldRMapSum
                    (
                        fieldb[nccPatchi],
                        origFvp.size(),
                        nccOrigPatchFace
                    );
            }

            // If not a flux then do an area-weighted sum
            else
            {
                nccFieldb[origPatchi] +=
                    fieldRMapSum
                    (
                        fieldb[nccPatchi]*magSfb[nccPatchi],
                        origFvp.size(),
                        nccOrigPatchFace
                    );
            }
        }
    }

    const labelList origPatchIDs =
        nonConformalBoundary::New(mesh_).allOrigPatchIDs();

    // Scale or average as appropriate
    forAll(origPatchIDs, i)
    {
        const label origPatchi = origPatchIDs[i];

        // If this is a flux then scale to the total size of the face
        if (isFluxField)
        {
            const scalarField origSumMagSf
            (
                magSfb[origPatchi] + origNccMagSfb[origPatchi]
            );

            nccFieldb[origPatchi] *=
                origSumMagSf
               /max(origNccMagSfb[origPatchi], small*origSumMagSf);
        }

        // If this is not a flux then convert to an area-weighted average
        else
        {
            nccFieldb[origPatchi] /=
                max(origNccMagSfb[origPatchi], vSmall);
        }
    }

    return tnccFieldb;
}


template<class Type>
Foam::tmp<Foam::fvMeshStitcher::SurfaceFieldBoundary<Type>>
Foam::fvMeshStitcher::conformalOrigBoundaryField
(
    const SurfaceFieldBoundary<Type>& fieldb
) const
{
    const bool isFluxField = isFlux(fieldb[0].internalField());

    const fvBoundaryMesh& fvbm = fieldb[0].patch().boundaryMesh();

    const surfaceScalarField::Boundary& magSfb =
        fvbm.mesh().magSf().boundaryField();

    tmp<SurfaceFieldBoundary<Type>> torigFieldb
    (
        new SurfaceFieldBoundary<Type>
        (
            SurfaceField<Type>::Internal::null(),
            fieldb
        )
    );

    // If this is a flux then scale up to the total face areas
    if (isFluxField)
    {
        const surfaceScalarField::Boundary origNccMagSfb
        (
            surfaceScalarField::Internal::null(),
            getOrigNccMagSfb()
        );

        SurfaceFieldBoundary<Type>& origFieldb = torigFieldb.ref();

        const labelList origPatchIDs =
            nonConformalBoundary::New(mesh_).allOrigPatchIDs();

        forAll(origPatchIDs, i)
        {
            const label origPatchi = origPatchIDs[i];

            const scalarField origSumMagSf
            (
                magSfb[origPatchi] + origNccMagSfb[origPatchi]
            );

            origFieldb[origPatchi] *=
                origSumMagSf
               /max(magSfb[origPatchi], small*origSumMagSf);
        }
    }

    return torigFieldb;
}


template<class Type>
Foam::tmp<Foam::fvMeshStitcher::SurfaceFieldBoundary<Type>>
Foam::fvMeshStitcher::nonConformalBoundaryField
(
    const SurfaceFieldBoundary<Type>& nccFieldb,
    const SurfaceFieldBoundary<Type>& origFieldb
) const
{
    const bool isFluxField = isFlux(origFieldb[0].internalField());

    const fvBoundaryMesh& fvbm = origFieldb[0].patch().boundaryMesh();

    const surfaceScalarField::Boundary& magSfb =
        fvbm.mesh().magSf().boundaryField();

    tmp<SurfaceFieldBoundary<Type>> tfieldb
    (
        new SurfaceFieldBoundary<Type>
        (
            SurfaceField<Type>::Internal::null(),
            origFieldb
        )
    );

    SurfaceFieldBoundary<Type>& fieldb = tfieldb.ref();

    // Set the coupled values
    forAll(fvbm, nccPatchi)
    {
        const fvPatch& fvp = fvbm[nccPatchi];

        if (isA<nonConformalCoupledFvPatch>(fvp))
        {
            const nonConformalCoupledFvPatch& nccFvp =
                refCast<const nonConformalCoupledFvPatch>(fvp);

            const label origPatchi = nccFvp.origPatchID();
            const fvPatch& origFvp = nccFvp.origPatch();

            const labelList nccOrigPatchFace =
                nccFvp.polyFaces() - origFvp.start();

            // Set the cyclic values
            fieldb[nccPatchi] =
                Field<Type>(nccFieldb[origPatchi], nccOrigPatchFace);
        }
    }

    // If a flux then scale down to the part face area
    if (isFluxField)
    {
        const surfaceScalarField::Boundary origNccMagSfb
        (
            surfaceScalarField::Internal::null(),
            getOrigNccMagSfb()
        );

        forAll(fvbm, nccPatchi)
        {
            const fvPatch& fvp = fvbm[nccPatchi];

            if (isA<nonConformalCoupledFvPatch>(fvp))
            {
                const nonConformalCoupledFvPatch& nccFvp =
                    refCast<const nonConformalCoupledFvPatch>(fvp);

                const label origPatchi = nccFvp.origPatchID();
                const fvPatch& origFvp = nccFvp.origPatch();

                const labelList nccOrigPatchFace =
                    nccFvp.polyFaces() - origFvp.start();

                const scalarField origSumMagSf
                (
                    magSfb[origPatchi] + origNccMagSfb[origPatchi]
                );
                const scalarField nccSumMagSf(origSumMagSf, nccOrigPatchFace);

                fieldb[nccPatchi] *= magSfb[nccPatchi]/nccSumMagSf;

                if (!isA<processorFvPatch>(fvp))
                {
                    fieldb[origPatchi] *= magSfb[origPatchi]/origSumMagSf;
                }
            }
        }
    }

    // Set error values
    forAll(fvbm, patchi)
    {
        const fvPatch& fvp = fvbm[patchi];

        if (isA<nonConformalErrorFvPatch>(fvp))
        {
            const label errorPatchi = patchi;

            const nonConformalErrorFvPatch& errorFvp =
                refCast<const nonConformalErrorFvPatch>(fvp);

            const label origPatchi = errorFvp.origPatchID();
            const fvPatch& origFvp = errorFvp.origPatch();

            const labelList errorOrigPatchFace =
                errorFvp.polyFaces() - origFvp.start();

            if (isFluxField)
            {
                fieldb[errorPatchi] = Zero;
            }
            else
            {
                fieldb[errorPatchi] =
                    Field<Type>(origFieldb[origPatchi], errorOrigPatchFace);
            }
        }
    }

    return tfieldb;
}


template<class Type>
Foam::tmp<Foam::fvMeshStitcher::SurfaceFieldBoundary<Type>>
Foam::fvMeshStitcher::synchronisedBoundaryField
(
    const SurfaceFieldBoundary<Type>& fieldb,
    const bool flip,
    const scalar ownerWeight,
    const scalar neighbourWeight
) const
{
    const fvBoundaryMesh& fvbm = fieldb[0].patch().boundaryMesh();

    tmp<SurfaceFieldBoundary<Type>> tsyncFieldb
    (
        new SurfaceFieldBoundary<Type>
        (
            SurfaceField<Type>::Internal::null(),
            fieldb
        )
    );

    SurfaceFieldBoundary<Type>& syncFieldb = tsyncFieldb.ref();

    SurfaceFieldBoundary<Type> fieldbNbr
    (
        SurfaceField<Type>::Internal::null(),
        fieldb.boundaryNeighbourField()
    );

    forAll(fvbm, patchi)
    {
        const fvPatch& fvp = fvbm[patchi];

        if (fvp.coupled())
        {
            const coupledFvPatch& cfvp = refCast<const coupledFvPatch>(fvp);

            const scalar w = cfvp.owner() ? ownerWeight : neighbourWeight;
            const scalar v = cfvp.owner() ? neighbourWeight : ownerWeight;

            syncFieldb[patchi] =
                w*syncFieldb[patchi] + (flip ? -v : +v)*fieldbNbr[patchi];
        }
    }

    return tsyncFieldb;
}


template<class Type>
Foam::tmp<Foam::fvMeshStitcher::SurfaceFieldBoundary<Type>>
Foam::fvMeshStitcher::synchronisedBoundaryField
(
    const SurfaceFieldBoundary<Type>& fieldb
) const
{
    const bool isFluxField = isFlux(fieldb[0].internalField());

    return synchronisedBoundaryField<Type>
    (
        fieldb,
        isFluxField,
        0.5,
        0.5
    );
}


// ************************************************************************* //
