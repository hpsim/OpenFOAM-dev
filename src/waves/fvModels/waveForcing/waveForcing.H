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

Class
    Foam::fv::waveForcing

Description
    This fvModel applies forcing to the liquid phase-fraction field and all
    components of the vector field to relax the fields towards those
    calculated from the current wave distribution.

    The forcing force coefficient \f$\lambda\f$ should be set based on the
    desired level of forcing and the residence time the waves through the
    forcing zone.  For example, if waves moving at 2 [m/s] are travelling
    through a forcing zone 8 [m] in length, then the residence time is 4 [s]. If
    it is deemed necessary to force for 5 time-scales, then \f$\lambda\f$ should
    be set to equal 5/(4 [s]) = 1.2 [1/s].

Usage
    Example usage:
    \verbatim
    waveForcing1
    {
        type            waveForcing;

        libs            ("libwaves.so");

        liquidPhase     water;

        // Define the line along which to apply the graduation
        origin          (600 0 0);
        direction       (-1 0 0);

        // // Or, define multiple lines
        // origins         ((600 0 0) (600 -300 0) (600 300 0));
        // directions      ((-1 0 0) (0 1 0) (0 -1 0));

        scale
        {
            type        halfCosineRamp;
            start       0;
            duration    300;
        }

        lambda          0.5; // Forcing coefficient
    }
    \endverbatim

See also
    Foam::fv::forcing

SourceFiles
    waveForcing.C

\*---------------------------------------------------------------------------*/

#ifndef waveForcing_H
#define waveForcing_H

#include "forcing.H"
#include "waveSuperposition.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace fv
{

/*---------------------------------------------------------------------------*\
                      Class waveForcing Declaration
\*---------------------------------------------------------------------------*/

class waveForcing
:
    public forcing
{
    // Private Data

        //- Reference to the waves
        const waveSuperposition& waves_;

        //- Name of the liquid phase
        const word liquidPhaseName_;

        //- Name of the liquid phase-fraction field
        const word alphaName_;

        //- Name of the velocity field
        const word UName_;

        //- Phase-fraction field calculated from the current wave form
        tmp<volScalarField::Internal> alphaWaves_;

        //- Velocity field calculated from the current wave form
        tmp<volVectorField::Internal> Uwaves_;

        //- Forcing coefficient scale field
        tmp<volScalarField::Internal> scale_;


public:

    //- Runtime type information
    TypeName("waveForcing");


    // Constructors

        //- Construct from components
        waveForcing
        (
            const word& name,
            const word& modelType,
            const dictionary& dict,
            const fvMesh& mesh
        );


    //- Destructor
    virtual ~waveForcing()
    {}


    // Member Functions

        // Checks

            //- Return the list of fields for which the fvModel adds source term
            //  to the transport equation
            virtual wordList addSupFields() const;


        // Add explicit and implicit contributions

            //- Source term to VoF phase-fraction equation
            virtual void addSup
            (
                fvMatrix<scalar>& eqn,
                const word& fieldName
            ) const;

            //- Source term to momentum equation
            virtual void addSup
            (
                const volScalarField& rho,
                fvMatrix<vector>& eqn,
                const word& fieldName
            ) const;


        // Mesh changes

            //- Update for mesh motion
            virtual bool movePoints();

            //- Update topology using the given map
            virtual void topoChange(const polyTopoChangeMap&);

            //- Update from another mesh using the given map
            virtual void mapMesh(const polyMeshMap&);

            //- Redistribute or update using the given distribution map
            virtual void distribute(const polyDistributionMap&);


        //- Correct the wave forcing coefficients
        virtual void correct();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fv
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
