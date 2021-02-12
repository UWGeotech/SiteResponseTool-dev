#ifndef JSONMATERIALFILE_READER_H
#define JSONMATERIALFILE_READER_H

#include "MaterialFile_Reader.h"
#include "LayerMaterial.h"
#include "json.hpp"

class JsonMaterialFile_Reader :
	public MaterialFile_Reader
{
public:
	JsonMaterialFile_Reader();
	~JsonMaterialFile_Reader();

	virtual void set_input_file(std::string filename);
	virtual int populate_materials(std::map<std::string, LayerMaterial>&);

private:
	std::string _input_filename;
	nlohmann::json _data_json;

	int read_json_file();
};

#endif // !JSONMATERIALFILE_READER_H