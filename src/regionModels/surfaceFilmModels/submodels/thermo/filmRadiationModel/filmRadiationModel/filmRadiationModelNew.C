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

#include "noRadiation.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace surfaceFilmSubModels
{

// * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * * //

autoPtr<radiationModel> radiationModel::New
(
    surfaceFilm& model,
    const dictionary& dict
)
{
    if
    (
        !dict.lookupEntryPtrBackwardsCompatible
        (
            {radiationModel::typeName, "radiationModel"},
            false,
            true
        )
    )
    {
        return autoPtr<radiationModel>(new noRadiation(model, dict));
    }

    const dictionary& radiationDict
    (
        dict.found(radiationModel::typeName)
      ? dict.subDict(radiationModel::typeName)
      : dict
    );

    const word modelType
    (
        dict.found(radiationModel::typeName)
      ? radiationDict.lookup("model")
      : radiationDict.lookup("radiationModel")
    );

    Info<< "    Selecting radiationModel " << modelType << endl;

    dictionaryConstructorTable::iterator cstrIter =
        dictionaryConstructorTablePtr_->find(modelType);

    if (cstrIter == dictionaryConstructorTablePtr_->end())
    {
        FatalErrorInFunction
            << "Unknown radiationModel type " << modelType << nl << nl
            << "Valid radiationModel types are:" << nl
            << dictionaryConstructorTablePtr_->toc()
            << exit(FatalError);
    }

    return autoPtr<radiationModel>(cstrIter()(model, radiationDict));
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmSubModels
} // End namespace regionModels
} // End namespace Foam

// ************************************************************************* //
