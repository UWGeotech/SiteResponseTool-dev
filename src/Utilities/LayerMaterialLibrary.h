#ifndef LAYERMATERIALLIBRARY_H 
#define LAYERMATERIALLIBRARY_H

#include <string> 

#include "MaterialFile_Reader.h"
#include "LayerMaterial.h"

class LayerMaterialLibrary
{
public:
	LayerMaterialLibrary();
	LayerMaterialLibrary(std::string materialLib_fn);
	LayerMaterialLibrary(std::string materialLib_fn, std::string filetype);
	~LayerMaterialLibrary();

	int read_material_file(std::string materialLib_fn, std::string filetype = "__na__");
	LayerMaterial get_material(std::string tag) { return material_library[tag]; };
	int get_num_material() { return _num_mat; };


private:
	MaterialFile_Reader* material_file;
	int _num_mat;
	std::map<std::string, LayerMaterial> material_library;
};

#endif // !LAYERMATERIALLIBRARY_H