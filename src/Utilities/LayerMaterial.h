#ifndef LAYERMATERIAL_H
#define LAYERMATERIAL_H

#include <string>
#include <map>

class LayerMaterial
{
public:
	LayerMaterial();
	LayerMaterial(std::string mat_tag, std::string mat_type, std::map <std::string, std::string> mat_props);
	~LayerMaterial();

	int set_tag(std::string tag) { _material_tag = tag; };
	int append_property(std::string tag, std::string value);
	std::string get_prperty(std::string tag) { return _properties[tag]; };
	std::string get_type() { return _material_type; };
	std::string get_tag() { return _material_tag; };

private:
	std::string _material_tag;
	std::string _material_type;
	std::map <std::string, std::string> _properties;
};

#endif // !LAYERMATERIAL_H

