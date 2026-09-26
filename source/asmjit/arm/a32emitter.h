// This file is based on part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A32EMITTER_H_INCLUDED
#define ASMJIT_ARM_A32EMITTER_H_INCLUDED

#include "../core/emitter.h"
#include "../core/support.h"
#include "../arm/a32globals.h"
#include "../arm/a32operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

//! \addtogroup asmjit_a32
//! \{

//! AArch32 emitter (A32 instruction set).
//!
//! Conventions:
//!
//!   - Every conditional instruction provides an overload that accepts \ref CondCode as the first argument, for
//!     example `a.add(CondCode::kEQ, r0, r1, 1)` emits `addeq r0, r1, #1`. Branches also provide suffixed
//!     variants like `b_eq(label)`.
//!
//!   - VFP and ASIMD instructions accept \ref DataType as the first argument (after an optional \ref CondCode),
//!     for example `a.vadd(DataType::kF32, q0, q1, q2)` emits `vadd.f32 q0, q1, q2`. VFP instructions that use
//!     S or D registers default to F32 / F64 when no data type is given. Conversions (VCVT) use two data types
//!     (destination and source) - `a.vcvt(DataType::kS32, DataType::kF32, s0, s1)` is `vcvt.s32.f32 s0, s1`.
//!     Sign agnostic integer types (`.i8`, `.i16`, `.i32`, `.i64`) are available as `DataType::kI8`, etc...
//!
//!   - Data processing instructions accept a flexible second operand: an immediate, a register, a register with
//!     an immediate shift `a.add(r0, r1, r2, lsl(3))`, or a register shifted by register `a.add(r0, r1, r2, lsl(r3))`.
//!     Immediates that cannot be encoded are automatically converted to the complementary instruction when
//!     possible (ADD <-> SUB, AND <-> BIC, ADC <-> SBC, CMP <-> CMN, MOV <-> MVN, and MOV #imm16 -> MOVW).
//!
//!   - LDM/STM/VLDM/VSTM accept either a register (no write-back) or a memory operand with pre/post index mode
//!     (write-back), for example `a.ldm(ptr_post(r0), GpList{r1, r2})` is `ldm r0!, {r1, r2}`.
//!
//!   - Branch targets, ADR and literal loads accept either a \ref Label or an absolute address given as \ref Imm
//!     (it's encoded relative to the base address of the \ref CodeHolder, or a relocation is created).
template<typename This>
struct EmitterExplicitT {
  //! \cond

  // These two are unfortunately reported by the sanitizer. We know what we do, however, the sanitizer doesn't.
  ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF ASMJIT_INLINE_NODEBUG This* _emitter() noexcept { return static_cast<This*>(this); }
  ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF ASMJIT_INLINE_NODEBUG const This* _emitter() const noexcept { return static_cast<const This*>(this); }

  //! \endcond

  //! \name Instructions
  //! \{

  // Data Processing

  inline Error adc(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdAdc, o0, o1, o2); }
  inline Error adc(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdc, cc), o0, o1, o2); }
  inline Error adc(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdAdc, o0, o1, o2); }
  inline Error adc(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdc, cc), o0, o1, o2); }
  inline Error adc(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdAdc, o0, o1, o2, o3); }
  inline Error adc(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdc, cc), o0, o1, o2, o3); }
  inline Error adc(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdAdc, o0, o1, o2, o3); }
  inline Error adc(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdc, cc), o0, o1, o2, o3); }
  inline Error adc(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdAdc, o0, o1); }
  inline Error adc(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdc, cc), o0, o1); }
  inline Error adc(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdAdc, o0, o1); }
  inline Error adc(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdc, cc), o0, o1); }
  inline Error adcs(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdAdcs, o0, o1, o2); }
  inline Error adcs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdcs, cc), o0, o1, o2); }
  inline Error adcs(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdAdcs, o0, o1, o2); }
  inline Error adcs(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdcs, cc), o0, o1, o2); }
  inline Error adcs(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdAdcs, o0, o1, o2, o3); }
  inline Error adcs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdcs, cc), o0, o1, o2, o3); }
  inline Error adcs(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdAdcs, o0, o1, o2, o3); }
  inline Error adcs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdcs, cc), o0, o1, o2, o3); }
  inline Error adcs(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdAdcs, o0, o1); }
  inline Error adcs(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdcs, cc), o0, o1); }
  inline Error adcs(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdAdcs, o0, o1); }
  inline Error adcs(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdcs, cc), o0, o1); }
  inline Error add(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdAdd, o0, o1, o2); }
  inline Error add(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdd, cc), o0, o1, o2); }
  inline Error add(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdAdd, o0, o1, o2); }
  inline Error add(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdd, cc), o0, o1, o2); }
  inline Error add(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdAdd, o0, o1, o2, o3); }
  inline Error add(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdd, cc), o0, o1, o2, o3); }
  inline Error add(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdAdd, o0, o1, o2, o3); }
  inline Error add(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdd, cc), o0, o1, o2, o3); }
  inline Error add(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdAdd, o0, o1); }
  inline Error add(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdd, cc), o0, o1); }
  inline Error add(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdAdd, o0, o1); }
  inline Error add(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdd, cc), o0, o1); }
  inline Error adds(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdAdds, o0, o1, o2); }
  inline Error adds(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdds, cc), o0, o1, o2); }
  inline Error adds(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdAdds, o0, o1, o2); }
  inline Error adds(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdds, cc), o0, o1, o2); }
  inline Error adds(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdAdds, o0, o1, o2, o3); }
  inline Error adds(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdds, cc), o0, o1, o2, o3); }
  inline Error adds(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdAdds, o0, o1, o2, o3); }
  inline Error adds(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdds, cc), o0, o1, o2, o3); }
  inline Error adds(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdAdds, o0, o1); }
  inline Error adds(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdds, cc), o0, o1); }
  inline Error adds(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdAdds, o0, o1); }
  inline Error adds(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdds, cc), o0, o1); }
  inline Error and_(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdAnd, o0, o1, o2); }
  inline Error and_(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAnd, cc), o0, o1, o2); }
  inline Error and_(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdAnd, o0, o1, o2); }
  inline Error and_(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAnd, cc), o0, o1, o2); }
  inline Error and_(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdAnd, o0, o1, o2, o3); }
  inline Error and_(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAnd, cc), o0, o1, o2, o3); }
  inline Error and_(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdAnd, o0, o1, o2, o3); }
  inline Error and_(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAnd, cc), o0, o1, o2, o3); }
  inline Error and_(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdAnd, o0, o1); }
  inline Error and_(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAnd, cc), o0, o1); }
  inline Error and_(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdAnd, o0, o1); }
  inline Error and_(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAnd, cc), o0, o1); }
  inline Error ands(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdAnds, o0, o1, o2); }
  inline Error ands(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAnds, cc), o0, o1, o2); }
  inline Error ands(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdAnds, o0, o1, o2); }
  inline Error ands(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAnds, cc), o0, o1, o2); }
  inline Error ands(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdAnds, o0, o1, o2, o3); }
  inline Error ands(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAnds, cc), o0, o1, o2, o3); }
  inline Error ands(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdAnds, o0, o1, o2, o3); }
  inline Error ands(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAnds, cc), o0, o1, o2, o3); }
  inline Error ands(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdAnds, o0, o1); }
  inline Error ands(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAnds, cc), o0, o1); }
  inline Error ands(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdAnds, o0, o1); }
  inline Error ands(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAnds, cc), o0, o1); }
  inline Error bic(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdBic, o0, o1, o2); }
  inline Error bic(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBic, cc), o0, o1, o2); }
  inline Error bic(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdBic, o0, o1, o2); }
  inline Error bic(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBic, cc), o0, o1, o2); }
  inline Error bic(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdBic, o0, o1, o2, o3); }
  inline Error bic(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBic, cc), o0, o1, o2, o3); }
  inline Error bic(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdBic, o0, o1, o2, o3); }
  inline Error bic(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBic, cc), o0, o1, o2, o3); }
  inline Error bic(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdBic, o0, o1); }
  inline Error bic(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBic, cc), o0, o1); }
  inline Error bic(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdBic, o0, o1); }
  inline Error bic(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBic, cc), o0, o1); }
  inline Error bics(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdBics, o0, o1, o2); }
  inline Error bics(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBics, cc), o0, o1, o2); }
  inline Error bics(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdBics, o0, o1, o2); }
  inline Error bics(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBics, cc), o0, o1, o2); }
  inline Error bics(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdBics, o0, o1, o2, o3); }
  inline Error bics(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBics, cc), o0, o1, o2, o3); }
  inline Error bics(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdBics, o0, o1, o2, o3); }
  inline Error bics(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBics, cc), o0, o1, o2, o3); }
  inline Error bics(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdBics, o0, o1); }
  inline Error bics(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBics, cc), o0, o1); }
  inline Error bics(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdBics, o0, o1); }
  inline Error bics(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBics, cc), o0, o1); }
  inline Error eor(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdEor, o0, o1, o2); }
  inline Error eor(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEor, cc), o0, o1, o2); }
  inline Error eor(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdEor, o0, o1, o2); }
  inline Error eor(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEor, cc), o0, o1, o2); }
  inline Error eor(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdEor, o0, o1, o2, o3); }
  inline Error eor(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEor, cc), o0, o1, o2, o3); }
  inline Error eor(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdEor, o0, o1, o2, o3); }
  inline Error eor(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEor, cc), o0, o1, o2, o3); }
  inline Error eor(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdEor, o0, o1); }
  inline Error eor(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEor, cc), o0, o1); }
  inline Error eor(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdEor, o0, o1); }
  inline Error eor(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEor, cc), o0, o1); }
  inline Error eors(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdEors, o0, o1, o2); }
  inline Error eors(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEors, cc), o0, o1, o2); }
  inline Error eors(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdEors, o0, o1, o2); }
  inline Error eors(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEors, cc), o0, o1, o2); }
  inline Error eors(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdEors, o0, o1, o2, o3); }
  inline Error eors(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEors, cc), o0, o1, o2, o3); }
  inline Error eors(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdEors, o0, o1, o2, o3); }
  inline Error eors(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEors, cc), o0, o1, o2, o3); }
  inline Error eors(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdEors, o0, o1); }
  inline Error eors(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEors, cc), o0, o1); }
  inline Error eors(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdEors, o0, o1); }
  inline Error eors(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEors, cc), o0, o1); }
  inline Error orr(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdOrr, o0, o1, o2); }
  inline Error orr(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdOrr, cc), o0, o1, o2); }
  inline Error orr(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdOrr, o0, o1, o2); }
  inline Error orr(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdOrr, cc), o0, o1, o2); }
  inline Error orr(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdOrr, o0, o1, o2, o3); }
  inline Error orr(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdOrr, cc), o0, o1, o2, o3); }
  inline Error orr(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdOrr, o0, o1, o2, o3); }
  inline Error orr(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdOrr, cc), o0, o1, o2, o3); }
  inline Error orr(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdOrr, o0, o1); }
  inline Error orr(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdOrr, cc), o0, o1); }
  inline Error orr(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdOrr, o0, o1); }
  inline Error orr(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdOrr, cc), o0, o1); }
  inline Error orrs(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdOrrs, o0, o1, o2); }
  inline Error orrs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdOrrs, cc), o0, o1, o2); }
  inline Error orrs(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdOrrs, o0, o1, o2); }
  inline Error orrs(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdOrrs, cc), o0, o1, o2); }
  inline Error orrs(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdOrrs, o0, o1, o2, o3); }
  inline Error orrs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdOrrs, cc), o0, o1, o2, o3); }
  inline Error orrs(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdOrrs, o0, o1, o2, o3); }
  inline Error orrs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdOrrs, cc), o0, o1, o2, o3); }
  inline Error orrs(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdOrrs, o0, o1); }
  inline Error orrs(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdOrrs, cc), o0, o1); }
  inline Error orrs(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdOrrs, o0, o1); }
  inline Error orrs(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdOrrs, cc), o0, o1); }
  inline Error rsb(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdRsb, o0, o1, o2); }
  inline Error rsb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsb, cc), o0, o1, o2); }
  inline Error rsb(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdRsb, o0, o1, o2); }
  inline Error rsb(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsb, cc), o0, o1, o2); }
  inline Error rsb(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdRsb, o0, o1, o2, o3); }
  inline Error rsb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsb, cc), o0, o1, o2, o3); }
  inline Error rsb(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdRsb, o0, o1, o2, o3); }
  inline Error rsb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsb, cc), o0, o1, o2, o3); }
  inline Error rsb(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdRsb, o0, o1); }
  inline Error rsb(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsb, cc), o0, o1); }
  inline Error rsb(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdRsb, o0, o1); }
  inline Error rsb(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsb, cc), o0, o1); }
  inline Error rsbs(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdRsbs, o0, o1, o2); }
  inline Error rsbs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsbs, cc), o0, o1, o2); }
  inline Error rsbs(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdRsbs, o0, o1, o2); }
  inline Error rsbs(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsbs, cc), o0, o1, o2); }
  inline Error rsbs(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdRsbs, o0, o1, o2, o3); }
  inline Error rsbs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsbs, cc), o0, o1, o2, o3); }
  inline Error rsbs(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdRsbs, o0, o1, o2, o3); }
  inline Error rsbs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsbs, cc), o0, o1, o2, o3); }
  inline Error rsbs(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdRsbs, o0, o1); }
  inline Error rsbs(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsbs, cc), o0, o1); }
  inline Error rsbs(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdRsbs, o0, o1); }
  inline Error rsbs(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsbs, cc), o0, o1); }
  inline Error rsc(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdRsc, o0, o1, o2); }
  inline Error rsc(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsc, cc), o0, o1, o2); }
  inline Error rsc(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdRsc, o0, o1, o2); }
  inline Error rsc(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsc, cc), o0, o1, o2); }
  inline Error rsc(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdRsc, o0, o1, o2, o3); }
  inline Error rsc(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsc, cc), o0, o1, o2, o3); }
  inline Error rsc(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdRsc, o0, o1, o2, o3); }
  inline Error rsc(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsc, cc), o0, o1, o2, o3); }
  inline Error rsc(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdRsc, o0, o1); }
  inline Error rsc(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsc, cc), o0, o1); }
  inline Error rsc(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdRsc, o0, o1); }
  inline Error rsc(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRsc, cc), o0, o1); }
  inline Error rscs(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdRscs, o0, o1, o2); }
  inline Error rscs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRscs, cc), o0, o1, o2); }
  inline Error rscs(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdRscs, o0, o1, o2); }
  inline Error rscs(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRscs, cc), o0, o1, o2); }
  inline Error rscs(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdRscs, o0, o1, o2, o3); }
  inline Error rscs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRscs, cc), o0, o1, o2, o3); }
  inline Error rscs(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdRscs, o0, o1, o2, o3); }
  inline Error rscs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRscs, cc), o0, o1, o2, o3); }
  inline Error rscs(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdRscs, o0, o1); }
  inline Error rscs(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRscs, cc), o0, o1); }
  inline Error rscs(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdRscs, o0, o1); }
  inline Error rscs(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRscs, cc), o0, o1); }
  inline Error sbc(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSbc, o0, o1, o2); }
  inline Error sbc(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbc, cc), o0, o1, o2); }
  inline Error sbc(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdSbc, o0, o1, o2); }
  inline Error sbc(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbc, cc), o0, o1, o2); }
  inline Error sbc(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdSbc, o0, o1, o2, o3); }
  inline Error sbc(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbc, cc), o0, o1, o2, o3); }
  inline Error sbc(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSbc, o0, o1, o2, o3); }
  inline Error sbc(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbc, cc), o0, o1, o2, o3); }
  inline Error sbc(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdSbc, o0, o1); }
  inline Error sbc(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbc, cc), o0, o1); }
  inline Error sbc(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdSbc, o0, o1); }
  inline Error sbc(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbc, cc), o0, o1); }
  inline Error sbcs(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSbcs, o0, o1, o2); }
  inline Error sbcs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbcs, cc), o0, o1, o2); }
  inline Error sbcs(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdSbcs, o0, o1, o2); }
  inline Error sbcs(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbcs, cc), o0, o1, o2); }
  inline Error sbcs(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdSbcs, o0, o1, o2, o3); }
  inline Error sbcs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbcs, cc), o0, o1, o2, o3); }
  inline Error sbcs(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSbcs, o0, o1, o2, o3); }
  inline Error sbcs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbcs, cc), o0, o1, o2, o3); }
  inline Error sbcs(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdSbcs, o0, o1); }
  inline Error sbcs(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbcs, cc), o0, o1); }
  inline Error sbcs(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdSbcs, o0, o1); }
  inline Error sbcs(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbcs, cc), o0, o1); }
  inline Error sub(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSub, o0, o1, o2); }
  inline Error sub(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSub, cc), o0, o1, o2); }
  inline Error sub(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdSub, o0, o1, o2); }
  inline Error sub(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSub, cc), o0, o1, o2); }
  inline Error sub(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdSub, o0, o1, o2, o3); }
  inline Error sub(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSub, cc), o0, o1, o2, o3); }
  inline Error sub(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSub, o0, o1, o2, o3); }
  inline Error sub(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSub, cc), o0, o1, o2, o3); }
  inline Error sub(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdSub, o0, o1); }
  inline Error sub(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSub, cc), o0, o1); }
  inline Error sub(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdSub, o0, o1); }
  inline Error sub(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSub, cc), o0, o1); }
  inline Error subs(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSubs, o0, o1, o2); }
  inline Error subs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSubs, cc), o0, o1, o2); }
  inline Error subs(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdSubs, o0, o1, o2); }
  inline Error subs(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSubs, cc), o0, o1, o2); }
  inline Error subs(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdSubs, o0, o1, o2, o3); }
  inline Error subs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSubs, cc), o0, o1, o2, o3); }
  inline Error subs(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSubs, o0, o1, o2, o3); }
  inline Error subs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSubs, cc), o0, o1, o2, o3); }
  inline Error subs(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdSubs, o0, o1); }
  inline Error subs(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSubs, cc), o0, o1); }
  inline Error subs(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdSubs, o0, o1); }
  inline Error subs(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSubs, cc), o0, o1); }
  inline Error mov(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdMov, o0, o1); }
  inline Error mov(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMov, cc), o0, o1); }
  inline Error mov(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdMov, o0, o1); }
  inline Error mov(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMov, cc), o0, o1); }
  inline Error mov(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdMov, o0, o1, o2); }
  inline Error mov(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMov, cc), o0, o1, o2); }
  inline Error mov(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdMov, o0, o1, o2); }
  inline Error mov(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMov, cc), o0, o1, o2); }
  inline Error movs(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdMovs, o0, o1); }
  inline Error movs(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMovs, cc), o0, o1); }
  inline Error movs(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdMovs, o0, o1); }
  inline Error movs(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMovs, cc), o0, o1); }
  inline Error movs(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdMovs, o0, o1, o2); }
  inline Error movs(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMovs, cc), o0, o1, o2); }
  inline Error movs(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdMovs, o0, o1, o2); }
  inline Error movs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMovs, cc), o0, o1, o2); }
  inline Error mvn(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdMvn, o0, o1); }
  inline Error mvn(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMvn, cc), o0, o1); }
  inline Error mvn(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdMvn, o0, o1); }
  inline Error mvn(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMvn, cc), o0, o1); }
  inline Error mvn(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdMvn, o0, o1, o2); }
  inline Error mvn(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMvn, cc), o0, o1, o2); }
  inline Error mvn(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdMvn, o0, o1, o2); }
  inline Error mvn(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMvn, cc), o0, o1, o2); }
  inline Error mvns(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdMvns, o0, o1); }
  inline Error mvns(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMvns, cc), o0, o1); }
  inline Error mvns(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdMvns, o0, o1); }
  inline Error mvns(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMvns, cc), o0, o1); }
  inline Error mvns(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdMvns, o0, o1, o2); }
  inline Error mvns(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMvns, cc), o0, o1, o2); }
  inline Error mvns(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdMvns, o0, o1, o2); }
  inline Error mvns(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMvns, cc), o0, o1, o2); }
  inline Error cmp(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdCmp, o0, o1); }
  inline Error cmp(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCmp, cc), o0, o1); }
  inline Error cmp(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdCmp, o0, o1); }
  inline Error cmp(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCmp, cc), o0, o1); }
  inline Error cmp(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdCmp, o0, o1, o2); }
  inline Error cmp(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCmp, cc), o0, o1, o2); }
  inline Error cmp(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdCmp, o0, o1, o2); }
  inline Error cmp(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCmp, cc), o0, o1, o2); }
  inline Error cmn(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdCmn, o0, o1); }
  inline Error cmn(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCmn, cc), o0, o1); }
  inline Error cmn(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdCmn, o0, o1); }
  inline Error cmn(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCmn, cc), o0, o1); }
  inline Error cmn(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdCmn, o0, o1, o2); }
  inline Error cmn(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCmn, cc), o0, o1, o2); }
  inline Error cmn(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdCmn, o0, o1, o2); }
  inline Error cmn(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCmn, cc), o0, o1, o2); }
  inline Error tst(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdTst, o0, o1); }
  inline Error tst(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdTst, cc), o0, o1); }
  inline Error tst(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdTst, o0, o1); }
  inline Error tst(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdTst, cc), o0, o1); }
  inline Error tst(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdTst, o0, o1, o2); }
  inline Error tst(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdTst, cc), o0, o1, o2); }
  inline Error tst(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdTst, o0, o1, o2); }
  inline Error tst(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdTst, cc), o0, o1, o2); }
  inline Error teq(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdTeq, o0, o1); }
  inline Error teq(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdTeq, cc), o0, o1); }
  inline Error teq(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdTeq, o0, o1); }
  inline Error teq(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdTeq, cc), o0, o1); }
  inline Error teq(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdTeq, o0, o1, o2); }
  inline Error teq(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdTeq, cc), o0, o1, o2); }
  inline Error teq(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdTeq, o0, o1, o2); }
  inline Error teq(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdTeq, cc), o0, o1, o2); }
  inline Error lsl(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdLsl, o0, o1, o2); }
  inline Error lsl(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsl, cc), o0, o1, o2); }
  inline Error lsl(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdLsl, o0, o1, o2); }
  inline Error lsl(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsl, cc), o0, o1, o2); }
  inline Error lsl(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdLsl, o0, o1); }
  inline Error lsl(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsl, cc), o0, o1); }
  inline Error lsl(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdLsl, o0, o1); }
  inline Error lsl(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsl, cc), o0, o1); }
  inline Error lsls(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdLsls, o0, o1, o2); }
  inline Error lsls(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsls, cc), o0, o1, o2); }
  inline Error lsls(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdLsls, o0, o1, o2); }
  inline Error lsls(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsls, cc), o0, o1, o2); }
  inline Error lsls(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdLsls, o0, o1); }
  inline Error lsls(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsls, cc), o0, o1); }
  inline Error lsls(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdLsls, o0, o1); }
  inline Error lsls(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsls, cc), o0, o1); }
  inline Error lsr(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdLsr, o0, o1, o2); }
  inline Error lsr(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsr, cc), o0, o1, o2); }
  inline Error lsr(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdLsr, o0, o1, o2); }
  inline Error lsr(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsr, cc), o0, o1, o2); }
  inline Error lsr(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdLsr, o0, o1); }
  inline Error lsr(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsr, cc), o0, o1); }
  inline Error lsr(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdLsr, o0, o1); }
  inline Error lsr(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsr, cc), o0, o1); }
  inline Error lsrs(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdLsrs, o0, o1, o2); }
  inline Error lsrs(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsrs, cc), o0, o1, o2); }
  inline Error lsrs(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdLsrs, o0, o1, o2); }
  inline Error lsrs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsrs, cc), o0, o1, o2); }
  inline Error lsrs(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdLsrs, o0, o1); }
  inline Error lsrs(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsrs, cc), o0, o1); }
  inline Error lsrs(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdLsrs, o0, o1); }
  inline Error lsrs(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLsrs, cc), o0, o1); }
  inline Error asr(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdAsr, o0, o1, o2); }
  inline Error asr(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAsr, cc), o0, o1, o2); }
  inline Error asr(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdAsr, o0, o1, o2); }
  inline Error asr(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAsr, cc), o0, o1, o2); }
  inline Error asr(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdAsr, o0, o1); }
  inline Error asr(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAsr, cc), o0, o1); }
  inline Error asr(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdAsr, o0, o1); }
  inline Error asr(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAsr, cc), o0, o1); }
  inline Error asrs(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdAsrs, o0, o1, o2); }
  inline Error asrs(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAsrs, cc), o0, o1, o2); }
  inline Error asrs(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdAsrs, o0, o1, o2); }
  inline Error asrs(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAsrs, cc), o0, o1, o2); }
  inline Error asrs(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdAsrs, o0, o1); }
  inline Error asrs(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAsrs, cc), o0, o1); }
  inline Error asrs(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdAsrs, o0, o1); }
  inline Error asrs(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAsrs, cc), o0, o1); }
  inline Error ror(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdRor, o0, o1, o2); }
  inline Error ror(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRor, cc), o0, o1, o2); }
  inline Error ror(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdRor, o0, o1, o2); }
  inline Error ror(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRor, cc), o0, o1, o2); }
  inline Error ror(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdRor, o0, o1); }
  inline Error ror(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRor, cc), o0, o1); }
  inline Error ror(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdRor, o0, o1); }
  inline Error ror(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRor, cc), o0, o1); }
  inline Error rors(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdRors, o0, o1, o2); }
  inline Error rors(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRors, cc), o0, o1, o2); }
  inline Error rors(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdRors, o0, o1, o2); }
  inline Error rors(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRors, cc), o0, o1, o2); }
  inline Error rors(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdRors, o0, o1); }
  inline Error rors(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRors, cc), o0, o1); }
  inline Error rors(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdRors, o0, o1); }
  inline Error rors(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRors, cc), o0, o1); }
  inline Error rrx(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdRrx, o0, o1); }
  inline Error rrx(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRrx, cc), o0, o1); }
  inline Error rrxs(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdRrxs, o0, o1); }
  inline Error rrxs(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRrxs, cc), o0, o1); }
  inline Error movw(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdMovw, o0, o1); }
  inline Error movw(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMovw, cc), o0, o1); }
  inline Error movt(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdMovt, o0, o1); }
  inline Error movt(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMovt, cc), o0, o1); }
  inline Error adr(const Gp& o0, const Label& o1) { return _emitter()->_emitI(Inst::kIdAdr, o0, o1); }
  inline Error adr(CondCode cc, const Gp& o0, const Label& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdr, cc), o0, o1); }
  inline Error adr(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdAdr, o0, o1); }
  inline Error adr(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAdr, cc), o0, o1); }

  // Multiply & Divide

  inline Error mul(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdMul, o0, o1, o2); }
  inline Error mul(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMul, cc), o0, o1, o2); }
  inline Error muls(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdMuls, o0, o1, o2); }
  inline Error muls(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMuls, cc), o0, o1, o2); }
  inline Error sdiv(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSdiv, o0, o1, o2); }
  inline Error sdiv(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSdiv, cc), o0, o1, o2); }
  inline Error udiv(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUdiv, o0, o1, o2); }
  inline Error udiv(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUdiv, cc), o0, o1, o2); }
  inline Error smulbb(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSmulbb, o0, o1, o2); }
  inline Error smulbb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmulbb, cc), o0, o1, o2); }
  inline Error smulbt(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSmulbt, o0, o1, o2); }
  inline Error smulbt(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmulbt, cc), o0, o1, o2); }
  inline Error smultb(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSmultb, o0, o1, o2); }
  inline Error smultb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmultb, cc), o0, o1, o2); }
  inline Error smultt(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSmultt, o0, o1, o2); }
  inline Error smultt(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmultt, cc), o0, o1, o2); }
  inline Error smulwb(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSmulwb, o0, o1, o2); }
  inline Error smulwb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmulwb, cc), o0, o1, o2); }
  inline Error smulwt(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSmulwt, o0, o1, o2); }
  inline Error smulwt(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmulwt, cc), o0, o1, o2); }
  inline Error smuad(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSmuad, o0, o1, o2); }
  inline Error smuad(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmuad, cc), o0, o1, o2); }
  inline Error smuadx(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSmuadx, o0, o1, o2); }
  inline Error smuadx(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmuadx, cc), o0, o1, o2); }
  inline Error smusd(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSmusd, o0, o1, o2); }
  inline Error smusd(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmusd, cc), o0, o1, o2); }
  inline Error smusdx(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSmusdx, o0, o1, o2); }
  inline Error smusdx(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmusdx, cc), o0, o1, o2); }
  inline Error smmul(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSmmul, o0, o1, o2); }
  inline Error smmul(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmmul, cc), o0, o1, o2); }
  inline Error smmulr(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSmmulr, o0, o1, o2); }
  inline Error smmulr(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmmulr, cc), o0, o1, o2); }
  inline Error usad8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUsad8, o0, o1, o2); }
  inline Error usad8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUsad8, cc), o0, o1, o2); }
  inline Error mla(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdMla, o0, o1, o2, o3); }
  inline Error mla(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMla, cc), o0, o1, o2, o3); }
  inline Error mlas(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdMlas, o0, o1, o2, o3); }
  inline Error mlas(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMlas, cc), o0, o1, o2, o3); }
  inline Error mls(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdMls, o0, o1, o2, o3); }
  inline Error mls(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMls, cc), o0, o1, o2, o3); }
  inline Error smlal(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlal, o0, o1, o2, o3); }
  inline Error smlal(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlal, cc), o0, o1, o2, o3); }
  inline Error smlals(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlals, o0, o1, o2, o3); }
  inline Error smlals(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlals, cc), o0, o1, o2, o3); }
  inline Error smull(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmull, o0, o1, o2, o3); }
  inline Error smull(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmull, cc), o0, o1, o2, o3); }
  inline Error smulls(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmulls, o0, o1, o2, o3); }
  inline Error smulls(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmulls, cc), o0, o1, o2, o3); }
  inline Error umaal(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdUmaal, o0, o1, o2, o3); }
  inline Error umaal(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUmaal, cc), o0, o1, o2, o3); }
  inline Error umlal(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdUmlal, o0, o1, o2, o3); }
  inline Error umlal(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUmlal, cc), o0, o1, o2, o3); }
  inline Error umlals(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdUmlals, o0, o1, o2, o3); }
  inline Error umlals(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUmlals, cc), o0, o1, o2, o3); }
  inline Error umull(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdUmull, o0, o1, o2, o3); }
  inline Error umull(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUmull, cc), o0, o1, o2, o3); }
  inline Error umulls(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdUmulls, o0, o1, o2, o3); }
  inline Error umulls(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUmulls, cc), o0, o1, o2, o3); }
  inline Error smlabb(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlabb, o0, o1, o2, o3); }
  inline Error smlabb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlabb, cc), o0, o1, o2, o3); }
  inline Error smlabt(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlabt, o0, o1, o2, o3); }
  inline Error smlabt(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlabt, cc), o0, o1, o2, o3); }
  inline Error smlatb(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlatb, o0, o1, o2, o3); }
  inline Error smlatb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlatb, cc), o0, o1, o2, o3); }
  inline Error smlatt(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlatt, o0, o1, o2, o3); }
  inline Error smlatt(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlatt, cc), o0, o1, o2, o3); }
  inline Error smlawb(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlawb, o0, o1, o2, o3); }
  inline Error smlawb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlawb, cc), o0, o1, o2, o3); }
  inline Error smlawt(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlawt, o0, o1, o2, o3); }
  inline Error smlawt(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlawt, cc), o0, o1, o2, o3); }
  inline Error smlalbb(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlalbb, o0, o1, o2, o3); }
  inline Error smlalbb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlalbb, cc), o0, o1, o2, o3); }
  inline Error smlalbt(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlalbt, o0, o1, o2, o3); }
  inline Error smlalbt(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlalbt, cc), o0, o1, o2, o3); }
  inline Error smlaltb(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlaltb, o0, o1, o2, o3); }
  inline Error smlaltb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlaltb, cc), o0, o1, o2, o3); }
  inline Error smlaltt(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlaltt, o0, o1, o2, o3); }
  inline Error smlaltt(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlaltt, cc), o0, o1, o2, o3); }
  inline Error smlad(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlad, o0, o1, o2, o3); }
  inline Error smlad(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlad, cc), o0, o1, o2, o3); }
  inline Error smladx(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmladx, o0, o1, o2, o3); }
  inline Error smladx(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmladx, cc), o0, o1, o2, o3); }
  inline Error smlsd(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlsd, o0, o1, o2, o3); }
  inline Error smlsd(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlsd, cc), o0, o1, o2, o3); }
  inline Error smlsdx(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlsdx, o0, o1, o2, o3); }
  inline Error smlsdx(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlsdx, cc), o0, o1, o2, o3); }
  inline Error smlald(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlald, o0, o1, o2, o3); }
  inline Error smlald(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlald, cc), o0, o1, o2, o3); }
  inline Error smlaldx(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlaldx, o0, o1, o2, o3); }
  inline Error smlaldx(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlaldx, cc), o0, o1, o2, o3); }
  inline Error smlsld(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlsld, o0, o1, o2, o3); }
  inline Error smlsld(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlsld, cc), o0, o1, o2, o3); }
  inline Error smlsldx(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmlsldx, o0, o1, o2, o3); }
  inline Error smlsldx(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmlsldx, cc), o0, o1, o2, o3); }
  inline Error smmla(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmmla, o0, o1, o2, o3); }
  inline Error smmla(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmmla, cc), o0, o1, o2, o3); }
  inline Error smmlar(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmmlar, o0, o1, o2, o3); }
  inline Error smmlar(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmmlar, cc), o0, o1, o2, o3); }
  inline Error smmls(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmmls, o0, o1, o2, o3); }
  inline Error smmls(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmmls, cc), o0, o1, o2, o3); }
  inline Error smmlsr(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdSmmlsr, o0, o1, o2, o3); }
  inline Error smmlsr(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmmlsr, cc), o0, o1, o2, o3); }
  inline Error usada8(const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdUsada8, o0, o1, o2, o3); }
  inline Error usada8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUsada8, cc), o0, o1, o2, o3); }

  // Saturating & Parallel

  inline Error qadd(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdQadd, o0, o1, o2); }
  inline Error qadd(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdQadd, cc), o0, o1, o2); }
  inline Error qsub(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdQsub, o0, o1, o2); }
  inline Error qsub(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdQsub, cc), o0, o1, o2); }
  inline Error qdadd(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdQdadd, o0, o1, o2); }
  inline Error qdadd(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdQdadd, cc), o0, o1, o2); }
  inline Error qdsub(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdQdsub, o0, o1, o2); }
  inline Error qdsub(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdQdsub, cc), o0, o1, o2); }
  inline Error ssat(const Gp& o0, const Imm& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSsat, o0, o1, o2); }
  inline Error ssat(CondCode cc, const Gp& o0, const Imm& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSsat, cc), o0, o1, o2); }
  inline Error ssat(const Gp& o0, const Imm& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdSsat, o0, o1, o2, o3); }
  inline Error ssat(CondCode cc, const Gp& o0, const Imm& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSsat, cc), o0, o1, o2, o3); }
  inline Error usat(const Gp& o0, const Imm& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUsat, o0, o1, o2); }
  inline Error usat(CondCode cc, const Gp& o0, const Imm& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUsat, cc), o0, o1, o2); }
  inline Error usat(const Gp& o0, const Imm& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdUsat, o0, o1, o2, o3); }
  inline Error usat(CondCode cc, const Gp& o0, const Imm& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUsat, cc), o0, o1, o2, o3); }
  inline Error ssat16(const Gp& o0, const Imm& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSsat16, o0, o1, o2); }
  inline Error ssat16(CondCode cc, const Gp& o0, const Imm& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSsat16, cc), o0, o1, o2); }
  inline Error usat16(const Gp& o0, const Imm& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUsat16, o0, o1, o2); }
  inline Error usat16(CondCode cc, const Gp& o0, const Imm& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUsat16, cc), o0, o1, o2); }
  inline Error sadd16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSadd16, o0, o1, o2); }
  inline Error sadd16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSadd16, cc), o0, o1, o2); }
  inline Error sasx(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSasx, o0, o1, o2); }
  inline Error sasx(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSasx, cc), o0, o1, o2); }
  inline Error ssax(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSsax, o0, o1, o2); }
  inline Error ssax(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSsax, cc), o0, o1, o2); }
  inline Error ssub16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSsub16, o0, o1, o2); }
  inline Error ssub16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSsub16, cc), o0, o1, o2); }
  inline Error sadd8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSadd8, o0, o1, o2); }
  inline Error sadd8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSadd8, cc), o0, o1, o2); }
  inline Error ssub8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSsub8, o0, o1, o2); }
  inline Error ssub8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSsub8, cc), o0, o1, o2); }
  inline Error qadd16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdQadd16, o0, o1, o2); }
  inline Error qadd16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdQadd16, cc), o0, o1, o2); }
  inline Error qasx(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdQasx, o0, o1, o2); }
  inline Error qasx(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdQasx, cc), o0, o1, o2); }
  inline Error qsax(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdQsax, o0, o1, o2); }
  inline Error qsax(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdQsax, cc), o0, o1, o2); }
  inline Error qsub16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdQsub16, o0, o1, o2); }
  inline Error qsub16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdQsub16, cc), o0, o1, o2); }
  inline Error qadd8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdQadd8, o0, o1, o2); }
  inline Error qadd8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdQadd8, cc), o0, o1, o2); }
  inline Error qsub8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdQsub8, o0, o1, o2); }
  inline Error qsub8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdQsub8, cc), o0, o1, o2); }
  inline Error shadd16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdShadd16, o0, o1, o2); }
  inline Error shadd16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdShadd16, cc), o0, o1, o2); }
  inline Error shasx(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdShasx, o0, o1, o2); }
  inline Error shasx(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdShasx, cc), o0, o1, o2); }
  inline Error shsax(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdShsax, o0, o1, o2); }
  inline Error shsax(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdShsax, cc), o0, o1, o2); }
  inline Error shsub16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdShsub16, o0, o1, o2); }
  inline Error shsub16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdShsub16, cc), o0, o1, o2); }
  inline Error shadd8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdShadd8, o0, o1, o2); }
  inline Error shadd8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdShadd8, cc), o0, o1, o2); }
  inline Error shsub8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdShsub8, o0, o1, o2); }
  inline Error shsub8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdShsub8, cc), o0, o1, o2); }
  inline Error uadd16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUadd16, o0, o1, o2); }
  inline Error uadd16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUadd16, cc), o0, o1, o2); }
  inline Error uasx(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUasx, o0, o1, o2); }
  inline Error uasx(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUasx, cc), o0, o1, o2); }
  inline Error usax(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUsax, o0, o1, o2); }
  inline Error usax(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUsax, cc), o0, o1, o2); }
  inline Error usub16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUsub16, o0, o1, o2); }
  inline Error usub16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUsub16, cc), o0, o1, o2); }
  inline Error uadd8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUadd8, o0, o1, o2); }
  inline Error uadd8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUadd8, cc), o0, o1, o2); }
  inline Error usub8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUsub8, o0, o1, o2); }
  inline Error usub8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUsub8, cc), o0, o1, o2); }
  inline Error uqadd16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUqadd16, o0, o1, o2); }
  inline Error uqadd16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUqadd16, cc), o0, o1, o2); }
  inline Error uqasx(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUqasx, o0, o1, o2); }
  inline Error uqasx(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUqasx, cc), o0, o1, o2); }
  inline Error uqsax(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUqsax, o0, o1, o2); }
  inline Error uqsax(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUqsax, cc), o0, o1, o2); }
  inline Error uqsub16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUqsub16, o0, o1, o2); }
  inline Error uqsub16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUqsub16, cc), o0, o1, o2); }
  inline Error uqadd8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUqadd8, o0, o1, o2); }
  inline Error uqadd8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUqadd8, cc), o0, o1, o2); }
  inline Error uqsub8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUqsub8, o0, o1, o2); }
  inline Error uqsub8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUqsub8, cc), o0, o1, o2); }
  inline Error uhadd16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUhadd16, o0, o1, o2); }
  inline Error uhadd16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUhadd16, cc), o0, o1, o2); }
  inline Error uhasx(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUhasx, o0, o1, o2); }
  inline Error uhasx(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUhasx, cc), o0, o1, o2); }
  inline Error uhsax(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUhsax, o0, o1, o2); }
  inline Error uhsax(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUhsax, cc), o0, o1, o2); }
  inline Error uhsub16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUhsub16, o0, o1, o2); }
  inline Error uhsub16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUhsub16, cc), o0, o1, o2); }
  inline Error uhadd8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUhadd8, o0, o1, o2); }
  inline Error uhadd8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUhadd8, cc), o0, o1, o2); }
  inline Error uhsub8(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUhsub8, o0, o1, o2); }
  inline Error uhsub8(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUhsub8, cc), o0, o1, o2); }

  // Extend, Pack, Bitfield, Misc

  inline Error sxtb(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdSxtb, o0, o1); }
  inline Error sxtb(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSxtb, cc), o0, o1); }
  inline Error sxtb(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdSxtb, o0, o1, o2); }
  inline Error sxtb(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSxtb, cc), o0, o1, o2); }
  inline Error sxth(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdSxth, o0, o1); }
  inline Error sxth(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSxth, cc), o0, o1); }
  inline Error sxth(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdSxth, o0, o1, o2); }
  inline Error sxth(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSxth, cc), o0, o1, o2); }
  inline Error sxtb16(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdSxtb16, o0, o1); }
  inline Error sxtb16(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSxtb16, cc), o0, o1); }
  inline Error sxtb16(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdSxtb16, o0, o1, o2); }
  inline Error sxtb16(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSxtb16, cc), o0, o1, o2); }
  inline Error uxtb(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdUxtb, o0, o1); }
  inline Error uxtb(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUxtb, cc), o0, o1); }
  inline Error uxtb(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdUxtb, o0, o1, o2); }
  inline Error uxtb(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUxtb, cc), o0, o1, o2); }
  inline Error uxth(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdUxth, o0, o1); }
  inline Error uxth(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUxth, cc), o0, o1); }
  inline Error uxth(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdUxth, o0, o1, o2); }
  inline Error uxth(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUxth, cc), o0, o1, o2); }
  inline Error uxtb16(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdUxtb16, o0, o1); }
  inline Error uxtb16(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUxtb16, cc), o0, o1); }
  inline Error uxtb16(const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdUxtb16, o0, o1, o2); }
  inline Error uxtb16(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUxtb16, cc), o0, o1, o2); }
  inline Error sxtab(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSxtab, o0, o1, o2); }
  inline Error sxtab(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSxtab, cc), o0, o1, o2); }
  inline Error sxtab(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdSxtab, o0, o1, o2, o3); }
  inline Error sxtab(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSxtab, cc), o0, o1, o2, o3); }
  inline Error sxtah(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSxtah, o0, o1, o2); }
  inline Error sxtah(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSxtah, cc), o0, o1, o2); }
  inline Error sxtah(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdSxtah, o0, o1, o2, o3); }
  inline Error sxtah(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSxtah, cc), o0, o1, o2, o3); }
  inline Error sxtab16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSxtab16, o0, o1, o2); }
  inline Error sxtab16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSxtab16, cc), o0, o1, o2); }
  inline Error sxtab16(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdSxtab16, o0, o1, o2, o3); }
  inline Error sxtab16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSxtab16, cc), o0, o1, o2, o3); }
  inline Error uxtab(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUxtab, o0, o1, o2); }
  inline Error uxtab(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUxtab, cc), o0, o1, o2); }
  inline Error uxtab(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdUxtab, o0, o1, o2, o3); }
  inline Error uxtab(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUxtab, cc), o0, o1, o2, o3); }
  inline Error uxtah(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUxtah, o0, o1, o2); }
  inline Error uxtah(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUxtah, cc), o0, o1, o2); }
  inline Error uxtah(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdUxtah, o0, o1, o2, o3); }
  inline Error uxtah(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUxtah, cc), o0, o1, o2, o3); }
  inline Error uxtab16(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdUxtab16, o0, o1, o2); }
  inline Error uxtab16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUxtab16, cc), o0, o1, o2); }
  inline Error uxtab16(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdUxtab16, o0, o1, o2, o3); }
  inline Error uxtab16(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUxtab16, cc), o0, o1, o2, o3); }
  inline Error pkhbt(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdPkhbt, o0, o1, o2); }
  inline Error pkhbt(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdPkhbt, cc), o0, o1, o2); }
  inline Error pkhbt(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdPkhbt, o0, o1, o2, o3); }
  inline Error pkhbt(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdPkhbt, cc), o0, o1, o2, o3); }
  inline Error pkhtb(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdPkhtb, o0, o1, o2); }
  inline Error pkhtb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdPkhtb, cc), o0, o1, o2); }
  inline Error pkhtb(const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdPkhtb, o0, o1, o2, o3); }
  inline Error pkhtb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdPkhtb, cc), o0, o1, o2, o3); }
  inline Error sel(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdSel, o0, o1, o2); }
  inline Error sel(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSel, cc), o0, o1, o2); }
  inline Error crc32b(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdCrc32b, o0, o1, o2); }
  inline Error crc32b(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCrc32b, cc), o0, o1, o2); }
  inline Error crc32h(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdCrc32h, o0, o1, o2); }
  inline Error crc32h(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCrc32h, cc), o0, o1, o2); }
  inline Error crc32w(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdCrc32w, o0, o1, o2); }
  inline Error crc32w(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCrc32w, cc), o0, o1, o2); }
  inline Error crc32cb(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdCrc32cb, o0, o1, o2); }
  inline Error crc32cb(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCrc32cb, cc), o0, o1, o2); }
  inline Error crc32ch(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdCrc32ch, o0, o1, o2); }
  inline Error crc32ch(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCrc32ch, cc), o0, o1, o2); }
  inline Error crc32cw(const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdCrc32cw, o0, o1, o2); }
  inline Error crc32cw(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCrc32cw, cc), o0, o1, o2); }
  inline Error rev(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdRev, o0, o1); }
  inline Error rev(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRev, cc), o0, o1); }
  inline Error rev16(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdRev16, o0, o1); }
  inline Error rev16(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRev16, cc), o0, o1); }
  inline Error revsh(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdRevsh, o0, o1); }
  inline Error revsh(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRevsh, cc), o0, o1); }
  inline Error rbit(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdRbit, o0, o1); }
  inline Error rbit(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdRbit, cc), o0, o1); }
  inline Error clz(const Gp& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdClz, o0, o1); }
  inline Error clz(CondCode cc, const Gp& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdClz, cc), o0, o1); }
  inline Error bfc(const Gp& o0, const Imm& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdBfc, o0, o1, o2); }
  inline Error bfc(CondCode cc, const Gp& o0, const Imm& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBfc, cc), o0, o1, o2); }
  inline Error bfi(const Gp& o0, const Gp& o1, const Imm& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdBfi, o0, o1, o2, o3); }
  inline Error bfi(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBfi, cc), o0, o1, o2, o3); }
  inline Error sbfx(const Gp& o0, const Gp& o1, const Imm& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdSbfx, o0, o1, o2, o3); }
  inline Error sbfx(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSbfx, cc), o0, o1, o2, o3); }
  inline Error ubfx(const Gp& o0, const Gp& o1, const Imm& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdUbfx, o0, o1, o2, o3); }
  inline Error ubfx(CondCode cc, const Gp& o0, const Gp& o1, const Imm& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdUbfx, cc), o0, o1, o2, o3); }

  // Branch

  inline Error b(const Label& o0) { return _emitter()->_emitI(Inst::kIdB, o0); }
  inline Error b(CondCode cc, const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, cc), o0); }
  inline Error b_eq(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kEQ), o0); }
  inline Error b_ne(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kNE), o0); }
  inline Error b_cs(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kCS), o0); }
  inline Error b_hs(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kHS), o0); }
  inline Error b_cc(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kCC), o0); }
  inline Error b_lo(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kLO), o0); }
  inline Error b_mi(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kMI), o0); }
  inline Error b_pl(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kPL), o0); }
  inline Error b_vs(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kVS), o0); }
  inline Error b_vc(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kVC), o0); }
  inline Error b_hi(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kHI), o0); }
  inline Error b_ls(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kLS), o0); }
  inline Error b_ge(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kGE), o0); }
  inline Error b_lt(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kLT), o0); }
  inline Error b_gt(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kGT), o0); }
  inline Error b_le(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kLE), o0); }
  inline Error b_al(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kAL), o0); }
  inline Error b(const Imm& o0) { return _emitter()->_emitI(Inst::kIdB, o0); }
  inline Error b(CondCode cc, const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, cc), o0); }
  inline Error b_eq(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kEQ), o0); }
  inline Error b_ne(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kNE), o0); }
  inline Error b_cs(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kCS), o0); }
  inline Error b_hs(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kHS), o0); }
  inline Error b_cc(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kCC), o0); }
  inline Error b_lo(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kLO), o0); }
  inline Error b_mi(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kMI), o0); }
  inline Error b_pl(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kPL), o0); }
  inline Error b_vs(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kVS), o0); }
  inline Error b_vc(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kVC), o0); }
  inline Error b_hi(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kHI), o0); }
  inline Error b_ls(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kLS), o0); }
  inline Error b_ge(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kGE), o0); }
  inline Error b_lt(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kLT), o0); }
  inline Error b_gt(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kGT), o0); }
  inline Error b_le(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kLE), o0); }
  inline Error b_al(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdB, CondCode::kAL), o0); }
  inline Error bl(const Label& o0) { return _emitter()->_emitI(Inst::kIdBl, o0); }
  inline Error bl(CondCode cc, const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, cc), o0); }
  inline Error bl_eq(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kEQ), o0); }
  inline Error bl_ne(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kNE), o0); }
  inline Error bl_cs(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kCS), o0); }
  inline Error bl_hs(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kHS), o0); }
  inline Error bl_cc(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kCC), o0); }
  inline Error bl_lo(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kLO), o0); }
  inline Error bl_mi(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kMI), o0); }
  inline Error bl_pl(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kPL), o0); }
  inline Error bl_vs(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kVS), o0); }
  inline Error bl_vc(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kVC), o0); }
  inline Error bl_hi(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kHI), o0); }
  inline Error bl_ls(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kLS), o0); }
  inline Error bl_ge(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kGE), o0); }
  inline Error bl_lt(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kLT), o0); }
  inline Error bl_gt(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kGT), o0); }
  inline Error bl_le(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kLE), o0); }
  inline Error bl_al(const Label& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kAL), o0); }
  inline Error bl(const Imm& o0) { return _emitter()->_emitI(Inst::kIdBl, o0); }
  inline Error bl(CondCode cc, const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, cc), o0); }
  inline Error bl_eq(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kEQ), o0); }
  inline Error bl_ne(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kNE), o0); }
  inline Error bl_cs(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kCS), o0); }
  inline Error bl_hs(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kHS), o0); }
  inline Error bl_cc(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kCC), o0); }
  inline Error bl_lo(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kLO), o0); }
  inline Error bl_mi(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kMI), o0); }
  inline Error bl_pl(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kPL), o0); }
  inline Error bl_vs(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kVS), o0); }
  inline Error bl_vc(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kVC), o0); }
  inline Error bl_hi(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kHI), o0); }
  inline Error bl_ls(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kLS), o0); }
  inline Error bl_ge(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kGE), o0); }
  inline Error bl_lt(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kLT), o0); }
  inline Error bl_gt(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kGT), o0); }
  inline Error bl_le(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kLE), o0); }
  inline Error bl_al(const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBl, CondCode::kAL), o0); }
  inline Error blx(const Label& o0) { return _emitter()->_emitI(Inst::kIdBlx, o0); }
  Error blx(CondCode cc, const Label& o0) = delete; //!< Unconditional instruction.
  inline Error blx(const Imm& o0) { return _emitter()->_emitI(Inst::kIdBlx, o0); }
  Error blx(CondCode cc, const Imm& o0) = delete; //!< Unconditional instruction.
  inline Error blx(const Gp& o0) { return _emitter()->_emitI(Inst::kIdBlx, o0); }
  inline Error blx(CondCode cc, const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, cc), o0); }
  inline Error blx_eq(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kEQ), o0); }
  inline Error blx_ne(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kNE), o0); }
  inline Error blx_cs(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kCS), o0); }
  inline Error blx_hs(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kHS), o0); }
  inline Error blx_cc(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kCC), o0); }
  inline Error blx_lo(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kLO), o0); }
  inline Error blx_mi(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kMI), o0); }
  inline Error blx_pl(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kPL), o0); }
  inline Error blx_vs(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kVS), o0); }
  inline Error blx_vc(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kVC), o0); }
  inline Error blx_hi(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kHI), o0); }
  inline Error blx_ls(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kLS), o0); }
  inline Error blx_ge(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kGE), o0); }
  inline Error blx_lt(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kLT), o0); }
  inline Error blx_gt(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kGT), o0); }
  inline Error blx_le(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kLE), o0); }
  inline Error blx_al(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBlx, CondCode::kAL), o0); }
  inline Error bx(const Gp& o0) { return _emitter()->_emitI(Inst::kIdBx, o0); }
  inline Error bx(CondCode cc, const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, cc), o0); }
  inline Error bx_eq(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kEQ), o0); }
  inline Error bx_ne(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kNE), o0); }
  inline Error bx_cs(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kCS), o0); }
  inline Error bx_hs(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kHS), o0); }
  inline Error bx_cc(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kCC), o0); }
  inline Error bx_lo(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kLO), o0); }
  inline Error bx_mi(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kMI), o0); }
  inline Error bx_pl(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kPL), o0); }
  inline Error bx_vs(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kVS), o0); }
  inline Error bx_vc(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kVC), o0); }
  inline Error bx_hi(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kHI), o0); }
  inline Error bx_ls(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kLS), o0); }
  inline Error bx_ge(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kGE), o0); }
  inline Error bx_lt(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kLT), o0); }
  inline Error bx_gt(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kGT), o0); }
  inline Error bx_le(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kLE), o0); }
  inline Error bx_al(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBx, CondCode::kAL), o0); }
  inline Error bxj(const Gp& o0) { return _emitter()->_emitI(Inst::kIdBxj, o0); }
  inline Error bxj(CondCode cc, const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, cc), o0); }
  inline Error bxj_eq(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kEQ), o0); }
  inline Error bxj_ne(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kNE), o0); }
  inline Error bxj_cs(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kCS), o0); }
  inline Error bxj_hs(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kHS), o0); }
  inline Error bxj_cc(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kCC), o0); }
  inline Error bxj_lo(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kLO), o0); }
  inline Error bxj_mi(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kMI), o0); }
  inline Error bxj_pl(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kPL), o0); }
  inline Error bxj_vs(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kVS), o0); }
  inline Error bxj_vc(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kVC), o0); }
  inline Error bxj_hi(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kHI), o0); }
  inline Error bxj_ls(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kLS), o0); }
  inline Error bxj_ge(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kGE), o0); }
  inline Error bxj_lt(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kLT), o0); }
  inline Error bxj_gt(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kGT), o0); }
  inline Error bxj_le(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kLE), o0); }
  inline Error bxj_al(const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdBxj, CondCode::kAL), o0); }

  // Load & Store

  inline Error ldr(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdr, o0, o1); }
  inline Error ldr(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdr, cc), o0, o1); }
  inline Error ldr(const Gp& o0, const Label& o1) { return _emitter()->_emitI(Inst::kIdLdr, o0, o1); }
  inline Error ldr(CondCode cc, const Gp& o0, const Label& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdr, cc), o0, o1); }
  inline Error ldrb(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrb, o0, o1); }
  inline Error ldrb(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrb, cc), o0, o1); }
  inline Error ldrb(const Gp& o0, const Label& o1) { return _emitter()->_emitI(Inst::kIdLdrb, o0, o1); }
  inline Error ldrb(CondCode cc, const Gp& o0, const Label& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrb, cc), o0, o1); }
  inline Error ldrh(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrh, o0, o1); }
  inline Error ldrh(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrh, cc), o0, o1); }
  inline Error ldrh(const Gp& o0, const Label& o1) { return _emitter()->_emitI(Inst::kIdLdrh, o0, o1); }
  inline Error ldrh(CondCode cc, const Gp& o0, const Label& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrh, cc), o0, o1); }
  inline Error ldrsb(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrsb, o0, o1); }
  inline Error ldrsb(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrsb, cc), o0, o1); }
  inline Error ldrsb(const Gp& o0, const Label& o1) { return _emitter()->_emitI(Inst::kIdLdrsb, o0, o1); }
  inline Error ldrsb(CondCode cc, const Gp& o0, const Label& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrsb, cc), o0, o1); }
  inline Error ldrsh(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrsh, o0, o1); }
  inline Error ldrsh(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrsh, cc), o0, o1); }
  inline Error ldrsh(const Gp& o0, const Label& o1) { return _emitter()->_emitI(Inst::kIdLdrsh, o0, o1); }
  inline Error ldrsh(CondCode cc, const Gp& o0, const Label& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrsh, cc), o0, o1); }
  inline Error str(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdStr, o0, o1); }
  inline Error str(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStr, cc), o0, o1); }
  inline Error strb(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdStrb, o0, o1); }
  inline Error strb(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStrb, cc), o0, o1); }
  inline Error strh(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdStrh, o0, o1); }
  inline Error strh(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStrh, cc), o0, o1); }
  inline Error ldrd(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrd, o0, o1); }
  inline Error ldrd(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrd, cc), o0, o1); }
  inline Error ldrd(const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdLdrd, o0, o1, o2); }
  inline Error ldrd(CondCode cc, const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrd, cc), o0, o1, o2); }
  inline Error ldrd(const Gp& o0, const Label& o1) { return _emitter()->_emitI(Inst::kIdLdrd, o0, o1); }
  inline Error ldrd(CondCode cc, const Gp& o0, const Label& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrd, cc), o0, o1); }
  inline Error ldrd(const Gp& o0, const Gp& o1, const Label& o2) { return _emitter()->_emitI(Inst::kIdLdrd, o0, o1, o2); }
  inline Error ldrd(CondCode cc, const Gp& o0, const Gp& o1, const Label& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrd, cc), o0, o1, o2); }
  inline Error strd(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdStrd, o0, o1); }
  inline Error strd(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStrd, cc), o0, o1); }
  inline Error strd(const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdStrd, o0, o1, o2); }
  inline Error strd(CondCode cc, const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStrd, cc), o0, o1, o2); }
  inline Error ldrt(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrt, o0, o1); }
  inline Error ldrt(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrt, cc), o0, o1); }
  inline Error ldrbt(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrbt, o0, o1); }
  inline Error ldrbt(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrbt, cc), o0, o1); }
  inline Error ldrht(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrht, o0, o1); }
  inline Error ldrht(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrht, cc), o0, o1); }
  inline Error ldrsbt(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrsbt, o0, o1); }
  inline Error ldrsbt(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrsbt, cc), o0, o1); }
  inline Error ldrsht(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrsht, o0, o1); }
  inline Error ldrsht(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrsht, cc), o0, o1); }
  inline Error strt(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdStrt, o0, o1); }
  inline Error strt(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStrt, cc), o0, o1); }
  inline Error strbt(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdStrbt, o0, o1); }
  inline Error strbt(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStrbt, cc), o0, o1); }
  inline Error strht(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdStrht, o0, o1); }
  inline Error strht(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStrht, cc), o0, o1); }
  inline Error ldm(const Gp& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdLdm, o0, o1); }
  inline Error ldm(CondCode cc, const Gp& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdm, cc), o0, o1); }
  inline Error ldm(const Mem& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdLdm, o0, o1); }
  inline Error ldm(CondCode cc, const Mem& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdm, cc), o0, o1); }
  inline Error ldmib(const Gp& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdLdmib, o0, o1); }
  inline Error ldmib(CondCode cc, const Gp& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdmib, cc), o0, o1); }
  inline Error ldmib(const Mem& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdLdmib, o0, o1); }
  inline Error ldmib(CondCode cc, const Mem& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdmib, cc), o0, o1); }
  inline Error ldmda(const Gp& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdLdmda, o0, o1); }
  inline Error ldmda(CondCode cc, const Gp& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdmda, cc), o0, o1); }
  inline Error ldmda(const Mem& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdLdmda, o0, o1); }
  inline Error ldmda(CondCode cc, const Mem& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdmda, cc), o0, o1); }
  inline Error ldmdb(const Gp& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdLdmdb, o0, o1); }
  inline Error ldmdb(CondCode cc, const Gp& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdmdb, cc), o0, o1); }
  inline Error ldmdb(const Mem& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdLdmdb, o0, o1); }
  inline Error ldmdb(CondCode cc, const Mem& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdmdb, cc), o0, o1); }
  inline Error stm(const Gp& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdStm, o0, o1); }
  inline Error stm(CondCode cc, const Gp& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStm, cc), o0, o1); }
  inline Error stm(const Mem& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdStm, o0, o1); }
  inline Error stm(CondCode cc, const Mem& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStm, cc), o0, o1); }
  inline Error stmib(const Gp& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdStmib, o0, o1); }
  inline Error stmib(CondCode cc, const Gp& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStmib, cc), o0, o1); }
  inline Error stmib(const Mem& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdStmib, o0, o1); }
  inline Error stmib(CondCode cc, const Mem& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStmib, cc), o0, o1); }
  inline Error stmda(const Gp& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdStmda, o0, o1); }
  inline Error stmda(CondCode cc, const Gp& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStmda, cc), o0, o1); }
  inline Error stmda(const Mem& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdStmda, o0, o1); }
  inline Error stmda(CondCode cc, const Mem& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStmda, cc), o0, o1); }
  inline Error stmdb(const Gp& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdStmdb, o0, o1); }
  inline Error stmdb(CondCode cc, const Gp& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStmdb, cc), o0, o1); }
  inline Error stmdb(const Mem& o0, const GpList& o1) { return _emitter()->_emitI(Inst::kIdStmdb, o0, o1); }
  inline Error stmdb(CondCode cc, const Mem& o0, const GpList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStmdb, cc), o0, o1); }
  inline Error push(const GpList& o0) { return _emitter()->_emitI(Inst::kIdPush, o0); }
  inline Error push(CondCode cc, const GpList& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdPush, cc), o0); }
  inline Error push(const Gp& o0) { return _emitter()->_emitI(Inst::kIdPush, o0); }
  inline Error push(CondCode cc, const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdPush, cc), o0); }
  inline Error pop(const GpList& o0) { return _emitter()->_emitI(Inst::kIdPop, o0); }
  inline Error pop(CondCode cc, const GpList& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdPop, cc), o0); }
  inline Error pop(const Gp& o0) { return _emitter()->_emitI(Inst::kIdPop, o0); }
  inline Error pop(CondCode cc, const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdPop, cc), o0); }
  inline Error ldrex(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrex, o0, o1); }
  inline Error ldrex(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrex, cc), o0, o1); }
  inline Error ldrexb(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrexb, o0, o1); }
  inline Error ldrexb(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrexb, cc), o0, o1); }
  inline Error ldrexh(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrexh, o0, o1); }
  inline Error ldrexh(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrexh, cc), o0, o1); }
  inline Error lda(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLda, o0, o1); }
  inline Error lda(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLda, cc), o0, o1); }
  inline Error ldab(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdab, o0, o1); }
  inline Error ldab(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdab, cc), o0, o1); }
  inline Error ldah(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdah, o0, o1); }
  inline Error ldah(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdah, cc), o0, o1); }
  inline Error ldaex(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdaex, o0, o1); }
  inline Error ldaex(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdaex, cc), o0, o1); }
  inline Error ldaexb(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdaexb, o0, o1); }
  inline Error ldaexb(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdaexb, cc), o0, o1); }
  inline Error ldaexh(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdaexh, o0, o1); }
  inline Error ldaexh(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdaexh, cc), o0, o1); }
  inline Error stl(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdStl, o0, o1); }
  inline Error stl(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStl, cc), o0, o1); }
  inline Error stlb(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdStlb, o0, o1); }
  inline Error stlb(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStlb, cc), o0, o1); }
  inline Error stlh(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdStlh, o0, o1); }
  inline Error stlh(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStlh, cc), o0, o1); }
  inline Error ldrexd(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdrexd, o0, o1); }
  inline Error ldrexd(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrexd, cc), o0, o1); }
  inline Error ldrexd(const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdLdrexd, o0, o1, o2); }
  inline Error ldrexd(CondCode cc, const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdrexd, cc), o0, o1, o2); }
  inline Error ldaexd(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdLdaexd, o0, o1); }
  inline Error ldaexd(CondCode cc, const Gp& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdaexd, cc), o0, o1); }
  inline Error ldaexd(const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdLdaexd, o0, o1, o2); }
  inline Error ldaexd(CondCode cc, const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdLdaexd, cc), o0, o1, o2); }
  inline Error strex(const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdStrex, o0, o1, o2); }
  inline Error strex(CondCode cc, const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStrex, cc), o0, o1, o2); }
  inline Error strexb(const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdStrexb, o0, o1, o2); }
  inline Error strexb(CondCode cc, const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStrexb, cc), o0, o1, o2); }
  inline Error strexh(const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdStrexh, o0, o1, o2); }
  inline Error strexh(CondCode cc, const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStrexh, cc), o0, o1, o2); }
  inline Error stlex(const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdStlex, o0, o1, o2); }
  inline Error stlex(CondCode cc, const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStlex, cc), o0, o1, o2); }
  inline Error stlexb(const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdStlexb, o0, o1, o2); }
  inline Error stlexb(CondCode cc, const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStlexb, cc), o0, o1, o2); }
  inline Error stlexh(const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdStlexh, o0, o1, o2); }
  inline Error stlexh(CondCode cc, const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStlexh, cc), o0, o1, o2); }
  inline Error strexd(const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdStrexd, o0, o1, o2); }
  inline Error strexd(CondCode cc, const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStrexd, cc), o0, o1, o2); }
  inline Error strexd(const Gp& o0, const Gp& o1, const Gp& o2, const Mem& o3) { return _emitter()->_emitI(Inst::kIdStrexd, o0, o1, o2, o3); }
  inline Error strexd(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStrexd, cc), o0, o1, o2, o3); }
  inline Error stlexd(const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdStlexd, o0, o1, o2); }
  inline Error stlexd(CondCode cc, const Gp& o0, const Gp& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStlexd, cc), o0, o1, o2); }
  inline Error stlexd(const Gp& o0, const Gp& o1, const Gp& o2, const Mem& o3) { return _emitter()->_emitI(Inst::kIdStlexd, o0, o1, o2, o3); }
  inline Error stlexd(CondCode cc, const Gp& o0, const Gp& o1, const Gp& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdStlexd, cc), o0, o1, o2, o3); }
  inline Error clrex() { return _emitter()->_emitI(Inst::kIdClrex); }
  Error clrex(CondCode cc) = delete; //!< Unconditional instruction.

  // Hints, Exceptions, Barriers, System

  inline Error nop() { return _emitter()->_emitI(Inst::kIdNop); }
  inline Error nop(CondCode cc) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdNop, cc)); }
  inline Error yield() { return _emitter()->_emitI(Inst::kIdYield); }
  inline Error yield(CondCode cc) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdYield, cc)); }
  inline Error wfe() { return _emitter()->_emitI(Inst::kIdWfe); }
  inline Error wfe(CondCode cc) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdWfe, cc)); }
  inline Error wfi() { return _emitter()->_emitI(Inst::kIdWfi); }
  inline Error wfi(CondCode cc) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdWfi, cc)); }
  inline Error sev() { return _emitter()->_emitI(Inst::kIdSev); }
  inline Error sev(CondCode cc) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSev, cc)); }
  inline Error sevl() { return _emitter()->_emitI(Inst::kIdSevl); }
  inline Error sevl(CondCode cc) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSevl, cc)); }
  inline Error csdb() { return _emitter()->_emitI(Inst::kIdCsdb); }
  inline Error csdb(CondCode cc) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdCsdb, cc)); }
  inline Error eret() { return _emitter()->_emitI(Inst::kIdEret); }
  inline Error eret(CondCode cc) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdEret, cc)); }
  inline Error dbg(const Imm& o0) { return _emitter()->_emitI(Inst::kIdDbg, o0); }
  inline Error dbg(CondCode cc, const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdDbg, cc), o0); }
  inline Error smc(const Imm& o0) { return _emitter()->_emitI(Inst::kIdSmc, o0); }
  inline Error smc(CondCode cc, const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSmc, cc), o0); }
  inline Error svc(const Imm& o0) { return _emitter()->_emitI(Inst::kIdSvc, o0); }
  inline Error svc(CondCode cc, const Imm& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSvc, cc), o0); }
  inline Error bkpt(const Imm& o0) { return _emitter()->_emitI(Inst::kIdBkpt, o0); }
  Error bkpt(CondCode cc, const Imm& o0) = delete; //!< Unconditional instruction.
  inline Error udf(const Imm& o0) { return _emitter()->_emitI(Inst::kIdUdf, o0); }
  Error udf(CondCode cc, const Imm& o0) = delete; //!< Unconditional instruction.
  inline Error hvc(const Imm& o0) { return _emitter()->_emitI(Inst::kIdHvc, o0); }
  Error hvc(CondCode cc, const Imm& o0) = delete; //!< Unconditional instruction.
  inline Error bkpt() { return _emitter()->_emitI(Inst::kIdBkpt); }
  Error bkpt(CondCode cc) = delete; //!< Unconditional instruction.
  inline Error dmb() { return _emitter()->_emitI(Inst::kIdDmb); }
  Error dmb(CondCode cc) = delete; //!< Unconditional instruction.
  inline Error dmb(const Imm& o0) { return _emitter()->_emitI(Inst::kIdDmb, o0); }
  Error dmb(CondCode cc, const Imm& o0) = delete; //!< Unconditional instruction.
  inline Error dsb() { return _emitter()->_emitI(Inst::kIdDsb); }
  Error dsb(CondCode cc) = delete; //!< Unconditional instruction.
  inline Error dsb(const Imm& o0) { return _emitter()->_emitI(Inst::kIdDsb, o0); }
  Error dsb(CondCode cc, const Imm& o0) = delete; //!< Unconditional instruction.
  inline Error isb() { return _emitter()->_emitI(Inst::kIdIsb); }
  Error isb(CondCode cc) = delete; //!< Unconditional instruction.
  inline Error isb(const Imm& o0) { return _emitter()->_emitI(Inst::kIdIsb, o0); }
  Error isb(CondCode cc, const Imm& o0) = delete; //!< Unconditional instruction.
  inline Error mrs(const Gp& o0) { return _emitter()->_emitI(Inst::kIdMrs, o0); }
  inline Error mrs(CondCode cc, const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMrs, cc), o0); }
  inline Error mrs(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdMrs, o0, o1); }
  inline Error mrs(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMrs, cc), o0, o1); }
  inline Error msr(const Imm& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdMsr, o0, o1); }
  inline Error msr(CondCode cc, const Imm& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMsr, cc), o0, o1); }
  inline Error msr(const Imm& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdMsr, o0, o1); }
  inline Error msr(CondCode cc, const Imm& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMsr, cc), o0, o1); }
  inline Error cps(const Imm& o0) { return _emitter()->_emitI(Inst::kIdCps, o0); }
  Error cps(CondCode cc, const Imm& o0) = delete; //!< Unconditional instruction.
  inline Error cpsie(const Imm& o0) { return _emitter()->_emitI(Inst::kIdCpsie, o0); }
  Error cpsie(CondCode cc, const Imm& o0) = delete; //!< Unconditional instruction.
  inline Error cpsie(const Imm& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdCpsie, o0, o1); }
  Error cpsie(CondCode cc, const Imm& o0, const Imm& o1) = delete; //!< Unconditional instruction.
  inline Error cpsid(const Imm& o0) { return _emitter()->_emitI(Inst::kIdCpsid, o0); }
  Error cpsid(CondCode cc, const Imm& o0) = delete; //!< Unconditional instruction.
  inline Error cpsid(const Imm& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdCpsid, o0, o1); }
  Error cpsid(CondCode cc, const Imm& o0, const Imm& o1) = delete; //!< Unconditional instruction.
  inline Error setend(const Imm& o0) { return _emitter()->_emitI(Inst::kIdSetend, o0); }
  Error setend(CondCode cc, const Imm& o0) = delete; //!< Unconditional instruction.
  inline Error pld(const Mem& o0) { return _emitter()->_emitI(Inst::kIdPld, o0); }
  Error pld(CondCode cc, const Mem& o0) = delete; //!< Unconditional instruction.
  inline Error pldw(const Mem& o0) { return _emitter()->_emitI(Inst::kIdPldw, o0); }
  Error pldw(CondCode cc, const Mem& o0) = delete; //!< Unconditional instruction.
  inline Error pli(const Mem& o0) { return _emitter()->_emitI(Inst::kIdPli, o0); }
  Error pli(CondCode cc, const Mem& o0) = delete; //!< Unconditional instruction.
  inline Error pld(const Label& o0) { return _emitter()->_emitI(Inst::kIdPld, o0); }
  Error pld(CondCode cc, const Label& o0) = delete; //!< Unconditional instruction.
  inline Error pli(const Label& o0) { return _emitter()->_emitI(Inst::kIdPli, o0); }
  Error pli(CondCode cc, const Label& o0) = delete; //!< Unconditional instruction.
  inline Error mrc(const Imm& o0, const Imm& o1, const Gp& o2, const Imm& o3, const Imm& o4) { return _emitter()->_emitI(Inst::kIdMrc, o0, o1, o2, o3, o4); }
  inline Error mrc(CondCode cc, const Imm& o0, const Imm& o1, const Gp& o2, const Imm& o3, const Imm& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMrc, cc), o0, o1, o2, o3, o4); }
  inline Error mrc(const Imm& o0, const Imm& o1, const Gp& o2, const Imm& o3, const Imm& o4, const Imm& o5) { return _emitter()->_emitI(Inst::kIdMrc, o0, o1, o2, o3, o4, o5); }
  inline Error mrc(CondCode cc, const Imm& o0, const Imm& o1, const Gp& o2, const Imm& o3, const Imm& o4, const Imm& o5) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMrc, cc), o0, o1, o2, o3, o4, o5); }
  inline Error mcr(const Imm& o0, const Imm& o1, const Gp& o2, const Imm& o3, const Imm& o4) { return _emitter()->_emitI(Inst::kIdMcr, o0, o1, o2, o3, o4); }
  inline Error mcr(CondCode cc, const Imm& o0, const Imm& o1, const Gp& o2, const Imm& o3, const Imm& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMcr, cc), o0, o1, o2, o3, o4); }
  inline Error mcr(const Imm& o0, const Imm& o1, const Gp& o2, const Imm& o3, const Imm& o4, const Imm& o5) { return _emitter()->_emitI(Inst::kIdMcr, o0, o1, o2, o3, o4, o5); }
  inline Error mcr(CondCode cc, const Imm& o0, const Imm& o1, const Gp& o2, const Imm& o3, const Imm& o4, const Imm& o5) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMcr, cc), o0, o1, o2, o3, o4, o5); }
  inline Error mrrc(const Imm& o0, const Imm& o1, const Gp& o2, const Gp& o3, const Imm& o4) { return _emitter()->_emitI(Inst::kIdMrrc, o0, o1, o2, o3, o4); }
  inline Error mrrc(CondCode cc, const Imm& o0, const Imm& o1, const Gp& o2, const Gp& o3, const Imm& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMrrc, cc), o0, o1, o2, o3, o4); }
  inline Error mcrr(const Imm& o0, const Imm& o1, const Gp& o2, const Gp& o3, const Imm& o4) { return _emitter()->_emitI(Inst::kIdMcrr, o0, o1, o2, o3, o4); }
  inline Error mcrr(CondCode cc, const Imm& o0, const Imm& o1, const Gp& o2, const Gp& o3, const Imm& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdMcrr, cc), o0, o1, o2, o3, o4); }

  // VFP & ASIMD

  inline Error vabs(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVabs, o0, o1); }
  inline Error vabs(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabs, cc), o0, o1); }
  inline Error vabs(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabs, dt), o0, o1); }
  inline Error vabs(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabs, dt, cc), o0, o1); }
  inline Error vneg(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVneg, o0, o1); }
  inline Error vneg(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVneg, cc), o0, o1); }
  inline Error vneg(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVneg, dt), o0, o1); }
  inline Error vneg(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVneg, dt, cc), o0, o1); }
  inline Error vsqrt(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVsqrt, o0, o1); }
  inline Error vsqrt(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsqrt, cc), o0, o1); }
  inline Error vsqrt(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsqrt, dt), o0, o1); }
  inline Error vsqrt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsqrt, dt, cc), o0, o1); }
  inline Error vrintr(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrintr, o0, o1); }
  inline Error vrintr(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintr, cc), o0, o1); }
  inline Error vrintr(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintr, dt), o0, o1); }
  inline Error vrintr(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintr, dt, cc), o0, o1); }
  inline Error vrintx(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrintx, o0, o1); }
  inline Error vrintx(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintx, cc), o0, o1); }
  inline Error vrintx(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintx, dt), o0, o1); }
  inline Error vrintx(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintx, dt, cc), o0, o1); }
  inline Error vrintz(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrintz, o0, o1); }
  inline Error vrintz(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintz, cc), o0, o1); }
  inline Error vrintz(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintz, dt), o0, o1); }
  inline Error vrintz(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintz, dt, cc), o0, o1); }
  inline Error vrinta(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrinta, o0, o1); }
  inline Error vrinta(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrinta, cc), o0, o1); }
  inline Error vrinta(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrinta, dt), o0, o1); }
  inline Error vrinta(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrinta, dt, cc), o0, o1); }
  inline Error vrintn(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrintn, o0, o1); }
  inline Error vrintn(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintn, cc), o0, o1); }
  inline Error vrintn(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintn, dt), o0, o1); }
  inline Error vrintn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintn, dt, cc), o0, o1); }
  inline Error vrintp(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrintp, o0, o1); }
  inline Error vrintp(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintp, cc), o0, o1); }
  inline Error vrintp(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintp, dt), o0, o1); }
  inline Error vrintp(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintp, dt, cc), o0, o1); }
  inline Error vrintm(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrintm, o0, o1); }
  inline Error vrintm(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintm, cc), o0, o1); }
  inline Error vrintm(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintm, dt), o0, o1); }
  inline Error vrintm(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrintm, dt, cc), o0, o1); }
  inline Error vcmp(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcmp, o0, o1); }
  inline Error vcmp(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcmp, cc), o0, o1); }
  inline Error vcmp(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcmp, dt), o0, o1); }
  inline Error vcmp(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcmp, dt, cc), o0, o1); }
  inline Error vcmp(const Vec& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdVcmp, o0, o1); }
  inline Error vcmp(CondCode cc, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcmp, cc), o0, o1); }
  inline Error vcmp(DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcmp, dt), o0, o1); }
  inline Error vcmp(CondCode cc, DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcmp, dt, cc), o0, o1); }
  inline Error vcmpe(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcmpe, o0, o1); }
  inline Error vcmpe(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcmpe, cc), o0, o1); }
  inline Error vcmpe(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcmpe, dt), o0, o1); }
  inline Error vcmpe(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcmpe, dt, cc), o0, o1); }
  inline Error vcmpe(const Vec& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdVcmpe, o0, o1); }
  inline Error vcmpe(CondCode cc, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcmpe, cc), o0, o1); }
  inline Error vcmpe(DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcmpe, dt), o0, o1); }
  inline Error vcmpe(CondCode cc, DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcmpe, dt, cc), o0, o1); }
  inline Error vadd(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVadd, o0, o1, o2); }
  inline Error vadd(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVadd, cc), o0, o1, o2); }
  inline Error vadd(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVadd, dt), o0, o1, o2); }
  inline Error vadd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVadd, dt, cc), o0, o1, o2); }
  inline Error vadd(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVadd, o0, o1); }
  inline Error vadd(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVadd, cc), o0, o1); }
  inline Error vadd(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVadd, dt), o0, o1); }
  inline Error vadd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVadd, dt, cc), o0, o1); }
  inline Error vsub(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVsub, o0, o1, o2); }
  inline Error vsub(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsub, cc), o0, o1, o2); }
  inline Error vsub(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsub, dt), o0, o1, o2); }
  inline Error vsub(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsub, dt, cc), o0, o1, o2); }
  inline Error vsub(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVsub, o0, o1); }
  inline Error vsub(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsub, cc), o0, o1); }
  inline Error vsub(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsub, dt), o0, o1); }
  inline Error vsub(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsub, dt, cc), o0, o1); }
  inline Error vmul(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVmul, o0, o1, o2); }
  inline Error vmul(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmul, cc), o0, o1, o2); }
  inline Error vmul(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmul, dt), o0, o1, o2); }
  inline Error vmul(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmul, dt, cc), o0, o1, o2); }
  inline Error vmul(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVmul, o0, o1); }
  inline Error vmul(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmul, cc), o0, o1); }
  inline Error vmul(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmul, dt), o0, o1); }
  inline Error vmul(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmul, dt, cc), o0, o1); }
  inline Error vnmul(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVnmul, o0, o1, o2); }
  inline Error vnmul(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmul, cc), o0, o1, o2); }
  inline Error vnmul(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmul, dt), o0, o1, o2); }
  inline Error vnmul(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmul, dt, cc), o0, o1, o2); }
  inline Error vnmul(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVnmul, o0, o1); }
  inline Error vnmul(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmul, cc), o0, o1); }
  inline Error vnmul(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmul, dt), o0, o1); }
  inline Error vnmul(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmul, dt, cc), o0, o1); }
  inline Error vdiv(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVdiv, o0, o1, o2); }
  inline Error vdiv(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVdiv, cc), o0, o1, o2); }
  inline Error vdiv(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVdiv, dt), o0, o1, o2); }
  inline Error vdiv(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVdiv, dt, cc), o0, o1, o2); }
  inline Error vdiv(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVdiv, o0, o1); }
  inline Error vdiv(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVdiv, cc), o0, o1); }
  inline Error vdiv(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVdiv, dt), o0, o1); }
  inline Error vdiv(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVdiv, dt, cc), o0, o1); }
  inline Error vmla(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVmla, o0, o1, o2); }
  inline Error vmla(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmla, cc), o0, o1, o2); }
  inline Error vmla(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmla, dt), o0, o1, o2); }
  inline Error vmla(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmla, dt, cc), o0, o1, o2); }
  inline Error vmla(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVmla, o0, o1); }
  inline Error vmla(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmla, cc), o0, o1); }
  inline Error vmla(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmla, dt), o0, o1); }
  inline Error vmla(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmla, dt, cc), o0, o1); }
  inline Error vmls(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVmls, o0, o1, o2); }
  inline Error vmls(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmls, cc), o0, o1, o2); }
  inline Error vmls(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmls, dt), o0, o1, o2); }
  inline Error vmls(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmls, dt, cc), o0, o1, o2); }
  inline Error vmls(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVmls, o0, o1); }
  inline Error vmls(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmls, cc), o0, o1); }
  inline Error vmls(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmls, dt), o0, o1); }
  inline Error vmls(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmls, dt, cc), o0, o1); }
  inline Error vnmla(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVnmla, o0, o1, o2); }
  inline Error vnmla(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmla, cc), o0, o1, o2); }
  inline Error vnmla(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmla, dt), o0, o1, o2); }
  inline Error vnmla(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmla, dt, cc), o0, o1, o2); }
  inline Error vnmla(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVnmla, o0, o1); }
  inline Error vnmla(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmla, cc), o0, o1); }
  inline Error vnmla(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmla, dt), o0, o1); }
  inline Error vnmla(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmla, dt, cc), o0, o1); }
  inline Error vnmls(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVnmls, o0, o1, o2); }
  inline Error vnmls(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmls, cc), o0, o1, o2); }
  inline Error vnmls(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmls, dt), o0, o1, o2); }
  inline Error vnmls(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmls, dt, cc), o0, o1, o2); }
  inline Error vnmls(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVnmls, o0, o1); }
  inline Error vnmls(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmls, cc), o0, o1); }
  inline Error vnmls(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmls, dt), o0, o1); }
  inline Error vnmls(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVnmls, dt, cc), o0, o1); }
  inline Error vfma(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVfma, o0, o1, o2); }
  inline Error vfma(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfma, cc), o0, o1, o2); }
  inline Error vfma(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfma, dt), o0, o1, o2); }
  inline Error vfma(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfma, dt, cc), o0, o1, o2); }
  inline Error vfma(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVfma, o0, o1); }
  inline Error vfma(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfma, cc), o0, o1); }
  inline Error vfma(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfma, dt), o0, o1); }
  inline Error vfma(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfma, dt, cc), o0, o1); }
  inline Error vfms(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVfms, o0, o1, o2); }
  inline Error vfms(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfms, cc), o0, o1, o2); }
  inline Error vfms(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfms, dt), o0, o1, o2); }
  inline Error vfms(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfms, dt, cc), o0, o1, o2); }
  inline Error vfms(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVfms, o0, o1); }
  inline Error vfms(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfms, cc), o0, o1); }
  inline Error vfms(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfms, dt), o0, o1); }
  inline Error vfms(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfms, dt, cc), o0, o1); }
  inline Error vfnma(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVfnma, o0, o1, o2); }
  inline Error vfnma(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfnma, cc), o0, o1, o2); }
  inline Error vfnma(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfnma, dt), o0, o1, o2); }
  inline Error vfnma(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfnma, dt, cc), o0, o1, o2); }
  inline Error vfnma(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVfnma, o0, o1); }
  inline Error vfnma(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfnma, cc), o0, o1); }
  inline Error vfnma(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfnma, dt), o0, o1); }
  inline Error vfnma(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfnma, dt, cc), o0, o1); }
  inline Error vfnms(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVfnms, o0, o1, o2); }
  inline Error vfnms(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfnms, cc), o0, o1, o2); }
  inline Error vfnms(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfnms, dt), o0, o1, o2); }
  inline Error vfnms(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfnms, dt, cc), o0, o1, o2); }
  inline Error vfnms(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVfnms, o0, o1); }
  inline Error vfnms(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfnms, cc), o0, o1); }
  inline Error vfnms(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfnms, dt), o0, o1); }
  inline Error vfnms(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVfnms, dt, cc), o0, o1); }
  inline Error vmaxnm(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVmaxnm, o0, o1, o2); }
  inline Error vmaxnm(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmaxnm, cc), o0, o1, o2); }
  inline Error vmaxnm(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmaxnm, dt), o0, o1, o2); }
  inline Error vmaxnm(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmaxnm, dt, cc), o0, o1, o2); }
  inline Error vmaxnm(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVmaxnm, o0, o1); }
  inline Error vmaxnm(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmaxnm, cc), o0, o1); }
  inline Error vmaxnm(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmaxnm, dt), o0, o1); }
  inline Error vmaxnm(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmaxnm, dt, cc), o0, o1); }
  inline Error vminnm(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVminnm, o0, o1, o2); }
  inline Error vminnm(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVminnm, cc), o0, o1, o2); }
  inline Error vminnm(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVminnm, dt), o0, o1, o2); }
  inline Error vminnm(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVminnm, dt, cc), o0, o1, o2); }
  inline Error vminnm(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVminnm, o0, o1); }
  inline Error vminnm(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVminnm, cc), o0, o1); }
  inline Error vminnm(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVminnm, dt), o0, o1); }
  inline Error vminnm(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVminnm, dt, cc), o0, o1); }
  inline Error vseleq(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVseleq, o0, o1, o2); }
  inline Error vseleq(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVseleq, cc), o0, o1, o2); }
  inline Error vseleq(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVseleq, dt), o0, o1, o2); }
  inline Error vseleq(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVseleq, dt, cc), o0, o1, o2); }
  inline Error vseleq(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVseleq, o0, o1); }
  inline Error vseleq(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVseleq, cc), o0, o1); }
  inline Error vseleq(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVseleq, dt), o0, o1); }
  inline Error vseleq(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVseleq, dt, cc), o0, o1); }
  inline Error vselge(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVselge, o0, o1, o2); }
  inline Error vselge(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselge, cc), o0, o1, o2); }
  inline Error vselge(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselge, dt), o0, o1, o2); }
  inline Error vselge(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselge, dt, cc), o0, o1, o2); }
  inline Error vselge(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVselge, o0, o1); }
  inline Error vselge(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselge, cc), o0, o1); }
  inline Error vselge(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselge, dt), o0, o1); }
  inline Error vselge(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselge, dt, cc), o0, o1); }
  inline Error vselgt(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVselgt, o0, o1, o2); }
  inline Error vselgt(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselgt, cc), o0, o1, o2); }
  inline Error vselgt(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselgt, dt), o0, o1, o2); }
  inline Error vselgt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselgt, dt, cc), o0, o1, o2); }
  inline Error vselgt(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVselgt, o0, o1); }
  inline Error vselgt(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselgt, cc), o0, o1); }
  inline Error vselgt(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselgt, dt), o0, o1); }
  inline Error vselgt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselgt, dt, cc), o0, o1); }
  inline Error vselvs(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVselvs, o0, o1, o2); }
  inline Error vselvs(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselvs, cc), o0, o1, o2); }
  inline Error vselvs(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselvs, dt), o0, o1, o2); }
  inline Error vselvs(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselvs, dt, cc), o0, o1, o2); }
  inline Error vselvs(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVselvs, o0, o1); }
  inline Error vselvs(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselvs, cc), o0, o1); }
  inline Error vselvs(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselvs, dt), o0, o1); }
  inline Error vselvs(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVselvs, dt, cc), o0, o1); }
  inline Error vcvt(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcvt, o0, o1); }
  inline Error vcvt(DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvt, dt, dt2), o0, o1); }
  inline Error vcvt(CondCode cc, DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvt, dt, dt2, cc), o0, o1); }
  inline Error vcvt(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVcvt, o0, o1, o2); }
  inline Error vcvt(DataType dt, DataType dt2, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvt, dt, dt2), o0, o1, o2); }
  inline Error vcvt(CondCode cc, DataType dt, DataType dt2, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvt, dt, dt2, cc), o0, o1, o2); }
  inline Error vcvtr(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcvtr, o0, o1); }
  inline Error vcvtr(DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvtr, dt, dt2), o0, o1); }
  inline Error vcvtr(CondCode cc, DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvtr, dt, dt2, cc), o0, o1); }
  inline Error vcvtb(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcvtb, o0, o1); }
  inline Error vcvtb(DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvtb, dt, dt2), o0, o1); }
  inline Error vcvtb(CondCode cc, DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvtb, dt, dt2, cc), o0, o1); }
  inline Error vcvtt(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcvtt, o0, o1); }
  inline Error vcvtt(DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvtt, dt, dt2), o0, o1); }
  inline Error vcvtt(CondCode cc, DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvtt, dt, dt2, cc), o0, o1); }
  inline Error vcvta(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcvta, o0, o1); }
  inline Error vcvta(DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvta, dt, dt2), o0, o1); }
  inline Error vcvta(CondCode cc, DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvta, dt, dt2, cc), o0, o1); }
  inline Error vcvtn(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcvtn, o0, o1); }
  inline Error vcvtn(DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvtn, dt, dt2), o0, o1); }
  inline Error vcvtn(CondCode cc, DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvtn, dt, dt2, cc), o0, o1); }
  inline Error vcvtp(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcvtp, o0, o1); }
  inline Error vcvtp(DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvtp, dt, dt2), o0, o1); }
  inline Error vcvtp(CondCode cc, DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvtp, dt, dt2, cc), o0, o1); }
  inline Error vcvtm(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcvtm, o0, o1); }
  inline Error vcvtm(DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvtm, dt, dt2), o0, o1); }
  inline Error vcvtm(CondCode cc, DataType dt, DataType dt2, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcvtm, dt, dt2, cc), o0, o1); }
  inline Error vldr(const Vec& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVldr, o0, o1); }
  inline Error vldr(CondCode cc, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVldr, cc), o0, o1); }
  inline Error vldr(DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVldr, dt), o0, o1); }
  inline Error vldr(CondCode cc, DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVldr, dt, cc), o0, o1); }
  inline Error vldr(const Vec& o0, const Label& o1) { return _emitter()->_emitI(Inst::kIdVldr, o0, o1); }
  inline Error vldr(CondCode cc, const Vec& o0, const Label& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVldr, cc), o0, o1); }
  inline Error vldr(DataType dt, const Vec& o0, const Label& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVldr, dt), o0, o1); }
  inline Error vldr(CondCode cc, DataType dt, const Vec& o0, const Label& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVldr, dt, cc), o0, o1); }
  inline Error vstr(const Vec& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVstr, o0, o1); }
  inline Error vstr(CondCode cc, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVstr, cc), o0, o1); }
  inline Error vstr(DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVstr, dt), o0, o1); }
  inline Error vstr(CondCode cc, DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVstr, dt, cc), o0, o1); }
  inline Error vldm(const Gp& o0, const VecList& o1) { return _emitter()->_emitI(Inst::kIdVldm, o0, o1); }
  inline Error vldm(CondCode cc, const Gp& o0, const VecList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVldm, cc), o0, o1); }
  inline Error vldm(const Mem& o0, const VecList& o1) { return _emitter()->_emitI(Inst::kIdVldm, o0, o1); }
  inline Error vldm(CondCode cc, const Mem& o0, const VecList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVldm, cc), o0, o1); }
  inline Error vldmdb(const Gp& o0, const VecList& o1) { return _emitter()->_emitI(Inst::kIdVldmdb, o0, o1); }
  inline Error vldmdb(CondCode cc, const Gp& o0, const VecList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVldmdb, cc), o0, o1); }
  inline Error vldmdb(const Mem& o0, const VecList& o1) { return _emitter()->_emitI(Inst::kIdVldmdb, o0, o1); }
  inline Error vldmdb(CondCode cc, const Mem& o0, const VecList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVldmdb, cc), o0, o1); }
  inline Error vstm(const Gp& o0, const VecList& o1) { return _emitter()->_emitI(Inst::kIdVstm, o0, o1); }
  inline Error vstm(CondCode cc, const Gp& o0, const VecList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVstm, cc), o0, o1); }
  inline Error vstm(const Mem& o0, const VecList& o1) { return _emitter()->_emitI(Inst::kIdVstm, o0, o1); }
  inline Error vstm(CondCode cc, const Mem& o0, const VecList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVstm, cc), o0, o1); }
  inline Error vstmdb(const Gp& o0, const VecList& o1) { return _emitter()->_emitI(Inst::kIdVstmdb, o0, o1); }
  inline Error vstmdb(CondCode cc, const Gp& o0, const VecList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVstmdb, cc), o0, o1); }
  inline Error vstmdb(const Mem& o0, const VecList& o1) { return _emitter()->_emitI(Inst::kIdVstmdb, o0, o1); }
  inline Error vstmdb(CondCode cc, const Mem& o0, const VecList& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVstmdb, cc), o0, o1); }
  inline Error vpush(const VecList& o0) { return _emitter()->_emitI(Inst::kIdVpush, o0); }
  inline Error vpush(CondCode cc, const VecList& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpush, cc), o0); }
  inline Error vpush(const Vec& o0) { return _emitter()->_emitI(Inst::kIdVpush, o0); }
  inline Error vpush(CondCode cc, const Vec& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpush, cc), o0); }
  inline Error vpop(const VecList& o0) { return _emitter()->_emitI(Inst::kIdVpop, o0); }
  inline Error vpop(CondCode cc, const VecList& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpop, cc), o0); }
  inline Error vpop(const Vec& o0) { return _emitter()->_emitI(Inst::kIdVpop, o0); }
  inline Error vpop(CondCode cc, const Vec& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpop, cc), o0); }
  inline Error vmrs(const Gp& o0) { return _emitter()->_emitI(Inst::kIdVmrs, o0); }
  inline Error vmrs(CondCode cc, const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmrs, cc), o0); }
  inline Error vmrs(const Gp& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdVmrs, o0, o1); }
  inline Error vmrs(CondCode cc, const Gp& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmrs, cc), o0, o1); }
  inline Error vmsr(const Gp& o0) { return _emitter()->_emitI(Inst::kIdVmsr, o0); }
  inline Error vmsr(CondCode cc, const Gp& o0) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmsr, cc), o0); }
  inline Error vmsr(const Imm& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdVmsr, o0, o1); }
  inline Error vmsr(CondCode cc, const Imm& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmsr, cc), o0, o1); }
  inline Error vmov(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVmov, o0, o1); }
  inline Error vmov(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, cc), o0, o1); }
  inline Error vmov(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt), o0, o1); }
  inline Error vmov(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt, cc), o0, o1); }
  inline Error vmov(const Vec& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdVmov, o0, o1); }
  inline Error vmov(CondCode cc, const Vec& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, cc), o0, o1); }
  inline Error vmov(DataType dt, const Vec& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt), o0, o1); }
  inline Error vmov(CondCode cc, DataType dt, const Vec& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt, cc), o0, o1); }
  inline Error vmov(const Gp& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVmov, o0, o1); }
  inline Error vmov(CondCode cc, const Gp& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, cc), o0, o1); }
  inline Error vmov(DataType dt, const Gp& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt), o0, o1); }
  inline Error vmov(CondCode cc, DataType dt, const Gp& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt, cc), o0, o1); }
  inline Error vmov(const Vec& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdVmov, o0, o1); }
  inline Error vmov(CondCode cc, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, cc), o0, o1); }
  inline Error vmov(DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt), o0, o1); }
  inline Error vmov(CondCode cc, DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt, cc), o0, o1); }
  inline Error vmov(const Vec& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(Inst::kIdVmov, o0, o1, o2); }
  inline Error vmov(CondCode cc, const Vec& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, cc), o0, o1, o2); }
  inline Error vmov(DataType dt, const Vec& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt), o0, o1, o2); }
  inline Error vmov(CondCode cc, DataType dt, const Vec& o0, const Gp& o1, const Gp& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt, cc), o0, o1, o2); }
  inline Error vmov(const Gp& o0, const Gp& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVmov, o0, o1, o2); }
  inline Error vmov(CondCode cc, const Gp& o0, const Gp& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, cc), o0, o1, o2); }
  inline Error vmov(DataType dt, const Gp& o0, const Gp& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt), o0, o1, o2); }
  inline Error vmov(CondCode cc, DataType dt, const Gp& o0, const Gp& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt, cc), o0, o1, o2); }
  inline Error vmov(const Vec& o0, const Vec& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(Inst::kIdVmov, o0, o1, o2, o3); }
  inline Error vmov(CondCode cc, const Vec& o0, const Vec& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, cc), o0, o1, o2, o3); }
  inline Error vmov(DataType dt, const Vec& o0, const Vec& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt), o0, o1, o2, o3); }
  inline Error vmov(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Gp& o2, const Gp& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt, cc), o0, o1, o2, o3); }
  inline Error vmov(const Gp& o0, const Gp& o1, const Vec& o2, const Vec& o3) { return _emitter()->_emitI(Inst::kIdVmov, o0, o1, o2, o3); }
  inline Error vmov(CondCode cc, const Gp& o0, const Gp& o1, const Vec& o2, const Vec& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, cc), o0, o1, o2, o3); }
  inline Error vmov(DataType dt, const Gp& o0, const Gp& o1, const Vec& o2, const Vec& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt), o0, o1, o2, o3); }
  inline Error vmov(CondCode cc, DataType dt, const Gp& o0, const Gp& o1, const Vec& o2, const Vec& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmov, dt, cc), o0, o1, o2, o3); }
  inline Error vaba(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVaba, o0, o1, o2); }
  inline Error vaba(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaba, cc), o0, o1, o2); }
  inline Error vaba(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaba, dt), o0, o1, o2); }
  inline Error vaba(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaba, dt, cc), o0, o1, o2); }
  inline Error vaba(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVaba, o0, o1); }
  inline Error vaba(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaba, cc), o0, o1); }
  inline Error vaba(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaba, dt), o0, o1); }
  inline Error vaba(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaba, dt, cc), o0, o1); }
  inline Error vabd(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVabd, o0, o1, o2); }
  inline Error vabd(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabd, cc), o0, o1, o2); }
  inline Error vabd(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabd, dt), o0, o1, o2); }
  inline Error vabd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabd, dt, cc), o0, o1, o2); }
  inline Error vabd(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVabd, o0, o1); }
  inline Error vabd(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabd, cc), o0, o1); }
  inline Error vabd(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabd, dt), o0, o1); }
  inline Error vabd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabd, dt, cc), o0, o1); }
  inline Error vacge(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVacge, o0, o1, o2); }
  inline Error vacge(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacge, cc), o0, o1, o2); }
  inline Error vacge(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacge, dt), o0, o1, o2); }
  inline Error vacge(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacge, dt, cc), o0, o1, o2); }
  inline Error vacge(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVacge, o0, o1); }
  inline Error vacge(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacge, cc), o0, o1); }
  inline Error vacge(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacge, dt), o0, o1); }
  inline Error vacge(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacge, dt, cc), o0, o1); }
  inline Error vacgt(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVacgt, o0, o1, o2); }
  inline Error vacgt(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacgt, cc), o0, o1, o2); }
  inline Error vacgt(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacgt, dt), o0, o1, o2); }
  inline Error vacgt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacgt, dt, cc), o0, o1, o2); }
  inline Error vacgt(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVacgt, o0, o1); }
  inline Error vacgt(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacgt, cc), o0, o1); }
  inline Error vacgt(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacgt, dt), o0, o1); }
  inline Error vacgt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacgt, dt, cc), o0, o1); }
  inline Error vacle(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVacle, o0, o1, o2); }
  inline Error vacle(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacle, cc), o0, o1, o2); }
  inline Error vacle(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacle, dt), o0, o1, o2); }
  inline Error vacle(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacle, dt, cc), o0, o1, o2); }
  inline Error vacle(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVacle, o0, o1); }
  inline Error vacle(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacle, cc), o0, o1); }
  inline Error vacle(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacle, dt), o0, o1); }
  inline Error vacle(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVacle, dt, cc), o0, o1); }
  inline Error vaclt(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVaclt, o0, o1, o2); }
  inline Error vaclt(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaclt, cc), o0, o1, o2); }
  inline Error vaclt(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaclt, dt), o0, o1, o2); }
  inline Error vaclt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaclt, dt, cc), o0, o1, o2); }
  inline Error vaclt(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVaclt, o0, o1); }
  inline Error vaclt(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaclt, cc), o0, o1); }
  inline Error vaclt(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaclt, dt), o0, o1); }
  inline Error vaclt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaclt, dt, cc), o0, o1); }
  inline Error vand(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVand, o0, o1, o2); }
  inline Error vand(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVand, cc), o0, o1, o2); }
  inline Error vand(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVand, dt), o0, o1, o2); }
  inline Error vand(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVand, dt, cc), o0, o1, o2); }
  inline Error vand(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVand, o0, o1); }
  inline Error vand(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVand, cc), o0, o1); }
  inline Error vand(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVand, dt), o0, o1); }
  inline Error vand(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVand, dt, cc), o0, o1); }
  inline Error vbic(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVbic, o0, o1, o2); }
  inline Error vbic(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbic, cc), o0, o1, o2); }
  inline Error vbic(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbic, dt), o0, o1, o2); }
  inline Error vbic(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbic, dt, cc), o0, o1, o2); }
  inline Error vbic(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVbic, o0, o1); }
  inline Error vbic(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbic, cc), o0, o1); }
  inline Error vbic(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbic, dt), o0, o1); }
  inline Error vbic(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbic, dt, cc), o0, o1); }
  inline Error vbif(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVbif, o0, o1, o2); }
  inline Error vbif(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbif, cc), o0, o1, o2); }
  inline Error vbif(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbif, dt), o0, o1, o2); }
  inline Error vbif(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbif, dt, cc), o0, o1, o2); }
  inline Error vbif(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVbif, o0, o1); }
  inline Error vbif(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbif, cc), o0, o1); }
  inline Error vbif(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbif, dt), o0, o1); }
  inline Error vbif(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbif, dt, cc), o0, o1); }
  inline Error vbit(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVbit, o0, o1, o2); }
  inline Error vbit(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbit, cc), o0, o1, o2); }
  inline Error vbit(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbit, dt), o0, o1, o2); }
  inline Error vbit(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbit, dt, cc), o0, o1, o2); }
  inline Error vbit(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVbit, o0, o1); }
  inline Error vbit(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbit, cc), o0, o1); }
  inline Error vbit(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbit, dt), o0, o1); }
  inline Error vbit(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbit, dt, cc), o0, o1); }
  inline Error vbsl(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVbsl, o0, o1, o2); }
  inline Error vbsl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbsl, cc), o0, o1, o2); }
  inline Error vbsl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbsl, dt), o0, o1, o2); }
  inline Error vbsl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbsl, dt, cc), o0, o1, o2); }
  inline Error vbsl(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVbsl, o0, o1); }
  inline Error vbsl(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbsl, cc), o0, o1); }
  inline Error vbsl(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbsl, dt), o0, o1); }
  inline Error vbsl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbsl, dt, cc), o0, o1); }
  inline Error vceq(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVceq, o0, o1, o2); }
  inline Error vceq(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVceq, cc), o0, o1, o2); }
  inline Error vceq(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVceq, dt), o0, o1, o2); }
  inline Error vceq(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVceq, dt, cc), o0, o1, o2); }
  inline Error vceq(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVceq, o0, o1); }
  inline Error vceq(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVceq, cc), o0, o1); }
  inline Error vceq(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVceq, dt), o0, o1); }
  inline Error vceq(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVceq, dt, cc), o0, o1); }
  inline Error vcge(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVcge, o0, o1, o2); }
  inline Error vcge(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcge, cc), o0, o1, o2); }
  inline Error vcge(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcge, dt), o0, o1, o2); }
  inline Error vcge(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcge, dt, cc), o0, o1, o2); }
  inline Error vcge(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcge, o0, o1); }
  inline Error vcge(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcge, cc), o0, o1); }
  inline Error vcge(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcge, dt), o0, o1); }
  inline Error vcge(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcge, dt, cc), o0, o1); }
  inline Error vcgt(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVcgt, o0, o1, o2); }
  inline Error vcgt(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcgt, cc), o0, o1, o2); }
  inline Error vcgt(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcgt, dt), o0, o1, o2); }
  inline Error vcgt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcgt, dt, cc), o0, o1, o2); }
  inline Error vcgt(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcgt, o0, o1); }
  inline Error vcgt(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcgt, cc), o0, o1); }
  inline Error vcgt(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcgt, dt), o0, o1); }
  inline Error vcgt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcgt, dt, cc), o0, o1); }
  inline Error vcle(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVcle, o0, o1, o2); }
  inline Error vcle(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcle, cc), o0, o1, o2); }
  inline Error vcle(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcle, dt), o0, o1, o2); }
  inline Error vcle(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcle, dt, cc), o0, o1, o2); }
  inline Error vcle(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcle, o0, o1); }
  inline Error vcle(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcle, cc), o0, o1); }
  inline Error vcle(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcle, dt), o0, o1); }
  inline Error vcle(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcle, dt, cc), o0, o1); }
  inline Error vclt(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVclt, o0, o1, o2); }
  inline Error vclt(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVclt, cc), o0, o1, o2); }
  inline Error vclt(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVclt, dt), o0, o1, o2); }
  inline Error vclt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVclt, dt, cc), o0, o1, o2); }
  inline Error vclt(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVclt, o0, o1); }
  inline Error vclt(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVclt, cc), o0, o1); }
  inline Error vclt(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVclt, dt), o0, o1); }
  inline Error vclt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVclt, dt, cc), o0, o1); }
  inline Error veor(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVeor, o0, o1, o2); }
  inline Error veor(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVeor, cc), o0, o1, o2); }
  inline Error veor(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVeor, dt), o0, o1, o2); }
  inline Error veor(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVeor, dt, cc), o0, o1, o2); }
  inline Error veor(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVeor, o0, o1); }
  inline Error veor(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVeor, cc), o0, o1); }
  inline Error veor(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVeor, dt), o0, o1); }
  inline Error veor(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVeor, dt, cc), o0, o1); }
  inline Error vhadd(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVhadd, o0, o1, o2); }
  inline Error vhadd(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVhadd, cc), o0, o1, o2); }
  inline Error vhadd(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVhadd, dt), o0, o1, o2); }
  inline Error vhadd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVhadd, dt, cc), o0, o1, o2); }
  inline Error vhadd(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVhadd, o0, o1); }
  inline Error vhadd(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVhadd, cc), o0, o1); }
  inline Error vhadd(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVhadd, dt), o0, o1); }
  inline Error vhadd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVhadd, dt, cc), o0, o1); }
  inline Error vhsub(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVhsub, o0, o1, o2); }
  inline Error vhsub(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVhsub, cc), o0, o1, o2); }
  inline Error vhsub(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVhsub, dt), o0, o1, o2); }
  inline Error vhsub(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVhsub, dt, cc), o0, o1, o2); }
  inline Error vhsub(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVhsub, o0, o1); }
  inline Error vhsub(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVhsub, cc), o0, o1); }
  inline Error vhsub(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVhsub, dt), o0, o1); }
  inline Error vhsub(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVhsub, dt, cc), o0, o1); }
  inline Error vmax(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVmax, o0, o1, o2); }
  inline Error vmax(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmax, cc), o0, o1, o2); }
  inline Error vmax(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmax, dt), o0, o1, o2); }
  inline Error vmax(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmax, dt, cc), o0, o1, o2); }
  inline Error vmax(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVmax, o0, o1); }
  inline Error vmax(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmax, cc), o0, o1); }
  inline Error vmax(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmax, dt), o0, o1); }
  inline Error vmax(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmax, dt, cc), o0, o1); }
  inline Error vmin(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVmin, o0, o1, o2); }
  inline Error vmin(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmin, cc), o0, o1, o2); }
  inline Error vmin(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmin, dt), o0, o1, o2); }
  inline Error vmin(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmin, dt, cc), o0, o1, o2); }
  inline Error vmin(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVmin, o0, o1); }
  inline Error vmin(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmin, cc), o0, o1); }
  inline Error vmin(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmin, dt), o0, o1); }
  inline Error vmin(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmin, dt, cc), o0, o1); }
  inline Error vorn(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVorn, o0, o1, o2); }
  inline Error vorn(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorn, cc), o0, o1, o2); }
  inline Error vorn(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorn, dt), o0, o1, o2); }
  inline Error vorn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorn, dt, cc), o0, o1, o2); }
  inline Error vorn(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVorn, o0, o1); }
  inline Error vorn(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorn, cc), o0, o1); }
  inline Error vorn(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorn, dt), o0, o1); }
  inline Error vorn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorn, dt, cc), o0, o1); }
  inline Error vorr(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVorr, o0, o1, o2); }
  inline Error vorr(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorr, cc), o0, o1, o2); }
  inline Error vorr(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorr, dt), o0, o1, o2); }
  inline Error vorr(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorr, dt, cc), o0, o1, o2); }
  inline Error vorr(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVorr, o0, o1); }
  inline Error vorr(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorr, cc), o0, o1); }
  inline Error vorr(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorr, dt), o0, o1); }
  inline Error vorr(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorr, dt, cc), o0, o1); }
  inline Error vpadd(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVpadd, o0, o1, o2); }
  inline Error vpadd(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpadd, cc), o0, o1, o2); }
  inline Error vpadd(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpadd, dt), o0, o1, o2); }
  inline Error vpadd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpadd, dt, cc), o0, o1, o2); }
  inline Error vpadd(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVpadd, o0, o1); }
  inline Error vpadd(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpadd, cc), o0, o1); }
  inline Error vpadd(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpadd, dt), o0, o1); }
  inline Error vpadd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpadd, dt, cc), o0, o1); }
  inline Error vpmax(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVpmax, o0, o1, o2); }
  inline Error vpmax(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpmax, cc), o0, o1, o2); }
  inline Error vpmax(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpmax, dt), o0, o1, o2); }
  inline Error vpmax(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpmax, dt, cc), o0, o1, o2); }
  inline Error vpmax(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVpmax, o0, o1); }
  inline Error vpmax(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpmax, cc), o0, o1); }
  inline Error vpmax(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpmax, dt), o0, o1); }
  inline Error vpmax(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpmax, dt, cc), o0, o1); }
  inline Error vpmin(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVpmin, o0, o1, o2); }
  inline Error vpmin(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpmin, cc), o0, o1, o2); }
  inline Error vpmin(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpmin, dt), o0, o1, o2); }
  inline Error vpmin(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpmin, dt, cc), o0, o1, o2); }
  inline Error vpmin(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVpmin, o0, o1); }
  inline Error vpmin(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpmin, cc), o0, o1); }
  inline Error vpmin(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpmin, dt), o0, o1); }
  inline Error vpmin(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpmin, dt, cc), o0, o1); }
  inline Error vqadd(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVqadd, o0, o1, o2); }
  inline Error vqadd(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqadd, cc), o0, o1, o2); }
  inline Error vqadd(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqadd, dt), o0, o1, o2); }
  inline Error vqadd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqadd, dt, cc), o0, o1, o2); }
  inline Error vqadd(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVqadd, o0, o1); }
  inline Error vqadd(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqadd, cc), o0, o1); }
  inline Error vqadd(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqadd, dt), o0, o1); }
  inline Error vqadd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqadd, dt, cc), o0, o1); }
  inline Error vqdmulh(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVqdmulh, o0, o1, o2); }
  inline Error vqdmulh(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmulh, cc), o0, o1, o2); }
  inline Error vqdmulh(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmulh, dt), o0, o1, o2); }
  inline Error vqdmulh(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmulh, dt, cc), o0, o1, o2); }
  inline Error vqdmulh(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVqdmulh, o0, o1); }
  inline Error vqdmulh(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmulh, cc), o0, o1); }
  inline Error vqdmulh(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmulh, dt), o0, o1); }
  inline Error vqdmulh(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmulh, dt, cc), o0, o1); }
  inline Error vqrdmulh(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVqrdmulh, o0, o1, o2); }
  inline Error vqrdmulh(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrdmulh, cc), o0, o1, o2); }
  inline Error vqrdmulh(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrdmulh, dt), o0, o1, o2); }
  inline Error vqrdmulh(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrdmulh, dt, cc), o0, o1, o2); }
  inline Error vqrdmulh(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVqrdmulh, o0, o1); }
  inline Error vqrdmulh(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrdmulh, cc), o0, o1); }
  inline Error vqrdmulh(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrdmulh, dt), o0, o1); }
  inline Error vqrdmulh(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrdmulh, dt, cc), o0, o1); }
  inline Error vqrshl(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVqrshl, o0, o1, o2); }
  inline Error vqrshl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrshl, cc), o0, o1, o2); }
  inline Error vqrshl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrshl, dt), o0, o1, o2); }
  inline Error vqrshl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrshl, dt, cc), o0, o1, o2); }
  inline Error vqrshl(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVqrshl, o0, o1); }
  inline Error vqrshl(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrshl, cc), o0, o1); }
  inline Error vqrshl(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrshl, dt), o0, o1); }
  inline Error vqrshl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrshl, dt, cc), o0, o1); }
  inline Error vqshl(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVqshl, o0, o1, o2); }
  inline Error vqshl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshl, cc), o0, o1, o2); }
  inline Error vqshl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshl, dt), o0, o1, o2); }
  inline Error vqshl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshl, dt, cc), o0, o1, o2); }
  inline Error vqshl(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVqshl, o0, o1); }
  inline Error vqshl(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshl, cc), o0, o1); }
  inline Error vqshl(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshl, dt), o0, o1); }
  inline Error vqshl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshl, dt, cc), o0, o1); }
  inline Error vqsub(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVqsub, o0, o1, o2); }
  inline Error vqsub(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqsub, cc), o0, o1, o2); }
  inline Error vqsub(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqsub, dt), o0, o1, o2); }
  inline Error vqsub(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqsub, dt, cc), o0, o1, o2); }
  inline Error vqsub(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVqsub, o0, o1); }
  inline Error vqsub(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqsub, cc), o0, o1); }
  inline Error vqsub(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqsub, dt), o0, o1); }
  inline Error vqsub(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqsub, dt, cc), o0, o1); }
  inline Error vrecps(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVrecps, o0, o1, o2); }
  inline Error vrecps(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrecps, cc), o0, o1, o2); }
  inline Error vrecps(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrecps, dt), o0, o1, o2); }
  inline Error vrecps(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrecps, dt, cc), o0, o1, o2); }
  inline Error vrecps(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrecps, o0, o1); }
  inline Error vrecps(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrecps, cc), o0, o1); }
  inline Error vrecps(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrecps, dt), o0, o1); }
  inline Error vrecps(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrecps, dt, cc), o0, o1); }
  inline Error vrhadd(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVrhadd, o0, o1, o2); }
  inline Error vrhadd(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrhadd, cc), o0, o1, o2); }
  inline Error vrhadd(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrhadd, dt), o0, o1, o2); }
  inline Error vrhadd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrhadd, dt, cc), o0, o1, o2); }
  inline Error vrhadd(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrhadd, o0, o1); }
  inline Error vrhadd(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrhadd, cc), o0, o1); }
  inline Error vrhadd(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrhadd, dt), o0, o1); }
  inline Error vrhadd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrhadd, dt, cc), o0, o1); }
  inline Error vrshl(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVrshl, o0, o1, o2); }
  inline Error vrshl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrshl, cc), o0, o1, o2); }
  inline Error vrshl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrshl, dt), o0, o1, o2); }
  inline Error vrshl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrshl, dt, cc), o0, o1, o2); }
  inline Error vrshl(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrshl, o0, o1); }
  inline Error vrshl(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrshl, cc), o0, o1); }
  inline Error vrshl(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrshl, dt), o0, o1); }
  inline Error vrshl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrshl, dt, cc), o0, o1); }
  inline Error vrsqrts(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVrsqrts, o0, o1, o2); }
  inline Error vrsqrts(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsqrts, cc), o0, o1, o2); }
  inline Error vrsqrts(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsqrts, dt), o0, o1, o2); }
  inline Error vrsqrts(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsqrts, dt, cc), o0, o1, o2); }
  inline Error vrsqrts(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrsqrts, o0, o1); }
  inline Error vrsqrts(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsqrts, cc), o0, o1); }
  inline Error vrsqrts(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsqrts, dt), o0, o1); }
  inline Error vrsqrts(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsqrts, dt, cc), o0, o1); }
  inline Error vshl(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVshl, o0, o1, o2); }
  inline Error vshl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshl, cc), o0, o1, o2); }
  inline Error vshl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshl, dt), o0, o1, o2); }
  inline Error vshl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshl, dt, cc), o0, o1, o2); }
  inline Error vshl(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVshl, o0, o1); }
  inline Error vshl(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshl, cc), o0, o1); }
  inline Error vshl(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshl, dt), o0, o1); }
  inline Error vshl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshl, dt, cc), o0, o1); }
  inline Error vtst(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVtst, o0, o1, o2); }
  inline Error vtst(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtst, cc), o0, o1, o2); }
  inline Error vtst(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtst, dt), o0, o1, o2); }
  inline Error vtst(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtst, dt, cc), o0, o1, o2); }
  inline Error vtst(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVtst, o0, o1); }
  inline Error vtst(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtst, cc), o0, o1); }
  inline Error vtst(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtst, dt), o0, o1); }
  inline Error vtst(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtst, dt, cc), o0, o1); }
  inline Error vand(const Vec& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdVand, o0, o1); }
  inline Error vand(CondCode cc, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVand, cc), o0, o1); }
  inline Error vand(DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVand, dt), o0, o1); }
  inline Error vand(CondCode cc, DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVand, dt, cc), o0, o1); }
  inline Error vbic(const Vec& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdVbic, o0, o1); }
  inline Error vbic(CondCode cc, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbic, cc), o0, o1); }
  inline Error vbic(DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbic, dt), o0, o1); }
  inline Error vbic(CondCode cc, DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVbic, dt, cc), o0, o1); }
  inline Error vorn(const Vec& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdVorn, o0, o1); }
  inline Error vorn(CondCode cc, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorn, cc), o0, o1); }
  inline Error vorn(DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorn, dt), o0, o1); }
  inline Error vorn(CondCode cc, DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorn, dt, cc), o0, o1); }
  inline Error vorr(const Vec& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdVorr, o0, o1); }
  inline Error vorr(CondCode cc, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorr, cc), o0, o1); }
  inline Error vorr(DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorr, dt), o0, o1); }
  inline Error vorr(CondCode cc, DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVorr, dt, cc), o0, o1); }
  inline Error vmvn(const Vec& o0, const Imm& o1) { return _emitter()->_emitI(Inst::kIdVmvn, o0, o1); }
  inline Error vmvn(CondCode cc, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmvn, cc), o0, o1); }
  inline Error vmvn(DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmvn, dt), o0, o1); }
  inline Error vmvn(CondCode cc, DataType dt, const Vec& o0, const Imm& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmvn, dt, cc), o0, o1); }
  inline Error vceq(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVceq, o0, o1, o2); }
  inline Error vceq(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVceq, cc), o0, o1, o2); }
  inline Error vceq(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVceq, dt), o0, o1, o2); }
  inline Error vceq(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVceq, dt, cc), o0, o1, o2); }
  inline Error vcge(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVcge, o0, o1, o2); }
  inline Error vcge(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcge, cc), o0, o1, o2); }
  inline Error vcge(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcge, dt), o0, o1, o2); }
  inline Error vcge(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcge, dt, cc), o0, o1, o2); }
  inline Error vcgt(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVcgt, o0, o1, o2); }
  inline Error vcgt(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcgt, cc), o0, o1, o2); }
  inline Error vcgt(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcgt, dt), o0, o1, o2); }
  inline Error vcgt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcgt, dt, cc), o0, o1, o2); }
  inline Error vcle(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVcle, o0, o1, o2); }
  inline Error vcle(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcle, cc), o0, o1, o2); }
  inline Error vcle(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcle, dt), o0, o1, o2); }
  inline Error vcle(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcle, dt, cc), o0, o1, o2); }
  inline Error vclt(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVclt, o0, o1, o2); }
  inline Error vclt(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVclt, cc), o0, o1, o2); }
  inline Error vclt(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVclt, dt), o0, o1, o2); }
  inline Error vclt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVclt, dt, cc), o0, o1, o2); }
  inline Error vqshl(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVqshl, o0, o1, o2); }
  inline Error vqshl(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshl, cc), o0, o1, o2); }
  inline Error vqshl(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshl, dt), o0, o1, o2); }
  inline Error vqshl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshl, dt, cc), o0, o1, o2); }
  inline Error vshl(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVshl, o0, o1, o2); }
  inline Error vshl(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshl, cc), o0, o1, o2); }
  inline Error vshl(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshl, dt), o0, o1, o2); }
  inline Error vshl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshl, dt, cc), o0, o1, o2); }
  inline Error vabal(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVabal, o0, o1, o2); }
  inline Error vabal(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabal, cc), o0, o1, o2); }
  inline Error vabal(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabal, dt), o0, o1, o2); }
  inline Error vabal(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabal, dt, cc), o0, o1, o2); }
  inline Error vabdl(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVabdl, o0, o1, o2); }
  inline Error vabdl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabdl, cc), o0, o1, o2); }
  inline Error vabdl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabdl, dt), o0, o1, o2); }
  inline Error vabdl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVabdl, dt, cc), o0, o1, o2); }
  inline Error vaddhn(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVaddhn, o0, o1, o2); }
  inline Error vaddhn(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaddhn, cc), o0, o1, o2); }
  inline Error vaddhn(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaddhn, dt), o0, o1, o2); }
  inline Error vaddhn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaddhn, dt, cc), o0, o1, o2); }
  inline Error vaddl(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVaddl, o0, o1, o2); }
  inline Error vaddl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaddl, cc), o0, o1, o2); }
  inline Error vaddl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaddl, dt), o0, o1, o2); }
  inline Error vaddl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaddl, dt, cc), o0, o1, o2); }
  inline Error vaddw(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVaddw, o0, o1, o2); }
  inline Error vaddw(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaddw, cc), o0, o1, o2); }
  inline Error vaddw(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaddw, dt), o0, o1, o2); }
  inline Error vaddw(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVaddw, dt, cc), o0, o1, o2); }
  inline Error vmlal(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVmlal, o0, o1, o2); }
  inline Error vmlal(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmlal, cc), o0, o1, o2); }
  inline Error vmlal(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmlal, dt), o0, o1, o2); }
  inline Error vmlal(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmlal, dt, cc), o0, o1, o2); }
  inline Error vmlsl(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVmlsl, o0, o1, o2); }
  inline Error vmlsl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmlsl, cc), o0, o1, o2); }
  inline Error vmlsl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmlsl, dt), o0, o1, o2); }
  inline Error vmlsl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmlsl, dt, cc), o0, o1, o2); }
  inline Error vmull(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVmull, o0, o1, o2); }
  inline Error vmull(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmull, cc), o0, o1, o2); }
  inline Error vmull(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmull, dt), o0, o1, o2); }
  inline Error vmull(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmull, dt, cc), o0, o1, o2); }
  inline Error vqdmlal(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVqdmlal, o0, o1, o2); }
  inline Error vqdmlal(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmlal, cc), o0, o1, o2); }
  inline Error vqdmlal(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmlal, dt), o0, o1, o2); }
  inline Error vqdmlal(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmlal, dt, cc), o0, o1, o2); }
  inline Error vqdmlsl(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVqdmlsl, o0, o1, o2); }
  inline Error vqdmlsl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmlsl, cc), o0, o1, o2); }
  inline Error vqdmlsl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmlsl, dt), o0, o1, o2); }
  inline Error vqdmlsl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmlsl, dt, cc), o0, o1, o2); }
  inline Error vqdmull(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVqdmull, o0, o1, o2); }
  inline Error vqdmull(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmull, cc), o0, o1, o2); }
  inline Error vqdmull(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmull, dt), o0, o1, o2); }
  inline Error vqdmull(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqdmull, dt, cc), o0, o1, o2); }
  inline Error vraddhn(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVraddhn, o0, o1, o2); }
  inline Error vraddhn(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVraddhn, cc), o0, o1, o2); }
  inline Error vraddhn(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVraddhn, dt), o0, o1, o2); }
  inline Error vraddhn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVraddhn, dt, cc), o0, o1, o2); }
  inline Error vrsubhn(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVrsubhn, o0, o1, o2); }
  inline Error vrsubhn(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsubhn, cc), o0, o1, o2); }
  inline Error vrsubhn(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsubhn, dt), o0, o1, o2); }
  inline Error vrsubhn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsubhn, dt, cc), o0, o1, o2); }
  inline Error vsubhn(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVsubhn, o0, o1, o2); }
  inline Error vsubhn(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsubhn, cc), o0, o1, o2); }
  inline Error vsubhn(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsubhn, dt), o0, o1, o2); }
  inline Error vsubhn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsubhn, dt, cc), o0, o1, o2); }
  inline Error vsubl(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVsubl, o0, o1, o2); }
  inline Error vsubl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsubl, cc), o0, o1, o2); }
  inline Error vsubl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsubl, dt), o0, o1, o2); }
  inline Error vsubl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsubl, dt, cc), o0, o1, o2); }
  inline Error vsubw(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVsubw, o0, o1, o2); }
  inline Error vsubw(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsubw, cc), o0, o1, o2); }
  inline Error vsubw(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsubw, dt), o0, o1, o2); }
  inline Error vsubw(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsubw, dt, cc), o0, o1, o2); }
  inline Error vcls(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcls, o0, o1); }
  inline Error vcls(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcls, cc), o0, o1); }
  inline Error vcls(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcls, dt), o0, o1); }
  inline Error vcls(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcls, dt, cc), o0, o1); }
  inline Error vclz(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVclz, o0, o1); }
  inline Error vclz(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVclz, cc), o0, o1); }
  inline Error vclz(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVclz, dt), o0, o1); }
  inline Error vclz(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVclz, dt, cc), o0, o1); }
  inline Error vcnt(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVcnt, o0, o1); }
  inline Error vcnt(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcnt, cc), o0, o1); }
  inline Error vcnt(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcnt, dt), o0, o1); }
  inline Error vcnt(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVcnt, dt, cc), o0, o1); }
  inline Error vmovl(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVmovl, o0, o1); }
  inline Error vmovl(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmovl, cc), o0, o1); }
  inline Error vmovl(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmovl, dt), o0, o1); }
  inline Error vmovl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmovl, dt, cc), o0, o1); }
  inline Error vmovn(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVmovn, o0, o1); }
  inline Error vmovn(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmovn, cc), o0, o1); }
  inline Error vmovn(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmovn, dt), o0, o1); }
  inline Error vmovn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmovn, dt, cc), o0, o1); }
  inline Error vmvn(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVmvn, o0, o1); }
  inline Error vmvn(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmvn, cc), o0, o1); }
  inline Error vmvn(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmvn, dt), o0, o1); }
  inline Error vmvn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVmvn, dt, cc), o0, o1); }
  inline Error vpadal(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVpadal, o0, o1); }
  inline Error vpadal(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpadal, cc), o0, o1); }
  inline Error vpadal(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpadal, dt), o0, o1); }
  inline Error vpadal(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpadal, dt, cc), o0, o1); }
  inline Error vpaddl(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVpaddl, o0, o1); }
  inline Error vpaddl(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpaddl, cc), o0, o1); }
  inline Error vpaddl(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpaddl, dt), o0, o1); }
  inline Error vpaddl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVpaddl, dt, cc), o0, o1); }
  inline Error vqabs(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVqabs, o0, o1); }
  inline Error vqabs(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqabs, cc), o0, o1); }
  inline Error vqabs(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqabs, dt), o0, o1); }
  inline Error vqabs(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqabs, dt, cc), o0, o1); }
  inline Error vqmovn(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVqmovn, o0, o1); }
  inline Error vqmovn(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqmovn, cc), o0, o1); }
  inline Error vqmovn(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqmovn, dt), o0, o1); }
  inline Error vqmovn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqmovn, dt, cc), o0, o1); }
  inline Error vqmovun(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVqmovun, o0, o1); }
  inline Error vqmovun(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqmovun, cc), o0, o1); }
  inline Error vqmovun(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqmovun, dt), o0, o1); }
  inline Error vqmovun(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqmovun, dt, cc), o0, o1); }
  inline Error vqneg(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVqneg, o0, o1); }
  inline Error vqneg(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqneg, cc), o0, o1); }
  inline Error vqneg(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqneg, dt), o0, o1); }
  inline Error vqneg(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqneg, dt, cc), o0, o1); }
  inline Error vrecpe(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrecpe, o0, o1); }
  inline Error vrecpe(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrecpe, cc), o0, o1); }
  inline Error vrecpe(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrecpe, dt), o0, o1); }
  inline Error vrecpe(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrecpe, dt, cc), o0, o1); }
  inline Error vrev16(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrev16, o0, o1); }
  inline Error vrev16(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrev16, cc), o0, o1); }
  inline Error vrev16(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrev16, dt), o0, o1); }
  inline Error vrev16(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrev16, dt, cc), o0, o1); }
  inline Error vrev32(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrev32, o0, o1); }
  inline Error vrev32(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrev32, cc), o0, o1); }
  inline Error vrev32(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrev32, dt), o0, o1); }
  inline Error vrev32(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrev32, dt, cc), o0, o1); }
  inline Error vrev64(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrev64, o0, o1); }
  inline Error vrev64(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrev64, cc), o0, o1); }
  inline Error vrev64(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrev64, dt), o0, o1); }
  inline Error vrev64(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrev64, dt, cc), o0, o1); }
  inline Error vrsqrte(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVrsqrte, o0, o1); }
  inline Error vrsqrte(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsqrte, cc), o0, o1); }
  inline Error vrsqrte(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsqrte, dt), o0, o1); }
  inline Error vrsqrte(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsqrte, dt, cc), o0, o1); }
  inline Error vswp(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVswp, o0, o1); }
  inline Error vswp(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVswp, cc), o0, o1); }
  inline Error vswp(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVswp, dt), o0, o1); }
  inline Error vswp(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVswp, dt, cc), o0, o1); }
  inline Error vtrn(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVtrn, o0, o1); }
  inline Error vtrn(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtrn, cc), o0, o1); }
  inline Error vtrn(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtrn, dt), o0, o1); }
  inline Error vtrn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtrn, dt, cc), o0, o1); }
  inline Error vuzp(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVuzp, o0, o1); }
  inline Error vuzp(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVuzp, cc), o0, o1); }
  inline Error vuzp(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVuzp, dt), o0, o1); }
  inline Error vuzp(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVuzp, dt, cc), o0, o1); }
  inline Error vzip(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVzip, o0, o1); }
  inline Error vzip(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVzip, cc), o0, o1); }
  inline Error vzip(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVzip, dt), o0, o1); }
  inline Error vzip(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVzip, dt, cc), o0, o1); }
  inline Error vqrshrn(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVqrshrn, o0, o1, o2); }
  inline Error vqrshrn(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrshrn, cc), o0, o1, o2); }
  inline Error vqrshrn(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrshrn, dt), o0, o1, o2); }
  inline Error vqrshrn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrshrn, dt, cc), o0, o1, o2); }
  inline Error vqrshrun(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVqrshrun, o0, o1, o2); }
  inline Error vqrshrun(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrshrun, cc), o0, o1, o2); }
  inline Error vqrshrun(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrshrun, dt), o0, o1, o2); }
  inline Error vqrshrun(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqrshrun, dt, cc), o0, o1, o2); }
  inline Error vqshlu(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVqshlu, o0, o1, o2); }
  inline Error vqshlu(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshlu, cc), o0, o1, o2); }
  inline Error vqshlu(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshlu, dt), o0, o1, o2); }
  inline Error vqshlu(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshlu, dt, cc), o0, o1, o2); }
  inline Error vqshrn(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVqshrn, o0, o1, o2); }
  inline Error vqshrn(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshrn, cc), o0, o1, o2); }
  inline Error vqshrn(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshrn, dt), o0, o1, o2); }
  inline Error vqshrn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshrn, dt, cc), o0, o1, o2); }
  inline Error vqshrun(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVqshrun, o0, o1, o2); }
  inline Error vqshrun(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshrun, cc), o0, o1, o2); }
  inline Error vqshrun(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshrun, dt), o0, o1, o2); }
  inline Error vqshrun(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVqshrun, dt, cc), o0, o1, o2); }
  inline Error vrshr(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVrshr, o0, o1, o2); }
  inline Error vrshr(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrshr, cc), o0, o1, o2); }
  inline Error vrshr(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrshr, dt), o0, o1, o2); }
  inline Error vrshr(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrshr, dt, cc), o0, o1, o2); }
  inline Error vrshrn(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVrshrn, o0, o1, o2); }
  inline Error vrshrn(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrshrn, cc), o0, o1, o2); }
  inline Error vrshrn(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrshrn, dt), o0, o1, o2); }
  inline Error vrshrn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrshrn, dt, cc), o0, o1, o2); }
  inline Error vrsra(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVrsra, o0, o1, o2); }
  inline Error vrsra(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsra, cc), o0, o1, o2); }
  inline Error vrsra(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsra, dt), o0, o1, o2); }
  inline Error vrsra(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVrsra, dt, cc), o0, o1, o2); }
  inline Error vshll(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVshll, o0, o1, o2); }
  inline Error vshll(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshll, cc), o0, o1, o2); }
  inline Error vshll(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshll, dt), o0, o1, o2); }
  inline Error vshll(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshll, dt, cc), o0, o1, o2); }
  inline Error vshr(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVshr, o0, o1, o2); }
  inline Error vshr(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshr, cc), o0, o1, o2); }
  inline Error vshr(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshr, dt), o0, o1, o2); }
  inline Error vshr(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshr, dt, cc), o0, o1, o2); }
  inline Error vshrn(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVshrn, o0, o1, o2); }
  inline Error vshrn(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshrn, cc), o0, o1, o2); }
  inline Error vshrn(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshrn, dt), o0, o1, o2); }
  inline Error vshrn(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVshrn, dt, cc), o0, o1, o2); }
  inline Error vsli(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVsli, o0, o1, o2); }
  inline Error vsli(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsli, cc), o0, o1, o2); }
  inline Error vsli(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsli, dt), o0, o1, o2); }
  inline Error vsli(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsli, dt, cc), o0, o1, o2); }
  inline Error vsra(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVsra, o0, o1, o2); }
  inline Error vsra(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsra, cc), o0, o1, o2); }
  inline Error vsra(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsra, dt), o0, o1, o2); }
  inline Error vsra(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsra, dt, cc), o0, o1, o2); }
  inline Error vsri(const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(Inst::kIdVsri, o0, o1, o2); }
  inline Error vsri(CondCode cc, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsri, cc), o0, o1, o2); }
  inline Error vsri(DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsri, dt), o0, o1, o2); }
  inline Error vsri(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Imm& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVsri, dt, cc), o0, o1, o2); }
  inline Error vext(const Vec& o0, const Vec& o1, const Vec& o2, const Imm& o3) { return _emitter()->_emitI(Inst::kIdVext, o0, o1, o2, o3); }
  inline Error vext(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVext, cc), o0, o1, o2, o3); }
  inline Error vext(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVext, dt), o0, o1, o2, o3); }
  inline Error vext(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Imm& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVext, dt, cc), o0, o1, o2, o3); }
  inline Error vtbl(const Vec& o0, const VecList& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVtbl, o0, o1, o2); }
  inline Error vtbl(CondCode cc, const Vec& o0, const VecList& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, cc), o0, o1, o2); }
  inline Error vtbl(DataType dt, const Vec& o0, const VecList& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, dt), o0, o1, o2); }
  inline Error vtbl(CondCode cc, DataType dt, const Vec& o0, const VecList& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, dt, cc), o0, o1, o2); }
  inline Error vtbl(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVtbl, o0, o1, o2); }
  inline Error vtbl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, cc), o0, o1, o2); }
  inline Error vtbl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, dt), o0, o1, o2); }
  inline Error vtbl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, dt, cc), o0, o1, o2); }
  inline Error vtbl(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3) { return _emitter()->_emitI(Inst::kIdVtbl, o0, o1, o2, o3); }
  inline Error vtbl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, cc), o0, o1, o2, o3); }
  inline Error vtbl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, dt), o0, o1, o2, o3); }
  inline Error vtbl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, dt, cc), o0, o1, o2, o3); }
  inline Error vtbl(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4) { return _emitter()->_emitI(Inst::kIdVtbl, o0, o1, o2, o3, o4); }
  inline Error vtbl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, cc), o0, o1, o2, o3, o4); }
  inline Error vtbl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, dt), o0, o1, o2, o3, o4); }
  inline Error vtbl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, dt, cc), o0, o1, o2, o3, o4); }
  inline Error vtbl(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4, const Vec& o5) { return _emitter()->_emitI(Inst::kIdVtbl, o0, o1, o2, o3, o4, o5); }
  inline Error vtbl(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4, const Vec& o5) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, cc), o0, o1, o2, o3, o4, o5); }
  inline Error vtbl(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4, const Vec& o5) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, dt), o0, o1, o2, o3, o4, o5); }
  inline Error vtbl(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4, const Vec& o5) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbl, dt, cc), o0, o1, o2, o3, o4, o5); }
  inline Error vtbx(const Vec& o0, const VecList& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVtbx, o0, o1, o2); }
  inline Error vtbx(CondCode cc, const Vec& o0, const VecList& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, cc), o0, o1, o2); }
  inline Error vtbx(DataType dt, const Vec& o0, const VecList& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, dt), o0, o1, o2); }
  inline Error vtbx(CondCode cc, DataType dt, const Vec& o0, const VecList& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, dt, cc), o0, o1, o2); }
  inline Error vtbx(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdVtbx, o0, o1, o2); }
  inline Error vtbx(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, cc), o0, o1, o2); }
  inline Error vtbx(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, dt), o0, o1, o2); }
  inline Error vtbx(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, dt, cc), o0, o1, o2); }
  inline Error vtbx(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3) { return _emitter()->_emitI(Inst::kIdVtbx, o0, o1, o2, o3); }
  inline Error vtbx(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, cc), o0, o1, o2, o3); }
  inline Error vtbx(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, dt), o0, o1, o2, o3); }
  inline Error vtbx(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, dt, cc), o0, o1, o2, o3); }
  inline Error vtbx(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4) { return _emitter()->_emitI(Inst::kIdVtbx, o0, o1, o2, o3, o4); }
  inline Error vtbx(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, cc), o0, o1, o2, o3, o4); }
  inline Error vtbx(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, dt), o0, o1, o2, o3, o4); }
  inline Error vtbx(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, dt, cc), o0, o1, o2, o3, o4); }
  inline Error vtbx(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4, const Vec& o5) { return _emitter()->_emitI(Inst::kIdVtbx, o0, o1, o2, o3, o4, o5); }
  inline Error vtbx(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4, const Vec& o5) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, cc), o0, o1, o2, o3, o4, o5); }
  inline Error vtbx(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4, const Vec& o5) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, dt), o0, o1, o2, o3, o4, o5); }
  inline Error vtbx(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Vec& o4, const Vec& o5) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVtbx, dt, cc), o0, o1, o2, o3, o4, o5); }
  inline Error vdup(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdVdup, o0, o1); }
  inline Error vdup(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVdup, cc), o0, o1); }
  inline Error vdup(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVdup, dt), o0, o1); }
  inline Error vdup(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVdup, dt, cc), o0, o1); }
  inline Error vdup(const Vec& o0, const Gp& o1) { return _emitter()->_emitI(Inst::kIdVdup, o0, o1); }
  inline Error vdup(CondCode cc, const Vec& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVdup, cc), o0, o1); }
  inline Error vdup(DataType dt, const Vec& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVdup, dt), o0, o1); }
  inline Error vdup(CondCode cc, DataType dt, const Vec& o0, const Gp& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVdup, dt, cc), o0, o1); }
  inline Error vld1(const VecList& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVld1, o0, o1); }
  inline Error vld1(CondCode cc, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, cc), o0, o1); }
  inline Error vld1(DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, dt), o0, o1); }
  inline Error vld1(CondCode cc, DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, dt, cc), o0, o1); }
  inline Error vld1(const Vec& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVld1, o0, o1); }
  inline Error vld1(CondCode cc, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, cc), o0, o1); }
  inline Error vld1(DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, dt), o0, o1); }
  inline Error vld1(CondCode cc, DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, dt, cc), o0, o1); }
  inline Error vld1(const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdVld1, o0, o1, o2); }
  inline Error vld1(CondCode cc, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, cc), o0, o1, o2); }
  inline Error vld1(DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, dt), o0, o1, o2); }
  inline Error vld1(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, dt, cc), o0, o1, o2); }
  inline Error vld1(const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(Inst::kIdVld1, o0, o1, o2, o3); }
  inline Error vld1(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, cc), o0, o1, o2, o3); }
  inline Error vld1(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, dt), o0, o1, o2, o3); }
  inline Error vld1(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, dt, cc), o0, o1, o2, o3); }
  inline Error vld1(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(Inst::kIdVld1, o0, o1, o2, o3, o4); }
  inline Error vld1(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, cc), o0, o1, o2, o3, o4); }
  inline Error vld1(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, dt), o0, o1, o2, o3, o4); }
  inline Error vld1(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld1, dt, cc), o0, o1, o2, o3, o4); }
  inline Error vld2(const VecList& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVld2, o0, o1); }
  inline Error vld2(CondCode cc, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, cc), o0, o1); }
  inline Error vld2(DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, dt), o0, o1); }
  inline Error vld2(CondCode cc, DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, dt, cc), o0, o1); }
  inline Error vld2(const Vec& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVld2, o0, o1); }
  inline Error vld2(CondCode cc, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, cc), o0, o1); }
  inline Error vld2(DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, dt), o0, o1); }
  inline Error vld2(CondCode cc, DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, dt, cc), o0, o1); }
  inline Error vld2(const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdVld2, o0, o1, o2); }
  inline Error vld2(CondCode cc, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, cc), o0, o1, o2); }
  inline Error vld2(DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, dt), o0, o1, o2); }
  inline Error vld2(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, dt, cc), o0, o1, o2); }
  inline Error vld2(const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(Inst::kIdVld2, o0, o1, o2, o3); }
  inline Error vld2(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, cc), o0, o1, o2, o3); }
  inline Error vld2(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, dt), o0, o1, o2, o3); }
  inline Error vld2(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, dt, cc), o0, o1, o2, o3); }
  inline Error vld2(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(Inst::kIdVld2, o0, o1, o2, o3, o4); }
  inline Error vld2(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, cc), o0, o1, o2, o3, o4); }
  inline Error vld2(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, dt), o0, o1, o2, o3, o4); }
  inline Error vld2(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld2, dt, cc), o0, o1, o2, o3, o4); }
  inline Error vld3(const VecList& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVld3, o0, o1); }
  inline Error vld3(CondCode cc, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, cc), o0, o1); }
  inline Error vld3(DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, dt), o0, o1); }
  inline Error vld3(CondCode cc, DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, dt, cc), o0, o1); }
  inline Error vld3(const Vec& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVld3, o0, o1); }
  inline Error vld3(CondCode cc, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, cc), o0, o1); }
  inline Error vld3(DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, dt), o0, o1); }
  inline Error vld3(CondCode cc, DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, dt, cc), o0, o1); }
  inline Error vld3(const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdVld3, o0, o1, o2); }
  inline Error vld3(CondCode cc, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, cc), o0, o1, o2); }
  inline Error vld3(DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, dt), o0, o1, o2); }
  inline Error vld3(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, dt, cc), o0, o1, o2); }
  inline Error vld3(const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(Inst::kIdVld3, o0, o1, o2, o3); }
  inline Error vld3(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, cc), o0, o1, o2, o3); }
  inline Error vld3(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, dt), o0, o1, o2, o3); }
  inline Error vld3(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, dt, cc), o0, o1, o2, o3); }
  inline Error vld3(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(Inst::kIdVld3, o0, o1, o2, o3, o4); }
  inline Error vld3(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, cc), o0, o1, o2, o3, o4); }
  inline Error vld3(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, dt), o0, o1, o2, o3, o4); }
  inline Error vld3(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld3, dt, cc), o0, o1, o2, o3, o4); }
  inline Error vld4(const VecList& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVld4, o0, o1); }
  inline Error vld4(CondCode cc, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, cc), o0, o1); }
  inline Error vld4(DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, dt), o0, o1); }
  inline Error vld4(CondCode cc, DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, dt, cc), o0, o1); }
  inline Error vld4(const Vec& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVld4, o0, o1); }
  inline Error vld4(CondCode cc, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, cc), o0, o1); }
  inline Error vld4(DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, dt), o0, o1); }
  inline Error vld4(CondCode cc, DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, dt, cc), o0, o1); }
  inline Error vld4(const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdVld4, o0, o1, o2); }
  inline Error vld4(CondCode cc, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, cc), o0, o1, o2); }
  inline Error vld4(DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, dt), o0, o1, o2); }
  inline Error vld4(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, dt, cc), o0, o1, o2); }
  inline Error vld4(const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(Inst::kIdVld4, o0, o1, o2, o3); }
  inline Error vld4(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, cc), o0, o1, o2, o3); }
  inline Error vld4(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, dt), o0, o1, o2, o3); }
  inline Error vld4(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, dt, cc), o0, o1, o2, o3); }
  inline Error vld4(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(Inst::kIdVld4, o0, o1, o2, o3, o4); }
  inline Error vld4(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, cc), o0, o1, o2, o3, o4); }
  inline Error vld4(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, dt), o0, o1, o2, o3, o4); }
  inline Error vld4(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVld4, dt, cc), o0, o1, o2, o3, o4); }
  inline Error vst1(const VecList& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVst1, o0, o1); }
  inline Error vst1(CondCode cc, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, cc), o0, o1); }
  inline Error vst1(DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, dt), o0, o1); }
  inline Error vst1(CondCode cc, DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, dt, cc), o0, o1); }
  inline Error vst1(const Vec& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVst1, o0, o1); }
  inline Error vst1(CondCode cc, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, cc), o0, o1); }
  inline Error vst1(DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, dt), o0, o1); }
  inline Error vst1(CondCode cc, DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, dt, cc), o0, o1); }
  inline Error vst1(const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdVst1, o0, o1, o2); }
  inline Error vst1(CondCode cc, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, cc), o0, o1, o2); }
  inline Error vst1(DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, dt), o0, o1, o2); }
  inline Error vst1(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, dt, cc), o0, o1, o2); }
  inline Error vst1(const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(Inst::kIdVst1, o0, o1, o2, o3); }
  inline Error vst1(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, cc), o0, o1, o2, o3); }
  inline Error vst1(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, dt), o0, o1, o2, o3); }
  inline Error vst1(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, dt, cc), o0, o1, o2, o3); }
  inline Error vst1(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(Inst::kIdVst1, o0, o1, o2, o3, o4); }
  inline Error vst1(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, cc), o0, o1, o2, o3, o4); }
  inline Error vst1(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, dt), o0, o1, o2, o3, o4); }
  inline Error vst1(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst1, dt, cc), o0, o1, o2, o3, o4); }
  inline Error vst2(const VecList& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVst2, o0, o1); }
  inline Error vst2(CondCode cc, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, cc), o0, o1); }
  inline Error vst2(DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, dt), o0, o1); }
  inline Error vst2(CondCode cc, DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, dt, cc), o0, o1); }
  inline Error vst2(const Vec& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVst2, o0, o1); }
  inline Error vst2(CondCode cc, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, cc), o0, o1); }
  inline Error vst2(DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, dt), o0, o1); }
  inline Error vst2(CondCode cc, DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, dt, cc), o0, o1); }
  inline Error vst2(const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdVst2, o0, o1, o2); }
  inline Error vst2(CondCode cc, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, cc), o0, o1, o2); }
  inline Error vst2(DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, dt), o0, o1, o2); }
  inline Error vst2(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, dt, cc), o0, o1, o2); }
  inline Error vst2(const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(Inst::kIdVst2, o0, o1, o2, o3); }
  inline Error vst2(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, cc), o0, o1, o2, o3); }
  inline Error vst2(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, dt), o0, o1, o2, o3); }
  inline Error vst2(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, dt, cc), o0, o1, o2, o3); }
  inline Error vst2(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(Inst::kIdVst2, o0, o1, o2, o3, o4); }
  inline Error vst2(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, cc), o0, o1, o2, o3, o4); }
  inline Error vst2(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, dt), o0, o1, o2, o3, o4); }
  inline Error vst2(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst2, dt, cc), o0, o1, o2, o3, o4); }
  inline Error vst3(const VecList& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVst3, o0, o1); }
  inline Error vst3(CondCode cc, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, cc), o0, o1); }
  inline Error vst3(DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, dt), o0, o1); }
  inline Error vst3(CondCode cc, DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, dt, cc), o0, o1); }
  inline Error vst3(const Vec& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVst3, o0, o1); }
  inline Error vst3(CondCode cc, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, cc), o0, o1); }
  inline Error vst3(DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, dt), o0, o1); }
  inline Error vst3(CondCode cc, DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, dt, cc), o0, o1); }
  inline Error vst3(const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdVst3, o0, o1, o2); }
  inline Error vst3(CondCode cc, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, cc), o0, o1, o2); }
  inline Error vst3(DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, dt), o0, o1, o2); }
  inline Error vst3(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, dt, cc), o0, o1, o2); }
  inline Error vst3(const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(Inst::kIdVst3, o0, o1, o2, o3); }
  inline Error vst3(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, cc), o0, o1, o2, o3); }
  inline Error vst3(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, dt), o0, o1, o2, o3); }
  inline Error vst3(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, dt, cc), o0, o1, o2, o3); }
  inline Error vst3(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(Inst::kIdVst3, o0, o1, o2, o3, o4); }
  inline Error vst3(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, cc), o0, o1, o2, o3, o4); }
  inline Error vst3(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, dt), o0, o1, o2, o3, o4); }
  inline Error vst3(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst3, dt, cc), o0, o1, o2, o3, o4); }
  inline Error vst4(const VecList& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVst4, o0, o1); }
  inline Error vst4(CondCode cc, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, cc), o0, o1); }
  inline Error vst4(DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, dt), o0, o1); }
  inline Error vst4(CondCode cc, DataType dt, const VecList& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, dt, cc), o0, o1); }
  inline Error vst4(const Vec& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdVst4, o0, o1); }
  inline Error vst4(CondCode cc, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, cc), o0, o1); }
  inline Error vst4(DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, dt), o0, o1); }
  inline Error vst4(CondCode cc, DataType dt, const Vec& o0, const Mem& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, dt, cc), o0, o1); }
  inline Error vst4(const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(Inst::kIdVst4, o0, o1, o2); }
  inline Error vst4(CondCode cc, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, cc), o0, o1, o2); }
  inline Error vst4(DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, dt), o0, o1, o2); }
  inline Error vst4(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Mem& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, dt, cc), o0, o1, o2); }
  inline Error vst4(const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(Inst::kIdVst4, o0, o1, o2, o3); }
  inline Error vst4(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, cc), o0, o1, o2, o3); }
  inline Error vst4(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, dt), o0, o1, o2, o3); }
  inline Error vst4(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Mem& o3) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, dt, cc), o0, o1, o2, o3); }
  inline Error vst4(const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(Inst::kIdVst4, o0, o1, o2, o3, o4); }
  inline Error vst4(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, cc), o0, o1, o2, o3, o4); }
  inline Error vst4(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, dt), o0, o1, o2, o3, o4); }
  inline Error vst4(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2, const Vec& o3, const Mem& o4) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdVst4, dt, cc), o0, o1, o2, o3, o4); }

  // Crypto

  inline Error aesd(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdAesd, o0, o1); }
  inline Error aesd(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAesd, cc), o0, o1); }
  inline Error aesd(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAesd, dt), o0, o1); }
  inline Error aesd(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAesd, dt, cc), o0, o1); }
  inline Error aese(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdAese, o0, o1); }
  inline Error aese(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAese, cc), o0, o1); }
  inline Error aese(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAese, dt), o0, o1); }
  inline Error aese(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAese, dt, cc), o0, o1); }
  inline Error aesimc(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdAesimc, o0, o1); }
  inline Error aesimc(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAesimc, cc), o0, o1); }
  inline Error aesimc(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAesimc, dt), o0, o1); }
  inline Error aesimc(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAesimc, dt, cc), o0, o1); }
  inline Error aesmc(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdAesmc, o0, o1); }
  inline Error aesmc(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAesmc, cc), o0, o1); }
  inline Error aesmc(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAesmc, dt), o0, o1); }
  inline Error aesmc(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdAesmc, dt, cc), o0, o1); }
  inline Error sha1h(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdSha1h, o0, o1); }
  inline Error sha1h(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1h, cc), o0, o1); }
  inline Error sha1h(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1h, dt), o0, o1); }
  inline Error sha1h(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1h, dt, cc), o0, o1); }
  inline Error sha1su1(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdSha1su1, o0, o1); }
  inline Error sha1su1(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1su1, cc), o0, o1); }
  inline Error sha1su1(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1su1, dt), o0, o1); }
  inline Error sha1su1(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1su1, dt, cc), o0, o1); }
  inline Error sha256su0(const Vec& o0, const Vec& o1) { return _emitter()->_emitI(Inst::kIdSha256su0, o0, o1); }
  inline Error sha256su0(CondCode cc, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha256su0, cc), o0, o1); }
  inline Error sha256su0(DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha256su0, dt), o0, o1); }
  inline Error sha256su0(CondCode cc, DataType dt, const Vec& o0, const Vec& o1) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha256su0, dt, cc), o0, o1); }
  inline Error sha1c(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdSha1c, o0, o1, o2); }
  inline Error sha1c(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1c, cc), o0, o1, o2); }
  inline Error sha1c(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1c, dt), o0, o1, o2); }
  inline Error sha1c(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1c, dt, cc), o0, o1, o2); }
  inline Error sha1m(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdSha1m, o0, o1, o2); }
  inline Error sha1m(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1m, cc), o0, o1, o2); }
  inline Error sha1m(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1m, dt), o0, o1, o2); }
  inline Error sha1m(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1m, dt, cc), o0, o1, o2); }
  inline Error sha1p(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdSha1p, o0, o1, o2); }
  inline Error sha1p(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1p, cc), o0, o1, o2); }
  inline Error sha1p(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1p, dt), o0, o1, o2); }
  inline Error sha1p(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1p, dt, cc), o0, o1, o2); }
  inline Error sha1su0(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdSha1su0, o0, o1, o2); }
  inline Error sha1su0(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1su0, cc), o0, o1, o2); }
  inline Error sha1su0(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1su0, dt), o0, o1, o2); }
  inline Error sha1su0(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha1su0, dt, cc), o0, o1, o2); }
  inline Error sha256h(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdSha256h, o0, o1, o2); }
  inline Error sha256h(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha256h, cc), o0, o1, o2); }
  inline Error sha256h(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha256h, dt), o0, o1, o2); }
  inline Error sha256h(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha256h, dt, cc), o0, o1, o2); }
  inline Error sha256h2(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdSha256h2, o0, o1, o2); }
  inline Error sha256h2(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha256h2, cc), o0, o1, o2); }
  inline Error sha256h2(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha256h2, dt), o0, o1, o2); }
  inline Error sha256h2(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha256h2, dt, cc), o0, o1, o2); }
  inline Error sha256su1(const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(Inst::kIdSha256su1, o0, o1, o2); }
  inline Error sha256su1(CondCode cc, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha256su1, cc), o0, o1, o2); }
  inline Error sha256su1(DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha256su1, dt), o0, o1, o2); }
  inline Error sha256su1(CondCode cc, DataType dt, const Vec& o0, const Vec& o1, const Vec& o2) { return _emitter()->_emitI(BaseInst::composeARMInstId(Inst::kIdSha256su1, dt, cc), o0, o1, o2); }

  // Aliases & Convenience
  inline Error ldmia(const Gp& o0, const GpList& o1) { return ldm(o0, o1); }
  inline Error ldmia(const Mem& o0, const GpList& o1) { return ldm(o0, o1); }
  inline Error ldmfd(const Gp& o0, const GpList& o1) { return ldm(o0, o1); }
  inline Error ldmfd(const Mem& o0, const GpList& o1) { return ldm(o0, o1); }
  inline Error stmia(const Gp& o0, const GpList& o1) { return stm(o0, o1); }
  inline Error stmia(const Mem& o0, const GpList& o1) { return stm(o0, o1); }
  inline Error stmfd(const Gp& o0, const GpList& o1) { return stmdb(o0, o1); }
  inline Error stmfd(const Mem& o0, const GpList& o1) { return stmdb(o0, o1); }
  inline Error vldmia(const Gp& o0, const VecList& o1) { return vldm(o0, o1); }
  inline Error vldmia(const Mem& o0, const VecList& o1) { return vldm(o0, o1); }
  inline Error vstmia(const Gp& o0, const VecList& o1) { return vstm(o0, o1); }
  inline Error vstmia(const Mem& o0, const VecList& o1) { return vstm(o0, o1); }

  inline Error dmb(BarrierOption option) { return dmb(Imm(uint32_t(option))); }
  inline Error dsb(BarrierOption option) { return dsb(Imm(uint32_t(option))); }
  inline Error isb(BarrierOption option) { return isb(Imm(uint32_t(option))); }

  inline Error setend(Endian e) { return setend(Imm(uint32_t(e))); }

  inline Error vmrs(const Gp& o0, FpSysReg reg) { return vmrs(o0, Imm(uint32_t(reg))); }
  inline Error vmrs(CondCode cc, const Gp& o0, FpSysReg reg) { return vmrs(cc, o0, Imm(uint32_t(reg))); }
  inline Error vmsr(FpSysReg reg, const Gp& o1) { return vmsr(Imm(uint32_t(reg)), o1); }
  inline Error vmsr(CondCode cc, FpSysReg reg, const Gp& o1) { return vmsr(cc, Imm(uint32_t(reg)), o1); }

  //! \}
};

//! Emitter (AArch32).
//!
//! \note This class cannot be instantiated, you can only cast to it and use it as emitter that emits to
//! `a32::Assembler`.
class Emitter : public BaseEmitter, public EmitterExplicitT<Emitter> {
  ASMJIT_NONCONSTRUCTIBLE(Emitter)
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A32EMITTER_H_INCLUDED
