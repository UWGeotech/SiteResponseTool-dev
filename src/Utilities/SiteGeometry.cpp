#include "SiteGeometry.h"
#include "JsonSiteGeometryFile_Reader.h"

SiteGeometry::SiteGeometry() :
	sl_natPeriod(0.0),
	sl_numLayers(0),
	sl_totalThickness(0.0),
	sl_equivalentVS(0.0),
	sl_equivalentVP(0.0)
{

}

SiteGeometry::SiteGeometry(std::string geometry_fn) :
	sl_natPeriod(0.0),
	sl_numLayers(0),
	sl_totalThickness(0.0),
	sl_equivalentVS(0.0),
	sl_equivalentVP(0.0)
{
	read_geometry_file(geometry_fn);
}

SiteGeometry::SiteGeometry(std::string geometry_fn, std::string filetype) :
	sl_natPeriod(0.0),
	sl_numLayers(0),
	sl_totalThickness(0.0),
	sl_equivalentVS(0.0),
	sl_equivalentVP(0.0)
{
	read_geometry_file(geometry_fn, filetype);
}

SiteGeometry::~SiteGeometry()
{

}

GeometryLayer
SiteGeometry::getLayer(int index)
{
	return sl_layers[index];
}

GeometryBase 
SiteGeometry::getBase()
{
	return sl_base;
}

void
SiteGeometry::addNewLayer(GeometryLayer newLayer)
{
	sl_layers.push_back(newLayer);
	sl_numLayers += 1;
}

void
SiteGeometry::addNewLayerToLibrary(GeometryLayer newLayer)
{
	sl_layer_library.add_new_layer(newLayer);
}

double
SiteGeometry::getNaturalPeriod()
{
	return 0.0;
}

int
SiteGeometry::read_geometry_file(std::string geometry_fn, std::string filetype)
{
	std::string geometry_filetype;

	if (filetype == "__na__")
	{
		// get the file extension
		size_t pos = geometry_fn.find_last_of('.');
		if (pos == std::string::npos)
		{
			// Error, geometry file type not defined.
			return -1;
		}
		std::string file_ext = geometry_fn.substr(pos + 1);

		if (file_ext == "json")
			geometry_filetype = "json";
		else if (file_ext == "txt")
			geometry_filetype = "text";
	}
	else {
		geometry_filetype = filetype;
	}

	if (geometry_filetype == "json")
	{
		if (!geometry_file)
		{
			geometry_file = new JsonSiteGeometryFile_Reader();
			geometry_file->set_input_file(geometry_fn);
			geometry_file->populate_layering(this);
		}
		else {
			geometry_file->set_input_file(geometry_fn);
			geometry_file->populate_layering(this);
		}
	}
	else {
		// Error: File type either not detected or not appropriate
		return -1;
	}
	return 0;
}