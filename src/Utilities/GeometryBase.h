#ifndef GEOMETRYBASE_H
#define GEOMETRYBASE_H

#include <string>

enum class SITEBASE_TYPE {RIGID, COMPLIANT};

class GeometryBase
{
public:
	GeometryBase();
	GeometryBase(std::string layer_tag, std::string material_tag, SITEBASE_TYPE base_type = SITEBASE_TYPE::RIGID);
	~GeometryBase();

	int set_material_tag(std::string tag);
	int set_base_type(SITEBASE_TYPE base_type);

	std::string get_tag() { return _layer_tag; };
	std::string get_material_tag() { return _material_tag; };
	SITEBASE_TYPE get_base_type() { return _base_type; };

private:
	std::string _layer_tag;
	std::string _material_tag;
	SITEBASE_TYPE _base_type;
};

#endif // !GEOMETRYBASE_H