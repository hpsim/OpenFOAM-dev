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

#include "ThermoSurfaceFilm.H"
#include "thermoSurfaceFilm.H"
#include "addToRunTimeSelectionTable.H"
#include "mathematicalConstants.H"
#include "Pstream.H"
#include "ThermoCloud.H"
#include "meshTools.H"

using namespace Foam::constant::mathematical;

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

template<class CloudType>
Foam::vector Foam::ThermoSurfaceFilm<CloudType>::splashDirection
(
    const vector& tanVec1,
    const vector& tanVec2,
    const vector& nf
) const
{
    // Azimuthal angle [rad]
    const scalar phiSi = twoPi*rndGen_.sample01<scalar>();

    // Ejection angle [rad]
    const scalar thetaSi = pi/180.0*(rndGen_.sample01<scalar>()*(50 - 5) + 5);

    // Direction vector of new parcel
    const scalar alpha = sin(thetaSi);
    const scalar dcorr = cos(thetaSi);
    const vector normal = alpha*(tanVec1*cos(phiSi) + tanVec2*sin(phiSi));
    vector dirVec = dcorr*nf;
    dirVec += normal;

    return dirVec/mag(dirVec);
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::absorbInteraction
(
    regionModels::surfaceFilm& filmModel,
    const parcelType& p,
    const polyPatch& pp,
    const label facei,
    const scalar mass,
    bool& keepParticle
)
{
    if (debug)
    {
        Info<< "Parcel " << p.origId() << " absorbInteraction" << endl;
    }

    const fluidThermo& carrierThermo =
        static_cast<const ThermoCloud<CloudType>&>(this->owner())
       .carrierThermo();

    const parcelThermo& thermo =
        static_cast<const ThermoCloud<CloudType>&>(this->owner()).thermo();

    // Patch face normal
    const vector& nf = pp.faceNormals()[facei];

    // Patch velocity
    const vector& Up = this->owner().U().boundaryField()[pp.index()][facei];

    // Relative parcel velocity
    const vector Urel = p.U() - Up;

    // Parcel normal velocity
    const vector Un = nf*(Urel & nf);

    // Parcel tangential velocity
    const vector Ut = Urel - Un;

    const liquidProperties& liq = thermo.liquids().properties()[0];

    // Local pressure
    const scalar pc = carrierThermo.p()[p.cell()];

    filmModel.addSources
    (
        pp.index(),
        facei,
        mass,                           // mass
        mass*Ut,                        // tangential momentum
        mass*mag(Un),                   // impingement pressure
        mass*liq.Hs(pc, p.T())          // energy
    );

    this->nParcelsTransferred()++;

    keepParticle = false;
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::bounceInteraction
(
    parcelType& p,
    const polyPatch& pp,
    const label facei,
    bool& keepParticle
) const
{
    if (debug)
    {
        Info<< "Parcel " << p.origId() << " bounceInteraction" << endl;
    }

    // Patch face normal
    const vector& nf = pp.faceNormals()[facei];

    // Patch velocity
    const vector& Up = this->owner().U().boundaryField()[pp.index()][facei];

    // Relative parcel velocity
    const vector Urel = p.U() - Up;

    // Flip parcel normal velocity component
    p.U() -= 2.0*nf*(Urel & nf);

    keepParticle = true;
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::drySplashInteraction
(
    regionModels::surfaceFilm& filmModel,
    const parcelType& p,
    const polyPatch& pp,
    const label facei,
    bool& keepParticle
)
{
    if (debug)
    {
        Info<< "Parcel " << p.origId() << " drySplashInteraction" << endl;
    }

    const fluidThermo& carrierThermo =
        static_cast<const ThermoCloud<CloudType>&>(this->owner())
       .carrierThermo();

    const parcelThermo& thermo =
        static_cast<const ThermoCloud<CloudType>&>(this->owner()).thermo();

    const liquidProperties& liq = thermo.liquids().properties()[0];

    // Patch face velocity and normal
    const vector& Up = this->owner().U().boundaryField()[pp.index()][facei];
    const vector& nf = pp.faceNormals()[facei];

    // Local pressure
    const scalar pc = carrierThermo.p()[p.cell()];

    // Retrieve parcel properties
    const scalar m = p.mass()*p.nParticle();
    const scalar rho = p.rho();
    const scalar d = p.d();
    const scalar sigma = liq.sigma(pc, p.T());
    const scalar mu = liq.mu(pc, p.T());
    const vector Urel = p.U() - Up;
    const vector Un = nf*(Urel & nf);

    // Laplace number
    const scalar La = rho*sigma*d/sqr(mu);

    // Weber number
    const scalar We = rho*magSqr(Un)*d/sigma;

    // Critical Weber number
    const scalar Wec = Adry_*pow(La, -0.183);

    if (We < Wec) // Adhesion - assume absorb
    {
        absorbInteraction(filmModel, p, pp, facei, m, keepParticle);
    }
    else // Splash
    {
        // Ratio of incident mass to splashing mass
        const scalar mRatio = 0.2 + 0.6*rndGen_.sample01<scalar>();
        splashInteraction
            (filmModel, p, pp, facei, mRatio, We, Wec, sigma, keepParticle);
    }
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::wetSplashInteraction
(
    regionModels::surfaceFilm& filmModel,
    parcelType& p,
    const polyPatch& pp,
    const label facei,
    bool& keepParticle
)
{
    if (debug)
    {
        Info<< "Parcel " << p.origId() << " wetSplashInteraction" << endl;
    }

    const fluidThermo& carrierThermo =
        static_cast<const ThermoCloud<CloudType>&>(this->owner())
       .carrierThermo();

    const parcelThermo& thermo =
        static_cast<const ThermoCloud<CloudType>&>(this->owner()).thermo();

    const liquidProperties& liq = thermo.liquids().properties()[0];

    // Patch face velocity and normal
    const vector& Up = this->owner().U().boundaryField()[pp.index()][facei];
    const vector& nf = pp.faceNormals()[facei];

    // Local pressure
    const scalar pc = carrierThermo.p()[p.cell()];

    // Retrieve parcel properties
    const scalar m = p.mass()*p.nParticle();
    const scalar rho = p.rho();
    const scalar d = p.d();
    vector& U = p.U();
    const scalar sigma = liq.sigma(pc, p.T());
    const scalar mu = liq.mu(pc, p.T());
    const vector Urel = p.U() - Up;
    const vector Un = nf*(Urel & nf);
    const vector Ut = Urel - Un;

    // Laplace number
    const scalar La = rho*sigma*d/sqr(mu);

    // Weber number
    const scalar We = rho*magSqr(Un)*d/sigma;

    // Critical Weber number
    const scalar Wec = Awet_*pow(La, -0.183);

    if (We < 2) // Adhesion - assume absorb
    {
        absorbInteraction(filmModel, p, pp, facei, m, keepParticle);
    }
    else if ((We >= 2) && (We < 20)) // Bounce
    {
        // Incident angle of impingement
        const scalar theta = pi/2 - acos(U/mag(U) & nf);

        // Restitution coefficient
        const scalar epsilon = 0.993 - theta*(1.76 - theta*(1.56 - theta*0.49));

        // Update parcel velocity
        U = -epsilon*(Un) + 5.0/7.0*(Ut);

        keepParticle = true;
        return;
    }
    else if ((We >= 20) && (We < Wec)) // Spread - assume absorb
    {
        absorbInteraction(filmModel, p, pp, facei, m, keepParticle);
    }
    else    // Splash
    {
        // Ratio of incident mass to splashing mass
        // splash mass can be > incident mass due to film entrainment
        const scalar mRatio = 0.2 + 0.9*rndGen_.sample01<scalar>();
        splashInteraction
            (filmModel, p, pp, facei, mRatio, We, Wec, sigma, keepParticle);
    }
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::splashInteraction
(
    regionModels::surfaceFilm& filmModel,
    const parcelType& p,
    const polyPatch& pp,
    const label facei,
    const scalar mRatio,
    const scalar We,
    const scalar Wec,
    const scalar sigma,
    bool& keepParticle
)
{
    // Patch face velocity and normal
    const fvMesh& mesh = this->owner().mesh();
    const vector& Up = this->owner().U().boundaryField()[pp.index()][facei];
    const vector& nf = pp.faceNormals()[facei];

    // Determine direction vectors tangential to patch normal
    const vector tanVec1 = normalised(perpendicular(nf));
    const vector tanVec2 = nf^tanVec1;

    // Retrieve parcel properties
    const scalar np = p.nParticle();
    const scalar m = p.mass()*np;
    const scalar d = p.d();
    const vector Urel = p.U() - Up;
    const vector Un = nf*(Urel & nf);
    const vector Ut = Urel - Un;
    const vector& posC = mesh.C()[p.cell()];
    const vector& posCf = mesh.Cf().boundaryField()[pp.index()][facei];

    // Total mass of (all) splashed parcels
    const scalar mSplash = m*mRatio;

    // Number of splashed particles per incoming particle
    const scalar Ns = 5.0*(We/Wec - 1.0);

    // Average diameter of splashed particles
    const scalar dBarSplash = 1/cbrt(6.0)*cbrt(mRatio/Ns)*d + rootVSmall;

    // Cumulative diameter splash distribution
    const scalar dMax = 0.9*cbrt(mRatio)*d;
    const scalar dMin = 0.1*dMax;
    const scalar K = exp(-dMin/dBarSplash) - exp(-dMax/dBarSplash);

    // Surface energy of secondary parcels [J]
    scalar ESigmaSec = 0;

    // Sample splash distribution to determine secondary parcel diameters
    scalarList dNew(parcelsPerSplash_);
    scalarList npNew(parcelsPerSplash_);
    forAll(dNew, i)
    {
        const scalar y = rndGen_.sample01<scalar>();
        dNew[i] = -dBarSplash*log(exp(-dMin/dBarSplash) - y*K);
        npNew[i] = mRatio*np*pow3(d)/pow3(dNew[i])/parcelsPerSplash_;
        ESigmaSec += npNew[i]*sigma*p.areaS(dNew[i]);
    }

    // Incident kinetic energy [J]
    const scalar EKIn = 0.5*m*magSqr(Un);

    // Incident surface energy [J]
    const scalar ESigmaIn = np*sigma*p.areaS(d);

    // Dissipative energy
    const scalar Ed = max(0.8*EKIn, np*Wec/12*pi*sigma*sqr(d));

    // Total energy [J]
    const scalar EKs = EKIn + ESigmaIn - ESigmaSec - Ed;

    // Switch to absorb if insufficient energy for splash
    if (EKs <= 0)
    {
        absorbInteraction(filmModel, p, pp, facei, m, keepParticle);
        return;
    }

    // Helper variables to calculate magUns0
    const scalar logD = log(d);
    const scalar coeff2 = log(dNew[0]) - logD + rootVSmall;
    scalar coeff1 = 0.0;
    forAll(dNew, i)
    {
        coeff1 += sqr(log(dNew[i]) - logD);
    }

    // Magnitude of the normal velocity of the first splashed parcel
    const scalar magUns0 =
        sqrt(2.0*parcelsPerSplash_*EKs/mSplash/(1.0 + coeff1/sqr(coeff2)));

    // Set splashed parcel properties
    forAll(dNew, i)
    {
        const vector dirVec = splashDirection(tanVec1, tanVec2, -nf);

        // Create a new parcel by copying source parcel
        parcelType* pPtr = new parcelType(p);

        pPtr->origId() = pPtr->getNewParticleID();

        pPtr->origProc() = Pstream::myProcNo();

        if (splashParcelType_ >= 0)
        {
            pPtr->typeId() = splashParcelType_;
        }

        // Perturb new parcels towards the owner cell centre
        pPtr->track(mesh, 0.5*rndGen_.sample01<scalar>()*(posC - posCf), 0);

        pPtr->nParticle() = npNew[i];

        pPtr->d() = dNew[i];

        pPtr->U() = dirVec*(mag(Cf_*Ut) + magUns0*(log(dNew[i]) - logD)/coeff2);

        // Apply correction to velocity for 2-D cases
        meshTools::constrainDirection(mesh, mesh.solutionD(), pPtr->U());

        // Add the new parcel
        this->owner().addParticle(pPtr);

        nParcelsSplashed_++;
    }

    // Transfer remaining part of parcel to film 0 - splashMass can be -ve
    // if entraining from the film
    const scalar mDash = m - mSplash;
    absorbInteraction(filmModel, p, pp, facei, mDash, keepParticle);
}


template<class CloudType>
Foam::UPtrList<Foam::regionModels::surfaceFilm>&
Foam::ThermoSurfaceFilm<CloudType>::surfaceFilmPtrs() const
{
    const objectRegistry& db = this->owner().mesh().time();

    if (!surfaceFilmNames_.empty() && surfaceFilms_.empty())
    {
        // If there are multiple surface films and/or surface films with
        // non-default names, then require the surfaceFilms entry to have been
        // specified
        {
            HashTable<const regionModels::surfaceFilm*> surfaceFilmPtrs =
                db.lookupClass<regionModels::surfaceFilm>();

            const word defaultName =
                regionModels::surfaceFilm::typeName + "Properties";

            if
            (
                (
                    surfaceFilmPtrs.size() == 1
                 && surfaceFilmPtrs.begin().key() != defaultName
                )
             || (surfaceFilmPtrs.size() > 1)
            )
            {
                this->coeffDict().lookup("surfaceFilms");
            }
        }

        // Cache pointers to the surface film models
        surfaceFilms_.resize(surfaceFilmNames_.size());
        forAll(surfaceFilms_, filmi)
        {
            surfaceFilms_.set
            (
                filmi,
               &db.template lookupObjectRef<regionModels::surfaceFilm>
                (
                    surfaceFilmNames_[filmi] + "Properties"
                )
            );
        }
    }

    return surfaceFilms_;
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::cacheFilmFields
(
    const label filmPatchi,
    const label primaryPatchi,
    const regionModels::surfaceFilm& filmModel
)
{
    SurfaceFilmModel<CloudType>::cacheFilmFields
    (
        filmPatchi,
        primaryPatchi,
        filmModel
    );

    const regionModels::thermoSurfaceFilm& thermalFilmModel =
        refCast<const regionModels::thermoSurfaceFilm>(filmModel);

    TFilmPatch_ = thermalFilmModel.thermo().T().boundaryField()[filmPatchi];
    filmModel.toPrimary(filmPatchi, TFilmPatch_);

    CpFilmPatch_ =
        thermalFilmModel.thermo().Cpv().boundaryField()[filmPatchi];
    filmModel.toPrimary(filmPatchi, CpFilmPatch_);
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::setParcelProperties
(
    parcelType& p,
    const label filmFacei
) const
{
    SurfaceFilmModel<CloudType>::setParcelProperties(p, filmFacei);

    // Set parcel properties
    p.T() = TFilmPatch_[filmFacei];
    p.Cp() = CpFilmPatch_[filmFacei];
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
Foam::ThermoSurfaceFilm<CloudType>::ThermoSurfaceFilm
(
    const dictionary& dict,
    CloudType& owner
)
:
    SurfaceFilmModel<CloudType>(dict, owner, typeName),
    rndGen_(owner.rndGen()),
    surfaceFilmNames_
    (
        this->coeffDict().lookupOrDefault
        (
            "surfaceFilms",
            wordList(1, regionModels::surfaceFilm::typeName)
        )
    ),
    surfaceFilms_(),
    TFilmPatch_(0),
    CpFilmPatch_(0),
    interactionType_
    (
        interactionTypeNames_.read(this->coeffDict().lookup("interactionType"))
    ),
    deltaWet_(0.0),
    splashParcelType_(0),
    parcelsPerSplash_(0),
    Adry_(0.0),
    Awet_(0.0),
    Cf_(0.0),
    nParcelsSplashed_(0)
{
    Info<< "    Applying " << interactionTypeNames_[interactionType_]
        << " interaction model" << endl;

    if (interactionType_ == interactionType::splashBai)
    {
        this->coeffDict().lookup("deltaWet") >> deltaWet_;
        splashParcelType_ =
            this->coeffDict().lookupOrDefault("splashParcelType", -1);
        parcelsPerSplash_ =
            this->coeffDict().lookupOrDefault("parcelsPerSplash", 2);
        this->coeffDict().lookup("Adry") >> Adry_;
        this->coeffDict().lookup("Awet") >> Awet_;
        this->coeffDict().lookup("Cf") >> Cf_;
    }
}


template<class CloudType>
Foam::ThermoSurfaceFilm<CloudType>::ThermoSurfaceFilm
(
    const ThermoSurfaceFilm<CloudType>& sfm
)
:
    SurfaceFilmModel<CloudType>(sfm),
    rndGen_(sfm.rndGen_),
    surfaceFilmNames_(sfm.surfaceFilmNames_),
    surfaceFilms_(),
    TFilmPatch_(sfm.TFilmPatch_),
    CpFilmPatch_(sfm.CpFilmPatch_),
    interactionType_(sfm.interactionType_),
    deltaWet_(sfm.deltaWet_),
    splashParcelType_(sfm.splashParcelType_),
    parcelsPerSplash_(sfm.parcelsPerSplash_),
    Adry_(sfm.Adry_),
    Awet_(sfm.Awet_),
    Cf_(sfm.Cf_),
    nParcelsSplashed_(sfm.nParcelsSplashed_)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class CloudType>
Foam::ThermoSurfaceFilm<CloudType>::~ThermoSurfaceFilm()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
bool Foam::ThermoSurfaceFilm<CloudType>::transferParcel
(
    parcelType& p,
    const polyPatch& pp,
    bool& keepParticle
)
{
    const label patchi = pp.index();

    forAll(this->surfaceFilmPtrs(), filmi)
    {
        regionModels::surfaceFilm& filmModel = this->surfaceFilmPtrs()[filmi];

        if (filmModel.isRegionPatch(patchi))
        {
            const label facei = pp.whichFace(p.face());

            switch (interactionType_)
            {
                case interactionType::bounce:
                {
                    bounceInteraction(p, pp, facei, keepParticle);
                    break;
                }
                case interactionType::absorb:
                {
                    absorbInteraction
                    (
                        filmModel,
                        p,
                        pp,
                        facei,
                        p.nParticle()*p.mass(),
                        keepParticle
                    );
                    break;
                }
                case interactionType::splashBai:
                {
                    if (this->deltaFilmPatch_[patchi][facei] < deltaWet_)
                    {
                        drySplashInteraction
                        (
                            filmModel,
                            p,
                            pp,
                            facei,
                            keepParticle
                        );
                    }
                    else
                    {
                        wetSplashInteraction
                        (
                            filmModel,
                            p,
                            pp,
                            facei,
                            keepParticle
                        );
                    }
                    break;
                }
                default:
                {
                    FatalErrorInFunction
                        << "Unknown interaction type enumeration"
                        << abort(FatalError);
                }
            }

            // Transfer parcel/parcel interactions complete
            return true;
        }
    }

    // Parcel not interacting with film
    return false;
}


template<class CloudType>
void Foam::ThermoSurfaceFilm<CloudType>::info(Ostream& os)
{
    SurfaceFilmModel<CloudType>::info(os);

    label nSplash0 = this->template getModelProperty<label>("nParcelsSplashed");
    label nSplashTotal =
        nSplash0 + returnReduce(nParcelsSplashed_, sumOp<label>());

    os  << "    New film splash parcels         = " << nSplashTotal << endl;

    if (this->writeTime())
    {
        this->setModelProperty("nParcelsSplashed", nSplashTotal);
        nParcelsSplashed_ = 0;
    }
}


// ************************************************************************* //
