/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2016-2022 OpenFOAM Foundation
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
    Foam::diameterModels::IATEsources::wallBoiling

Description
    Wall-boiling IATE source.

SourceFiles
    wallBoiling.C

\*---------------------------------------------------------------------------*/

#ifndef IATEwallBoiling_H
#define IATEwallBoiling_H

#include "IATEsource.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace diameterModels
{
namespace IATEsources
{

/*---------------------------------------------------------------------------*\
                         Class wallBoiling Declaration
\*---------------------------------------------------------------------------*/

class wallBoiling
:
    public IATEsource
{

public:

    //- Runtime type information
    TypeName("wallBoiling");


    // Constructors

        wallBoiling
        (
            const IATE& iate,
            const dictionary& dict
        );


    //- Destructor
    virtual ~wallBoiling()
    {}


    // Member Functions

        virtual tmp<fvScalarMatrix> R
        (
            const volScalarField& alphai,
            volScalarField& kappai
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace IATEsources
} // End namespace diameterModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
