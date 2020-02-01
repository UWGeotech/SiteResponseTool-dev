#include <stddef.h> // defines NULL
#include <fstream>
#include "ConfigManager.h"
#include "OPS_Globals.h"
#include "SRT_Globals.h"
#include "ConfigIO.h"
#include "DefaultConfig.h"
#include "JsonConfig.h"

// Global static pointer used to insure a single instance of the class
ConfigManager* ConfigManager::m_pInstance = NULL;

ConfigManager* ConfigManager::get_Instance()
{
    if (!m_pInstance)
        m_pInstance = new ConfigManager;

    return m_pInstance;
}

ConfigManager::ConfigManager()
{
    #if (CONFIG_TYPE == json)
        m_configIO = new JSONConfig("config.srtc");
    #else   
        m_configIO = new DefaultConfig();
    #endif
}


std::string 
ConfigManager::getStringProperty(const char* key)
{
    return m_configIO->getStringProperty(std::string(key));
}

int 
ConfigManager::getIntProperty(const char* key)
{
    return m_configIO->getIntProperty(std::string(key));
}

double 
ConfigManager::getFloatProperty(const char* key)
{
    return m_configIO->getFloatProperty(std::string(key));
}

bool 
ConfigManager::getBooleanProperty(const char* key)
{
    return m_configIO->getBooleanProperty(std::string(key));
}