#include <string>
#include <locale>
#include <exception>
#include "DefaultConfig.h"
#include "OPS_Globals.h"

DefaultConfig::DefaultConfig()
{
	config_data[std::string("Meshing|Manual")] = std::string("False");
	config_data[std::string("Meshing|MaxFrequency")] = std::string("50.0");
    config_data[std::string("Meshing|NumNodesPerWaveLength")] = std::string("4");
    config_data[std::string("General|PrintDebug")] = std::string("False");
    config_data[std::string("Units|g")] = std::string("9.81");
    config_data[std::string("Units|Patm")] = std::string("101.3");
    config_data[std::string("Analysis|Gravity|PerformGravity")] = std::string("True");
    config_data[std::string("Analysis|Gravity|ConvergenceTest|Tolerance")] = std::string("1.0e-7");
    config_data[std::string("Analysis|Gravity|ConvergenceTest|MaxNumIterations")] = std::string("30");
    config_data[std::string("Analysis|Gravity|ConvergenceTest|PrintTag")] = std::string("1");
    config_data[std::string("Analysis|Dynamic|ConvergenceTest|Tolerance")] = std::string("1.0e-7");
    config_data[std::string("Analysis|Dynamic|ConvergenceTest|MaxNumIterations")] = std::string("30");
    config_data[std::string("Analysis|Dynamic|ConvergenceTest|PrintTag")] = std::string("1");
	config_data[std::string("Analysis|Dynamic|Solver")] = std::string("BandGeneral");
	config_data[std::string("Analysis|Dynamic|Integrator")] = std::string("Newmark");
    config_data[std::string("Analysis|Dynamic|Newmark_Beta")] = std::string("0.25");
    config_data[std::string("Analysis|Dynamic|Newmark_Gamma")] = std::string("0.50");
    config_data[std::string("Analysis|Dynamic|HHT_Alpha")] = std::string("1.0");
    config_data[std::string("Analysis|RigidBase")] = std::string("False");
    config_data[std::string("Analysis|Damping|Ratio")] = std::string("0.01");
    config_data[std::string("Analysis|Damping|Frequency1")] = std::string("0.1");
    config_data[std::string("Analysis|Damping|Frequency2")] = std::string("10.0");
    config_data[std::string("Analysis|Damping|Mode1")] = std::string("1");
    config_data[std::string("Analysis|Damping|Mode2")] = std::string("3");
    config_data[std::string("Analysis|Damping|ModalRayleigh")] = std::string("False");
    config_data[std::string("Analysis|Damping|ElemByElem")] = std::string("False");
}

DefaultConfig::~DefaultConfig()
{

}

std::string
DefaultConfig::getValueFromKey(std::string key)
{
    // TODO: Make sure the key exists and the return the value.
    try
    {
    
        std::string res = this->config_data.at(key);
        return res;
        
    }
    catch(const std::exception& e)
    {
        opserr << "The configuration container is missing the config key named \"" << key.c_str() << "\"" << endln;;
        exit(-1);
    }
}

bool
DefaultConfig::getBooleanProperty(std::string key)
{
    // TODO: Make sure the value can be converted to boolean
    std::string value = getValueFromKey(key);
    if (value == "true" || value == "True" || value == "TRUE")
        return true;
    return false;    
}

int
DefaultConfig::getIntProperty(std::string key)
{
    // TODO: Make sure the value can be converted to int
    std::string value = getValueFromKey(key);
    return std::stoi(value);
}

double
DefaultConfig::getFloatProperty(std::string key)
{
    // TODO: Make sure the value is a floating point number
    std::string value = getValueFromKey(key);
    return std::stod(value);
}

std::string
DefaultConfig::getStringProperty(std::string key)
{
    std::string value = getValueFromKey(key);
    return value;
}
