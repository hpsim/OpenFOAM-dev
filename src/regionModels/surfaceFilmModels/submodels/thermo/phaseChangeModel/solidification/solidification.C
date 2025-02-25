/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2013-2022 OpenFOAM Foundation
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

#include "solidification.H"
#include "addToRunTimeSelectionTable.H"
#include "thermoSurfaceFilm.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace surfaceFilmSubModels
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(solidification, 0);

addToRunTimeSelectionTable
(
    phaseChangeModel,
    solidification,
    dictionary
);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

solidification::solidification
(
    surfaceFilm& film,
    const dictionary& dict
)
:
    phaseChangeModel(typeName, film, dict),
    T0_(coeffDict_.lookup<scalar>("T0")),
    maxSolidificationFrac_
    (
        coeffDict_.lookupOrDefault("maxSolidificationFrac", 0.2)
    ),
    maxSolidificationRate_
    (
        dimensioned<scalar>::lookupOrDefault
        (
            "maxSolidificationRate",
            coeffDict_,
            dimless/dimTime,
            great
        )
    ),
    mass_
    (
        IOobject
        (
            typedName("mass"),
            film.regionMesh().time().timeName(),
            film.regionMesh(),
            IOobject::READ_IF_PRESENT,
            IOobject::AUTO_WRITE
        ),
        film.regionMesh(),
        dimensionedScalar(dimMass, 0)
    ),
    thickness_
    (
        IOobject
        (
            typedName("thickness"),
            film.regionMesh().time().timeName(),
            film.regionMesh(),
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        film.regionMesh(),
        dimensionedScalar(dimLength, 0)
    )
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

solidification::~solidification()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void solidification::correctModel
(
    const scalar dt,
    scalarField& availableMass,
    scalarField& dMass,
    scalarField& dEnergy
)
{
    const thermoSurfaceFilm& film = filmType<thermoSurfaceFilm>();

    const scalarField& T = film.thermo().T();
    const scalarField& he = film.thermo().he();
    const scalarField& coverage = film.coverage();

    const scalar rateLimiter = min
    (
        maxSolidificationFrac_,
        (
            maxSolidificationRate_
           *filmModel_.regionMesh().time().deltaTValue()
        ).value()
    );

    forAll(coverage, celli)
    {
        if (coverage[celli] > 0.5)
        {
            if (T[celli] < T0_)
            {
                const scalar dm = rateLimiter*availableMass[celli];

                mass_[celli] += dm;
                dMass[celli] += dm;

                // Heat is assumed to be removed by heat-transfer to the wall
                // so the energy remains unchanged by the phase-change.
                dEnergy[celli] += dm*he[celli];
            }
        }
    }

    thickness_ = mass_/film.magSf()/film.rho();
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmSubModels
} // End namespace regionModels
} // End namespace Foam

// ************************************************************************* //
