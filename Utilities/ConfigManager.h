#include <string>
#include <fstream>
#include "ConfigIO.h"

#ifndef ConfigManager_H
#define ConfigManager_H

class ConfigManager
{
public:

    static ConfigManager* get_Instance();
    virtual std::string getStringProperty(const char* key);
    virtual int getIntProperty(const char* key);
    virtual double getFloatProperty(const char* key);
    virtual bool getBooleanProperty(const char* key);

private:

    ConfigManager(); // this method is private so that it cannot be called
    ConfigManager(ConfigManager const&){}; // same with the copy constructor
    ConfigManager& operator=(ConfigManager const&){}; // assignment operator as well
    
    static ConfigManager* m_pInstance;
    ConfigIO* m_configIO;
};


#endif