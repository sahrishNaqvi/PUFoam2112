{
    word alphaScheme("div(phi,alpha)");
    word alpharScheme("div(phirb,alpha)");

    surfaceScalarField phir(phic*interface.nHatf());

    for (int gCorr=0; gCorr<nAlphaCorr; gCorr++)
    {
        volScalarField Sp
        (
            IOobject
            (
                "Sp",
                runTime.timeName(),
                mesh
            ),
            mesh,
            dimensionedScalar("Sp", dgdt.dimensions(), 0.0)
        );

        volScalarField Su
        (
            IOobject
            (
                "Su",
                runTime.timeName(),
                mesh
            ),
            // Divergence term is handled explicitly to be
            // consistent with the explicit transport solution
            divU*min(alpha1, scalar(1)) 
        );

        forAll(dgdt, celli)
        {
            if (dgdt[celli] > 0.0 && alpha1[celli] > 0.0)
            {
                Sp[celli] -= dgdt[celli]*alpha1[celli];
                Su[celli] += dgdt[celli]*alpha1[celli];
            }
            else if (dgdt[celli] < 0.0 && alpha1[celli] < 1.0)
            {
                Sp[celli] += dgdt[celli]*(1.0 - alpha1[celli]);
            }
        }


        surfaceScalarField phiAlpha1
        (
            fvc::flux
            (
                phi,
                alpha1,
                alphaScheme
            )
          + fvc::flux
            (
                -fvc::flux(-phir, alpha2, alpharScheme),
                alpha1,
                alpharScheme
            )
        );

        MULES::explicitSolve
        (
            geometricOneField(),
            alpha1,
            phiAlpha1,
            Sp,
            Su
        );

        surfaceScalarField rho1f(fvc::interpolate(rho_gas));
        surfaceScalarField rho2f(fvc::interpolate(rho_foam));
        rhoPhi = phiAlpha1*(rho1f - rho2f) + phi*rho2f;
//Info<<"\nRhoPhi AlphaEqn \n:"<<rhoPhi<<endl;
        alpha2 = scalar(1) - alpha1;
    }

    Info<< "Liquid phase volume fraction = "
        << alpha1.weightedAverage(mesh.V()).value()
        << "  Min(alpha1) = " << min(alpha1).value()
        << "  Min(alpha2) = " << min(alpha2).value()
        << endl;
}
