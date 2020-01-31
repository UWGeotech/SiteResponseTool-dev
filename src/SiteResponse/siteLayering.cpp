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


#include "siteLayering.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

SiteLayering::SiteLayering()
{
	sl_natPeriod = 0.0;
    sl_numLayers = 0;
	sl_natPeriod = 0.0;
	sl_totalThickness = 0.0;
	sl_equivalentVS = 0.0;
	sl_equivalentVP = 0.0;
}

SiteLayering::SiteLayering(const char* layerFileName)
{
	sl_natPeriod = 0.0;
    sl_numLayers = 0;
	sl_natPeriod = 0.0;
	sl_totalThickness = 0.0;
	sl_equivalentVS = 0.0;
	sl_equivalentVP = 0.0;
	this->readFromFile(layerFileName);
}

SiteLayering::~SiteLayering()
{

}

SoilLayer
SiteLayering::getLayer(int index)
{
		return sl_layers[index];
}


void 
SiteLayering::addNewLayer(SoilLayer newLayer)
{
	if (sl_totalThickness == 0.0)
	{
		sl_equivalentVS = newLayer.getShearVelocity();
		sl_equivalentVP = newLayer.getCompVelocity();
		sl_totalThickness += newLayer.getThickness();
		sl_numLayers++;
		sl_layers.push_back(newLayer);
		return;
	}
	sl_equivalentVS = (sl_totalThickness + newLayer.getThickness()) / (sl_totalThickness / sl_equivalentVS + newLayer.getThickness() / newLayer.getShearVelocity());
	sl_equivalentVP = (sl_totalThickness + newLayer.getThickness()) / (sl_totalThickness / sl_equivalentVP + newLayer.getThickness() / newLayer.getCompVelocity());
	sl_totalThickness += newLayer.getThickness();
	sl_numLayers++;
	sl_layers.push_back(newLayer);
}

double 
SiteLayering::getNaturalPeriod()
{
	if (sl_equivalentVS != 0.0)
		return 4.0 * sl_totalThickness / sl_equivalentVS;
	else
		return -1.0;
}

int 
SiteLayering::readFromFile(const char* layerFileName)
{
	std::string line;
	std::ifstream layerFile(layerFileName);
	while (getline(layerFile, line))
	{
		// skip comment lines
		if ((line == "") || (line[0] == '%') || (line[0] == '#'))
			continue;	
		std::string layerName, IOStr;
		double thick, vs, vp, rho, su, hG, m, h0, chi;
		bool IO;
		std::istringstream lines(line);
		lines >> layerName >> thick >> vs >> vp >> rho >> su >> hG >> m >> h0 >> chi >> IOStr;
		//std::cout << "Layer : " << layerName << ", " << thick << ", " << vs << ", " << vp << ", " << rho << ", " << su << ", " << hG << ", " << m << ", " << h0 << ", " << chi << ", " << IOStr <<std::endl;
		if (IOStr == "true" || IOStr == "True" || IOStr == "TRUE")
			IO = true;
		else
		    IO = false;
		
		SoilLayer newLayer(layerName, thick, vs, vp, rho, su, hG, m, h0, chi, IO);
		this->addNewLayer(newLayer);
	}
	layerFile.close();
	return 0;
}
