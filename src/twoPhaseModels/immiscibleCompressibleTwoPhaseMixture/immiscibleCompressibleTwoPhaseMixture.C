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

\*---------------------------------------------------------------------------*/

#include "immiscibleCompressibleTwoPhaseMixture.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::immiscibleCompressibleTwoPhaseMixture::
immiscibleCompressibleTwoPhaseMixture(const volVectorField& U)
:
    compressibleTwoPhaseMixture(U.mesh()),
    interfaceProperties(*this, alpha1(), alpha2(), U)
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Foam::immiscibleCompressibleTwoPhaseMixture::correct()
{
    compressibleTwoPhaseMixture::correct();
    interfaceProperties::correct();
}


bool Foam::immiscibleCompressibleTwoPhaseMixture::read()
{
    return
        compressibleTwoPhaseMixture::read()
     && interfaceProperties::read();
}


// ************************************************************************* //
