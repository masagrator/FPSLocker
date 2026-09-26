// This file is based on part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A32OPERAND_H_INCLUDED
#define ASMJIT_ARM_A32OPERAND_H_INCLUDED

#include "../core/operand.h"
#include "../arm/armglobals.h"
#include "../arm/a32globals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

//! \addtogroup asmjit_a32
//! \{

//! General purpose register (AArch32) - `r0` to `r15`.
//!
//! A register can optionally carry a shift operation (stored as a register predicate), which is used to describe
//! a register-shifted-register operand like `r2, lsl r3` - see \ref lsl(const Gp&) and friends.
class Gp : public UniGp {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Gp, UniGp)

  //! Special register ids.
  enum Id : uint32_t {
    //! Static base register (r9).
    kIdSb = 9,
    //! Intra-procedure call register (r12).
    kIdIp = 12,
    //! Frame pointer register id (r11 in A32 code).
    kIdFp = 11,
    //! Stack pointer register id.
    kIdSp = 13,
    //! Link register id.
    kIdLr = 14,
    //! Program counter register id.
    kIdPc = 15
  };

  //! Creates a new 32-bit general purpose register having the given register id `regId`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r32(uint32_t regId) noexcept { return Gp(_signatureOf<RegType::kGp32>(), regId); }

  //! Creates a new 32-bit general purpose register having the given register id `regId`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r(uint32_t regId) noexcept { return make_r32(regId); }

  //! Tests whether the register is SP.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isSp() const noexcept { return id() == kIdSp; }

  //! Tests whether the register is LR.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isLr() const noexcept { return id() == kIdLr; }

  //! Tests whether the register is PC.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isPc() const noexcept { return id() == kIdPc; }

  //! Returns the shift operation associated with this register (only meaningful for shift-by-register operands).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR ShiftOp shiftOp() const noexcept { return ShiftOp(predicate()); }

  //! Returns a copy of this register with the shift operation `op` associated.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp withShiftOp(ShiftOp op) const noexcept {
    Gp r(*this);
    r.setPredicate(uint32_t(op));
    return r;
  }
};

//! Vector register (AArch32) - `s0-s31` (32-bit), `d0-d31` (64-bit), `q0-q15` (128-bit).
//!
//! D registers can be indexed to describe a scalar (for example `d0[1]` is `d0.at(1)` or `d0[1]`), which is used by
//! scalar instructions (VMUL/VMLA by scalar, VDUP, VMOV core<->scalar) and lane load/store instructions (VLDn/VSTn).
//! A D register can also describe "all lanes" (`d0[]`), which is expressed as `d0.all()` and only used by VLDn.
class Vec : public UniVec {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Vec, UniVec)

  //! \cond INTERNAL
  // |........|........|XXXX_X..X|........|
  static inline constexpr uint32_t kSignatureRegAllLanesShift = 12;
  static inline constexpr uint32_t kSignatureRegAllLanesMask = 0x01u << kSignatureRegAllLanesShift;

  static inline constexpr uint32_t kSignatureRegElementFlagShift = 15;
  static inline constexpr uint32_t kSignatureRegElementFlagMask = 0x01u << kSignatureRegElementFlagShift;

  static inline constexpr uint32_t kSignatureRegElementIndexShift = 16;
  static inline constexpr uint32_t kSignatureRegElementIndexMask = 0x0Fu << kSignatureRegElementIndexShift;
  //! \endcond

  //! Creates a new 32-bit (S) vector register.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v32(uint32_t regId) noexcept { return Vec(_signatureOf<RegType::kVec32>(), regId); }
  //! Creates a new 64-bit (D) vector register.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v64(uint32_t regId) noexcept { return Vec(_signatureOf<RegType::kVec64>(), regId); }
  //! Creates a new 128-bit (Q) vector register.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v128(uint32_t regId) noexcept { return Vec(_signatureOf<RegType::kVec128>(), regId); }

  //! Creates a new S register.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_s(uint32_t regId) noexcept { return make_v32(regId); }
  //! Creates a new D register.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_d(uint32_t regId) noexcept { return make_v64(regId); }
  //! Creates a new Q register.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_q(uint32_t regId) noexcept { return make_v128(regId); }

  //! Tests whether the register is S register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isS() const noexcept { return isVec32(); }
  //! Tests whether the register is D register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isD() const noexcept { return isVec64(); }
  //! Tests whether the register is Q register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isQ() const noexcept { return isVec128(); }

  //! Tests whether the register has an element index (scalar), like `d0[1]`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool hasElementIndex() const noexcept { return _signature.hasField<kSignatureRegElementFlagMask>(); }
  //! Returns element index.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t elementIndex() const noexcept { return _signature.getField<kSignatureRegElementIndexMask>(); }
  //! Tests whether the register describes all lanes, like `d0[]`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isAllLanes() const noexcept { return _signature.hasField<kSignatureRegAllLanesMask>(); }
  //! Tests whether the register is a plain register (no element index, no all-lanes).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isPlain() const noexcept { return !_signature.hasField<kSignatureRegElementFlagMask | kSignatureRegAllLanesMask>(); }

  //! Returns a copy of this register with element index `index` (scalar), like `d0[index]`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec at(uint32_t index) const noexcept {
    return Vec(OperandSignature{(signature().bits() & ~(kSignatureRegElementIndexMask | kSignatureRegAllLanesMask)) |
                                kSignatureRegElementFlagMask | ((index & 0xFu) << kSignatureRegElementIndexShift)}, id());
  }

  //! Returns a copy of this register with element index `index` (scalar), like `d0[index]`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec operator[](uint32_t index) const noexcept { return at(index); }

  //! Returns a copy of this register that describes all lanes, like `d0[]` (used by VLDn to all lanes).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec all() const noexcept {
    return Vec(OperandSignature{(signature().bits() & ~(kSignatureRegElementIndexMask | kSignatureRegElementFlagMask)) |
                                kSignatureRegAllLanesMask}, id());
  }

  //! Returns a plain copy of this register (without element index / all-lanes).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec plain() const noexcept {
    return Vec(OperandSignature{signature().bits() & ~(kSignatureRegElementIndexMask | kSignatureRegElementFlagMask | kSignatureRegAllLanesMask)}, id());
  }

  //! Casts to S register of the same id.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec s() const noexcept { return make_v32(id()); }
  //! Casts to D register of the same id.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec d() const noexcept { return make_v64(id()); }
  //! Casts to Q register of the same id.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec q() const noexcept { return make_v128(id()); }

  //! Returns the low D register that overlaps with this Q register (`q1` -> `d2`).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec lo() const noexcept { return make_v64(id() * 2u); }
  //! Returns the high D register that overlaps with this Q register (`q1` -> `d3`).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec hi() const noexcept { return make_v64(id() * 2u + 1u); }
};

//! List of general purpose registers (used by LDM/STM/PUSH/POP).
class GpList : public RegListT<Gp> {
public:
  static inline constexpr uint32_t kSignature =
    OperandSignature::fromOpType(OperandType::kRegList).bits() |
    OperandSignature::fromRegTypeAndGroup(RegType::kGp32, RegGroup::kGp).bits();

  ASMJIT_INLINE_CONSTEXPR GpList() noexcept
    : RegListT<Gp>(OperandSignature{kSignature}, RegMask(0)) {}

  ASMJIT_INLINE_CONSTEXPR GpList(const GpList& other) noexcept = default;

  //! Creates a register list from a register mask (bit N represents rN).
  ASMJIT_INLINE_CONSTEXPR explicit GpList(RegMask mask) noexcept
    : RegListT<Gp>(OperandSignature{kSignature}, mask & 0xFFFFu) {}

  //! Creates a register list from a list of registers `{r0, r4, lr}`.
  ASMJIT_INLINE_NODEBUG GpList(std::initializer_list<Gp> regs) noexcept
    : RegListT<Gp>(OperandSignature{kSignature}, RegMask(0)) { addRegs(regs); }

  ASMJIT_INLINE_CONSTEXPR GpList& operator=(const GpList& other) noexcept = default;

  //! Creates a register list containing registers from `first` to `last` (inclusive).
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR GpList range(const Gp& first, const Gp& last) noexcept {
    uint32_t a = first.id() & 0xFu;
    uint32_t b = last.id() & 0xFu;
    uint32_t mask = 0;
    for (uint32_t i = a; i <= b; i++) {
      mask |= 1u << i;
    }
    return GpList(mask);
  }
};

//! List of consecutive vector registers of the same type (used by VLDM/VSTM/VPUSH/VPOP/VTBL/VTBX/VLDn/VSTn).
class VecList : public RegListT<Vec> {
public:
  ASMJIT_INLINE_CONSTEXPR VecList() noexcept
    : RegListT<Vec>() {}

  ASMJIT_INLINE_CONSTEXPR VecList(const VecList& other) noexcept = default;

  //! Creates a list of `count` consecutive registers starting at `first` (type of `first` is used).
  ASMJIT_INLINE_CONSTEXPR VecList(const Vec& first, uint32_t count) noexcept
    : RegListT<Vec>(OperandSignature::fromOpType(OperandType::kRegList) |
                    OperandSignature::fromRegTypeAndGroup(first.regType(), RegGroup::kVec),
                    makeMask(first.id(), count)) {}

  //! Creates a list from registers `{d8, d9, d10}` (all registers must be of the same type).
  ASMJIT_INLINE_NODEBUG VecList(std::initializer_list<Vec> regs) noexcept
    : RegListT<Vec>(OperandSignature::fromOpType(OperandType::kRegList) |
                    OperandSignature::fromRegTypeAndGroup(regs.size() ? regs.begin()->regType() : RegType::kVec64, RegGroup::kVec),
                    RegMask(0)) { addRegs(regs); }

  ASMJIT_INLINE_CONSTEXPR VecList& operator=(const VecList& other) noexcept = default;

  //! Creates a list containing registers from `first` to `last` (inclusive).
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR VecList range(const Vec& first, const Vec& last) noexcept {
    return VecList(first, last.id() >= first.id() ? last.id() - first.id() + 1u : 0u);
  }

  //! \cond INTERNAL
  static ASMJIT_INLINE_CONSTEXPR RegMask makeMask(uint32_t first, uint32_t count) noexcept {
    uint32_t mask = 0;
    for (uint32_t i = 0; i < count; i++) {
      if (first + i < 32u) {
        mask |= 1u << (first + i);
      }
    }
    return mask;
  }
  //! \endcond
};

//! Memory operand (AArch32).
//!
//! Supported addressing modes:
//!
//!   - `[base, #offset]` - \ref ptr(const Gp&, int32_t).
//!   - `[base, #offset]!` - \ref ptr_pre(const Gp&, int32_t).
//!   - `[base], #offset` - \ref ptr_post(const Gp&, int32_t).
//!   - `[base, +/-index {, shift}]` - \ref ptr(const Gp&, const Gp&, const Shift&), use \ref Mem::neg() to subtract.
//!   - `[base, +/-index {, shift}]!` and `[base], +/-index {, shift}` - pre/post variants.
//!   - `[label, #offset]` - PC relative (literal).
//!   - `[base :align]` - alignment of VLDn/VSTn, see \ref Mem::aligned().
class Mem : public BaseMem {
public:
  //! \cond INTERNAL

  // Index shift value (6 bits, AArch32 allows shift by 32 in LSR/ASR case).
  // |........|....XXXX|XX......|........|
  static inline constexpr uint32_t kSignatureMemShiftValueShift = 14;
  static inline constexpr uint32_t kSignatureMemShiftValueMask = 0x3Fu << kSignatureMemShiftValueShift;

  // Index shift operation (4 bits).
  // |........|XXXX....|........|........|
  static inline constexpr uint32_t kSignatureMemShiftOpShift = 20;
  static inline constexpr uint32_t kSignatureMemShiftOpMask = 0x0Fu << kSignatureMemShiftOpShift;

  // Offset mode type (2 bits).
  // |......XX|........|........|........|
  static inline constexpr uint32_t kSignatureMemOffsetModeShift = 24;
  static inline constexpr uint32_t kSignatureMemOffsetModeMask = 0x03u << kSignatureMemOffsetModeShift;

  // Index is subtracted from base (1 bit).
  // |.....X..|........|........|........|
  static inline constexpr uint32_t kSignatureMemNegIndexShift = 26;
  static inline constexpr uint32_t kSignatureMemNegIndexMask = 0x01u << kSignatureMemNegIndexShift;

  // Alignment (3 bits) - log2(alignment in bits) - 5 [0 = none, 1 = 64, 2 = 128, 3 = 256].
  // |..XXX...|........|........|........|
  static inline constexpr uint32_t kSignatureMemAlignShift = 27;
  static inline constexpr uint32_t kSignatureMemAlignMask = 0x07u << kSignatureMemAlignShift;

  //! \endcond

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_CONSTEXPR Mem() noexcept
    : BaseMem() {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Mem& other) noexcept
    : BaseMem(other) {}

  ASMJIT_INLINE_NODEBUG explicit Mem(Globals::NoInit_) noexcept
    : BaseMem(Globals::NoInit) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Signature& signature, uint32_t baseId, uint32_t indexId, int32_t offset) noexcept
    : BaseMem(signature, baseId, indexId, offset) {}

  ASMJIT_INLINE_CONSTEXPR explicit Mem(const Label& base, int32_t off = 0, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              Signature::fromMemBaseType(RegType::kLabelTag) |
              signature, base.id(), 0, off) {}

  ASMJIT_INLINE_CONSTEXPR explicit Mem(const Reg& base, int32_t off = 0, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              Signature::fromMemBaseType(base.regType()) |
              signature, base.id(), 0, off) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Reg& base, const Reg& index, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              Signature::fromMemBaseType(base.regType()) |
              Signature::fromMemIndexType(index.regType()) |
              signature, base.id(), index.id(), 0) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Reg& base, const Reg& index, const Shift& shift, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              Signature::fromMemBaseType(base.regType()) |
              Signature::fromMemIndexType(index.regType()) |
              Signature::fromValue<kSignatureMemShiftOpMask>(uint32_t(shift.op())) |
              Signature::fromValue<kSignatureMemShiftValueMask>(shift.value()) |
              signature, base.id(), index.id(), 0) {}

  //! Absolute address (encoded as PC relative when the base address is known or through relocation).
  ASMJIT_INLINE_CONSTEXPR explicit Mem(uint64_t base, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::fromOpType(OperandType::kMem) |
              signature, uint32_t(base >> 32), 0, int32_t(uint32_t(base & 0xFFFFFFFFu))) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_CONSTEXPR Mem& operator=(const Mem& other) noexcept {
    copyFrom(other);
    return *this;
  }

  //! \}

  //! \name Clone
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem clone() const noexcept { return Mem(*this); }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem cloneAdjusted(int64_t off) const noexcept {
    Mem result(*this);
    result.addOffset(off);
    return result;
  }

  //! Clones the memory operand and makes it pre-index (`[base, offset]!`).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem pre() const noexcept {
    Mem result(*this);
    result.setOffsetMode(OffsetMode::kPreIndex);
    return result;
  }

  //! Clones the memory operand, applies a given offset `off` and makes it pre-index.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem pre(int64_t off) const noexcept {
    Mem result(*this);
    result.setOffsetMode(OffsetMode::kPreIndex);
    result.addOffset(off);
    return result;
  }

  //! Clones the memory operand and makes it post-index (`[base], offset`).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem post() const noexcept {
    Mem result(*this);
    result.setOffsetMode(OffsetMode::kPostIndex);
    return result;
  }

  //! Clones the memory operand, applies a given offset `off` and makes it post-index.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem post(int64_t off) const noexcept {
    Mem result(*this);
    result.setOffsetMode(OffsetMode::kPostIndex);
    result.addOffset(off);
    return result;
  }

  //! Clones the memory operand and makes the index register subtracted from base (`[base, -index]`).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem neg() const noexcept {
    Mem result(*this);
    result.setNegIndex(true);
    return result;
  }

  //! Clones the memory operand and sets its alignment to `alignInBits` (64, 128, or 256) - `[base :align]`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem aligned(uint32_t alignInBits) const noexcept {
    Mem result(*this);
    result.setAlignment(alignInBits);
    return result;
  }

  //! \}

  //! \name Base & Index
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Reg baseReg() const noexcept { return Reg::fromTypeAndId(baseType(), baseId()); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Reg indexReg() const noexcept { return Reg::fromTypeAndId(indexType(), indexId()); }

  using BaseMem::setIndex;

  ASMJIT_INLINE_CONSTEXPR void setIndex(const Reg& index, uint32_t shift) noexcept {
    setIndex(index);
    setShift(shift);
  }

  ASMJIT_INLINE_CONSTEXPR void setIndex(const Reg& index, Shift shift) noexcept {
    setIndex(index);
    setShift(shift);
  }

  //! \}

  //! \name ARM Specific Features
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR OffsetMode offsetMode() const noexcept { return OffsetMode(_signature.getField<kSignatureMemOffsetModeMask>()); }
  ASMJIT_INLINE_CONSTEXPR void setOffsetMode(OffsetMode mode) noexcept { _signature.setField<kSignatureMemOffsetModeMask>(uint32_t(mode)); }
  ASMJIT_INLINE_CONSTEXPR void resetOffsetMode() noexcept { _signature.setField<kSignatureMemOffsetModeMask>(uint32_t(OffsetMode::kFixed)); }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isFixedOffset() const noexcept { return offsetMode() == OffsetMode::kFixed; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isPreOrPost() const noexcept { return offsetMode() != OffsetMode::kFixed; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isPreIndex() const noexcept { return offsetMode() == OffsetMode::kPreIndex; }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isPostIndex() const noexcept { return offsetMode() == OffsetMode::kPostIndex; }

  ASMJIT_INLINE_CONSTEXPR void makePreIndex() noexcept { setOffsetMode(OffsetMode::kPreIndex); }
  ASMJIT_INLINE_CONSTEXPR void makePostIndex() noexcept { setOffsetMode(OffsetMode::kPostIndex); }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR ShiftOp shiftOp() const noexcept { return ShiftOp(_signature.getField<kSignatureMemShiftOpMask>()); }
  ASMJIT_INLINE_CONSTEXPR void setShiftOp(ShiftOp sop) noexcept { _signature.setField<kSignatureMemShiftOpMask>(uint32_t(sop)); }
  ASMJIT_INLINE_CONSTEXPR void resetShiftOp() noexcept { _signature.setField<kSignatureMemShiftOpMask>(uint32_t(ShiftOp::kLSL)); }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool hasShift() const noexcept { return _signature.hasField<kSignatureMemShiftValueMask>(); }
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t shift() const noexcept { return _signature.getField<kSignatureMemShiftValueMask>(); }
  ASMJIT_INLINE_CONSTEXPR void setShift(uint32_t shift) noexcept { _signature.setField<kSignatureMemShiftValueMask>(shift); }

  ASMJIT_INLINE_CONSTEXPR void setShift(Shift shift) noexcept {
    _signature.setField<kSignatureMemShiftOpMask>(uint32_t(shift.op()));
    _signature.setField<kSignatureMemShiftValueMask>(shift.value());
  }

  ASMJIT_INLINE_CONSTEXPR void resetShift() noexcept { _signature.setField<kSignatureMemShiftValueMask>(0); }

  //! Tests whether the index register is subtracted from the base register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool isNegIndex() const noexcept { return _signature.hasField<kSignatureMemNegIndexMask>(); }
  ASMJIT_INLINE_CONSTEXPR void setNegIndex(bool value) noexcept { _signature.setField<kSignatureMemNegIndexMask>(uint32_t(value)); }

  //! Returns alignment in bits (0 if not specified).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t alignment() const noexcept {
    uint32_t v = _signature.getField<kSignatureMemAlignMask>();
    return v == 7u ? 16u : v == 6u ? 32u : v ? (1u << (v + 5u)) : 0u;
  }

  //! Sets alignment in bits (valid values are 0 [none], 16, 32, 64, 128, 256).
  ASMJIT_INLINE_CONSTEXPR void setAlignment(uint32_t alignInBits) noexcept {
    uint32_t v = alignInBits == 16  ? 7u :
                 alignInBits == 32  ? 6u :
                 alignInBits == 64  ? 1u :
                 alignInBits == 128 ? 2u :
                 alignInBits == 256 ? 3u : 0u;
    _signature.setField<kSignatureMemAlignMask>(v);
  }

  //! Returns the raw alignment field (internal).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t alignmentField() const noexcept { return _signature.getField<kSignatureMemAlignMask>(); }

  //! \}
};

//! Returns the alignment in bits of the given alignment field.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR uint32_t memAlignFieldToBits(uint32_t v) noexcept {
  return v == 7u ? 16u : v == 6u ? 32u : v ? (1u << (v + 5u)) : 0u;
}

#ifndef _DOXYGEN
namespace regs {
#endif

//! Creates a 32-bit general purpose register (`r`).
static ASMJIT_INLINE_CONSTEXPR Gp r(uint32_t id) noexcept { return Gp::make_r32(id); }
//! Creates a 32-bit general purpose register (`r`).
static ASMJIT_INLINE_CONSTEXPR Gp gp32(uint32_t id) noexcept { return Gp::make_r32(id); }
//! Creates a 32-bit S register.
static ASMJIT_INLINE_CONSTEXPR Vec s(uint32_t id) noexcept { return Vec::make_v32(id); }
//! Creates a 64-bit D register.
static ASMJIT_INLINE_CONSTEXPR Vec d(uint32_t id) noexcept { return Vec::make_v64(id); }
//! Creates a 128-bit Q register.
static ASMJIT_INLINE_CONSTEXPR Vec q(uint32_t id) noexcept { return Vec::make_v128(id); }

static constexpr Gp r0 = Gp::make_r32(0);
static constexpr Gp r1 = Gp::make_r32(1);
static constexpr Gp r2 = Gp::make_r32(2);
static constexpr Gp r3 = Gp::make_r32(3);
static constexpr Gp r4 = Gp::make_r32(4);
static constexpr Gp r5 = Gp::make_r32(5);
static constexpr Gp r6 = Gp::make_r32(6);
static constexpr Gp r7 = Gp::make_r32(7);
static constexpr Gp r8 = Gp::make_r32(8);
static constexpr Gp r9 = Gp::make_r32(9);
static constexpr Gp r10 = Gp::make_r32(10);
static constexpr Gp r11 = Gp::make_r32(11);
static constexpr Gp r12 = Gp::make_r32(12);
static constexpr Gp r13 = Gp::make_r32(13);
static constexpr Gp r14 = Gp::make_r32(14);
static constexpr Gp r15 = Gp::make_r32(15);

static constexpr Gp sb = Gp::make_r32(Gp::kIdSb);
static constexpr Gp fp = Gp::make_r32(Gp::kIdFp);
static constexpr Gp ip = Gp::make_r32(Gp::kIdIp);
static constexpr Gp sp = Gp::make_r32(Gp::kIdSp);
static constexpr Gp lr = Gp::make_r32(Gp::kIdLr);
static constexpr Gp pc = Gp::make_r32(Gp::kIdPc);

//! `APSR_nzcv` destination of `VMRS` (encoded as r15).
static constexpr Gp apsr_nzcv = Gp::make_r32(Gp::kIdPc);

static constexpr Vec s0 = Vec::make_v32(0);
static constexpr Vec s1 = Vec::make_v32(1);
static constexpr Vec s2 = Vec::make_v32(2);
static constexpr Vec s3 = Vec::make_v32(3);
static constexpr Vec s4 = Vec::make_v32(4);
static constexpr Vec s5 = Vec::make_v32(5);
static constexpr Vec s6 = Vec::make_v32(6);
static constexpr Vec s7 = Vec::make_v32(7);
static constexpr Vec s8 = Vec::make_v32(8);
static constexpr Vec s9 = Vec::make_v32(9);
static constexpr Vec s10 = Vec::make_v32(10);
static constexpr Vec s11 = Vec::make_v32(11);
static constexpr Vec s12 = Vec::make_v32(12);
static constexpr Vec s13 = Vec::make_v32(13);
static constexpr Vec s14 = Vec::make_v32(14);
static constexpr Vec s15 = Vec::make_v32(15);
static constexpr Vec s16 = Vec::make_v32(16);
static constexpr Vec s17 = Vec::make_v32(17);
static constexpr Vec s18 = Vec::make_v32(18);
static constexpr Vec s19 = Vec::make_v32(19);
static constexpr Vec s20 = Vec::make_v32(20);
static constexpr Vec s21 = Vec::make_v32(21);
static constexpr Vec s22 = Vec::make_v32(22);
static constexpr Vec s23 = Vec::make_v32(23);
static constexpr Vec s24 = Vec::make_v32(24);
static constexpr Vec s25 = Vec::make_v32(25);
static constexpr Vec s26 = Vec::make_v32(26);
static constexpr Vec s27 = Vec::make_v32(27);
static constexpr Vec s28 = Vec::make_v32(28);
static constexpr Vec s29 = Vec::make_v32(29);
static constexpr Vec s30 = Vec::make_v32(30);
static constexpr Vec s31 = Vec::make_v32(31);

static constexpr Vec d0 = Vec::make_v64(0);
static constexpr Vec d1 = Vec::make_v64(1);
static constexpr Vec d2 = Vec::make_v64(2);
static constexpr Vec d3 = Vec::make_v64(3);
static constexpr Vec d4 = Vec::make_v64(4);
static constexpr Vec d5 = Vec::make_v64(5);
static constexpr Vec d6 = Vec::make_v64(6);
static constexpr Vec d7 = Vec::make_v64(7);
static constexpr Vec d8 = Vec::make_v64(8);
static constexpr Vec d9 = Vec::make_v64(9);
static constexpr Vec d10 = Vec::make_v64(10);
static constexpr Vec d11 = Vec::make_v64(11);
static constexpr Vec d12 = Vec::make_v64(12);
static constexpr Vec d13 = Vec::make_v64(13);
static constexpr Vec d14 = Vec::make_v64(14);
static constexpr Vec d15 = Vec::make_v64(15);
static constexpr Vec d16 = Vec::make_v64(16);
static constexpr Vec d17 = Vec::make_v64(17);
static constexpr Vec d18 = Vec::make_v64(18);
static constexpr Vec d19 = Vec::make_v64(19);
static constexpr Vec d20 = Vec::make_v64(20);
static constexpr Vec d21 = Vec::make_v64(21);
static constexpr Vec d22 = Vec::make_v64(22);
static constexpr Vec d23 = Vec::make_v64(23);
static constexpr Vec d24 = Vec::make_v64(24);
static constexpr Vec d25 = Vec::make_v64(25);
static constexpr Vec d26 = Vec::make_v64(26);
static constexpr Vec d27 = Vec::make_v64(27);
static constexpr Vec d28 = Vec::make_v64(28);
static constexpr Vec d29 = Vec::make_v64(29);
static constexpr Vec d30 = Vec::make_v64(30);
static constexpr Vec d31 = Vec::make_v64(31);

static constexpr Vec q0 = Vec::make_v128(0);
static constexpr Vec q1 = Vec::make_v128(1);
static constexpr Vec q2 = Vec::make_v128(2);
static constexpr Vec q3 = Vec::make_v128(3);
static constexpr Vec q4 = Vec::make_v128(4);
static constexpr Vec q5 = Vec::make_v128(5);
static constexpr Vec q6 = Vec::make_v128(6);
static constexpr Vec q7 = Vec::make_v128(7);
static constexpr Vec q8 = Vec::make_v128(8);
static constexpr Vec q9 = Vec::make_v128(9);
static constexpr Vec q10 = Vec::make_v128(10);
static constexpr Vec q11 = Vec::make_v128(11);
static constexpr Vec q12 = Vec::make_v128(12);
static constexpr Vec q13 = Vec::make_v128(13);
static constexpr Vec q14 = Vec::make_v128(14);
static constexpr Vec q15 = Vec::make_v128(15);

#ifndef _DOXYGEN
} // {regs}

// Make `a32::regs` accessible through `a32` namespace as well.
using namespace regs;
#endif

//! \name Shift Operation Construction
//! \{

//! Constructs a `LSL #value` shift (logical shift left).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift lsl(uint32_t value) noexcept { return Shift(ShiftOp::kLSL, value); }
//! Constructs a `LSR #value` shift (logical shift right).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift lsr(uint32_t value) noexcept { return Shift(ShiftOp::kLSR, value); }
//! Constructs a `ASR #value` shift (arithmetic shift right).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift asr(uint32_t value) noexcept { return Shift(ShiftOp::kASR, value); }
//! Constructs a `ROR #value` shift (rotate right).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift ror(uint32_t value) noexcept { return Shift(ShiftOp::kROR, value); }
//! Constructs a `RRX` shift (rotate right with extend by 1 bit).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift rrx() noexcept { return Shift(ShiftOp::kRRX, 0); }

//! Constructs a `LSL <Rs>` register shift (register-shifted-register operand).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp lsl(const Gp& rs) noexcept { return rs.withShiftOp(ShiftOp::kLSL); }
//! Constructs a `LSR <Rs>` register shift (register-shifted-register operand).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp lsr(const Gp& rs) noexcept { return rs.withShiftOp(ShiftOp::kLSR); }
//! Constructs a `ASR <Rs>` register shift (register-shifted-register operand).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp asr(const Gp& rs) noexcept { return rs.withShiftOp(ShiftOp::kASR); }
//! Constructs a `ROR <Rs>` register shift (register-shifted-register operand).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp ror(const Gp& rs) noexcept { return rs.withShiftOp(ShiftOp::kROR); }

//! \}

//! \name Memory Operand Construction
//! \{

//! Creates `[base, #offset]` memory operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, int32_t offset = 0) noexcept {
  return Mem(base, offset);
}

//! Creates `[base, #offset]!` memory operand (pre-index mode).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_pre(const Gp& base, int32_t offset = 0) noexcept {
  return Mem(base, offset, OperandSignature::fromValue<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPreIndex));
}

//! Creates `[base], #offset` memory operand (post-index mode).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_post(const Gp& base, int32_t offset = 0) noexcept {
  return Mem(base, offset, OperandSignature::fromValue<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPostIndex));
}

//! Creates `[base, index]` memory operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, const Gp& index) noexcept {
  return Mem(base, index);
}

//! Creates `[base, index]!` memory operand (pre-index mode).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_pre(const Gp& base, const Gp& index) noexcept {
  return Mem(base, index, OperandSignature::fromValue<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPreIndex));
}

//! Creates `[base], index` memory operand (post-index mode).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_post(const Gp& base, const Gp& index) noexcept {
  return Mem(base, index, OperandSignature::fromValue<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPostIndex));
}

//! Creates `[base, index, SHIFT_OP #shift]` memory operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, const Gp& index, const Shift& shift) noexcept {
  return Mem(base, index, shift);
}

//! Creates `[base, index, SHIFT_OP #shift]!` memory operand (pre-index mode).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_pre(const Gp& base, const Gp& index, const Shift& shift) noexcept {
  return Mem(base, index, shift, OperandSignature::fromValue<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPreIndex));
}

//! Creates `[base], index, SHIFT_OP #shift` memory operand (post-index mode).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_post(const Gp& base, const Gp& index, const Shift& shift) noexcept {
  return Mem(base, index, shift, OperandSignature::fromValue<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPostIndex));
}

//! Creates `[label, #offset]` (PC relative) memory operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Label& base, int32_t offset = 0) noexcept {
  return Mem(base, offset);
}

//! Creates `[base]` absolute memory operand (encoded as PC relative).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(uint64_t base) noexcept { return Mem(base); }

//! \}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A32OPERAND_H_INCLUDED
