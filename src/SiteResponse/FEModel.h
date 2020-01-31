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

#ifndef FEMODEL_H
#define FEMODEL_H

#include "Domain.h"
#include "Matrix.h"

#include "siteLayering.h"
#include "soillayer.h"
#include "outcropMotion.h"

#define MAX_FREQUENCY 50.0
#define NODES_PER_WAVELENGTH 10

class SiteResponseModel {

public:
	SiteResponseModel();
	SiteResponseModel(SiteLayering, OutcropMotion*, OutcropMotion*, OutcropMotion*);
	~SiteResponseModel();

	int   runTestModel();
	int   runTotalStressModel3DLotung();
	int   runTotalStressModel3D();
	int   runEffectiveStressModel2D();
	void  setOutputDir(std::string outDir) { theOutputDir = outDir; };

private:
	Domain *theDomain;
	SiteLayering    SRM_layering;
	OutcropMotion*  theMotionX;
	OutcropMotion*  theMotionY;
	OutcropMotion*  theMotionZ;
	std::string     theOutputDir;
};


#endif