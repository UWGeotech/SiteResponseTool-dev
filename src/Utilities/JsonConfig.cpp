#include <string>
#include <sstream>
#include <locale>
#include <vector>
#include <exception>
#include "JsonReader.h"
#include "JsonConfig.h"
#include "OPS_Globals.h"

extern void split_string(const std::string& str, std::vector<std::string>& container, char delimiter='|');

JSONConfig::JSONConfig(const char* json_file_name)
{
    theReader = new JSONReader(json_file_name);
    config_data = theReader->getDataAsMap();
}

JSONConfig::~JSONConfig()
{

}

std::string
JSONConfig::getValueFromKey(std::string key)
{
    return static_cast<JSONReader*>(theReader)->getValueFromKey(key);
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
