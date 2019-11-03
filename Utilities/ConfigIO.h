#include <string>

#ifndef CONFIGIO_H
#define CONFIGIO_H

class ConfigIO
{
public:
    ConfigIO(){};
    ~ConfigIO(){};

    virtual std::string getStringProperty(std::string key) { return std::string("");};
    virtual int getIntProperty(std::string key) {return 0;};
    virtual double getFloatProperty(std::string key) {return 0.0;};
    virtual bool getBooleanProperty(std::string key) {return false;};
};

#endif