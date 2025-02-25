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

\*---------------------------------------------------------------------------*/

#include "BrunDrippingEjection.H"
#include "addToRunTimeSelectionTable.H"
#include "momentumSurfaceFilm.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace surfaceFilmSubModels
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(BrunDrippingEjection, 0);
addToRunTimeSelectionTable(ejectionModel, BrunDrippingEjection, dictionary);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

BrunDrippingEjection::BrunDrippingEjection
(
    surfaceFilm& film,
    const dictionary& dict
)
:
    ejectionModel(type(), film, dict),
    ubarStar_(coeffDict_.lookupOrDefault("ubarStar", 1.62208)),
    dCoeff_(coeffDict_.lookupOrDefault("dCoeff", 3.3)),
    deltaStable_(coeffDict_.lookupOrDefault("deltaStable", scalar(0))),
    diameter_(film.regionMesh().nCells(), -1.0)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

BrunDrippingEjection::~BrunDrippingEjection()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void BrunDrippingEjection::correct
(
    scalarField& availableMass,
    scalarField& massToEject,
    scalarField& diameterToEject
)
{
    const momentumSurfaceFilm& film =
        refCast<const momentumSurfaceFilm>(this->film());

    // Calculate available dripping mass
    tmp<volScalarField> tsinAlpha((film.g()/mag(film.g())) & film.nHat());
    const scalarField& sinAlpha = tsinAlpha();
    const scalarField& magSf = film.magSf();

    const scalarField& delta = film.delta();
    const scalarField& rho = film.rho();

    const tmp<volScalarField> tsigma = film.sigma();
    const volScalarField::Internal& sigma = tsigma();

    const scalar magg = mag(film.g().value());

    forAll(delta, celli)
    {
        bool dripping = false;

        if (sinAlpha[celli] > small && delta[celli] > deltaStable_)
        {
            const scalar rhoc = rho[celli];
            const scalar lc = sqrt(sigma[celli]/(rhoc*magg));
            const scalar deltaStable = max
            (
                3*lc*sqrt(1 - sqr(sinAlpha[celli]))
               /(ubarStar_*sqrt(sinAlpha[celli])*sinAlpha[celli]),
                deltaStable_
            );

            if (delta[celli] > deltaStable)
            {
                const scalar ddelta = max(delta[celli] - deltaStable, 0);

                const scalar massDrip =
                    min(availableMass[celli], max(ddelta*rhoc*magSf[celli], 0));

                if (massDrip > 0)
                {
                    const scalar diam = dCoeff_*lc;
                    diameter_[celli] = diam;

                    massToEject[celli] += massDrip;
                    availableMass[celli] -= massDrip;

                    diameterToEject[celli] = diam;
                    addToEjectedMass(massDrip);

                    dripping = true;
                }
            }
        }

        if (!dripping)
        {
            diameterToEject[celli] = 0;
            massToEject[celli] = 0;
        }
    }

    ejectionModel::correct();
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmSubModels
} // End namespace regionModels
} // End namespace Foam

// ************************************************************************* //
