/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2015-2022 OpenFOAM Foundation
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
    Foam::fixedMultiphaseHeatFluxFvPatchScalarField

Description
    Calculates a wall temperature that produces the specified overall wall heat
    flux across all the phases in an Eulerian multi-phase simulation.

    Intended to be used with copiedFixedValue to ensure that phase wall
    temperature are consistent:
        - Set 'fixedMultiphaseHeatFlux' boundary for one of the phases
        - Use 'copiedFixedValue' for all the other phases.

See also
    Foam::fixedValueFvPatchField

SourceFiles
    fixedMultiphaseHeatFluxFvPatchScalarField.C

\*---------------------------------------------------------------------------*/

#ifndef fixedMultiphaseHeatFluxFvPatchScalarField_H
#define fixedMultiphaseHeatFluxFvPatchScalarField_H

#include "fixedValueFvPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
          Class fixedMultiphaseHeatFluxFvPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class fixedMultiphaseHeatFluxFvPatchScalarField
:
    public fixedValueFvPatchScalarField
{
    // Private Data

        //- Heat power [W] or flux [W/m^2]
        scalarField q_;

        //- Relaxation factor
        scalar relax_;

        //- Minimum temperature limit [K]
        scalar Tmin_;


public:

    //- Runtime type information
    TypeName("fixedMultiphaseHeatFlux");


    // Constructors

        //- Construct from patch and internal field
        fixedMultiphaseHeatFluxFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        fixedMultiphaseHeatFluxFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given
        //  fixedMultiphaseHeatFluxFvPatchScalarField
        //  onto a new patch
        fixedMultiphaseHeatFluxFvPatchScalarField
        (
            const fixedMultiphaseHeatFluxFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Disallow copy without setting internal field reference
        fixedMultiphaseHeatFluxFvPatchScalarField
        (
            const fixedMultiphaseHeatFluxFvPatchScalarField&
        ) = delete;

        //- Copy constructor setting internal field reference
        fixedMultiphaseHeatFluxFvPatchScalarField
        (
            const fixedMultiphaseHeatFluxFvPatchScalarField&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchScalarField> clone
        (
            const DimensionedField<scalar, volMesh>& iF
        ) const
        {
            return tmp<fvPatchScalarField>
            (
                new fixedMultiphaseHeatFluxFvPatchScalarField(*this, iF)
            );
        }


    // Member Functions

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


        // Evaluation functions

            //- Update the coefficients associated with the patch field
            virtual void updateCoeffs();


        // I-O

            //- Write
            virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
