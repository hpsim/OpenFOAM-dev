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

#include "volFields.H"
#include "surfaceFields.H"
#include "fvMatrix.H"
#include "laplacianScheme.H"
#include "gaussLaplacianScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace fvm
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
tmp<fvMatrix<Type>>
laplacian
(
    const GeometricField<Type, fvPatchField, volMesh>& vf,
    const word& name
)
{
    surfaceScalarField Gamma
    (
        IOobject
        (
            "1",
            vf.time().constant(),
            vf.mesh(),
            IOobject::NO_READ
        ),
        vf.mesh(),
        dimensionedScalar(dimless, 1.0)
    );

    return fvm::laplacian(Gamma, vf, name);
}


template<class Type>
tmp<fvMatrix<Type>>
laplacian
(
    const GeometricField<Type, fvPatchField, volMesh>& vf
)
{
    surfaceScalarField Gamma
    (
        IOobject
        (
            "1",
            vf.time().constant(),
            vf.mesh(),
            IOobject::NO_READ
        ),
        vf.mesh(),
        dimensionedScalar(dimless, 1.0)
    );

    return fvm::laplacian
    (
        Gamma,
        vf,
        "laplacian(" + vf.name() + ')'
    );
}


template<class Type>
tmp<fvMatrix<Type>>
laplacian
(
    const zero&,
    const GeometricField<Type, fvPatchField, volMesh>& vf,
    const word& name
)
{
    return tmp<fvMatrix<Type>>
    (
        new fvMatrix<Type>(vf, dimensionSet(0, 0, -2, 0, 0))
    );
}


template<class Type>
tmp<fvMatrix<Type>>
laplacian
(
    const zero&,
    const GeometricField<Type, fvPatchField, volMesh>& vf
)
{
    return tmp<fvMatrix<Type>>
    (
        new fvMatrix<Type>(vf, dimensionSet(0, 0, -2, 0, 0))
    );
}


template<class Type>
tmp<fvMatrix<Type>>
laplacian
(
    const one&,
    const GeometricField<Type, fvPatchField, volMesh>& vf,
    const word& name
)
{
    return fvm::laplacian(vf, name);
}


template<class Type>
tmp<fvMatrix<Type>>
laplacian
(
    const one&,
    const GeometricField<Type, fvPatchField, volMesh>& vf
)
{
    return fvm::laplacian(vf);
}


template<class Type, class GType>
tmp<fvMatrix<Type>>
laplacian
(
    const dimensioned<GType>& gamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf,
    const word& name
)
{
    const GeometricField<GType, fvsPatchField, surfaceMesh> Gamma
    (
        IOobject
        (
            gamma.name(),
            vf.instance(),
            vf.mesh(),
            IOobject::NO_READ
        ),
        vf.mesh(),
        gamma
    );

    return fvm::laplacian(Gamma, vf, name);
}


template<class Type, class GType>
tmp<fvMatrix<Type>>
laplacian
(
    const dimensioned<GType>& gamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf
)
{
    const GeometricField<GType, fvsPatchField, surfaceMesh> Gamma
    (
        IOobject
        (
            gamma.name(),
            vf.instance(),
            vf.mesh(),
            IOobject::NO_READ
        ),
        vf.mesh(),
        gamma
    );

    return fvm::laplacian(Gamma, vf);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type, class GType>
tmp<fvMatrix<Type>>
laplacian
(
    const GeometricField<GType, fvPatchField, volMesh>& gamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf,
    const word& name
)
{
    return fv::laplacianScheme<Type, GType>::New
    (
        vf.mesh(),
        vf.mesh().schemes().laplacian(name)
    ).ref().fvmLaplacian(gamma, vf);
}


template<class Type, class GType>
tmp<fvMatrix<Type>>
laplacian
(
    const tmp<GeometricField<GType, fvPatchField, volMesh>>& tgamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf,
    const word& name
)
{
    tmp<fvMatrix<Type>> tLaplacian(fvm::laplacian(tgamma(), vf, name));
    tgamma.clear();
    return tLaplacian;
}


template<class Type, class GType>
tmp<fvMatrix<Type>>
laplacian
(
    const GeometricField<GType, fvPatchField, volMesh>& gamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf
)
{
    return fvm::laplacian
    (
        gamma,
        vf,
        "laplacian(" + gamma.name() + ',' + vf.name() + ')'
    );
}


template<class Type, class GType>
tmp<fvMatrix<Type>>
laplacian
(
    const tmp<GeometricField<GType, fvPatchField, volMesh>>& tgamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf
)
{
    tmp<fvMatrix<Type>> tLaplacian(fvm::laplacian(tgamma(), vf));
    tgamma.clear();
    return tLaplacian;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type, class GType>
tmp<fvMatrix<Type>>
laplacian
(
    const GeometricField<GType, fvsPatchField, surfaceMesh>& gamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf,
    const word& name
)
{
    return fv::laplacianScheme<Type, GType>::New
    (
        vf.mesh(),
        vf.mesh().schemes().laplacian(name)
    ).ref().fvmLaplacian(gamma, vf);
}


template<class Type, class GType>
tmp<fvMatrix<Type>>
laplacian
(
    const tmp<GeometricField<GType, fvsPatchField, surfaceMesh>>& tgamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf,
    const word& name
)
{
    tmp<fvMatrix<Type>> tLaplacian = fvm::laplacian(tgamma(), vf, name);
    tgamma.clear();
    return tLaplacian;
}


template<class Type, class GType>
tmp<fvMatrix<Type>>
laplacian
(
    const GeometricField<GType, fvsPatchField, surfaceMesh>& gamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf
)
{
    return fvm::laplacian
    (
        gamma,
        vf,
        "laplacian(" + gamma.name() + ',' + vf.name() + ')'
    );
}


template<class Type, class GType>
tmp<fvMatrix<Type>>
laplacian
(
    const tmp<GeometricField<GType, fvsPatchField, surfaceMesh>>& tGamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf
)
{
    tmp<fvMatrix<Type>> tLaplacian(fvm::laplacian(tGamma(), vf));
    tGamma.clear();
    return tLaplacian;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
tmp<fvMatrix<Type>>
laplacianCorrection
(
    const GeometricField<scalar, fvPatchField, volMesh>& gamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf
)
{
    return fvm::laplacianCorrection(fvc::interpolate(gamma), vf);
}


template<class Type>
tmp<fvMatrix<Type>>
laplacianCorrection
(
    const tmp<GeometricField<scalar, fvPatchField, volMesh>>& tgamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf
)
{
    tmp<fvMatrix<Type>> tLaplacianCorrection
    (
        fvm::laplacianCorrection(tgamma(), vf)
    );
    tgamma.clear();
    return tLaplacianCorrection;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
tmp<fvMatrix<Type>>
laplacianCorrection
(
    const GeometricField<scalar, fvsPatchField, surfaceMesh>& gamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf
)
{
    return correction
    (
        fv::gaussLaplacianScheme<Type, scalar>::fvmLaplacianUncorrected
        (
            gamma*vf.mesh().magSf(),
            vf.mesh().nonOrthDeltaCoeffs(),
            vf
        )
    );
}


template<class Type>
tmp<fvMatrix<Type>>
laplacianCorrection
(
    const tmp<GeometricField<scalar, fvsPatchField, surfaceMesh>>& tGamma,
    const GeometricField<Type, fvPatchField, volMesh>& vf
)
{
    tmp<fvMatrix<Type>> tLaplacianCorrection
    (
        fvm::laplacianCorrection(tGamma(), vf)
    );
    tGamma.clear();
    return tLaplacianCorrection;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fvm

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
