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

Class
    Foam::regionModels::momentumSurfaceFilm

Description
    Kinematic form of single-cell layer surface film model

SourceFiles
    momentumSurfaceFilm.C

\*---------------------------------------------------------------------------*/

#ifndef momentumSurfaceFilm_H
#define momentumSurfaceFilm_H

#include "surfaceFilm.H"
#include "fvMesh.H"
#include "volFields.H"
#include "surfaceFields.H"
#include "fvMatrices.H"
#include "pimpleControl.H"
#include "rhoThermo.H"
#include "Function1.H"

#include "ejectionModelList.H"
#include "transferModelList.H"
#include "forceList.H"
#include "filmMomentumTransportModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{

// Forward class declarations
namespace surfaceFilmSubModels
{
    class viscosityModel;
}

/*---------------------------------------------------------------------------*\
                    Class momentumSurfaceFilm Declaration
\*---------------------------------------------------------------------------*/

class momentumSurfaceFilm
:
    public surfaceFilm
{

protected:

    // Protected data

        const word phaseName_;

        // Solution parameters

            pimpleControl pimple_;

            //- Cumulative continuity error
            scalar cumulativeContErr_;

            //- Small delta
            const dimensionedScalar deltaSmall_;

            //- Optional maximum Courant number for stable film solution
            scalar maxCo_;


        // Thermo properties

            //- Pressure [Pa]
            volScalarField p_;

            //- Thermo package
            autoPtr<rhoThermo> thermo_;


        // Fields

            //- Dynamic viscosity [Pa.s]
            volScalarField mu_;

            //- Film thickness [m]
            volScalarField delta_;

            //- Film volume fraction in the cell layer []
            volScalarField alpha_;

            //- Velocity - mean [m/s]
            volVectorField U_;

            //- Velocity - wall [m/s]
            volVectorField::Internal Uw_;

            //- Mass flux [kg m/s]
            surfaceScalarField phi_;

            //- Film velocity flux [m^3/s]
            surfaceScalarField phiU_;

            //- Current continuity error caused by delta_ bounding
            volScalarField::Internal continuityErr_;

            //- Film coverage indicator, 1 = covered, 0 = uncovered []
            volScalarField coverage_;


            // Transfer fields

                //- Film mass available for transfer to the primary region
                volScalarField primaryMassTrans_;

                //- Film mass available for transfer to cloud
                volScalarField cloudMassTrans_;

                //- Parcel diameters originating from film to cloud
                volScalarField cloudDiameterTrans_;

                //- Film momentum transfer
                volVectorField primaryMomentumTrans_;


        // Source term fields

            // Film region - registered to the film region mesh
            // Note: need boundary value mapped from primary region, and then
            // pushed into the patch internal field

                //- Mass [kg/m^2/s]
                volScalarField::Internal rhoSp_;

                //- Momentum [kg/m/s^2]
                volVectorField::Internal USp_;

                //- Pressure [Pa]
                volScalarField::Internal pSp_;


            // Primary region - registered to the primary region mesh
            // Internal use only - not read-in

                //- Primary region mass source [kg]
                volScalarField rhoSpPrimary_;

                //- Primary region tangential momentum source [kg m/s]
                volVectorField USpPrimary_;

                //- Primary region normal momentum source (pressure) [kg m/s]
                volScalarField pSpPrimary_;


        // Fields mapped from primary region - registered to the film region
        // Note: need both boundary AND patch internal fields to be mapped

            //- Velocity [m/s]
            volVectorField UPrimary_;

            //- Density [kg/m^3]
            volScalarField rhoPrimary_;

            //- Viscosity [Pa.s]
            volScalarField muPrimary_;


        // Sub-models

            //- Viscosity model
            autoPtr<surfaceFilmSubModels::viscosityModel> viscosity_;

            //- Surface tension function
            autoPtr<Function1<scalar>> sigma_;

            //- Available mass for transfer via sub-models
            scalarField availableMass_;

            //- Cloud ejection
            surfaceFilmSubModels::ejectionModelList ejection_;

            //- Transfer with the continuous phase
            surfaceFilmSubModels::transferModelList transfer_;

            //- Momentum transport model
            autoPtr<surfaceFilmSubModels::momentumTransportModel>
                momentumTransport_;

            //- List of film forces
            surfaceFilmSubModels::forceList forces_;


       // Checks

           //- Cumulative mass added via sources [kg]
           scalar addedMassTotal_;


    // Protected member functions

        //- Read control parameters from dictionary
        virtual bool read();

        //- Reset source term fields
        virtual void resetPrimaryRegionSourceTerms();

        //- Transfer thermo fields from the primary region to the film region
        virtual void transferPrimaryRegionThermoFields();

        //- Transfer source fields from the primary region to the film region
        virtual void transferPrimaryRegionSourceFields();

        //- Hydrostatic pressure coefficient
        tmp<surfaceScalarField> rhog() const;

        //- Hydrostatic pressure coefficient gradient
        tmp<surfaceScalarField> gGradRho() const;

        //- Capillary pressure
        tmp<volScalarField> pc();

        //- Explicit pressure
        tmp<volScalarField> pe();

        //- Correct film coverage field
        virtual void correctCoverage();

        //- Update the film sub-models
        virtual void updateSubmodels();

        // Update continuity error
        virtual void updateContinuityErr();

        //- Continuity check
        virtual void continuityCheck();

        //- Constrain a film region master/slave boundaries of a field to a
        //  given value
        template<class Type>
        tmp<Type> constrainFilmField
        (
            const tmp<Type>& field,
            const typename Type::cmptType& value
        );


        // Equations

            //- Predict delta_ from the continuity equation
            virtual void predictDelta();

            //- Solve for film velocity
            virtual tmp<fvVectorMatrix> solveMomentum
            (
                const volScalarField& pc,
                const volScalarField& pe
            );

            //- Solve for film volume fraction and thickness
            virtual void solveAlpha
            (
                const fvVectorMatrix& UEqn,
                const volScalarField& pc,
                const volScalarField& pe
            );


public:

    //- Runtime type information
    TypeName("momentumSurfaceFilm");


    // Constructors

        //- Construct from components
        momentumSurfaceFilm
        (
            const word& modelType,
            const fvMesh& mesh,
            const dimensionedVector& g,
            const word& regionType,
            const bool readFields = true
        );

        //- Disallow default bitwise copy construction
        momentumSurfaceFilm(const momentumSurfaceFilm&) = delete;


    //- Destructor
    virtual ~momentumSurfaceFilm();


    // Member Functions

        // Solution parameters

            //- Courant number
            virtual scalar CourantNumber() const;

            //- Return the maximum time-step for stable operation
            //  based on the Courant number
            virtual scalar maxDeltaT() const;

            //- Return small delta
            inline const dimensionedScalar& deltaSmall() const;


        // Thermo properties

            //- Return the name of the phase deposited to form the film
            inline const word& phaseName() const;

            //- Return the film density [kg/m^3]
            inline const volScalarField& rho() const;

            //- Return const access to the dynamic viscosity [Pa.s]
            inline const volScalarField& mu() const;

            //- Return the surface tension coefficient [kg/s^2]
            tmp<volScalarField> sigma() const;


        // Fields

            //- Return const access to the film thickness [m]
            inline const volScalarField& delta() const;

            //- Return const access to the film volume fraction []
            inline const volScalarField& alpha() const;

            //- Return the film velocity [m/s]
            inline const volVectorField& U() const;

            //- Return the film flux [kg m/s]
            inline const surfaceScalarField& phi() const;

            //- Return the film velocity flux [m^3/s]
            inline const surfaceScalarField& phiU() const;

            //- Return the current continuity error
            inline const volScalarField::Internal& continuityErr() const;

            //- Return the film coverage, 1 = covered, 0 = uncovered []
            inline const volScalarField& coverage() const;


        // Derived Fields

            //- Return the film surface velocity [m/s]
            inline tmp<volVectorField::Internal> Us() const;

            //- Return the film wall velocity [m/s]
            tmp<volVectorField::Internal> Uw() const;


        // Transfer fields - to the primary region

            //- Return mass transfer source - Eulerian phase only
            virtual tmp<volScalarField> primaryMassTrans() const;

            //- Return the film mass available for transfer to cloud
            virtual const volScalarField& cloudMassTrans() const;

            //- Return the parcel diameters originating from film to cloud
            virtual const volScalarField& cloudDiameterTrans() const;

            //- Return momentum transfer source - Eulerian phase only
            virtual tmp<volVectorField> primaryMomentumTrans() const;


        // External helper functions

            //- External hook to add sources to the film
            virtual void addSources
            (
                const label patchi,            // patchi on primary region
                const label facei,             // facei of patchi
                const scalar massSource,       // [kg]
                const vector& momentumSource,  // [kg m/s] (tang'l momentum)
                const scalar pressureSource,   // [kg m/s] (normal momentum)
                const scalar energySource = 0  // [J]
            );


         // Source fields (read/write access)

            // Primary region

                //- Momentum [kg/m/s^2]
                inline volVectorField& USpPrimary();

                //- Pressure [Pa]
                inline volScalarField& pSpPrimary();

                //- Mass [kg/m^2/s]
                inline volScalarField& rhoSpPrimary();


            // Film region

                //- Momentum [kg/m/s^2]
                inline volVectorField::Internal& USp();

                //- Pressure [Pa]
                inline volScalarField::Internal& pSp();

                //- Mass [kg/m^2/s]
                inline volScalarField::Internal& rhoSp();

                //- Momentum [kg/m/s^2]
                inline const volVectorField::Internal& USp() const;

                //- Pressure [Pa]
                inline const volScalarField::Internal& pSp() const;

                //- Mass [kg/m^2/s]
                inline const volScalarField::Internal& rhoSp() const;


        // Fields mapped from primary region

            //- Velocity [m/s]
            inline const volVectorField& UPrimary() const;

            //- Pressure [Pa]
            inline const volScalarField& pPrimary() const;

            //- Density [kg/m^3]
            inline const volScalarField& rhoPrimary() const;

            //- Viscosity [Pa.s]
            inline const volScalarField& muPrimary() const;


        // Sub-models

            //- Film thermo
            inline const rhoThermo& thermo() const;

            //- Ejection
            inline surfaceFilmSubModels::ejectionModelList& ejection();

            //- Transfer
            inline surfaceFilmSubModels::transferModelList& transfer();

            //- Momentum transport
            inline const surfaceFilmSubModels::momentumTransportModel&
                momentumTransport() const;


        // Helper functions

            //- Return the current film mass
            inline tmp<volScalarField::Internal> mass() const;

            //- Return the change in film mass due to sources/sinks
            inline tmp<volScalarField::Internal> deltaMass() const;


        // Evolution

            //- Pre-evolve film hook
            virtual void preEvolveRegion();

            //- Evolve the film equations
            virtual void evolveRegion();


        // Primary region source fields

            //- Return total mass source - Eulerian phase only
            virtual tmp<volScalarField::Internal> Srho() const;

            //- Return mass source for specie i - Eulerian phase only
            virtual tmp<volScalarField::Internal> SYi
            (
                const label i
            ) const;

            //- Return momentum source - Eulerian phase only
            virtual tmp<volVectorField::Internal> SU() const;

            //- Return enthalpy source - Eulerian phase only
            virtual tmp<volScalarField::Internal> Sh() const;


        // I-O

            //- Provide some feedback
            virtual void info();


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const momentumSurfaceFilm&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace regionModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "momentumSurfaceFilmTemplates.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "momentumSurfaceFilmI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
