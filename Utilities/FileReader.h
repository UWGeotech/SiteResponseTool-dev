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

    virtual std::map<std::string, std::string> getDataAsMap(){};
    virtual std::string getVaueFromKey(std::string key){};
    virtual int setDataFile(const char* file){ return 0;};

};

#endif