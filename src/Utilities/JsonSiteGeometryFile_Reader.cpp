#include "JsonSiteGeometryFile_Reader.h"

#include <fstream>
#include <iostream>

JsonSiteGeometryFile_Reader::JsonSiteGeometryFile_Reader()
{

}

JsonSiteGeometryFile_Reader::~JsonSiteGeometryFile_Reader()
{

}

int
JsonSiteGeometryFile_Reader::read_json_file()
{
	std::ifstream json_file(_input_filename);
	_data_json = nlohmann::json::parse(json_file)["Site_Geometry"];

	return 0;
}

int
JsonSiteGeometryFile_Reader::populate_layering(SiteGeometry* sLayering)
{
	read_json_file();
	std::vector<std::string> temp_layer_order;

	try
	{
		for (nlohmann::json::iterator it = _data_json.begin(); it != _data_json.end(); ++it)
		{
			if (it.key() == "Base_Layer")
			{
				nlohmann::json temp = it.value();
				
				SITEBASE_TYPE base_type = SITEBASE_TYPE::COMPLIANT;
				if (temp["Type"] == "Compliant")
					base_type = SITEBASE_TYPE::COMPLIANT;
				else if (temp["Type"] == "Rigid")
					base_type = SITEBASE_TYPE::RIGID;
				
				sLayering->setBaseLayer(GeometryBase(temp["Name"], temp["Material"], base_type));
			
			}
			else if (it.key() == "Layers") {
				nlohmann::json temp = it.value();
				for (nlohmann::json::iterator layer_it = temp.begin(); layer_it != temp.end(); ++layer_it)
				{
					nlohmann::json temp2 = layer_it.value();
					sLayering->addNewLayerToLibrary(GeometryLayer(layer_it.key(), temp2["Thickness"], temp2["Material"]));
				}
			}
			else if (it.key() == "Layers_Order") {
				nlohmann::json temp = it.value();
				for (nlohmann::json::iterator layer_it = temp.begin(); layer_it != temp.end(); ++layer_it)
				{
					nlohmann::json temp2 = layer_it.value();
					temp_layer_order.push_back(temp2);
				}
			}
			else if (it.key() == "Groundwater_Depth") {
				sLayering->set_GWT_depth(it.value());
			}
			std::cout << it.key() << " : " << it.value() << std::endl;
		}
		for (std::vector<std::string>::iterator layer_it = temp_layer_order.begin(); layer_it != temp_layer_order.end(); ++layer_it)
		{
			sLayering->addNewLayer(sLayering->getLayerFromLibrary(*layer_it));
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Error in parsing the json file." << std::endl;
	}

	return 0;
}
