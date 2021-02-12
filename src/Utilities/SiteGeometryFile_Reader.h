#ifndef SiteGeometryFILEREADER_H
#define SiteGeometryFILEREADER_H

#include "SiteGeometry.h"

#include <string>

class SiteGeometryFile_Reader
{
public:
	SiteGeometryFile_Reader();
	~SiteGeometryFile_Reader();

	virtual void set_input_file(std::string) = 0;
	virtual int populate_layering(SiteGeometry*) = 0;
};

#endif // !SiteGeometryFILEREADER_H