#ifndef GEOMETRYFILE_READER_H
#define GEOMETRYFILE_READER_H

#include <vector>
#include <map>
#include <string>

#include "GeometryLayer.h"
#include "GeometryBase.h"

class GeometryFile_Reader
{
public:
	GeometryFile_Reader();
	~GeometryFile_Reader();

	virtual void set_input_file(std::string) = 0;
	virtual int populate_layers(std::map<std::string, GeometryLayer>&, std::map<std::string, GeometryBase>&) = 0;

};

#endif // !GEOMETRYFILE_READER
