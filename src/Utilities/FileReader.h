#include <map>
#include <string>

#ifndef FILEREADER_H
#define FILEREADER_H

class FileReader
{
public:
    FileReader();
    FileReader(const char* file) ;
    ~FileReader();

    virtual std::map<std::string, std::string> getDataAsMap() { return std::map<std::string, std::string> (); };
    virtual std::string getVaueFromKey(std::string key) { return std::string(""); };
    virtual int setDataFile(const char* file){ return 0;};

};

#endif