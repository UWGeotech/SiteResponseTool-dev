#include "outcropMotion.h"
#include <string>
#include <fstream>

bool fileExists(const char* fileName)
{
	std::ifstream file(fileName);
	if (file)
		return true;
	else
		return false;
}

OutcropMotion::OutcropMotion() :
	theGroundMotion(NULL),
	theAccSeries(),
	theVelSeries(),
	theDispSeries(),
	isThisInitialized(false)
{

}

OutcropMotion::OutcropMotion(const char* fName):
	theGroundMotion(NULL),
	theAccSeries(),
	theVelSeries(),
	theDispSeries(),
	isThisInitialized(true)
{
	std::string motionName(fName);
	std::string timeFName = motionName  + ".time";
	std::string accFName  = motionName  + ".acc";
	std::string velFName  = motionName  + ".vel";
	std::string dispFName = motionName  + ".disp";
	
	if (fileExists(timeFName.c_str()))
	{
		if (fileExists(accFName.c_str()))
			theAccSeries = new PathTimeSeries(1, accFName.c_str(), timeFName.c_str(), 9.81, true);
		if (fileExists(velFName.c_str()))
			theVelSeries = new PathTimeSeries(2, velFName.c_str(), timeFName.c_str(), 1.0, true);
		if (fileExists(dispFName.c_str()))
			theDispSeries = new PathTimeSeries(3, dispFName.c_str(), timeFName.c_str(), 1.0, true);
		if ((theAccSeries != NULL) || (theVelSeries != NULL) || (theDispSeries != NULL) )
			theGroundMotion = new GroundMotion(theDispSeries, theVelSeries, theAccSeries, NULL);
		else
		{
			isThisInitialized = false;
			opserr << "None of the files " << accFName.c_str() << " or " << velFName.c_str() << " or " << dispFName.c_str() << " exist." << endln;
		}
	}
	else {
		isThisInitialized = false;
		opserr << "The file " << timeFName.c_str() << " containing the array of time does not exist." << endln;
	}
}

OutcropMotion::~OutcropMotion() 
{

}