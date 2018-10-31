#include "outcropMotion.h"
#include <string>
#include <fstream>
#include <sstream>

bool fileExists(const char* fileName)
{
	std::ifstream file(fileName);
	if (file)
		return true;

	return false;
}

int readDT(const char* fileName, int& numSteps, std::vector<double>& dt)
{
	int res = -1;
	numSteps = 0;
	std::ifstream file(fileName);
	if (file)
	{
		std::string line;
		res = 1;
		double t_n, t_n1;
		while (getline(file, line))
		{
			// skip comment lines
			if ((line == "") || (line[0] == '%') || (line[0] == '#'))
				continue;
			std::istringstream lines(line);
			lines >> t_n;
			break;
		}

		while (getline(file, line))
		{
			// skip comment lines
			if ((line == "") || (line[0] == '%') || (line[0] == '#'))
				continue;
			std::istringstream lines(line);
			lines >> t_n1;
			dt.push_back(t_n1 - t_n);
			t_n = t_n1;
			++numSteps;
		}
	}
	return res;
}

OutcropMotion::OutcropMotion() :
	theGroundMotion(NULL),
	theAccSeries(),
	theVelSeries(),
	theDispSeries(),
	isThisInitialized(false),
	m_numSteps(0)
{

}

OutcropMotion::OutcropMotion(const char* fName):
	theGroundMotion(NULL),
	theAccSeries(),
	theVelSeries(),
	theDispSeries(),
	isThisInitialized(true),
	m_numSteps(0)
{
	std::string motionName(fName);
	std::string timeFName = motionName  + ".time";
	std::string accFName  = motionName  + ".acc";
	std::string velFName  = motionName  + ".vel";
	std::string dispFName = motionName  + ".disp";
	
	if (readDT(timeFName.c_str(), m_numSteps, m_dt) > 0)
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