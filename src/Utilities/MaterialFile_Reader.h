#ifndef MATERIALFILE_READER_H
#define MATERIALFILE_READER_H

#include <string>
#include <map>

#include "LayerMaterial.h"

class MaterialFile_Reader
{
public:
	MaterialFile_Reader();
	~MaterialFile_Reader();

	virtual void set_input_file(std::string) = 0;
	virtual int populate_materials(std::map<std::string, LayerMaterial>&) = 0;

};

#endif // !MATERIALFILE_READER_H
