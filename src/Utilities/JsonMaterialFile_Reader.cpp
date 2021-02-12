#include "JsonMaterialFile_Reader.h"

#include <fstream>
#include <iostream>

JsonMaterialFile_Reader::JsonMaterialFile_Reader()
{

}

JsonMaterialFile_Reader::~JsonMaterialFile_Reader()
{
}

void 
JsonMaterialFile_Reader::set_input_file(std::string filename)
{
	_input_filename = filename;
}

int 
JsonMaterialFile_Reader::populate_materials(std::map<std::string, LayerMaterial>& mat_list)
{
	read_json_file();

	try
	{
		for (nlohmann::json::iterator it = _data_json.begin(); it != _data_json.end(); ++it)
		{
			nlohmann::json temp = it.value();
			std::string tag = it.key();
			std::string mat_type = temp.at("Material_Type");
			nlohmann::json mat_props = temp["Material_Properties"];
			std::map<std::string, std::string> props_temp;
			for (nlohmann::json::iterator layer_it = mat_props.begin(); layer_it != mat_props.end(); ++layer_it)
			{
				props_temp[layer_it.key()] = layer_it.value().get<std::string>();
			}

			mat_list[tag] = LayerMaterial(tag, mat_type, props_temp);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Error in parsing the json file." << std::endl;
	}
	return 0;
}

int 
JsonMaterialFile_Reader::read_json_file()
{
	std::ifstream json_file(_input_filename);
	_data_json = nlohmann::json::parse(json_file)["Material_Library"];

	return 0;
}


