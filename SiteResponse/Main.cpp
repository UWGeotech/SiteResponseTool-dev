#include <iostream>
#include "FEModel.h"
#include "siteLayering.h"
#include "soillayer.h"
#include "outcropMotion.h"

#include "StandardStream.h"
#include "OPS_Stream.h"


StandardStream sserr;
OPS_Stream *opserrPtr = &sserr;

SiteLayering setupDummyLayers()
{
	SiteLayering res;
	res.addNewLayer(SoilLayer("Clay1", 2.0,  98.78, 184.79, 2.05, 200.0, 1.0, 1.0));
	res.addNewLayer(SoilLayer("Clay2", 6.0, 100.76, 188.50, 1.97, 200.0, 1.0, 1.0));
	res.addNewLayer(SoilLayer("Clay3", 5.0,  98.53, 184.34, 2.06, 200.0, 1.0, 1.0));
	res.addNewLayer(SoilLayer("Clay4", 2.4,  98.53, 184.34, 2.06, 200.0, 1.0, 1.0));
	res.addNewLayer(SoilLayer("Clay5", 2.6,  98.53, 184.34, 2.06, 200.0, 1.0, 1.0));
	res.addNewLayer(SoilLayer("Rock", 0.0, 1000.0, 3000.0, 2.5, 0.0, 0.0, 0.0));

	return res;
}

int main(int argc, char** argv)
{

	if (argc < 3)
	{
		opserr << "Need at least 2 arguments. The .loc (layering) file and the name of the motion in the x-direction." << endln;
		std::getchar();
		return -1;
	}

	// read the layering file
	std::string layersFN(argv[1]);
	SiteLayering siteLayers(layersFN.c_str());


	// read the motion
	OutcropMotion motionX;
	OutcropMotion motionZ;

	if (strcmp(argv[2], "-bbp") == 0)
	{
		// read bbp style motion

	}
	else {
		// read OpenSees style motion
		std::string motionXFN(argv[2]);
		motionX.setMotion(motionXFN.c_str());

		if (argc > 3)
		{
			std::string motionZFN(argv[3]);
			motionZ.setMotion(motionXFN.c_str());
		}
	}



	SiteResponseModel model(siteLayers, &motionX, &motionZ);
	model.runTotalStressModel();
	
	return 0;
}
