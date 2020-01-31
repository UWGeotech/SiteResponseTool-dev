#include "JsonReader.h"
#include "SRT_Globals.h"


extern void split_string(const std::string& str, std::vector<std::string>& container, char delimiter='|');


JSONReader::JSONReader()
{

}

JSONReader::JSONReader(const char* json_file_name)
{
    std::ifstream json_file(json_file_name);
    data_json = nlohmann::json::parse(json_file);
}


JSONReader::~JSONReader()
{

}

int
JSONReader::setDataFile(const char* file)
{
    std::ifstream json_file(file);
    data_json = nlohmann::json::parse(json_file);
    data_map = data_json.get<std::map<std::string, std::string>>();

    return 0;
}

std::map<std::string, std::string>
JSONReader::getDataAsMap()
{
    return data_map;
}

nlohmann::json
JSONReader::getJsonObject()
{
    return data_json;
}

std::string
JSONReader::getValueFromKey(std::string key)
{
    // TODO: Make sure the key exists and the return the value.
    std::vector<std::string> keys;
    split_string(key, keys);
    size_t key_size = keys.size();
    
    try
    {
        nlohmann::json object = data_json;
        for (size_t count = 0; count < key_size - 1; count++)
            object = object.at(keys[count]);

        std::string res = object.at(keys[key_size-1]);

        return res;        
    }
    catch(const std::exception& e)
    {
        opserr << "The configuration container is missing the config key named \"" << key.c_str() << "\"" << endln;;
        exit(-1);
    }
}
