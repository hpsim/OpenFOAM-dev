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

#include "mappedValueFvPatchField.H"
#include "volFields.H"

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

template<class Type>
const Foam::mappedPatchBase&
Foam::mappedValueFvPatchField<Type>::mapper() const
{
    if (mapperPtr_.valid())
    {
        return mapperPtr_();
    }

    if (isA<mappedPatchBase>(this->patch().patch()))
    {
        return refCast<const mappedPatchBase>(this->patch().patch());
    }

    FatalErrorInFunction
        << "Field " << this->internalField().name() << " on patch "
        << this->patch().name() << " in file "
        << this->internalField().objectPath()
        << " has neither a mapper specified nor is the patch of "
        << mappedPatchBase::typeName << " type"
        << exit(FatalError);

    return NullObjectRef<mappedPatchBase>();
}


template<class Type>
const Foam::fvPatchField<Type>&
Foam::mappedValueFvPatchField<Type>::nbrPatchField() const
{
    const fvMesh& nbrMesh =
        refCast<const fvMesh>(this->mapper().nbrMesh());

    const VolField<Type>& nbrField =
        this->mapper().sameRegion()
     && this->fieldName_ == this->internalField().name()
      ? refCast<const VolField<Type>>(this->internalField())
      : nbrMesh.template lookupObject<VolField<Type>>(this->fieldName_);

    const label nbrPatchi = this->mapper().nbrPolyPatch().index();

    return nbrField.boundaryField()[nbrPatchi];
}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::mappedValueFvPatchField<Type>::mappedValues
(
    const Field<Type>& nbrPatchField
) const
{
    // Since we're inside initEvaluate/evaluate there might be processor
    // comms underway. Change the tag we use.
    int oldTag = UPstream::msgType();
    UPstream::msgType() = oldTag + 1;

    // Map values
    tmp<Field<Type>> tResult = this->mapper().distribute(nbrPatchField);

    // Set the average, if necessary
    if (setAverage_)
    {
        const Type nbrAverageValue =
            gSum(this->patch().magSf()*tResult())
           /gSum(this->patch().magSf());

        if (mag(nbrAverageValue)/mag(average_) > 0.5)
        {
            tResult.ref() *= mag(average_)/mag(nbrAverageValue);
        }
        else
        {
            tResult.ref() += average_ - nbrAverageValue;
        }
    }

    // Restore tag
    UPstream::msgType() = oldTag;

    return tResult;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::mappedValueFvPatchField<Type>::mappedValueFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(p, iF),
    fieldName_(iF.name()),
    setAverage_(false),
    average_(Zero),
    mapperPtr_(nullptr)
{}


template<class Type>
Foam::mappedValueFvPatchField<Type>::mappedValueFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<Type>(p, iF, dict),
    fieldName_(dict.lookupOrDefault<word>("field", iF.name())),
    setAverage_
    (
        dict.lookupOrDefault<bool>("setAverage", dict.found("average"))
    ),
    average_(setAverage_ ? dict.lookup<Type>("average") : Zero),
    mapperPtr_
    (
        mappedPatchBase::specified(dict)
      ? new mappedPatchBase(p.patch(), dict, false)
      : nullptr
    )
{}


template<class Type>
Foam::mappedValueFvPatchField<Type>::mappedValueFvPatchField
(
    const mappedValueFvPatchField<Type>& ptf,
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchField<Type>(ptf, p, iF, mapper),
    fieldName_(ptf.fieldName_),
    setAverage_(ptf.setAverage_),
    average_(ptf.average_),
    mapperPtr_
    (
        ptf.mapperPtr_.valid()
      ? new mappedPatchBase(p.patch(), ptf.mapperPtr_())
      : nullptr
    )
{}


template<class Type>
Foam::mappedValueFvPatchField<Type>::mappedValueFvPatchField
(
    const mappedValueFvPatchField<Type>& ptf,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(ptf, iF),
    fieldName_(ptf.fieldName_),
    setAverage_(ptf.setAverage_),
    average_(ptf.average_),
    mapperPtr_
    (
        ptf.mapperPtr_.valid()
      ? new mappedPatchBase(ptf.patch().patch(), ptf.mapperPtr_())
      : nullptr
    )
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void Foam::mappedValueFvPatchField<Type>::autoMap
(
    const fvPatchFieldMapper& m
)
{
    fixedValueFvPatchField<Type>::autoMap(m);

    if (mapperPtr_.valid())
    {
        mapperPtr_->clearOut();
    }
}


template<class Type>
void Foam::mappedValueFvPatchField<Type>::rmap
(
    const fvPatchField<Type>& ptf,
    const labelList& addr
)
{
    fixedValueFvPatchField<Type>::rmap(ptf, addr);

    if (mapperPtr_.valid())
    {
        mapperPtr_->clearOut();
    }
}


template<class Type>
void Foam::mappedValueFvPatchField<Type>::reset
(
    const fvPatchField<Type>& ptf
)
{
    fixedValueFvPatchField<Type>::reset(ptf);

    if (mapperPtr_.valid())
    {
        mapperPtr_->clearOut();
    }
}


template<class Type>
void Foam::mappedValueFvPatchField<Type>::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    this->operator==(mappedValues(nbrPatchField()));

    fixedValueFvPatchField<Type>::updateCoeffs();
}


template<class Type>
void Foam::mappedValueFvPatchField<Type>::write(Ostream& os) const
{
    fvPatchField<Type>::write(os);

    writeEntryIfDifferent
    (
        os,
        "field",
        this->internalField().name(),
        fieldName_
    );

    if (setAverage_)
    {
        writeEntry(os, "average", average_);
    }

    if (mapperPtr_.valid())
    {
        mapperPtr_->write(os);
    }

    writeEntry(os, "value", *this);
}


// ************************************************************************* //
