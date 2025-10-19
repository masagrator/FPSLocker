#pragma once

#ifdef __SWITCH__
#include <switch.h>
#else
#include <cstdint>
typedef uint32_t Result;
#define R_FAILED(res)      ((res)!=0)
#define R_SUCCEEDED(res)   ((res)==0)
#endif
#include "rapidyaml/ryml.hpp"

namespace LOCK {

	extern char configBuffer[32770];
	extern ryml::Tree tree;
	
	Result createPatch(const char* path);
	Result readConfig(const char* path);

}