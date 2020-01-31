#include <map>
#include <string>
#include <fstream>
#include "FileReader.h"
#include "json.hpp"

#ifndef JSONREADER_H
#define JSONREADER_H

class JSONReader: public FileReader
{
public:
    JSONReader();
    JSONReader(const char* json_file);
    ~JSONReader();

    std::map<std::string, std::string> getDataAsMap();
    int setDataFile(const char* file);
    std::string getValueFromKey(std::string key);
    

    nlohmann::json getJsonObject();

private:
    nlohmann::json data_json;
    std::map <std::string, std::string> data_map;
};

#endif
