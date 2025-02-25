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

Class
    Foam::radiationCoupledBase

Description
    Common functions to emissivity. It gets supplied from lookup into a
    dictionary or calculated by the solidThermo:

    - 'lookup' : Read the patch emissivity field from the dictionary
    - 'solidRadiation' : Use the emissivity field mapped from the adjacent solid

SourceFiles
    radiationCoupledBase.C

\*---------------------------------------------------------------------------*/

#ifndef radiationCoupledBase_H
#define radiationCoupledBase_H

#include "scalarField.H"
#include "NamedEnum.H"
#include "fvPatch.H"
#include "fvPatchFieldMapper.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                    Class radiationCoupledBase Declaration
\*---------------------------------------------------------------------------*/

class radiationCoupledBase
{
public:

    //- Type of supplied emissivity
    enum emissivityMethodType
    {
        SOLIDRADIATION,
        LOOKUP
    };


private:

    // Private Data

        static const NamedEnum<emissivityMethodType, 2>
            emissivityMethodTypeNames_;

        //- Underlying patch
        const fvPatch& patch_;


protected:

    // Protected data

        //- How to get emissivity
        const emissivityMethodType method_;

        //- Emissivity
        // Cached locally when is read from dictionary (lookup mode)
        scalarField emissivity_;


public:

     //- Runtime type information
    TypeName("radiationCoupledBase");


    // Constructors

        //- Construct from patch, emissivity mode and emissivity
        radiationCoupledBase
        (
            const fvPatch& patch,
            const word& calculationMethod,
            const scalarField& emissivity
        );

        //- Construct from patch, emissivity mode and emissivity and mapper
        radiationCoupledBase
        (
            const fvPatch& patch,
            const word& calculationMethod,
            const scalarField& emissivity,
            const fvPatchFieldMapper& mapper
        );

        //- Construct from patch and dictionary
        radiationCoupledBase
        (
            const fvPatch& patch,
            const dictionary& dict
        );


    //- Destructor
        virtual ~radiationCoupledBase();


    // Member Functions

        // Access

            //- Method to obtain emissivity
            word emissivityMethod() const
            {
                return emissivityMethodTypeNames_[method_];
            }


            //- Calculate corresponding emissivity field
            tmp<scalarField> emissivity() const;


        // Mapping functions

            //- Map (and resize as needed) from self given a mapping object
            //  Used to update fields following mesh topology change
            virtual void autoMap(const fvPatchFieldMapper&);

            //- Reverse map the given fvPatchField onto this fvPatchField
            //  Used to reconstruct fields
            virtual void rmap(const fvPatchScalarField&, const labelList&);

            //- Reset the fvPatchField to the given fvPatchField
            //  Used for mesh to mesh mapping
            virtual void reset(const fvPatchScalarField&);


        //- Write
        void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
