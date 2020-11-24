#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H

#include <string> 

#include "LayerMaterialLibrary.h"
#include "NDMaterial.h"

class MaterialManager
{
public:
	MaterialManager();
	MaterialManager(std::string materialLib_fn);
	MaterialManager(std::string materialLib_fn, std::string filetype);
	~MaterialManager();

	LayerMaterial get_material_from_library(std::string tag) { return _material_lib.get_material(tag); };
	NDMaterial get_layer_material(std::string tag);

private:
	LayerMaterialLibrary _material_lib;
	std::map<std::string, NDMaterial> _ndmaterial_store;
};

#endif // !defien MATERIALMANAGER_H
