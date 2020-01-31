#include <string>
#include "FileReader.h"

#ifndef CONFIGIO_H
#define CONFIGIO_H

class ConfigIO
{
public:
    ConfigIO(){};
    ~ConfigIO(){};

    virtual std::string getStringProperty(std::string key) = 0;
    virtual int getIntProperty(std::string key) = 0;
    virtual double getFloatProperty(std::string key) = 0;
    virtual bool getBooleanProperty(std::string key) = 0;

    FileReader* theReader;
};

#endif