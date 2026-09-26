// This file is based on part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A32ARCHTRAITS_P_H_INCLUDED
#define ASMJIT_ARM_A32ARCHTRAITS_P_H_INCLUDED

#include "../core/archtraits.h"
#include "../core/misc_p.h"
#include "../core/type.h"
#include "../arm/a32globals.h"
#include "../arm/a32operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

//! \cond INTERNAL
//! \addtogroup asmjit_a32
//! \{

static const constexpr ArchTraits a32ArchTraits = {
  // SP/FP/LR/PC.
  Gp::kIdSp, Gp::kIdFp, Gp::kIdLr, Gp::kIdPc,

  // Reserved.
  { 0u, 0u, 0u },

  // HW stack alignment (AAPCS requires 8-byte alignment at public interfaces).
  8u,

  // Min/max stack offset.
  255, 4095,

  // Supported register types.
  0u | (1u << uint32_t(RegType::kGp32  ))
     | (1u << uint32_t(RegType::kVec32 ))
     | (1u << uint32_t(RegType::kVec64 ))
     | (1u << uint32_t(RegType::kVec128)),

  // Instruction hints [Gp, Vec, Mask, Extra].
  {{
    InstHints::kPushPop,
    InstHints::kPushPop,
    InstHints::kNoHints,
    InstHints::kNoHints
  }},

  // TypeIdToRegType.
  #define V(index) (index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt8)    ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt8)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt16)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt16)  ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kInt32)   ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUInt32)  ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kIntPtr)  ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kUIntPtr) ? RegType::kGp32   : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kFloat32) ? RegType::kVec32  : \
                    index + uint32_t(TypeId::_kBaseStart) == uint32_t(TypeId::kFloat64) ? RegType::kVec64  : RegType::kNone)
  {{ ASMJIT_LOOKUP_TABLE_32(V, 0) }},
  #undef V

  // Word names of 8-bit, 16-bit, 32-bit, and 64-bit quantities.
  {
    ArchTypeNameId::kByte,
    ArchTypeNameId::kHWord,
    ArchTypeNameId::kWord,
    ArchTypeNameId::kXWord
  }
};

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A32ARCHTRAITS_P_H_INCLUDED
