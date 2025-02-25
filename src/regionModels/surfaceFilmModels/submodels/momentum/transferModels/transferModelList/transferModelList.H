/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2017-2022 OpenFOAM Foundation
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
    Foam::regionModels::surfaceFilmSubModels::transferModelList

Description
    List container for film transfer models

SourceFiles
    transferModelList.C

\*---------------------------------------------------------------------------*/

#ifndef transferModelList_H
#define transferModelList_H

#include "PtrList.H"
#include "transferModel.H"
#include "filmSubModelBase.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace surfaceFilmSubModels
{

/*---------------------------------------------------------------------------*\
                      Class transferModelList Declaration
\*---------------------------------------------------------------------------*/

class transferModelList
:
    public PtrList<transferModel>,
    public filmSubModelBase
{
    // Private Data

        //- List of mass transferred per patch
        scalarField massTransferred_;


public:

    // Constructors

        //- Construct null
        transferModelList(surfaceFilm& film);

        //- Construct from type name, dictionary and surface film model
        transferModelList
        (
            surfaceFilm& film,
            const dictionary& dict
        );

        //- Disallow default bitwise copy construction
        transferModelList(const transferModelList&) = delete;


    //- Destructor
    virtual ~transferModelList();


    // Member Functions

        //- Correct kinematic transfers
        virtual void correct
        (
            scalarField& availableMass,
            volScalarField& massToTransfer,
            volVectorField& momentumToTransfer
        );

        //- Correct kinematic and thermodynamic transfers
        virtual void correct
        (
            scalarField& availableMass,
            volScalarField& massToTransfer,
            volVectorField& momentumToTransfer,
            volScalarField& energyToTransfer
        );

        //- Provide some info
        virtual void info(Ostream& os);


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const transferModelList&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmSubModels
} // End namespace regionModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
