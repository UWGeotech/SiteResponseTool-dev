#include "GeometryLayer.h"

GeometryLayer::GeometryLayer()
	:_thickness(0.0)
{

}

GeometryLayer::GeometryLayer(std::string layer_tag, double thickness, std::string material_tag)
{
	_layer_tag = layer_tag;
	_thickness = thickness;
	_material_tag = material_tag;
}

GeometryLayer::~GeometryLayer()
{

}

int GeometryLayer::set_thickness(double t)
{
	if (t >= 0)
	{
		_thickness = t;
		return 0;
	}
	else 
	{
		// Error: negative thickness for layer
		return -1;
	}
	return 0;
}

int GeometryLayer::set_material_tag(std::string tag)
{
	_material_tag = tag;
	return 0;
}

