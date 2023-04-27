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
    Foam::UList

Description
    A 1D vector of objects of type \<T\>, where the size of the vector is
    known and can be used for subscript bounds checking, etc.

    Storage is not allocated during construction or use but is supplied to
    the constructor as an argument.  This type of list is particularly useful
    for lists that refer to parts of existing lists such as SubList.

SourceFiles
    UList.C
    UListI.H
    UListIO.C

\*---------------------------------------------------------------------------*/

#ifndef UList_H
#define UList_H


// all files are collected in central folder disregarding sub-folders
#ifdef WM_COMPUTE_BACKEND_Default
    #pragma message "[Compile] Default CPU memory manager"
    #include "UListDefault.H"
#endif
#ifdef WM_COMPUTE_BACKEND_Ginkgo
    #pragma message "[Compile] Ginkgo memory manager"
    #include "UListGinkgo.H"
#endif

#endif

// ************************************************************************* //
