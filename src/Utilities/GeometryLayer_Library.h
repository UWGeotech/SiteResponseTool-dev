#ifndef GeometryLayer_Library_H 
#define GeometryLayer_Library_H 

#include <string> 

#include "GeometryFile_Reader.h"
#include "GeometryLayer.h"
#include "GeometryBase.h"

class GeometryLayer_Library
{
public:
	GeometryLayer_Library();
	GeometryLayer_Library(std::string geometry_fn);
	GeometryLayer_Library(std::string geometry_fn, std::string filetype);
	~GeometryLayer_Library();

	/*int read_geometry_file(std::string geometry_fn, std::string filetype = "__na__");*/
	int add_new_layer(GeometryLayer newLayer);
	GeometryLayer get_layer(std::string tag) { return layer_library[tag]; };
	GeometryBase get_base(std::string tag) { return base_library[tag]; };

private:
	GeometryFile_Reader* geometry_file;
	std::map<std::string, GeometryLayer> layer_library;
	std::map<std::string, GeometryBase> base_library;
};

#endif // !GeometryLayer_Library_H