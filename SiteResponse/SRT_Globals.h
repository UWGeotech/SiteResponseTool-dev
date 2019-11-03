#include "OPS_Globals.h"
#include "ConfigManager.h"


extern ConfigManager* program_config;

#define PRINTDEBUG true

// fix the directory separator in windows versus unix
#if defined(WIN32) || defined(_WIN32) 
#define PATH_SEPARATOR "\\" 
#else 
#define PATH_SEPARATOR "/" 
#endif 
