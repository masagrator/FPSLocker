#pragma once

#include <switch.h>
#include "rapidyaml/ryml.hpp"

namespace LOCK {

	extern char configBuffer[32770];
	extern ryml::Tree tree;
	
	Result createPatch(const char* path);
	Result readConfig(char* path);

}