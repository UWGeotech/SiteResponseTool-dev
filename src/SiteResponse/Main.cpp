/* ********************************************************************* **
**                 Site Response Analysis Tool                           **
**   -----------------------------------------------------------------   **
**                                                                       **
**   Developed by: UW Computational Geomechanics Group                   **
**                 Alborz Ghofrani (alborzgh@uw.edu)                     **
**                 Pedro Arduino (parduino@uw.edu)                       **
**                 University of Washington                              **
**                                                                       **
**   Date: October 2018                                                  **
**                                                                       **
** ********************************************************************* */

#include <iostream>
#include "FEModel.h"
#include "siteLayering.h"
#include "soillayer.h"
#include "outcropMotion.h"

#include "StandardStream.h"
#include "FileStream.h"
#include "OPS_Stream.h"
#include "ConfigManager.h"
#include "SRT_Globals.h"

StandardStream sserr;
FileStream ferr("log");
OPS_Stream *opserrPtr = &ferr;
OPS_Stream *opsoutPtr = &sserr;

ConfigManager* program_config;

SiteLayering setupDummyLayers()
{
	SiteLayering res;
	res.addNewLayer(SoilLayer("Clay1", 2.0,  98.78, 184.79, 2.05, 200.0, 1.0, 1.0, 0.0, 0.05, false));
	res.addNewLayer(SoilLayer("Clay2", 6.0, 100.76, 188.50, 1.97, 200.0, 1.0, 1.0, 0.0, 0.05, false));
	res.addNewLayer(SoilLayer("Clay3", 5.0,  98.53, 184.34, 2.06, 200.0, 1.0, 1.0, 0.0, 0.05, false));
	res.addNewLayer(SoilLayer("Clay4", 2.4,  98.53, 184.34, 2.06, 200.0, 1.0, 1.0, 0.0, 0.05, false));
	res.addNewLayer(SoilLayer("Clay5", 2.6,  98.53, 184.34, 2.06, 200.0, 1.0, 1.0, 0.0, 0.05, false));
	res.addNewLayer(SoilLayer("Rock", 0.0, 1000.0, 3000.0, 2.5, 0.0, 0.0, 0.0, 0.0, 0.05, false));

	return res;
}

int main(int argc, char** argv)
{

	if (argc < 3)
	{
		opserr << ">>> SiteResponseTool: Not enough arguments. <<<" << endln;
		std::getchar();
		exit(-1);
	}

	// read the layering file
	std::string layersFN(argv[1]);
	std::string bbpOName(".");
	SiteLayering siteLayers(layersFN.c_str());


	// read the motion, assumes Y is the vertical motion
	OutcropMotion motionX;
	OutcropMotion motionY;
	OutcropMotion motionZ;

	if (strcmp(argv[2], "-bbp") == 0)
	{
		std::string bbpFName(argv[3]);
		bbpOName = std::string(argv[4]);
		std::string bbpLName = std::string(argv[5]);
		ferr.setFile(bbpLName.c_str(), APPEND);
		// read bbp style motion
		motionX.setBBPMotion(bbpFName.c_str(), 1);
		motionZ.setBBPMotion(bbpFName.c_str(), 2);
	}
	else {
		// read OpenSees style motion
		std::string motionXFN(argv[2]);
		motionX.setMotion(motionXFN.c_str());

		if (argc > 3)
		{
			std::string motionZFN(argv[3]);
			motionZ.setMotion(motionZFN.c_str());
			if (argc > 4)
			{
				std::string motionYFN(argv[4]);
				motionY.setMotion(motionYFN.c_str());
			}
		}
	}

	program_config = ConfigManager::get_Instance();

	SiteResponseModel model(siteLayers, &motionX, &motionY, &motionZ);
	model.setOutputDir(bbpOName);
	//model.runEffectiveStressModel2D();
	model.runTotalStressModel3D();
	//model.runTotalStressModel3DLotung();

	return 0;
}
