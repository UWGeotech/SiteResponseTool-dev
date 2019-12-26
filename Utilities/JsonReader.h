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
    JSONReader(){};
    JSONReader(const char* json_file);
    ~JSONReader();

    virtual std::map<std::string, std::string> getDataAsMap();
    virtual int setDataFile(const char* file);

private:
    nlohmann::json config_json;
    std::map <std::string, std::string> data_map;
};

#endif