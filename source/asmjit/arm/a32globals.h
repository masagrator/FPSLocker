// This file is based on part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A32GLOBALS_H_INCLUDED
#define ASMJIT_ARM_A32GLOBALS_H_INCLUDED

#include "../arm/armglobals.h"

//! \namespace asmjit::a32
//! \ingroup asmjit_a32
//!
//! AArch32 backend (A32 instruction set only, no Thumb/T32).
//!
//! The backend targets AArch32 execution state of ARMv8-A processors. It provides the base A32 instruction set,
//! VFP (FP-ARMv8), Advanced SIMD (NEON), Crypto (AES/SHA1/SHA256/PMULL) and CRC32 extensions.

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

//! \addtogroup asmjit_a32
//! \{

//! List of all AArch32 instructions (used to generate `Inst::Id` and instruction names).
//!
//! The name of each instruction is the lower-cased identifier.
#define ASMJIT_A32_INST_LIST(V) \
  /* Base - Data Processing */ \
  V(Adc) V(Adcs) V(Add) V(Adds) V(Adr) V(And) V(Ands) V(Asr) V(Asrs) V(Bic) V(Bics) V(Cmn) V(Cmp) V(Eor) V(Eors) \
  V(Lsl) V(Lsls) V(Lsr) V(Lsrs) V(Mov) V(Movs) V(Movt) V(Movw) V(Mvn) V(Mvns) V(Orr) V(Orrs) V(Ror) V(Rors) \
  V(Rrx) V(Rrxs) V(Rsb) V(Rsbs) V(Rsc) V(Rscs) V(Sbc) V(Sbcs) V(Sub) V(Subs) V(Teq) V(Tst) \
  /* Base - Multiply & Divide */ \
  V(Mla) V(Mlas) V(Mls) V(Mul) V(Muls) V(Smlal) V(Smlals) V(Smull) V(Smulls) V(Umaal) V(Umlal) V(Umlals) \
  V(Umull) V(Umulls) V(Sdiv) V(Udiv) \
  V(Smlabb) V(Smlabt) V(Smlatb) V(Smlatt) V(Smlawb) V(Smlawt) V(Smulbb) V(Smulbt) V(Smultb) V(Smultt) \
  V(Smulwb) V(Smulwt) V(Smlalbb) V(Smlalbt) V(Smlaltb) V(Smlaltt) V(Smuad) V(Smuadx) V(Smusd) V(Smusdx) \
  V(Smlad) V(Smladx) V(Smlsd) V(Smlsdx) V(Smlald) V(Smlaldx) V(Smlsld) V(Smlsldx) V(Smmul) V(Smmulr) \
  V(Smmla) V(Smmlar) V(Smmls) V(Smmlsr) \
  /* Base - Saturating & Parallel */ \
  V(Qadd) V(Qsub) V(Qdadd) V(Qdsub) V(Ssat) V(Usat) V(Ssat16) V(Usat16) \
  V(Sadd16) V(Sasx) V(Ssax) V(Ssub16) V(Sadd8) V(Ssub8) \
  V(Qadd16) V(Qasx) V(Qsax) V(Qsub16) V(Qadd8) V(Qsub8) \
  V(Shadd16) V(Shasx) V(Shsax) V(Shsub16) V(Shadd8) V(Shsub8) \
  V(Uadd16) V(Uasx) V(Usax) V(Usub16) V(Uadd8) V(Usub8) \
  V(Uqadd16) V(Uqasx) V(Uqsax) V(Uqsub16) V(Uqadd8) V(Uqsub8) \
  V(Uhadd16) V(Uhasx) V(Uhsax) V(Uhsub16) V(Uhadd8) V(Uhsub8) \
  /* Base - Extend, Pack, Bitfield, Misc */ \
  V(Sxtb) V(Sxth) V(Sxtb16) V(Uxtb) V(Uxth) V(Uxtb16) V(Sxtab) V(Sxtah) V(Sxtab16) V(Uxtab) V(Uxtah) V(Uxtab16) \
  V(Pkhbt) V(Pkhtb) V(Sel) V(Rev) V(Rev16) V(Revsh) V(Rbit) V(Clz) V(Usad8) V(Usada8) \
  V(Bfc) V(Bfi) V(Sbfx) V(Ubfx) \
  V(Crc32b) V(Crc32h) V(Crc32w) V(Crc32cb) V(Crc32ch) V(Crc32cw) \
  /* Base - Branch */ \
  V(B) V(Bl) V(Blx) V(Bx) V(Bxj) \
  /* Base - Load & Store */ \
  V(Ldr) V(Ldrb) V(Ldrh) V(Ldrsb) V(Ldrsh) V(Ldrd) V(Str) V(Strb) V(Strh) V(Strd) \
  V(Ldrt) V(Ldrbt) V(Ldrht) V(Ldrsbt) V(Ldrsht) V(Strt) V(Strbt) V(Strht) \
  V(Ldm) V(Ldmib) V(Ldmda) V(Ldmdb) V(Stm) V(Stmib) V(Stmda) V(Stmdb) V(Push) V(Pop) \
  V(Ldrex) V(Ldrexb) V(Ldrexh) V(Ldrexd) V(Strex) V(Strexb) V(Strexh) V(Strexd) \
  V(Lda) V(Ldab) V(Ldah) V(Stl) V(Stlb) V(Stlh) V(Ldaex) V(Ldaexb) V(Ldaexh) V(Ldaexd) \
  V(Stlex) V(Stlexb) V(Stlexh) V(Stlexd) V(Clrex) \
  /* Base - Hints, Exceptions, Barriers, System */ \
  V(Nop) V(Yield) V(Wfe) V(Wfi) V(Sev) V(Sevl) V(Csdb) V(Dbg) \
  V(Svc) V(Bkpt) V(Udf) V(Hvc) V(Smc) V(Eret) \
  V(Dmb) V(Dsb) V(Isb) \
  V(Mrs) V(Msr) V(Cps) V(Cpsie) V(Cpsid) V(Setend) V(Pld) V(Pldw) V(Pli) \
  V(Mrc) V(Mcr) V(Mrrc) V(Mcrr) \
  /* VFP */ \
  V(Vabs) V(Vadd) V(Vcmp) V(Vcmpe) V(Vcvt) V(Vcvta) V(Vcvtb) V(Vcvtm) V(Vcvtn) V(Vcvtp) V(Vcvtr) V(Vcvtt) \
  V(Vdiv) V(Vfma) V(Vfms) V(Vfnma) V(Vfnms) V(Vldm) V(Vldmdb) V(Vldr) V(Vmaxnm) V(Vminnm) V(Vmla) V(Vmls) \
  V(Vmov) V(Vmrs) V(Vmsr) V(Vmul) V(Vneg) V(Vnmla) V(Vnmls) V(Vnmul) V(Vpop) V(Vpush) \
  V(Vrinta) V(Vrintm) V(Vrintn) V(Vrintp) V(Vrintr) V(Vrintx) V(Vrintz) \
  V(Vseleq) V(Vselge) V(Vselgt) V(Vselvs) V(Vsqrt) V(Vstm) V(Vstmdb) V(Vstr) V(Vsub) \
  /* Advanced SIMD */ \
  V(Vaba) V(Vabal) V(Vabd) V(Vabdl) V(Vacge) V(Vacgt) V(Vacle) V(Vaclt) V(Vaddhn) V(Vaddl) V(Vaddw) \
  V(Vand) V(Vbic) V(Vbif) V(Vbit) V(Vbsl) V(Vceq) V(Vcge) V(Vcgt) V(Vcle) V(Vclt) V(Vcls) V(Vclz) V(Vcnt) \
  V(Vdup) V(Veor) V(Vext) V(Vhadd) V(Vhsub) V(Vld1) V(Vld2) V(Vld3) V(Vld4) V(Vmax) V(Vmin) V(Vmlal) V(Vmlsl) \
  V(Vmovl) V(Vmovn) V(Vmull) V(Vmvn) V(Vorn) V(Vorr) V(Vpadal) V(Vpadd) V(Vpaddl) V(Vpmax) V(Vpmin) \
  V(Vqabs) V(Vqadd) V(Vqdmlal) V(Vqdmlsl) V(Vqdmulh) V(Vqdmull) V(Vqmovn) V(Vqmovun) V(Vqneg) V(Vqrdmulh) \
  V(Vqrshl) V(Vqrshrn) V(Vqrshrun) V(Vqshl) V(Vqshlu) V(Vqshrn) V(Vqshrun) V(Vqsub) V(Vraddhn) V(Vrecpe) \
  V(Vrecps) V(Vrev16) V(Vrev32) V(Vrev64) V(Vrhadd) V(Vrshl) V(Vrshr) V(Vrshrn) V(Vrsqrte) V(Vrsqrts) \
  V(Vrsra) V(Vrsubhn) V(Vshl) V(Vshll) V(Vshr) V(Vshrn) V(Vsli) V(Vsra) V(Vsri) V(Vst1) V(Vst2) V(Vst3) \
  V(Vst4) V(Vsubhn) V(Vsubl) V(Vsubw) V(Vswp) V(Vtbl) V(Vtbx) V(Vtrn) V(Vtst) V(Vuzp) V(Vzip) \
  /* Crypto */ \
  V(Aesd) V(Aese) V(Aesimc) V(Aesmc) V(Sha1c) V(Sha1h) V(Sha1m) V(Sha1p) V(Sha1su0) V(Sha1su1) \
  V(Sha256h) V(Sha256h2) V(Sha256su0) V(Sha256su1)

//! AArch32 instruction.
//!
//! \note Only used to hold ARM-specific enumerations and static functions.
namespace Inst {
  //! Instruction id.
  enum Id : uint32_t {
    //! Invalid instruction.
    kIdNone = 0,

#define ASMJIT_A32_INST_ID(NAME) kId##NAME,
    ASMJIT_A32_INST_LIST(ASMJIT_A32_INST_ID)
#undef ASMJIT_A32_INST_ID

    //! Count of all instructions.
    _kIdCount
  };

  //! Tests whether the `instId` is defined (counts also Inst::kIdNone, which must be zero).
  static ASMJIT_INLINE_NODEBUG bool isDefinedId(InstId instId) noexcept { return (instId & uint32_t(InstIdParts::kRealId)) < _kIdCount; }
}

//! Memory barrier option (DMB, DSB, ISB).
enum class BarrierOption : uint32_t {
  kOSHLD = 0x1u,
  kOSHST = 0x2u,
  kOSH   = 0x3u,
  kNSHLD = 0x5u,
  kNSHST = 0x6u,
  kNSH   = 0x7u,
  kISHLD = 0x9u,
  kISHST = 0xAu,
  kISH   = 0xBu,
  kLD    = 0xDu,
  kST    = 0xEu,
  kSY    = 0xFu
};

//! Program status register fields used by MRS/MSR instructions.
//!
//! The value is composed of a mask (bits 0-3, which maps to `mask` field in MSR) and bit 4 that specifies SPSR
//! (otherwise CPSR/APSR is used).
namespace Psr {
  enum : uint32_t {
    //! APSR (read by MRS).
    kAPSR = 0x00u,
    //! CPSR (read by MRS).
    kCPSR = 0x00u,
    //! SPSR (read by MRS).
    kSPSR = 0x10u,

    //! Control field mask bit (`c`).
    kC = 0x1u,
    //! Extension field mask bit (`x`).
    kX = 0x2u,
    //! Status field mask bit (`s`).
    kS = 0x4u,
    //! Flags field mask bit (`f`).
    kF = 0x8u,

    //! `APSR_nzcvq` (flags).
    kAPSR_nzcvq  = kF,
    //! `APSR_g` (GE bits).
    kAPSR_g      = kS,
    //! `APSR_nzcvqg` (flags + GE bits).
    kAPSR_nzcvqg = kF | kS,

    //! `CPSR_fsxc`.
    kCPSR_fsxc = kF | kS | kX | kC,
    //! `CPSR_c`.
    kCPSR_c    = kC,
    //! `SPSR_fsxc`.
    kSPSR_fsxc = kSPSR | kF | kS | kX | kC
  };
}

//! Floating point system registers (VMRS/VMSR).
enum class FpSysReg : uint32_t {
  kFPSID = 0x0u,
  kFPSCR = 0x1u,
  kMVFR2 = 0x5u,
  kMVFR1 = 0x6u,
  kMVFR0 = 0x7u,
  kFPEXC = 0x8u
};

//! CPS interrupt flags (CPSIE/CPSID).
namespace CpsFlags {
  enum : uint32_t {
    kF = 0x1u,
    kI = 0x2u,
    kA = 0x4u
  };
}

//! Endianness used by SETEND.
enum class Endian : uint32_t {
  kLE = 0,
  kBE = 1
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A32GLOBALS_H_INCLUDED
