#ifndef JSONSiteGeometryFILEREDER_H
#define JSONSiteGeometryFILEREDER_H

#include "SiteGeometryFile_Reader.h"
#include "json.hpp"

class JsonSiteGeometryFile_Reader :
    public SiteGeometryFile_Reader
{
public:
    JsonSiteGeometryFile_Reader();
    ~JsonSiteGeometryFile_Reader();

    virtual void set_input_file(std::string filename) { _input_filename = filename; };
    virtual int populate_layering(SiteGeometry*);

private:
    std::string _input_filename;
    nlohmann::json _data_json;

    int read_json_file();
};

#endif // !JSONSiteGeometryFILEREDER_H