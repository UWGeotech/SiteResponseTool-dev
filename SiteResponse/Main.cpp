#include <iostream>
#include "FEModel.h"
#include "siteLayering.h"
#include "soillayer.h"

#include "StandardStream.h"
#include "OPS_Stream.h"


StandardStream sserr;
OPS_Stream *opserrPtr = &sserr;

SiteLayering setupLayers()
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

	if (argc < 2)
	{
		opserr << "Need at least 2 arguments. The .loc (layering) file and the name of the motion." << endln;
		std::getchar();
		return -1;
	}

	std::string layersFN(argv[1]);
	std::string motionFN(argv[2]);

	opserr << layersFN.c_str() << " , " << motionFN.c_str() << endln;

	SiteLayering siteLayers(layersFN.c_str());

	SiteResponseModel model(siteLayers);
	
	return 0;
}
