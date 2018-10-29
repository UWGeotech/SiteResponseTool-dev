#include "PathSeries.h"
#include "PathTimeSeries.h"
#include "GroundMotion.h"

#ifndef OUTCROPMOTION_H
#define OUTCROPMOTION_H

class OutcropMotion
{
public:
	OutcropMotion();
	OutcropMotion(const char* fName);
	~OutcropMotion();

	PathTimeSeries*  getDispSeries() { return theDispSeries; };
	PathTimeSeries*  getVelSeries() { return theVelSeries; };
	PathTimeSeries*  getAccSeries() { return theAccSeries; };
	GroundMotion*    getGroundMotion() { return theGroundMotion; };
	bool             isInitialized() { return isThisInitialized; };

private:
	PathTimeSeries* theAccSeries;
	PathTimeSeries* theVelSeries;
	PathTimeSeries* theDispSeries;
	GroundMotion*  theGroundMotion;

	bool isThisInitialized;
};


#endif