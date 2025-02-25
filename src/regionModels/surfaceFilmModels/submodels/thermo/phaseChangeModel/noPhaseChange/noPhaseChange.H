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
    Foam::regionModels::surfaceFilmSubModels::noPhaseChange

Description
    Dummy phase change model for 'none'

SourceFiles
    noPhaseChange.C

\*---------------------------------------------------------------------------*/

#ifndef noPhaseChange_H
#define noPhaseChange_H

#include "phaseChangeModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace surfaceFilmSubModels
{

/*---------------------------------------------------------------------------*\
                        Class noPhaseChange Declaration
\*---------------------------------------------------------------------------*/

class noPhaseChange
:
    public phaseChangeModel
{
public:

    //- Runtime type information
    TypeName("none");


    // Constructors

        //- Construct from surface film model
        noPhaseChange(surfaceFilm& film, const dictionary& dict);

        //- Disallow default bitwise copy construction
        noPhaseChange(const noPhaseChange&) = delete;


    //- Destructor
    virtual ~noPhaseChange();


    // Member Functions

        // Evolution

            //- Correct
            virtual void correct
            (
                const scalar dt,
                scalarField& availableMass,
                volScalarField& dMass,
                volScalarField& dEnergy
            );

            //- Correct
            virtual void correctModel
            (
                const scalar dt,
                scalarField& availableMass,
                scalarField& dMass,
                scalarField& dEnergy
            );


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const noPhaseChange&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmSubModels
} // End namespace regionModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
