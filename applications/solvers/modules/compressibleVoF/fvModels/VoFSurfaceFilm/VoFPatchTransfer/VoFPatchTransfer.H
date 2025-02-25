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
    Foam::regionModels::surfaceFilmSubModels::VoFPatchTransfer

Description
    Transfer mass between the film and the VoF in the continuous phase.

SourceFiles
    VoFPatchTransfer.C

\*---------------------------------------------------------------------------*/

#ifndef VoFPatchTransfer_H
#define VoFPatchTransfer_H

#include "transferModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace surfaceFilmSubModels
{

/*---------------------------------------------------------------------------*\
                      Class VoFPatchTransfer Declaration
\*---------------------------------------------------------------------------*/

class VoFPatchTransfer
:
    public transferModel
{
protected:

        //- Factor of the cell height above which the film is transferred
        //  to the VoF
        scalar deltaFactorToVoF_;

        //- Factor of the cell height below which the VoF may be transferred
        //  to the film
        scalar deltaFactorToFilm_;

        //- VoF limit above which all of the film is transferred to the VoF
        scalar alphaToVoF_;

        //- VoF limit below which the VoF may be transferred to the film
        scalar alphaToFilm_;

        //- Transfer rate coefficient
        scalar transferRateCoeff_;

        //- List of patch IDs at which the film is removed
        labelList patchIDs_;

        //- Transferred mass for each patch at which the film is removed
        scalarField patchTransferredMasses_;


public:

    //- Runtime type information
    TypeName("VoFPatchTransfer");


    // Constructors

        //- Construct from surface film model
        VoFPatchTransfer(surfaceFilm& film, const dictionary& dict);

        //- Disallow default bitwise copy construction
        VoFPatchTransfer(const VoFPatchTransfer&) = delete;


    //- Destructor
    virtual ~VoFPatchTransfer();


    // Member Functions

        //- Correct
        virtual void correct
        (
            scalarField& availableMass,
            scalarField& massToTransfer,
            vectorField& momentumToTransfer
        );

        //- Correct kinematic and thermodynamic transfers
        virtual void correct
        (
            scalarField& availableMass,
            scalarField& massToTransfer,
            vectorField& momentumToTransfer,
            scalarField& energyToTransfer
        );

        //- Accumulate the total mass injected for the patches into the
        //  scalarField provided
        virtual void patchTransferredMassTotals
        (
            scalarField& patchMasses
        ) const;


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const VoFPatchTransfer&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmSubModels
} // End namespace regionModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
