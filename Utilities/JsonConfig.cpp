#include <string>
#include <sstream>
#include <locale>
#include <vector>
#include <exception>
#include "JsonConfig.h"
#include "OPS_Globals.h"

extern void split_string(const std::string& str, std::vector<std::string>& container, char delimiter='|');

JSONConfig::JSONConfig(const char* json_file_name)
{
    std::ifstream json_file(json_file_name);
    config_json = nlohmann::json::parse(json_file);
}

JSONConfig::~JSONConfig()
{

}

std::string
JSONConfig::getValueFromKey(std::string key)
{
    // TODO: Make sure the key exists and the return the value.
    std::vector<std::string> keys;
    split_string(key, keys);
    size_t key_size = keys.size();
    
    try
    {
        nlohmann::json object = config_json;
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

bool
JSONConfig::getBooleanProperty(std::string key)
{
    // TODO: Make sure the value can be converted to boolean
    std::string value = getValueFromKey(key);
    if (value == "true" || value == "True" || value == "TRUE")
        return true;
    return false;    
}

int
JSONConfig::getIntProperty(std::string key)
{
    // TODO: Make sure the value can be converted to int
    std::string value = getValueFromKey(key);
    return std::stoi(value);
}

double
JSONConfig::getFloatProperty(std::string key)
{
    // TODO: Make sure the value is a floating point number
    std::string value = getValueFromKey(key);
    return std::stod(value);
}

std::string
JSONConfig::getStringProperty(std::string key)
{
    std::string value = getValueFromKey(key);
    return value;
}
