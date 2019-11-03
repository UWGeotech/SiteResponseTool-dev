#include <string>
#include "DefaultConfig.h"

DefaultConfig::DefaultConfig()
{
    config_data[std::string("key1")] = std::string("value1");
    config_data[std::string("key2")] = std::string("false");
    config_data[std::string("key3")] = std::string("3.1415");
    config_data[std::string("key4")] = std::string("4");

}

DefaultConfig::~DefaultConfig()
{

}

bool
DefaultConfig::getBooleanProperty(std::string key)
{
    std::string value = this->config_data.at(key);
    if (value == "true")
        return true;
    return false;    
}

int
DefaultConfig::getIntProperty(std::string key)
{
    std::string value = this->config_data.at(key);
    return std::stoi(value);
}

double
DefaultConfig::getFloatProperty(std::string key)
{
    std::string value = this->config_data.at(key);
    return std::stod(value);
}

std::string
DefaultConfig::getStringProperty(std::string key)
{
    std::string value = this->config_data.at(key);
    return value;
}