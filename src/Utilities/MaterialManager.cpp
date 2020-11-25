#include "MaterialManager.h"
#include "JsonMaterialFile_Reader.h"

MaterialManager::MaterialManager()
{

}

MaterialManager::MaterialManager(std::string materialLib_fn)
{
	_material_lib.read_material_file(materialLib_fn);
}

MaterialManager::MaterialManager(std::string materialLib_fn, std::string filetype)
{
	_material_lib.read_material_file(materialLib_fn);
}

MaterialManager::~MaterialManager()
{

}

NDMaterial* 
MaterialManager::get_layer_material(std::string tag)
{
	if (_ndmaterial_store.find(tag) == _ndmaterial_store.end())
		return _ndmaterial_store[tag];
	else
	{
		NDMaterial* theMat = create_NDMaterial(_material_lib.get_material(tag));
		return theMat;
	}
}

NDMaterial*
MaterialManager::create_NDMaterial(LayerMaterial layer_mat)
{
	NDMaterial* theMat = NULL;
	if (layer_mat.get_type() == "ElasticIsotropic")
		theMat = mat_factory.create_elastic_material(layer_mat);
	else if (layer_mat.get_type() == "J2BoundingCyclic")
		theMat = mat_factory.create_J2Cyclic_material(layer_mat);
	else
	{
		std::cerr << "Material type \"" << layer_mat.get_type() << "\" (Layer: \"" << layer_mat.get_tag() << "\") is not defined." << std::endl;
		exit(-99);
	}

	return theMat;
}