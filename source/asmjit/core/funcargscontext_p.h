// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_FUNCARGSCONTEXT_P_H_INCLUDED
#define ASMJIT_CORE_FUNCARGSCONTEXT_P_H_INCLUDED

#include "../core/archtraits.h"
#include "../core/environment.h"
#include "../core/func.h"
#include "../core/operand.h"
#include "../core/radefs_p.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_core
//! \{

static inline OperandSignature getSuitableRegForMemToMemMove(Arch arch, TypeId dstTypeId, TypeId srcTypeId) noexcept {
  const ArchTraits& archTraits = ArchTraits::byArch(arch);

  uint32_t signature = 0u;
  uint32_t dstSize = TypeUtils::sizeOf(dstTypeId);
  uint32_t srcSize = TypeUtils::sizeOf(srcTypeId);
  uint32_t maxSize = Support::max<uint32_t>(dstSize, srcSize);
  uint32_t regSize = Environment::registerSizeFromArch(arch);

  if (maxSize <= regSize || (TypeUtils::isInt(dstTypeId) && TypeUtils::isInt(srcTypeId))) {
    signature = maxSize <= 4 ? RegTraits<RegType::kGp32>::kSignature
                             : RegTraits<RegType::kGp64>::kSignature;
  }
  else if (maxSize <= 8 && archTraits.hasRegType(RegType::kVec64)) {
    signature = RegTraits<RegType::kVec64>::kSignature;
  }
  else if (maxSize <= 16 && archTraits.hasRegType(RegType::kVec128)) {
    signature = RegTraits<RegType::kVec128>::kSignature;
  }
  else if (maxSize <= 32 && archTraits.hasRegType(RegType::kVec256)) {
    signature = RegTraits<RegType::kVec256>::kSignature;
  }
  else if (maxSize <= 64 && archTraits.hasRegType(RegType::kVec512)) {
    signature = RegTraits<RegType::kVec512>::kSignature;
  }

  return OperandSignature{signature};
}

class FuncArgsContext {
public:
  static inline constexpr uint32_t kVarIdNone = 0xFF;

  //! Contains information about a single argument or SA register that may need shuffling.
  struct Var {
    FuncValue cur;
    FuncValue out;

    inline void init(const FuncValue& cur_, const FuncValue& out_) noexcept {
      cur = cur_;
      out = out_;
    }

    //! Reset the value to its unassigned state.
    inline void reset() noexcept {
      cur.reset();
      out.reset();
    }

    ASMJIT_INLINE_NODEBUG bool isDone() const noexcept { return cur.isDone(); }
    ASMJIT_INLINE_NODEBUG void markDone() noexcept { cur.addFlags(FuncValue::kFlagIsDone); }
  };

  struct WorkData {
    //! All allocable registers provided by the architecture.
    RegMask _archRegs;
    //! All registers that can be used by the shuffler.
    RegMask _workRegs;
    //! Registers used by the shuffler (all).
    RegMask _usedRegs;
    //! Assigned registers.
    RegMask _assignedRegs;
    //! Destination registers assigned to arguments or SA.
    RegMask _dstRegs;
    //! Destination registers that require shuffling.
    RegMask _dstShuf;
    //! Number of register swaps.
    uint8_t _numSwaps;
    //! Number of stack loads.
    uint8_t _numStackArgs;
    //! Whether this work data would need reassignment.
    uint8_t _needsScratch;
    //! Reserved (only used as padding).
    uint8_t _reserved[5];
    //! Physical ID to variable ID mapping.
    uint8_t _physToVarId[32];

    inline void reset() noexcept {
      _archRegs = 0;
      _workRegs = 0;
      _usedRegs = 0;
      _assignedRegs = 0;
      _dstRegs = 0;
      _dstShuf = 0;
      _numSwaps = 0;
      _numStackArgs = 0;
      _needsScratch = 0;
      memset(_reserved, 0, sizeof(_reserved));
      memset(_physToVarId, kVarIdNone, 32);
    }

    [[nodiscard]]
    inline bool isAssigned(uint32_t regId) const noexcept {
      ASMJIT_ASSERT(regId < 32);
      return Support::bitTest(_assignedRegs, regId);
    }

    inline void assign(uint32_t varId, uint32_t regId) noexcept {
      ASMJIT_ASSERT(!isAssigned(regId));
      ASMJIT_ASSERT(_physToVarId[regId] == kVarIdNone);

      _physToVarId[regId] = uint8_t(varId);
      _assignedRegs ^= Support::bitMask(regId);
    }

    inline void reassign(uint32_t varId, uint32_t newId, uint32_t oldId) noexcept {
      ASMJIT_ASSERT( isAssigned(oldId));
      ASMJIT_ASSERT(!isAssigned(newId));
      ASMJIT_ASSERT(_physToVarId[oldId] == varId);
      ASMJIT_ASSERT(_physToVarId[newId] == kVarIdNone);

      _physToVarId[oldId] = uint8_t(kVarIdNone);
      _physToVarId[newId] = uint8_t(varId);
      _assignedRegs ^= Support::bitMask(newId) ^ Support::bitMask(oldId);
    }

    inline void swap(uint32_t aVarId, uint32_t aRegId, uint32_t bVarId, uint32_t bRegId) noexcept {
      ASMJIT_ASSERT(isAssigned(aRegId));
      ASMJIT_ASSERT(isAssigned(bRegId));
      ASMJIT_ASSERT(_physToVarId[aRegId] == aVarId);
      ASMJIT_ASSERT(_physToVarId[bRegId] == bVarId);

      _physToVarId[aRegId] = uint8_t(bVarId);
      _physToVarId[bRegId] = uint8_t(aVarId);
    }

    inline void unassign(uint32_t varId, uint32_t regId) noexcept {
      ASMJIT_ASSERT(isAssigned(regId));
      ASMJIT_ASSERT(_physToVarId[regId] == varId);

      DebugUtils::unused(varId);
      _physToVarId[regId] = uint8_t(kVarIdNone);
      _assignedRegs ^= Support::bitMask(regId);
    }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG RegMask archRegs() const noexcept { return _archRegs; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG RegMask workRegs() const noexcept { return _workRegs; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG RegMask usedRegs() const noexcept { return _usedRegs; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG RegMask assignedRegs() const noexcept { return _assignedRegs; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG RegMask dstRegs() const noexcept { return _dstRegs; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG RegMask availableRegs() const noexcept { return _workRegs & ~_assignedRegs; }
  };

  //! Architecture traits.
  const ArchTraits* _archTraits = nullptr;
  //! Architecture constraints.
  const RAConstraints* _constraints = nullptr;
  //! Target architecture.
  Arch _arch = Arch::kUnknown;
  //! Has arguments passed via stack (SRC).
  bool _hasStackSrc = false;
  //! Has preserved frame-pointer (FP).
  bool _hasPreservedFP = false;
  //! Has arguments assigned to stack (DST).
  uint8_t _stackDstMask = 0;
  //! Register swap groups (bit-mask).
  uint8_t _regSwapsMask = 0;
  uint8_t _saVarId = kVarIdNone;
  uint32_t _varCount = 0;
  Support::Array<WorkData, Globals::kNumVirtGroups> _workData;
  Var _vars[Globals::kMaxFuncArgs * Globals::kMaxValuePack + 1];

  FuncArgsContext() noexcept;

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ArchTraits& archTraits() const noexcept { return *_archTraits; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return _arch; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t varCount() const noexcept { return _varCount; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t indexOf(const Var* var) const noexcept { return (size_t)(var - _vars); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Var& var(size_t varId) noexcept { return _vars[varId]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Var& var(size_t varId) const noexcept { return _vars[varId]; }

  Error initWorkData(const FuncFrame& frame, const FuncArgsAssignment& args, const RAConstraints* constraints) noexcept;
  Error markScratchRegs(FuncFrame& frame) noexcept;
  Error markDstRegsDirty(FuncFrame& frame) noexcept;
  Error markStackArgsReg(FuncFrame& frame) noexcept;
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_FUNCARGSCONTEXT_P_H_INCLUDED
