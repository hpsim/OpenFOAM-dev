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

#include "UListGinkgo.H"
#include "ListLoopM.H"
#include "contiguous.H"


#include <algorithm>

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


template<class T>
void Foam::UList<T>::deepCopy(const UList<T>& a)
{
    if (a.size() != this->size())
    {
        FatalErrorInFunction
            << "ULists have different sizes: "
            << this->size() << " " << a.size()
            << abort(FatalError);
    }

    if (this->size())
    {
        #ifdef USEMEMCPY
        if (contiguous<T>())
        {
            memcpy(this->values_, a.values_, this->byteSize());
        }
        else
        #endif
        {
            List_ACCESS(T, (*this), vp);
            List_CONST_ACCESS(T, a, ap);
            List_FOR_ALL((*this), i)
                List_ELEM((*this), vp, i) = List_ELEM(a, ap, i);
            List_END_FOR_ALL
        }
    }
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

// NOTE this would be a fill
template<class T>
void Foam::UList<T>::operator=(const T& t)
{
    //values_.fill(t);
}


template<class T>
void Foam::UList<T>::operator=(const zero)
{
    //values_.fill(Zero);
}


// * * * * * * * * * * * * * * STL Member Functions  * * * * * * * * * * * * //

template<class T>
void Foam::UList<T>::swap(UList<T>& a)
{
    // TODO implement
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class T>
std::streamsize Foam::UList<T>::byteSize() const
{
    if (!contiguous<T>())
    {
        FatalErrorInFunction
            << "Cannot return the binary size of a list of "
               "non-primitive elements"
            << abort(FatalError);
    }

    return this->values_.get_num_elems()*sizeof(T);
}


// * * * * * * * * * * * * * * * Free Functions  * * * * * * * * * * * * * //

template<class T>
void Foam::sort(UList<T>& a)
{
    // TODO implement
    //std::sort(a.begin(), a.end());
}


template<class T, class Cmp>
void Foam::sort(UList<T>& a, const Cmp& cmp)
{
    // TODO implement
    //std::sort(a.begin(), a.end());
}


template<class T>
void Foam::stableSort(UList<T>& a)
{
    // TODO implement
}


template<class T, class Cmp>
void Foam::stableSort(UList<T>& a, const Cmp& cmp)
{
    // TODO implement
}


template<class T>
void Foam::shuffle(UList<T>& a)
{
    // TODO implement
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

template<class T>
bool Foam::UList<T>::operator==(const UList<T>& a) const
{
    // TODO implement
    return false;
}


template<class T>
bool Foam::UList<T>::operator!=(const UList<T>& a) const
{
    return !operator==(a);
}


template<class T>
bool Foam::UList<T>::operator<(const UList<T>& a) const
{
    // TODO implement
    return false;
}


template<class T>
bool Foam::UList<T>::operator>(const UList<T>& a) const
{
    return a.operator<(*this);
}


template<class T>
bool Foam::UList<T>::operator<=(const UList<T>& a) const
{
    // TODO implement
    return false;
}


template<class T>
bool Foam::UList<T>::operator>=(const UList<T>& a) const
{
    return !operator<(a);
}


// * * * * * * * * * * * * * * * *  IOStream operators * * * * * * * * * * * //

#include "UListIOGinkgo.C"

// ************************************************************************* //
