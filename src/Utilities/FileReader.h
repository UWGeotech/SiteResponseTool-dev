#include <map>
#include <string>

#ifndef FILEREADER_H
#define FILEREADER_H

class FileReader
{
public:
    FileReader() {};
    FileReader(const char* file) {};
    ~FileReader() {};

    virtual std::map<std::string, std::string> getDataAsMap() = 0;
    virtual std::string getValueFromKey(std::string key) = 0;
    virtual int setDataFile(const char* file) = 0;

};

#endif