/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2013 OpenFOAM Foundation
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
    PUFoam

Description
    Solver using "compressibleInterFoam" as the basis to simulate the expansion
    of Polyurethane foam (PU). It includes Quadrature Method of Moments (QMOM)
    to solve a 'Population Balance Equation' (PBE) determining the bubble size
    distribution inside PU foams.

    Kinetics of the reactions including gelling, blowing and evaporation of
    the physical blowing agent are incorporated into the solver.

    Finally, the kinetics and PBE has been coupled to describe the time
    evolution of foaming process.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "MULES.H"
#include "CMULES.H"
#include "subCycle.H"
#include "rhoThermo.H"
#include "interfaceProperties.H"
#include "twoPhaseMixture.H"
#include "twoPhaseMixtureThermo.H"
#include "turbulentFluidThermoModel.H"
#include "turbulenceModel.H"
#include "pimpleControl.H"
#include "fixedFluxPressureFvPatchScalarField.H"

extern "C"{void dsteqr_(char &, int *, double *, double *, double *, int *, double *, int *); }

//#include "Kinetics.H"
// #include "blowingAgents.H"
// #include "blowingReaction.H"
// #include "gellingReaction.H"
// #include "foamDensity.H"
// #include "Moments.H"
#include "PUgeneric.H"
#include "blowingReaction.H"
#include "blowingAgents.H"
#include "gellingReaction.H"
#include "foamDensity.H"
#include "rheologyPU.H"
#include "Moments.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"
    #include "readGravitationalAcceleration.H"

    pimpleControl pimple(mesh);

    #include "createTimeControls.H"
    #include "readTimeControls.H"
    #include "initContinuityErrs.H"
    #include "createFields.H"
    #include "CourantNo.H"
    #include "setInitialDeltaT.H"
    bool gellingPoint = false;


    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    Info<< "\nStarting time loop\n" << endl;
    Info<< "initialFoamMass : " << initialFoamMass << endl;
    while (runTime.run())
    {
        #include "readTimeControls.H"
        #include "CourantNo.H"
        #include "setDeltaT.H"

        runTime++;

        Info<< "Time = " << runTime.timeName() << nl << endl;


        // --- Pressure-velocity PIMPLE corrector loop
        while (pimple.loop())
        {
            if (simulationTarget == "mold-filling")
            {
                if
                (
                    alpha1.weightedAverage(mesh.V()).value() > 0.01
                    && gellingPoint == false
                )
                {
                    #include "alphaEqnsSubCycle.H"
                    // correct interface on first PIMPLE corrector
                    if (pimple.corr() == 1)
                    {
                        interface.correct();
                    }

                   #include "checkGellingPoint.H"
                    solve(fvm::ddt(rho) + fvc::div(rhoPhi));

                   #include "conversionSources.H"
                   #include "conversionEqns.H"
                   #include "conversionCheck.H"

                   #include "rheologyModel.H"

                   #include "UEqn.H"

                   #include "TSource.H"
                   #include "TEqn.H"
                   #include "TCheck.H"
                   #include "densityEqns.H"

                   #include "MomSources.H"
                   #include "MomEqns.H"
                   #include "MomConvert.H"
                    // --- Pressure corrector loop
                    while (pimple.correct())
                    {
                        #include "pEqn.H"
                    }

                   #include "BASources.H"
                   #include "BAEqns.H"
                   #include "BACheck.H"

                    if (pimple.turbCorr())
                    {
                        turbulence->correct();
                    }
                }
            }
            else
            {
                if (gellingPoint == false)
                {
                    #include "alphaEqnsSubCycle.H"

                    // correct interface on first PIMPLE corrector
                   if (pimple.corr() == 1)
                    {
                        interface.correct();
                    }

                   #include "checkGellingPoint.H"
                   solve(fvm::ddt(rho) + fvc::div(rhoPhi));
                   #include "conversionSources.H"
                   #include "conversionEqns.H"
                   #include "conversionCheck.H"

                   #include "rheologyModel.H"

                   #include "UEqn.H"

                   #include "TSource.H"
                   #include "TEqn.H"
                   #include "TCheck.H"
                   #include "densityEqns.H"

                   #include "MomSources.H"
                   #include "MomEqns.H"
                   #include "MomConvert.H"

                    // --- Pressure corrector loop
                    while (pimple.correct())
                    {
                        #include "pEqn.H"
                    }
                   #include "BASources.H"
                   #include "BAEqns.H"
                   #include "BACheck.H"

                    if (pimple.turbCorr())
                    {
                        turbulence->correct();
                    }
                }
            }

        #include "continuityErrors.H"

        Info<< "\nMass of foam: " << fvc::domainIntegrate(alpha2*rho_foam) << endl;
        }

        runTime.write();

        Info<< "ExecutionTime = "
            << runTime.elapsedCpuTime()
            << " s\n\n" << endl;
    }
    Info<< "finalFoamMass : " << fvc::domainIntegrate(rho_foam*alpha2) << endl;
    Info<< "End\n" << endl;

    return 0;
}
// ************************************************************************* //
