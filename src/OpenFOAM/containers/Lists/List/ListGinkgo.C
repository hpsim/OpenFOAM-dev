/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2020 OpenFOAM Foundation
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

#include "List.H"
#include "ListLoopM.H"
#include "FixedList.H"
#include "PtrList.H"
#include "SLList.H"
#include "IndirectList.H"
#include "UIndirectList.H"
#include "BiIndirectList.H"
#include "contiguous.H"

#include <type_traits>

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

template<class T>
Foam::List<T>::List(const label s)
:
    UList<T>(nullptr, s)
{
    if (this->size_ < 0)
    {
        FatalErrorInFunction
            << "bad size " << this->size_
            << abort(FatalError);
    }

    alloc();
}


template<class T>
Foam::List<T>::List(const label s, const T& a)
:
    UList<T>(nullptr, s)
{
    if (this->size_ < 0)
    {
        FatalErrorInFunction
            << "bad size " << this->size_
            << abort(FatalError);
    }

    alloc();


    if (this->size_)
    {
        if ( std::is_same<T, scalar>::value or 
		 std::is_same<T, label>::value
		 ) {
		 this->values_.fill(a);
		 return;
	}

        List_ACCESS(T, (*this), vp);
        List_FOR_ALL((*this), i)
            List_ELEM((*this), vp, i) = a;
        List_END_FOR_ALL
    }
}


template<class T>
Foam::List<T>::List(const label s, const zero)
:
    UList<T>(nullptr, s)
{
    if (this->size_ < 0)
    {
        FatalErrorInFunction
            << "bad size " << this->size_
            << abort(FatalError);
    }

    alloc();

    if (this->size_)
    {
    if ( std::is_same<T, scalar>::value or 
	 std::is_same<T, label>::value) {
	 this->values_.fill(Zero);
	 return;
    }
        List_ACCESS(T, (*this), vp);
        List_FOR_ALL((*this), i)
            List_ELEM((*this), vp, i) = Zero;
        List_END_FOR_ALL
    }
}


template<class T>
Foam::List<T>::List(const List<T>& a)
:
    UList<T>(nullptr, a.size_)
{
    if (this->size_)
    {
        alloc();
	if ( std::is_same<T, scalar>::value or 
	     std::is_same<T, label>::value) {
	     this->values_ = a.values_;
	     return;
	}

        #ifdef USEMEMCPY
        if (contiguous<T>())
        {
            memcpy(this->v_, a.v_, this->byteSize());
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


template<class T>
template<class T2>
Foam::List<T>::List(const List<T2>& a)
:
    UList<T>(nullptr, a.size())
{
    if (this->size_)
    {
        alloc();
	if ( std::is_same<T, scalar>::value or 
	     std::is_same<T, label>::value) {
	     // TODO implement this
	     //this->values_ = a.values_;
	     return;
	}

        List_ACCESS(T, (*this), vp);
        List_CONST_ACCESS(T2, a, ap);
        List_FOR_ALL((*this), i)
            List_ELEM((*this), vp, i) = T(List_ELEM(a, ap, i));
        List_END_FOR_ALL
    }
}


template<class T>
Foam::List<T>::List(List<T>&& lst)
{
    transfer(lst);
}


template<class T>
Foam::List<T>::List(List<T>& a, bool reuse)
:
    UList<T>(nullptr, a.size_)
{
    if (reuse)
    {
        this->v_ = a.v_;
        a.v_ = 0;
        a.size_ = 0;
    }
    else if (this->size_)
    {
        alloc();
	if ( std::is_same<T, scalar>::value or 
	     std::is_same<T, label>::value) {
	     this->values_ = a.values_;
	     return;
	}

        #ifdef USEMEMCPY
        if (contiguous<T>())
        {
            memcpy(this->v_, a.v_, this->byteSize());
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


template<class T>
Foam::List<T>::List(const UList<T>& a, const labelUList& map)
:
    UList<T>(nullptr, map.size())
{
    if (this->size_)
    {
        // Note:cannot use List_ELEM since third argument has to be index.

        alloc();

        forAll(*this, i)
        {
            this->operator[](i) = a[map[i]];
        }
    }
}


template<class T>
template<class InputIterator>
Foam::List<T>::List(InputIterator first, InputIterator last)
:
    List<T>(first, last, std::distance(first, last))
{}


template<class T>
template<unsigned Size>
Foam::List<T>::List(const FixedList<T, Size>& lst)
:
    UList<T>(nullptr, Size)
{
    allocCopyList(lst);
}


template<class T>
Foam::List<T>::List(const PtrList<T>& lst)
:
    UList<T>(nullptr, lst.size())
{
    allocCopyList(lst);
}


template<class T>
Foam::List<T>::List(const SLList<T>& lst)
:
    List<T>(lst.begin(), lst.end(), lst.size())
{}


template<class T>
Foam::List<T>::List(const UIndirectList<T>& lst)
:
    UList<T>(nullptr, lst.size())
{
    allocCopyList(lst);
}


template<class T>
Foam::List<T>::List(const BiIndirectList<T>& lst)
:
    UList<T>(nullptr, lst.size())
{
    allocCopyList(lst);
}


template<class T>
Foam::List<T>::List(std::initializer_list<T> lst)
:
    List<T>(lst.begin(), lst.end())
{}


// * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * * //

template<class T>
Foam::List<T>::~List()
{
    if (this->v_)
    {
         delete[] this->v_;
    }

    // TODO destructor needed here?
    //~this->values_;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class T>
void Foam::List<T>::setSize(const label newSize)
{
    if ( std::is_same<T, scalar>::value or 
	 std::is_same<T, label>::value) {
	 setSizeGkoImpl_(newSize);
	    return;
    }
    if (newSize < 0)
    {
        FatalErrorInFunction
            << "bad size " << newSize
            << abort(FatalError);
    }

    if (newSize != this->size_)
    {
        if (newSize > 0)
        {
            T* nv = new T[label(newSize)];

            if (this->size_)
            {
                label i = min(this->size_, newSize);

                #ifdef USEMEMCPY
                if (contiguous<T>())
                {
                    memcpy(nv, this->v_, i*sizeof(T));
                }
                else
                #endif
                {
                    T* vv = &this->v_[i];
                    T* av = &nv[i];
                    while (i--) *--av = *--vv;
                }
            }

            clear();
            this->size_ = newSize;
            this->v_ = nv;
        }
        else
        {
            clear();
        }
    }
}


template<class T>
void Foam::List<T>::setSizeGkoImpl_(const label newSize)
{
    if (newSize < 0)
    {
        FatalErrorInFunction
            << "bad size " << newSize
            << abort(FatalError);
    }

    if (newSize != this->size_)
    {
        if (newSize > 0)
        {
            auto newValues = gko::array<T>(
		gko::share(gko::ReferenceExecutor::create()),
		newSize);

	    // copy over existing data
            if (this->size_)
            {
            	auto newView = gko::array<T>::view(
			gko::share(gko::ReferenceExecutor::create()),
			this->size_,
			newValues.get_data());
		newView = this->values_; 
            }

            clear();
	    this->values_ = newValues;
        }
        else
        {
            clear();
        }
    }
}




template<class T>
void Foam::List<T>::setSize(const label newSize, const T& a)
{
    if ( std::is_same<T, scalar>::value or 
	 std::is_same<T, label>::value) {
	setSizeGkoImpl_(newSize, a);
	return;
    }
    label oldSize = label(this->size_);
    this->setSize(newSize);

    if (newSize > oldSize)
    {
        label i = newSize - oldSize;
        T* vv = &this->v_[newSize];
        while (i--) *--vv = a;
    }
}


template<class T>
void Foam::List<T>::setSizeGkoImpl_(const label newSize, const T& a)
{
    label oldSize = label(this->size_);
    this->setSize(newSize);

    if (newSize > oldSize)
    {
            auto newValues = gko::array<T>(
		gko::share(gko::ReferenceExecutor::create()),
		newSize - oldSize);

	    newValues.fill(a);

	    auto endView = gko::array<T>::view(
			gko::share(gko::ReferenceExecutor::create()),
			newSize - oldSize,
			&newValues.get_data()[oldSize]);

	    endView = newValues;
    }
}


template<class T>
void Foam::List<T>::transfer(List<T>& a)
{
    if ( std::is_same<T, scalar>::value or 
	 std::is_same<T, label>::value) {
	    transferGkoImpl_(a);
	    return;
    }
    clear();
    this->size_ = a.size_;
    this->v_ = a.v_;

    a.size_ = 0;
    a.v_ = 0;
}

// Transfer the ownership
template<class T>
void Foam::List<T>::transferGkoImpl_(List<T>& a)
{
    clear();
    this->size_ = a.size_;
    this->values_ = a.values_;

    a.size_ = 0;
    a.values_ = gko::array<T>(
		gko::share(gko::ReferenceExecutor::create()));
}


template<class T>
template<unsigned SizeInc, unsigned SizeMult, unsigned SizeDiv>
void Foam::List<T>::transfer(DynamicList<T, SizeInc, SizeMult, SizeDiv>& a)
{
    // Shrink the allocated space to the number of elements used
    a.shrink();
    transfer(static_cast<List<T>&>(a));
    a.clearStorage();
}


template<class T>
void Foam::List<T>::transfer(SortableList<T>& a)
{
    // Shrink away the sort indices
    a.shrink();
    transfer(static_cast<List<T>&>(a));
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

template<class T>
void Foam::List<T>::operator=(const UList<T>& a)
{
	operatorEqImpl(a);
}

template<class T>
void Foam::List<T>::operatorEqImpl(const UList<T>& a) {
    reAlloc(a.size_);

    if (!this->size_) return;

    #ifdef WM_COMPUTE_BACKEND_Ginkgo
    if ( std::is_same<T, scalar>::value or 
	 std::is_same<T, label>::value) {
	    this->values_ = a.values_;
	    return;
    }
    #endif

    #ifdef USEMEMCPY
    if (contiguous<T>())
    {
         memcpy(this->v_, a.v_, this->byteSize());
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




template<class T>
void Foam::List<T>::operator=(const List<T>& a)
{
    if (this == &a)
    {
        FatalErrorInFunction
            << "attempted assignment to self"
            << abort(FatalError);
    }

    operator=(static_cast<const UList<T>&>(a));
}


template<class T>
void Foam::List<T>::operator=(List<T>&& a)
{
    if (this == &a)
    {
        FatalErrorInFunction
            << "attempted assignment to self"
            << abort(FatalError);
    }

    transfer(a);
}


template<class T>
void Foam::List<T>::operator=(const SLList<T>& lst)
{
    reAlloc(lst.size());

    if (this->size_)
    {
        label i = 0;
        for
        (
            typename SLList<T>::const_iterator iter = lst.begin();
            iter != lst.end();
            ++iter
        )
        {
            this->operator[](i++) = iter();
        }
    }
}


template<class T>
void Foam::List<T>::operator=(const UIndirectList<T>& lst)
{
    reAlloc(lst.size());
    copyList(lst);
}


template<class T>
void Foam::List<T>::operator=(const BiIndirectList<T>& lst)
{
    reAlloc(lst.size());
    copyList(lst);
}


template<class T>
void Foam::List<T>::operator=(std::initializer_list<T> lst)
{
    reAlloc(lst.size());

    typename std::initializer_list<T>::iterator iter = lst.begin();
    forAll(*this, i)
    {
        this->operator[](i) = *iter++;
    }
}


// * * * * * * * * * * * * * * * *  IOStream operators * * * * * * * * * * * //

#include "ListIO.C"

// ************************************************************************* //
