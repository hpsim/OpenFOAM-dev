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
    Foam::regionModels::SurfaceFilmModel

Description
    Templated wall surface film model class.

SourceFiles
    SurfaceFilmModel.C
    SurfaceFilmModelNew.C

\*---------------------------------------------------------------------------*/

#ifndef SurfaceFilmModel_H
#define SurfaceFilmModel_H

#include "IOdictionary.H"
#include "autoPtr.H"
#include "runTimeSelectionTables.H"
#include "CloudSubModelBase.H"
#include "surfaceMesh.H"
#include "dimensionedTypes.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes
namespace regionModels
{
    class surfaceFilm;
}

/*---------------------------------------------------------------------------*\
                      Class SurfaceFilmModel Declaration
\*---------------------------------------------------------------------------*/

template<class CloudType>
class SurfaceFilmModel
:
    public CloudSubModelBase<CloudType>
{
protected:

    // Protected data

        //- Convenience typedef to the cloud's parcel type
        typedef typename CloudType::parcelType parcelType;

        //- Gravitational acceleration constant
        const dimensionedVector& g_;

        //- Ejected parcel type label - id assigned to identify parcel for
        //  post-processing. If not specified, defaults to originating cloud
        //  type
        label ejectedParcelType_;


        // Cached injector fields per film patch

            //- Parcel mass / patch face
            scalarField massParcelPatch_;

            //- Parcel diameter / patch face
            scalarField diameterParcelPatch_;

            //- Film velocity / patch face
            vectorField UFilmPatch_;

            //- Film density / patch face
            scalarField rhoFilmPatch_;

            //- Film height of all film patches / patch face
            List<scalarField> deltaFilmPatch_;


        // Counters

            //- Number of parcels transferred to the film model
            label nParcelsTransferred_;

            //- Number of parcels injected from the film model
            label nParcelsInjected_;


    // Protected functions

        //- Return pointers to the films
        virtual UPtrList<regionModels::surfaceFilm>&
            surfaceFilmPtrs() const = 0;

        //- Cache the film fields in preparation for injection
        virtual void cacheFilmFields
        (
            const label filmPatchi,
            const label primaryPatchi,
            const regionModels::surfaceFilm&
        );

        //- Set the individual parcel properties
        virtual void setParcelProperties
        (
            parcelType& p,
            const label filmFacei
        ) const;


public:

    //- Runtime type information
    TypeName("surfaceFilm");

    //- Declare runtime constructor selection table
    declareRunTimeSelectionTable
    (
        autoPtr,
        SurfaceFilmModel,
        dictionary,
        (
            const dictionary& dict,
            CloudType& owner
        ),
        (dict, owner)
    );


    // Constructors

        //- Construct null from owner
        SurfaceFilmModel(CloudType& owner);

        //- Construct from components
        SurfaceFilmModel
        (
            const dictionary& dict,
            CloudType& owner,
            const word& type
        );

        //- Construct copy
        SurfaceFilmModel(const SurfaceFilmModel<CloudType>& sfm);

        //- Construct and return a clone
        virtual autoPtr<SurfaceFilmModel<CloudType>> clone() const = 0;


    //- Destructor
    virtual ~SurfaceFilmModel();


    //- Selector
    static autoPtr<SurfaceFilmModel<CloudType>> New
    (
        const dictionary& dict,
        CloudType& owner
    );


    // Member Functions

        // Access

            //- Return gravitational acceleration constant
            inline const dimensionedVector& g() const;

            //- Return const access to the number of parcels transferred to the
            //  film model
            inline label nParcelsTransferred() const;

            //- Return non-const access to the number of parcels transferred to
            //  the film model
            inline label& nParcelsTransferred();

            //- Return const access to the number of parcels injected from the
            //  film model
            inline label nParcelsInjected() const;

            //- Return non-const access to the number of parcels injected from
            //  the film model
            inline label& nParcelsInjected();


        // Member Functions

            //- Transfer parcel from cloud to surface film
            //  Returns true if parcel is to be transferred
            virtual bool transferParcel
            (
                parcelType& p,
                const polyPatch& pp,
                bool& keepParticle
            ) = 0;

            //- Inject parcels into the cloud
            template<class TrackCloudType>
            void inject(TrackCloudType& cloud);


        // I-O

            //- Write surface film info to stream
            virtual void info(Ostream& os);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define makeSurfaceFilmModel(CloudType)                                        \
                                                                               \
    typedef Foam::CloudType::momentumCloudType momentumCloudType;              \
    defineNamedTemplateTypeNameAndDebug                                        \
    (                                                                          \
        Foam::SurfaceFilmModel<momentumCloudType>,                             \
        0                                                                      \
    );                                                                         \
    namespace Foam                                                             \
    {                                                                          \
        defineTemplateRunTimeSelectionTable                                    \
        (                                                                      \
            SurfaceFilmModel<momentumCloudType>,                               \
            dictionary                                                         \
        );                                                                     \
    }


#define makeSurfaceFilmModelType(SS, CloudType)                                \
                                                                               \
    typedef Foam::CloudType::momentumCloudType momentumCloudType;              \
    defineNamedTemplateTypeNameAndDebug(Foam::SS<momentumCloudType>, 0);       \
                                                                               \
    Foam::SurfaceFilmModel<momentumCloudType>::                                \
        adddictionaryConstructorToTable<Foam::SS<momentumCloudType>>           \
            add##SS##CloudType##momentumCloudType##ConstructorToTable_;


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "SurfaceFilmModelI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "SurfaceFilmModel.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
