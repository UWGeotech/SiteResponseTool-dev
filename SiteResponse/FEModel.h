#ifndef FEMODEL_H
#define FEMODEL_H

#include "Domain.h"
#include "Matrix.h"

#include "siteLayering.h"
#include "soillayer.h"

#define MAX_FREQUENCY 50.0
#define NODES_PER_WAVELENGTH 10

class SiteResponseModel {

public:
	SiteResponseModel();
	SiteResponseModel(SiteLayering);
	~SiteResponseModel();

private:
	Domain *theDomain;
	SiteLayering SRM_layering;

	int generateTestModel();
	int generateTotalStressModel();
};


#endif