#include "GeometryLayer_Library.h"
// #include "JsonGeometryFile_Reader.h"

GeometryLayer_Library::GeometryLayer_Library()
{

}

GeometryLayer_Library::GeometryLayer_Library(std::string geometry_fn)
{
	//read_geometry_file(geometry_fn);
}

GeometryLayer_Library::GeometryLayer_Library(std::string geometry_fn, std::string filetype)
{
	//read_geometry_file(geometry_fn, filetype);
}

GeometryLayer_Library::~GeometryLayer_Library()
{

}

int
GeometryLayer_Library::add_new_layer(GeometryLayer newLayer)
{
	layer_library[newLayer.get_tag()] = newLayer;
	return 0;
}

//int
//GeometryLayer_Library::read_geometry_file(std::string geometry_fn, std::string filetype)
//{
//	std::string geometry_filetype;
//
//	if (filetype == "__na__")
//	{
//		// get the file extension
//		size_t pos = geometry_fn.find_last_of('.');
//		if (pos == std::string::npos)
//		{
//			// Error, geometry file type not defined.
//			return -1;
//		}
//		std::string file_ext = geometry_fn.substr(pos + 1);
//
//		if (file_ext == "json")
//			geometry_filetype = "json";
//		else if (file_ext == "txt")
//			geometry_filetype = "text";
//	}
//	else {
//		geometry_filetype = filetype;
//	}
//
//	if (geometry_filetype == "json")
//	{
//		if (!geometry_file)
//		{
//			geometry_file = new JsonGeometryFile_Reader();
//			geometry_file->set_input_file(geometry_fn);
//			geometry_file->populate_layers(layer_library, base_library);
//		}
//		else {
//			geometry_file->set_input_file(geometry_fn);
//			geometry_file->populate_layers(layer_library, base_library);
//		}
//	}
//	else {
//		// Error: File type either not detected or not appropriate
//		return -1;
//	}
//	return 0;
//}
