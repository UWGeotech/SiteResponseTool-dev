#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H

#include <string> 
#include <iostream>

#include "LayerMaterialLibrary.h"
#include "NDMaterialFactory.h"
#include "NDMaterial.h"

class MaterialManager
{
public:
	MaterialManager();
	MaterialManager(std::string materialLib_fn);
	MaterialManager(std::string materialLib_fn, std::string filetype);
	~MaterialManager();

	int get_num_material_in_library() { return _material_lib.get_num_material(); };
	LayerMaterial get_material_from_library(std::string tag) { return _material_lib.get_material(tag); };
	NDMaterial* get_layer_material(std::string tag, int OS_tag = 1);

private:
	LayerMaterialLibrary _material_lib;
	std::map<std::string, NDMaterial*> _ndmaterial_store;
	NDMaterial* create_NDMaterial(LayerMaterial layer_mat, int OS_tag);
	NDMaterialFactory mat_factory;
};

#endif // !defien MATERIALMANAGER_H
