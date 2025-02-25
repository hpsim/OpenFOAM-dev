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

#include "setWriteIntervalFunctionObject.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{
    defineTypeNameAndDebug(setWriteIntervalFunctionObject, 0);

    addToRunTimeSelectionTable
    (
        functionObject,
        setWriteIntervalFunctionObject,
        dictionary
    );
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::functionObjects::setWriteIntervalFunctionObject::
setWriteIntervalFunctionObject
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    functionObject(name),
    time_(runTime)
{
    read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::functionObjects::setWriteIntervalFunctionObject::
~setWriteIntervalFunctionObject()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::functionObjects::setWriteIntervalFunctionObject::read
(
    const dictionary& dict
)
{
    writeIntervalPtr_ = Function1<scalar>::New("writeInterval", dict);

    return true;
}


bool Foam::functionObjects::setWriteIntervalFunctionObject::execute()
{
    const_cast<Time&>(time_).setWriteInterval
    (
        time_.userTimeToTime(writeIntervalPtr_().value(time_.userTimeValue()))
    );

    return true;
}


bool Foam::functionObjects::setWriteIntervalFunctionObject::write()
{
    return true;
}


// ************************************************************************* //
