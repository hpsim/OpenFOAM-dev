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

#include "interRegionModel.H"

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::fv::interRegionModel::interpolate
(
    const Field<Type>& field
) const
{
    if (master())
    {
        return meshInterp().mapTgtToSrc(field);
    }
    else
    {
        return nbrModel().meshInterp().mapSrcToTgt(field);
    }
}


template<class Type>
void Foam::fv::interRegionModel::interpolate
(
    const Field<Type>& field,
    Field<Type>& result
) const
{
    if (master())
    {
        meshInterp().mapTgtToSrc(field, result);
    }
    else
    {
        nbrModel().meshInterp().mapSrcToTgt(field, result);
    }
}


// ************************************************************************* //
