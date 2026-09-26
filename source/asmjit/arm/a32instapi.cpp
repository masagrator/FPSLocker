// This file is based on part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_AARCH32)

#include "../core/formatter.h"
#include "../core/misc_p.h"
#include "../core/support.h"
#include "../arm/a32globals.h"
#include "../arm/a32operand.h"
#include "../arm/a32instapi_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

namespace InstInternal {

// a32::InstInternal - Text
// ========================

#if !defined(ASMJIT_NO_TEXT) || !defined(ASMJIT_NO_LOGGING)
static const char* const instNameTable[] = {
  "",
#define ASMJIT_A32_INST_NAME(NAME) #NAME,
  ASMJIT_A32_INST_LIST(ASMJIT_A32_INST_NAME)
#undef ASMJIT_A32_INST_NAME
};

static_assert(sizeof(instNameTable) / sizeof(instNameTable[0]) == Inst::_kIdCount, "Instruction name table size mismatch");

//! Appends lower-cased instruction name of `instId` (real id only) to `sb`.
static Error appendInstName(String& sb, uint32_t instId) noexcept {
  const char* name = instNameTable[instId];
  size_t len = strlen(name);

  char buf[32];
  if (len >= sizeof(buf)) {
    return DebugUtils::errored(kErrorInvalidState);
  }

  for (size_t i = 0; i < len; i++) {
    buf[i] = Support::asciiToLower(name[i]);
  }

  return sb.append(buf, len);
}
#endif

#ifndef ASMJIT_NO_TEXT
Error ASMJIT_CDECL instIdToString(InstId instId, InstStringifyOptions options, String& output) noexcept {
  DebugUtils::unused(options);

  uint32_t realId = instId & uint32_t(InstIdParts::kRealId);
  if (ASMJIT_UNLIKELY(!Inst::isDefinedId(realId) || realId == 0)) {
    return DebugUtils::errored(kErrorInvalidInstruction);
  }

  return appendInstName(output, realId);
}

InstId ASMJIT_CDECL stringToInstId(const char* s, size_t len) noexcept {
  if (ASMJIT_UNLIKELY(!s || len == 0 || len >= 32)) {
    return BaseInst::kIdNone;
  }

  if (len == SIZE_MAX) {
    len = strlen(s);
  }

  for (uint32_t i = 1; i < Inst::_kIdCount; i++) {
    const char* name = instNameTable[i];
    if (strlen(name) != len) {
      continue;
    }

    size_t j = 0;
    while (j < len && Support::asciiToLower(name[j]) == Support::asciiToLower(s[j])) {
      j++;
    }

    if (j == len) {
      return i;
    }
  }

  return BaseInst::kIdNone;
}
#endif // !ASMJIT_NO_TEXT

// a32::InstInternal - Validate
// ============================

#ifndef ASMJIT_NO_VALIDATION
Error ASMJIT_CDECL validate(const BaseInst& inst, const Operand_* operands, size_t opCount, ValidationFlags validationFlags) noexcept {
  DebugUtils::unused(validationFlags);

  // The assembler performs a complete validation of operands during encoding, so only basic checks are done here.
  uint32_t realId = inst.realId();
  if (ASMJIT_UNLIKELY(!Inst::isDefinedId(realId) || realId == 0)) {
    return DebugUtils::errored(kErrorInvalidInstruction);
  }

  for (size_t i = 0; i < opCount; i++) {
    const Operand_& op = operands[i];
    if (op.isReg()) {
      const Reg& reg = op.as<Reg>();
      if (reg.isGp32()) {
        if (reg.id() > 15u) return DebugUtils::errored(kErrorInvalidPhysId);
      }
      else if (reg.isVec32() || reg.isVec64()) {
        if (reg.id() > 31u) return DebugUtils::errored(kErrorInvalidPhysId);
      }
      else if (reg.isVec128()) {
        if (reg.id() > 15u) return DebugUtils::errored(kErrorInvalidPhysId);
      }
      else {
        return DebugUtils::errored(kErrorInvalidRegType);
      }
    }
  }

  return kErrorOk;
}
#endif // !ASMJIT_NO_VALIDATION

// a32::InstInternal - Format
// ==========================

#ifndef ASMJIT_NO_LOGGING
static const char condNames[16][3] = {
  "", "nv", "eq", "ne", "hs", "lo", "mi", "pl", "vs", "vc", "hi", "ls", "ge", "lt", "gt", "le"
};

static const char* const dtNames[16] = {
  "", ".s8", ".s16", ".s32", ".s64", ".u8", ".u16", ".u32", ".u64", "", ".f16", ".f32", ".f64", ".p8", ".bf16", ".p64"
};

static const char shiftNames[5][4] = { "lsl", "lsr", "asr", "ror", "rrx" };

static Error formatReg(String& sb, RegType type, uint32_t id) noexcept {
  switch (type) {
    case RegType::kGp32:
      switch (id) {
        case Gp::kIdSp: return sb.append("sp");
        case Gp::kIdLr: return sb.append("lr");
        case Gp::kIdPc: return sb.append("pc");
        default: return sb.appendFormat("r%u", id);
      }
    case RegType::kVec32: return sb.appendFormat("s%u", id);
    case RegType::kVec64: return sb.appendFormat("d%u", id);
    case RegType::kVec128: return sb.appendFormat("q%u", id);
    default: return sb.appendFormat("<reg:%u:%u>", uint32_t(type), id);
  }
}

static Error formatOperand(String& sb, FormatFlags formatFlags, const BaseEmitter* emitter, const Operand_& op) noexcept {
  if (op.isReg()) {
    const Reg& reg = op.as<Reg>();
    ASMJIT_PROPAGATE(formatReg(sb, reg.regType(), reg.id()));

    if (reg.isVec64()) {
      const Vec& v = op.as<Vec>();
      if (v.hasElementIndex()) {
        ASMJIT_PROPAGATE(sb.appendFormat("[%u]", v.elementIndex()));
      }
      else if (v.isAllLanes()) {
        ASMJIT_PROPAGATE(sb.append("[]"));
      }
    }
    return kErrorOk;
  }

  if (op.isImm()) {
    const Imm& imm = op.as<Imm>();
    if (imm.isDouble()) {
      return sb.appendFormat("#%g", imm.valueAs<double>());
    }

    uint32_t pred = imm.predicate();
    if (pred != 0 && pred <= uint32_t(ShiftOp::kRRX)) {
      ASMJIT_PROPAGATE(sb.append(shiftNames[pred]));
      if (pred == uint32_t(ShiftOp::kRRX)) {
        return kErrorOk;
      }
      ASMJIT_PROPAGATE(sb.append(' '));
    }

    return sb.appendFormat("#%lld", (long long)imm.value());
  }

  if (op.isLabel()) {
    return Formatter::formatLabel(sb, formatFlags, emitter, op.id());
  }

  if (op.isRegList()) {
    const BaseRegList& list = op.as<BaseRegList>();
    ASMJIT_PROPAGATE(sb.append('{'));

    bool first = true;
    Support::BitWordIterator<uint32_t> it(list.list());
    while (it.hasNext()) {
      if (!first) {
        ASMJIT_PROPAGATE(sb.append(", "));
      }
      first = false;
      ASMJIT_PROPAGATE(formatReg(sb, list.regType(), it.next()));
    }

    return sb.append('}');
  }

  if (op.isMem()) {
    const Mem& m = op.as<Mem>();
    ASMJIT_PROPAGATE(sb.append('['));

    if (m.hasBaseLabel()) {
      ASMJIT_PROPAGATE(Formatter::formatLabel(sb, formatFlags, emitter, m.baseId()));
    }
    else if (m.hasBaseReg()) {
      ASMJIT_PROPAGATE(formatReg(sb, m.baseType(), m.baseId()));
    }
    else {
      return sb.appendFormat("0x%llX]", (unsigned long long)uint64_t(m.offset()));
    }

    if (m.alignment()) {
      ASMJIT_PROPAGATE(sb.appendFormat(":%u", m.alignment()));
    }

    if (m.isPostIndex()) {
      ASMJIT_PROPAGATE(sb.append(']'));
    }

    if (m.hasIndex()) {
      ASMJIT_PROPAGATE(sb.append(m.isNegIndex() ? ", -" : ", "));
      ASMJIT_PROPAGATE(formatReg(sb, m.indexType(), m.indexId()));
      if (m.hasShift() || m.shiftOp() == ShiftOp::kRRX) {
        uint32_t sop = uint32_t(m.shiftOp());
        if (sop <= uint32_t(ShiftOp::kRRX)) {
          ASMJIT_PROPAGATE(sb.appendFormat(", %s", shiftNames[sop]));
          if (sop != uint32_t(ShiftOp::kRRX)) {
            ASMJIT_PROPAGATE(sb.appendFormat(" #%u", m.shift()));
          }
        }
      }
    }
    else if (m.offsetLo32() != 0) {
      ASMJIT_PROPAGATE(sb.appendFormat(", #%d", m.offsetLo32()));
    }

    if (!m.isPostIndex()) {
      ASMJIT_PROPAGATE(sb.append(']'));
    }

    if (m.isPreIndex()) {
      ASMJIT_PROPAGATE(sb.append('!'));
    }

    return kErrorOk;
  }

  return sb.append("<none>");
}

Error ASMJIT_CDECL formatInstruction(
  String& sb,
  FormatFlags formatFlags,
  const BaseEmitter* emitter,
  Arch arch,
  const BaseInst& inst, const Operand_* operands, size_t opCount) noexcept {

  DebugUtils::unused(arch);

  InstId instId = inst.id();
  uint32_t realId = instId & uint32_t(InstIdParts::kRealId);

  if (ASMJIT_UNLIKELY(!Inst::isDefinedId(realId) || realId == 0)) {
    return sb.appendFormat("[InstId=#%u]", unsigned(instId));
  }

  ASMJIT_PROPAGATE(appendInstName(sb, realId));

  uint32_t cc = uint32_t(BaseInst::extractARMCondCode(instId));
  ASMJIT_PROPAGATE(sb.append(condNames[cc & 0xFu]));

  uint32_t dt = (instId & uint32_t(InstIdParts::kA32_DT)) >> Support::ConstCTZ<uint32_t(InstIdParts::kA32_DT)>::value;
  uint32_t dt2 = (instId & uint32_t(InstIdParts::kA32_DT2)) >> Support::ConstCTZ<uint32_t(InstIdParts::kA32_DT2)>::value;
  ASMJIT_PROPAGATE(sb.append(dtNames[dt & 0xFu]));
  ASMJIT_PROPAGATE(sb.append(dtNames[dt2 & 0xFu]));

  bool first = true;
  for (size_t i = 0; i < opCount; i++) {
    const Operand_& op = operands[i];
    if (op.isNone()) {
      break;
    }

    ASMJIT_PROPAGATE(sb.append(first ? " " : ", "));
    first = false;

    // Register-shifted-register operand.
    if (i >= 3 && op.isReg() && op.as<Reg>().isGp32() && operands[i - 1].isReg()) {
      uint32_t sop = op.as<Reg>().predicate();
      if (sop <= uint32_t(ShiftOp::kROR)) {
        ASMJIT_PROPAGATE(sb.append(shiftNames[sop]));
        ASMJIT_PROPAGATE(sb.append(' '));
      }
    }

    ASMJIT_PROPAGATE(formatOperand(sb, formatFlags, emitter, op));
  }

  return kErrorOk;
}
#endif // !ASMJIT_NO_LOGGING

} // {InstInternal}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH32
