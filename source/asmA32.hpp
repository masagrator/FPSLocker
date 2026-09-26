#pragma once
#ifdef __SWITCH__
#include <switch.h>
#else
#include <cstdint>
typedef uint32_t Result;
#endif
#include "c4/yml/node.hpp"
#include <unordered_map>

namespace ASM {

    Result processArm32(c4::yml::NodeRef entry, uint32_t* out, uint8_t* adjust_type_arg, uintptr_t pc_address, uintptr_t start_address, const std::unordered_map<std::string, uint32_t> gotos = {});

}
