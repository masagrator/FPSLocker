// This file is based on part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A32INSTAPI_P_H_INCLUDED
#define ASMJIT_ARM_A32INSTAPI_P_H_INCLUDED

#include "../core/inst.h"
#include "../core/operand.h"
#include "../core/formatter.h"
#include "../core/string.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

//! \cond INTERNAL
//! \addtogroup asmjit_a32
//! \{

namespace InstInternal {

#ifndef ASMJIT_NO_TEXT
Error ASMJIT_CDECL instIdToString(InstId instId, InstStringifyOptions options, String& output) noexcept;
InstId ASMJIT_CDECL stringToInstId(const char* s, size_t len) noexcept;
#endif // !ASMJIT_NO_TEXT

#ifndef ASMJIT_NO_VALIDATION
Error ASMJIT_CDECL validate(const BaseInst& inst, const Operand_* operands, size_t opCount, ValidationFlags validationFlags) noexcept;
#endif // !ASMJIT_NO_VALIDATION

#ifndef ASMJIT_NO_LOGGING
Error ASMJIT_CDECL formatInstruction(
  String& sb,
  FormatFlags formatFlags,
  const BaseEmitter* emitter,
  Arch arch,
  const BaseInst& inst, const Operand_* operands, size_t opCount) noexcept;
#endif // !ASMJIT_NO_LOGGING

} // {InstInternal}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A32INSTAPI_P_H_INCLUDED
