/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2021-2022 OpenFOAM Foundation
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

#include "speciePhaseChange.H"
#include "thermoSurfaceFilm.H"
#include "fluidThermo.H"
#include "basicSpecieMixture.H"
#include "liquidThermo.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace surfaceFilmSubModels
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(speciePhaseChange, 0);


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

speciePhaseChange::speciePhaseChange
(
    const word& modelType,
    surfaceFilm& film,
    const dictionary& dict
)
:
    phaseChangeModel(modelType, film, dict)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

speciePhaseChange::~speciePhaseChange()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

Foam::label speciePhaseChange::vapId() const
{
    const thermoSurfaceFilm& film = filmType<thermoSurfaceFilm>();

    // Set local liquidThermo properties
    const liquidProperties& liquidThermo =
        refCast<const heRhoThermopureMixtureliquidProperties>(film.thermo())
       .cellThermoMixture(0).properties();

    const basicSpecieMixture& primarySpecieThermo =
        refCast<const basicSpecieMixture>(film.primaryThermo());

    return primarySpecieThermo.species()[liquidThermo.name()];
}


Foam::scalar speciePhaseChange::Wvap() const
{
    const thermoSurfaceFilm& film = filmType<thermoSurfaceFilm>();

    const basicSpecieMixture& primarySpecieThermo =
        refCast<const basicSpecieMixture>(film.primaryThermo());

    return primarySpecieThermo.Wi(vapId());
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmSubModels
} // End namespace regionModels
} // End namespace Foam

// ************************************************************************* //
