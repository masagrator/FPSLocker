#include <switch.h>
#include "c4/yml/node.hpp"
#include <map>

namespace ASM {

    Result processArm64(c4::yml::NodeRef entry, uint32_t* out, uint8_t* adjust_type_arg, uintptr_t pc_address, const std::map<std::string, uint32_t> gotos);
    
}