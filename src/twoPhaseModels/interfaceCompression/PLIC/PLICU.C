/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2020-2022 OpenFOAM Foundation
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

#include "PLICU.H"
#include "slicedSurfaceFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(PLICU, 0);

    surfaceInterpolationScheme<scalar>::addMeshFluxConstructorToTable<PLICU>
        addPLICUScalarMeshFluxConstructorToTable_;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::tmp<Foam::surfaceScalarField> Foam::PLICU::interpolate
(
    const volScalarField& vf
) const
{
    tmp<surfaceScalarField> tvff(tScheme_().interpolate(vf));

    scalarField spicedTvff
    (
        slicedSurfaceScalarField
        (
            IOobject
            (
                "spicedTvff",
                vf.mesh().time().timeName(),
                vf.mesh()
            ),
            tvff.ref(),
            false
        ).splice()
    );

    return surfaceAlpha(vf, phi_, spicedTvff, false, 1e-6, false);
}


// ************************************************************************* //
