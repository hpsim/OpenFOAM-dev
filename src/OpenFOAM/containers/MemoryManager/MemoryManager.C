/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2018 OpenFOAM Foundation
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

#include "MemoryManager.H"

#include "addToRunTimeSelectionTable.H" // for addTemplateToRunSelectionTable

//Foam::autoPtr<Foam::MemoryHandler> Foam::MemoryHandler::New
//(
//    const word& handlerType,
//    const polyMesh& mesh,
//    const word& name,
//    const label size
//)
//{
//    sizeConstructorTable::iterator cstrIter =
//	sizeConstructorTablePtr_->find(handlerType);
//
//    if (cstrIter == sizeConstructorTablePtr_->end())
//    {
//	FatalErrorInFunction
//	    << "Unknown MemoryHandler type " << handlerType
//	    << endl << endl
//	    //<< "Valid MemoryHandler types : " << endl
//	    //<< sizeConstructorTablePtr_->sortedToc()
//	    << exit(FatalError);
//    }
//
//    return autoPtr<MemoryHandler>(cstrIter()(size));
//}

namespace Foam
{

    //template<>
    //const word MemoryHandlerFloat::typeName("MemoryHandlerFloat");

    //defineTypeNameAndDebug(MemoryHandler, 0);
    defineRunTimeSelectionTable(MemoryHandler, size);

    //template<>
    //const word OFDefaultMemoryHandlerFloat::typeName("OFDefaultMemoryHandlerFloat");
    
    //defineTemplateTypeNameAndDebug(OFDefaultMemoryHandlerFloat, 0);
    addTemplateToRunTimeSelectionTable(MemoryHandler, OFDefaultMemoryHandler, float, size);
}

Foam::MemoryHandler::MemoryHandler(const label size):
    size_(size)
{}

template<class T>
Foam::OFDefaultMemoryHandler<T>::OFDefaultMemoryHandler(const label size):
    MemoryHandler(size),
    v_(NULL)
{}

//template<> Foam::OFDefaultMemoryHandler<float>::OFDefaultMemoryHandler(const label);

