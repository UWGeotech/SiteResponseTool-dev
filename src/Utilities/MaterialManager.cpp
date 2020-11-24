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