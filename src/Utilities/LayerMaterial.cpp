#include "LayerMaterial.h"

LayerMaterial::LayerMaterial()
{
}

LayerMaterial::LayerMaterial(std::string mat_tag, std::string mat_type, std::map <std::string, std::string> mat_props)
{
	_material_tag = mat_tag;
	_material_type = mat_type;
	_properties = mat_props;
}

LayerMaterial::~LayerMaterial()
{
}

int LayerMaterial::append_property(std::string tag, std::string value)
{
	_properties[tag] = value;
	return 0;
}

