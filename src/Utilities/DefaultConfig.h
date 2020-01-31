#include <map>
#include <string>
#include "ConfigIO.h"

#ifndef DEFAULTCONFIG_H
#define DEFAULTCONFIG_H

class DefaultConfig: public ConfigIO
{
public:
    DefaultConfig();
    ~DefaultConfig();

    virtual std::string getStringProperty(std::string key);
    virtual int getIntProperty(std::string key);
    virtual double getFloatProperty(std::string key);
    virtual bool getBooleanProperty(std::string key);

private:
    std::map <std::string, std::string> config_data;
    std::string getValueFromKey(std::string key);
};

#endif