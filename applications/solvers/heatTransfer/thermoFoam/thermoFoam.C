/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2013-2022 OpenFOAM Foundation
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

Application
    thermoFoam

Description
    Solver for energy transport and thermodynamics on a frozen flow field.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "fluidThermo.H"
#include "compressibleMomentumTransportModels.H"
#include "fluidThermoThermophysicalTransportModel.H"
#include "LESModel.H"
#include "fvModels.H"
#include "fvConstraints.H"
#include "simpleControl.H"
#include "pimpleControl.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #define NO_CONTROL
    #include "postProcess.H"

    #include "setRootCaseLists.H"
    #include "createTime.H"
    #include "createMesh.H"
    #include "createFields.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nEvolving thermodynamics\n" << endl;

    if (mesh.solution().dict().found("SIMPLE"))
    {
        simpleControl simple(mesh);

        while (simple.loop(runTime))
        {
            Info<< "Time = " << runTime.userTimeName() << nl << endl;

            while (simple.correctNonOrthogonal())
            {
                #include "EEqn.H"
            }

            Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
                << "  ClockTime = " << runTime.elapsedClockTime() << " s"
                << nl << endl;

            runTime.write();
        }
    }
    else
    {
        pimpleControl pimple(mesh);

        while (runTime.run())
        {
            runTime++;

            Info<< "Time = " << runTime.userTimeName() << nl << endl;

            fvModels.correct();

            while (pimple.correctNonOrthogonal())
            {
                #include "EEqn.H"
            }

            Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
                << "  ClockTime = " << runTime.elapsedClockTime() << " s"
                << nl << endl;

            runTime.write();
        }
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
