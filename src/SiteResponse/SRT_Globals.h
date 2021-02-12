#include "OPS_Globals.h"
#include "ConfigManager.h"
#include "SiteGeometry.h"


extern ConfigManager* program_config;
extern SiteGeometry* site_geometry;
extern MaterialManager*  material_manager;

#define PI 3.14159265359

#define CONFIG_TYPE json
// #define CONFIG_TYPE default

// fix the directory separator in windows versus unix
#if defined(WIN32) || defined(_WIN32) 
#define PATH_SEPARATOR "\\" 
#else 
#define PATH_SEPARATOR "/" 
#endif 