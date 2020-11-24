#ifndef GEOMETRYLAYER_H
#define GEOMETRYLAYER_H

#include <string>

class GeometryLayer
{
public:
	GeometryLayer();
	GeometryLayer(std::string layer_tag, double thickness, std::string material_tag);
	~GeometryLayer();

	int set_thickness(double t);
	int set_material_tag(std::string tag);

	double get_thickess() { return _thickness; };
	std::string get_tag() { return _layer_tag; };
	std::string get_material_tag() { return _material_tag; };

private:
	std::string _layer_tag;
	std::string _material_tag;
	double _thickness;
};

#endif // !GEOMETRYLAYER_H