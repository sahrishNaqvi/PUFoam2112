scalar foamCellCounter = 0.0;
scalar gelCellCounter = 0.0;

forAll(mesh.C(), celli)
	{
//Pout << "celli:"<<  celli << endl;

		if (alpha2[celli] > 0.5)
		{
			foamCellCounter +=1;
		}
		if (XOH[celli] >= XOH_Gel)
		{
 			gelCellCounter +=1;
                } 
	}

Pout << "gell cell counter on this processor: " << gelCellCounter << endl;
reduce(gelCellCounter, sumOp<scalar>());
Info << "Total gell cell counter on all processors: "<< gelCellCounter << endl;

Pout << "foam cell counter on this processor: " << foamCellCounter << endl;
reduce(foamCellCounter, sumOp<scalar>());
Info << "Total foam cell counter on all processors: "<< foamCellCounter<< endl;

scalar ratio = gelCellCounter/foamCellCounter;
Pout << "Ratio on this processor: " << gelCellCounter/foamCellCounter << endl;
reduce(ratio, sumOp<scalar>());
Info << "Total ratio on all processors: "<< gelCellCounter/foamCellCounter<< endl;

if ( ratio > 0.5)
{
	gellingPoint = true;
}
else
{
    gellingPoint = false;
}


