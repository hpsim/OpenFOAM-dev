/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2021 OpenFOAM Foundation
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
    Foam::MemoryManager

Description


SourceFiles
    MemoryManager.C

\*---------------------------------------------------------------------------*/

#include "runTimeSelectionTables.H"
#include "labelList.H"
//#include "regIOobject.H"
#include "typeInfo.H"

namespace Foam
{

class polyMesh;
class MemoryHandler {

    // Private Data

        //- Number of elements in UList
        label size_;

public:

    //- Runtime type information
    TypeName("MemoryHandler");

    declareRunTimeSelectionTable(
        autoPtr,
        MemoryHandler,
        size,
        (
            const label size
        ),
        (size)
    );

    //- getter

    label size(){return size_;};

    // Selectors

    //- Return a pointer to a toposet read from file
    static autoPtr<MemoryHandler> New
    (
        const word& setType,
        const polyMesh& mesh,
        const word& name,
        const label size
    );

    // construct from size
    MemoryHandler(
        const label size
    );

};


template<class T>
class OFDefaultMemoryHandler:
	public MemoryHandler
{

        //- Vector of values of type T
        T* __restrict__ v_;


public:

    TypeName("OFDefaultMemoryHandlerFloat");

    OFDefaultMemoryHandler(const label size);

};

typedef OFDefaultMemoryHandler<float> OFDefaultMemoryHandlerFloat;;

}
