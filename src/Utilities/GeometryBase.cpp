#include "GeometryBase.h"

GeometryBase::GeometryBase()
{

}

GeometryBase::GeometryBase(std::string layer_tag, std::string material_tag, SITEBASE_TYPE base_type)
{
	_layer_tag = layer_tag;
	_material_tag = material_tag;
	_base_type = base_type;
}

GeometryBase::~GeometryBase()
{

}

int GeometryBase::set_material_tag(std::string tag)
{
	_material_tag = tag;
	return 0;
}

int GeometryBase::set_base_type(SITEBASE_TYPE base_type)
{
	_base_type = base_type;
	return 0;
}

