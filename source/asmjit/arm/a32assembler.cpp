// This file is based on part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_AARCH32)

#include "../core/codewriter_p.h"
#include "../core/emitterutils_p.h"
#include "../core/formatter.h"
#include "../core/logger.h"
#include "../core/misc_p.h"
#include "../core/support.h"
#include "../arm/armutils.h"
#include "../arm/a32assembler.h"
#include "../arm/a32instapi_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

// a32::Assembler - Encoding Utilities
// ===================================

namespace {

//! Returned by an encoder when the operands don't match the form (the next form is tried).
static constexpr Error kNoMatch = 0xFFFFFFFFu;

// Data Type Utilities
// -------------------

static constexpr uint32_t dtBit(DataType dt) noexcept { return 1u << uint32_t(dt); }
static constexpr uint32_t dt2Bit(DataType dt) noexcept { return 1u << (uint32_t(dt) + 16u); }

static constexpr uint32_t kDtAny = 0xFFFFFFFFu;
static constexpr uint32_t kDt2None = dt2Bit(DataType::kNone);

static constexpr uint32_t kDtNone = dtBit(DataType::kNone);
static constexpr uint32_t kDtS8 = dtBit(DataType::kS8);
static constexpr uint32_t kDtS16 = dtBit(DataType::kS16);
static constexpr uint32_t kDtS32 = dtBit(DataType::kS32);
static constexpr uint32_t kDtS64 = dtBit(DataType::kS64);
static constexpr uint32_t kDtU8 = dtBit(DataType::kU8);
static constexpr uint32_t kDtU16 = dtBit(DataType::kU16);
static constexpr uint32_t kDtU32 = dtBit(DataType::kU32);
static constexpr uint32_t kDtU64 = dtBit(DataType::kU64);
static constexpr uint32_t kDtF16 = dtBit(DataType::kF16);
static constexpr uint32_t kDtF32 = dtBit(DataType::kF32);
static constexpr uint32_t kDtF64 = dtBit(DataType::kF64);
static constexpr uint32_t kDtP8 = dtBit(DataType::kP8);
static constexpr uint32_t kDtBF16 = dtBit(DataType::kBF16);
static constexpr uint32_t kDtP64 = dtBit(DataType::kP64);

// Integer data types (sign agnostic `.iN` is an alias of `.sN`, `.uN` is accepted as well).
static constexpr uint32_t kDtI8 = kDtS8 | kDtU8;
static constexpr uint32_t kDtI16 = kDtS16 | kDtU16;
static constexpr uint32_t kDtI32 = kDtS32 | kDtU32;
static constexpr uint32_t kDtI64 = kDtS64 | kDtU64;

// Any data type of the given size (`.8`, `.16`, `.32`, `.64`).
static constexpr uint32_t kDtX8 = kDtI8 | kDtP8;
static constexpr uint32_t kDtX16 = kDtI16 | kDtF16 | kDtBF16;
static constexpr uint32_t kDtX32 = kDtI32 | kDtF32;
static constexpr uint32_t kDtX64 = kDtI64 | kDtF64 | kDtP64;

static constexpr uint32_t kDtSU8_32 = kDtI8 | kDtI16 | kDtI32;
static constexpr uint32_t kDtSU8_64 = kDtSU8_32 | kDtI64;
static constexpr uint32_t kDtS8_32 = kDtS8 | kDtS16 | kDtS32;
static constexpr uint32_t kDtS8_64 = kDtS8_32 | kDtS64;

static ASMJIT_INLINE uint32_t dtSizeLog2(DataType dt) noexcept {
  // None, S8, S16, S32, S64, U8, U16, U32, U64, <9>, F16, F32, F64, P8, BF16, P64.
  static constexpr uint8_t table[16] = { 0, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 3 };
  return table[uint32_t(dt) & 0xFu];
}

static ASMJIT_INLINE uint32_t dtSizeBits(DataType dt) noexcept { return 8u << dtSizeLog2(dt); }

static ASMJIT_INLINE bool dtIsUnsigned(DataType dt) noexcept {
  return uint32_t(dt) >= uint32_t(DataType::kU8) && uint32_t(dt) <= uint32_t(DataType::kU64);
}

// Condition Code Utilities
// ------------------------

static ASMJIT_INLINE uint32_t condCodeToOpcodeCond(CondCode cc) noexcept { return (uint32_t(cc) - 2u) & 0xFu; }

// Immediate Utilities
// -------------------

//! Encodes a modified immediate (imm8 rotated right by 2*rot) - picks the smallest rotation.
static bool encodeModImm(uint32_t v, uint32_t* out) noexcept {
  for (uint32_t rot = 0; rot < 16; rot++) {
    uint32_t s = rot * 2u;
    uint32_t x = s ? ((v << s) | (v >> (32u - s))) : v;
    if (x <= 0xFFu) {
      *out = (rot << 8) | x;
      return true;
    }
  }
  return false;
}

//! Converts immediate to a 32-bit value (accepts both signed and unsigned 32-bit ranges).
static ASMJIT_INLINE bool immToU32(const Operand_& op, uint32_t* out) noexcept {
  const Imm& imm = op.as<Imm>();
  if (imm.isDouble()) {
    return false;
  }
  int64_t v = imm.value();
  if (v < int64_t(INT32_MIN) || v > int64_t(UINT32_MAX)) {
    return false;
  }
  *out = uint32_t(uint64_t(v));
  return true;
}

//! Converts immediate to an unsigned value that must be within [lo, hi] range.
static ASMJIT_INLINE bool immInRange(const Operand_& op, int64_t lo, int64_t hi, uint32_t* out) noexcept {
  const Imm& imm = op.as<Imm>();
  if (imm.isDouble()) {
    return false;
  }
  int64_t v = imm.value();
  if (v < lo || v > hi) {
    return false;
  }
  *out = uint32_t(uint64_t(v));
  return true;
}

//! Returns immediate as double (if it's an integer it's converted).
static ASMJIT_INLINE double immAsDouble(const Operand_& op) noexcept {
  const Imm& imm = op.as<Imm>();
  return imm.isDouble() ? imm.valueAs<double>() : double(imm.value());
}

//! Encodes a VFP / ASIMD 8-bit floating point immediate of a 32-bit float.
static bool encodeFP32Imm8(float f, uint32_t* out) noexcept {
  uint32_t bits = Support::bitCast<uint32_t>(f);
  if (!arm::Utils::isFP32Imm8(bits)) {
    return false;
  }
  // imm8 = a:b:cdefgh, float = a:NOT(b):bbbbb:cdefgh:0{19}.
  *out = ((bits >> 24) & 0x80u) | ((bits >> 19) & 0x7Fu);
  return true;
}

static bool encodeFP64Imm8(double d, uint32_t* out) noexcept {
  uint64_t bits = Support::bitCast<uint64_t>(d);
  if (!arm::Utils::isFP64Imm8(bits)) {
    return false;
  }
  *out = arm::Utils::encodeFP64ToImm8(bits);
  return true;
}

// Register Utilities
// ------------------

static ASMJIT_INLINE bool isGp(const Operand_& op) noexcept { return op.isReg(RegType::kGp32); }
static ASMJIT_INLINE bool isImm(const Operand_& op) noexcept { return op.isImm(); }
static ASMJIT_INLINE bool isMem(const Operand_& op) noexcept { return op.isMem(); }
static ASMJIT_INLINE bool isLabel(const Operand_& op) noexcept { return op.isLabel(); }

static ASMJIT_INLINE bool isVecS(const Operand_& op) noexcept { return op.isReg(RegType::kVec32); }
static ASMJIT_INLINE bool isVecD(const Operand_& op) noexcept { return op.isReg(RegType::kVec64); }
static ASMJIT_INLINE bool isVecQ(const Operand_& op) noexcept { return op.isReg(RegType::kVec128); }
static ASMJIT_INLINE bool isVecDQ(const Operand_& op) noexcept { return isVecD(op) || isVecQ(op); }
static ASMJIT_INLINE bool isVecAny(const Operand_& op) noexcept { return isVecS(op) || isVecD(op) || isVecQ(op); }

static ASMJIT_INLINE bool isPlainVec(const Operand_& op) noexcept { return op.as<Vec>().isPlain(); }
static ASMJIT_INLINE bool isScalarD(const Operand_& op) noexcept { return isVecD(op) && op.as<Vec>().hasElementIndex(); }

static ASMJIT_INLINE bool isPlainS(const Operand_& op) noexcept { return isVecS(op) && isPlainVec(op); }
static ASMJIT_INLINE bool isPlainD(const Operand_& op) noexcept { return isVecD(op) && isPlainVec(op); }
static ASMJIT_INLINE bool isPlainQ(const Operand_& op) noexcept { return isVecQ(op) && isPlainVec(op); }
static ASMJIT_INLINE bool isPlainDQ(const Operand_& op) noexcept { return isVecDQ(op) && isPlainVec(op); }

static ASMJIT_INLINE uint32_t regId(const Operand_& op) noexcept { return op.as<Reg>().id(); }

static ASMJIT_INLINE bool isGpIdValid(const Operand_& op) noexcept { return regId(op) < 16u; }

static ASMJIT_INLINE bool isVecIdValid(const Operand_& op) noexcept {
  return regId(op) < (isVecQ(op) ? 16u : 32u);
}

//! Returns the D register index of a D or Q register (Q registers map to an even D register).
static ASMJIT_INLINE uint32_t dRegIndex(const Operand_& op) noexcept {
  return isVecQ(op) ? regId(op) * 2u : regId(op);
}

// Vd, Vn, Vm fields (S registers use Vx:bit, D/Q registers use bit:Vx).
static ASMJIT_INLINE uint32_t encVd(const Operand_& op) noexcept {
  uint32_t id = regId(op);
  if (isVecS(op)) {
    return ((id >> 1) << 12) | ((id & 1u) << 22);
  }
  id = dRegIndex(op);
  return ((id & 0xFu) << 12) | ((id >> 4) << 22);
}

static ASMJIT_INLINE uint32_t encVn(const Operand_& op) noexcept {
  uint32_t id = regId(op);
  if (isVecS(op)) {
    return ((id >> 1) << 16) | ((id & 1u) << 7);
  }
  id = dRegIndex(op);
  return ((id & 0xFu) << 16) | ((id >> 4) << 7);
}

static ASMJIT_INLINE uint32_t encVm(const Operand_& op) noexcept {
  uint32_t id = regId(op);
  if (isVecS(op)) {
    return (id >> 1) | ((id & 1u) << 5);
  }
  id = dRegIndex(op);
  return (id & 0xFu) | ((id >> 4) << 5);
}

static ASMJIT_INLINE uint32_t qBit(const Operand_& op) noexcept { return uint32_t(isVecQ(op)) << 6; }

// Shift Utilities
// ---------------

//! Encodes an immediate shift (type and amount) to bits [11:5] used by data-processing and load/store.
static bool encodeImmShift(ShiftOp op, uint32_t amount, uint32_t* out) noexcept {
  switch (op) {
    case ShiftOp::kLSL:
      if (amount > 31u) return false;
      *out = (amount << 7) | (0u << 5);
      return true;

    case ShiftOp::kLSR:
      if (amount == 0u) { *out = 0; return true; }
      if (amount > 32u) return false;
      *out = ((amount & 31u) << 7) | (1u << 5);
      return true;

    case ShiftOp::kASR:
      if (amount == 0u) { *out = 0; return true; }
      if (amount > 32u) return false;
      *out = ((amount & 31u) << 7) | (2u << 5);
      return true;

    case ShiftOp::kROR:
      if (amount == 0u) { *out = 0; return true; }
      if (amount > 31u) return false;
      *out = (amount << 7) | (3u << 5);
      return true;

    case ShiftOp::kRRX:
      *out = (3u << 5);
      return true;

    default:
      return false;
  }
}

// Encoding Forms
// --------------

enum Form : uint8_t {
  // Base.
  kF_DP,          // Rd, Rn, <op2>
  kF_Mov,         // Rd, <op2>
  kF_Cmp,         // Rn, <op2>
  kF_Shift,       // Rd, Rm, #imm|Rs
  kF_Rrx,         // Rd, Rm
  kF_MovW,        // Rd, #imm16
  kF_Regs,        // Generic GP registers.
  kF_Ext,         // Rd, [Rn,] Rm {, ROR #n}
  kF_Sat,         // Rd, #sat, Rn {, shift}
  kF_Sat16,       // Rd, #sat, Rn
  kF_Pkh,         // Rd, Rn, Rm {, shift}
  kF_Bfc,         // Rd, #lsb, #width
  kF_Bfi,         // Rd, Rn, #lsb, #width
  kF_Bfx,         // Rd, Rn, #lsb, #width
  kF_B,           // label|imm
  kF_Blx,         // label|imm
  kF_Adr,         // Rd, label|imm
  kF_LdSt,        // Rt, mem
  kF_LdStX,       // Rt, [Rt2,] mem
  kF_Ldm,         // Rn|mem, list
  kF_PushPop,     // list
  kF_MemRegs,     // Regs..., [Rn]
  kF_None,        // (no operands)
  kF_Barrier,     // {#option}
  kF_Imm,         // #imm
  kF_Pld,         // mem
  kF_Mrs,         // Rd, #psr
  kF_Msr,         // #psr, Rn|#imm
  kF_Cps,         // #flags {, #mode} | #mode
  kF_Setend,      // #endian
  kF_Mcr,         // #cp, #opc1, Rt, #CRn, #CRm {, #opc2}
  kF_Mcrr,        // #cp, #opc1, Rt, Rt2, #CRm

  // VFP.
  kF_V3,          // Sd|Dd, Sn|Dn, Sm|Dm
  kF_V2,          // Sd|Dd, Sm|Dm
  kF_VCmp0,       // Sd|Dd, #0
  kF_VCvt,        // VCVT & VCVTR.
  kF_VCvtRm,      // VCVTA, VCVTN, VCVTP, VCVTM.
  kF_VCvtBT,      // VCVTB, VCVTT.
  kF_VMov,        // VMOV (all forms).
  kF_VLdr,        // Sd|Dd, mem
  kF_VLdm,        // Rn|mem, list
  kF_VPushPop,    // list
  kF_VMrs,        // Rt {, #sysreg}
  kF_VMsr,        // {#sysreg,} Rt

  // ASIMD.
  kF_N3Same,      // Vd, Vn, Vm
  kF_N3Diff,      // Qd, Dn, Dm | Qd, Qn, Dm | Dd, Qn, Qm
  kF_NScalar,     // Vd, Vn, Dm[x]
  kF_NShift,      // Vd, Vm, #imm
  kF_N2Misc,      // Vd, Vm
  kF_NImm,        // Vd, #imm
  kF_NExt,        // Vd, Vn, Vm, #imm
  kF_NTbl,        // Dd, list, Dm
  kF_NDup,        // Vd, Dm[x]
  kF_NDupGp,      // Vd, Rt
  kF_NLdSt        // list, mem
};

enum RowFlags : uint8_t {
  //! Instruction is conditional (condition code is encoded in bits [31:28]).
  kRowCond = 0x01u
};

//! Instruction encoding row - an instruction can have multiple rows, the first that matches is used.
struct Row {
  uint16_t instId;
  uint8_t form;
  uint8_t flags;
  uint32_t opcode;
  uint32_t aux;
  uint32_t dtMask;
};

// Aux values used by various forms.
enum : uint32_t {
  // kF_DP / kF_Mov / kF_Cmp alternative mode (the alternative opcode field is stored in bits [24:21]).
  kAltNone = 0,
  kAltNeg = 1,
  kAltInv = 2,

  // kF_N3Same / kF_N3Diff / kF_NScalar / kF_N2Misc / kF_NShift - data type encoding.
  kDtEncNone = 0,
  kDtEncSize20 = 1,       // size at [21:20].
  kDtEncSize20U24 = 2,    // size at [21:20], U at 24.
  kDtEncSizeN20 = 3,      // size - 1 at [21:20] (narrowing).
  kDtEncSize18 = 4,       // size at [19:18].
  kDtEncSizeN18 = 5,      // size - 1 at [19:18] (narrowing).
  kDtEncSize18U7 = 6,     // size at [19:18], U at 7.
  kDtEncU24 = 7,          // U at 24.

  // Shapes (stored at bits [7:4] of aux).
  kShapeDQ = 0,           // D or Q (all operands have the same size).
  kShapeD = 1,            // D only.
  kShapeQ = 2,            // Q only.
  kShapeL = 3,            // Qd, Dn, Dm (long).
  kShapeW = 4,            // Qd, Qn, Dm (wide).
  kShapeN = 5,            // Dd, Qn, Qm (narrow) / Dd, Qm.
  kShapeLImm = 6,         // Qd, Dm, #esize (VSHLL maximum shift).
  kShapeZero = 7,         // Vd, Vm, #0.

  // Additional flags (stored at bits [11:8] of aux).
  kAuxSwapNM = 0x100u,    // Operands are Vd, Vm, Vn.

  // kF_NShift kinds.
  kShiftR = 0,            // Right shift (1..esize).
  kShiftL = 1,            // Left shift (0..esize-1).
  kShiftRN = 2,           // Right shift narrow (Dd, Qm, #1..esize/2).
  kShiftLL = 3,           // Left shift long (Qd, Dm, #0..esize-1).

  // kF_NImm operations.
  kNImmMov = 0,
  kNImmMvn = 1,
  kNImmOrr = 2,
  kNImmBic = 3,
  kNImmAnd = 4,
  kNImmOrn = 5
};

#define AUX_DT_SHAPE(DtEnc, Shape) (uint32_t(DtEnc) | (uint32_t(Shape) << 4))

static ASMJIT_INLINE uint32_t auxDtEnc(uint32_t aux) noexcept { return aux & 0xFu; }
static ASMJIT_INLINE uint32_t auxShape(uint32_t aux) noexcept { return (aux >> 4) & 0xFu; }

// kF_Regs / kF_MemRegs aux - count and positions of register fields.
static constexpr uint32_t kRegSkip = 31u; // Register must be previous + 1 and it's not encoded.

static constexpr uint32_t REGS(uint32_t n, uint32_t s0 = 0, uint32_t s1 = 0, uint32_t s2 = 0, uint32_t s3 = 0) noexcept {
  return n | (s0 << 3) | (s1 << 8) | (s2 << 13) | (s3 << 18);
}

static ASMJIT_INLINE uint32_t regsCount(uint32_t aux) noexcept { return aux & 0x7u; }
static ASMJIT_INLINE uint32_t regsShift(uint32_t aux, uint32_t i) noexcept { return (aux >> (3u + i * 5u)) & 0x1Fu; }

// Instruction Encoding Table
// --------------------------

#define ROW(Id, Form, Flags, Opcode, Aux, DtMask) { uint16_t(Inst::kId##Id), uint8_t(Form), uint8_t(Flags), uint32_t(Opcode), uint32_t(Aux), uint32_t(DtMask) }
#define C kRowCond
#define U 0
#define D1(Mask) ((Mask) | kDt2None)
#define D2(Mask1, Mask2) ((Mask1) | ((Mask2) << 16))

// Data processing opcode field [24:21].
#define DPOP(x) (uint32_t(x) << 21)
#define ALT(Mode, AltOpc) (uint32_t(Mode) | DPOP(AltOpc))

static constexpr Row rowTable[] = {
  // Base - Data Processing
  // ----------------------
  ROW(Adc   , kF_DP   , C, 0x00A00000u, ALT(kAltInv, 0x6), 0),
  ROW(Adcs  , kF_DP   , C, 0x00B00000u, ALT(kAltInv, 0x6), 0),
  ROW(Add   , kF_DP   , C, 0x00800000u, ALT(kAltNeg, 0x2), 0),
  ROW(Adds  , kF_DP   , C, 0x00900000u, ALT(kAltNeg, 0x2), 0),
  ROW(Adr   , kF_Adr  , C, 0x020F0000u, 0, 0),
  ROW(And   , kF_DP   , C, 0x00000000u, ALT(kAltInv, 0xE), 0),
  ROW(Ands  , kF_DP   , C, 0x00100000u, ALT(kAltInv, 0xE), 0),
  ROW(Asr   , kF_Shift, C, 0x01A00000u, 2, 0),
  ROW(Asrs  , kF_Shift, C, 0x01B00000u, 2, 0),
  ROW(Bic   , kF_DP   , C, 0x01C00000u, ALT(kAltInv, 0x0), 0),
  ROW(Bics  , kF_DP   , C, 0x01D00000u, ALT(kAltInv, 0x0), 0),
  ROW(Cmn   , kF_Cmp  , C, 0x01700000u, ALT(kAltNeg, 0xA), 0),
  ROW(Cmp   , kF_Cmp  , C, 0x01500000u, ALT(kAltNeg, 0xB), 0),
  ROW(Eor   , kF_DP   , C, 0x00200000u, 0, 0),
  ROW(Eors  , kF_DP   , C, 0x00300000u, 0, 0),
  ROW(Lsl   , kF_Shift, C, 0x01A00000u, 0, 0),
  ROW(Lsls  , kF_Shift, C, 0x01B00000u, 0, 0),
  ROW(Lsr   , kF_Shift, C, 0x01A00000u, 1, 0),
  ROW(Lsrs  , kF_Shift, C, 0x01B00000u, 1, 0),
  ROW(Mov   , kF_Mov  , C, 0x01A00000u, ALT(kAltInv, 0xF), 0),
  ROW(Movs  , kF_Mov  , C, 0x01B00000u, ALT(kAltInv, 0xF), 0),
  ROW(Movt  , kF_MovW , C, 0x03400000u, 0, 0),
  ROW(Movw  , kF_MovW , C, 0x03000000u, 0, 0),
  ROW(Mvn   , kF_Mov  , C, 0x01E00000u, ALT(kAltInv, 0xD), 0),
  ROW(Mvns  , kF_Mov  , C, 0x01F00000u, ALT(kAltInv, 0xD), 0),
  ROW(Orr   , kF_DP   , C, 0x01800000u, 0, 0),
  ROW(Orrs  , kF_DP   , C, 0x01900000u, 0, 0),
  ROW(Ror   , kF_Shift, C, 0x01A00000u, 3, 0),
  ROW(Rors  , kF_Shift, C, 0x01B00000u, 3, 0),
  ROW(Rrx   , kF_Rrx  , C, 0x01A00060u, 0, 0),
  ROW(Rrxs  , kF_Rrx  , C, 0x01B00060u, 0, 0),
  ROW(Rsb   , kF_DP   , C, 0x00600000u, 0, 0),
  ROW(Rsbs  , kF_DP   , C, 0x00700000u, 0, 0),
  ROW(Rsc   , kF_DP   , C, 0x00E00000u, 0, 0),
  ROW(Rscs  , kF_DP   , C, 0x00F00000u, 0, 0),
  ROW(Sbc   , kF_DP   , C, 0x00C00000u, ALT(kAltInv, 0x5), 0),
  ROW(Sbcs  , kF_DP   , C, 0x00D00000u, ALT(kAltInv, 0x5), 0),
  ROW(Sub   , kF_DP   , C, 0x00400000u, ALT(kAltNeg, 0x4), 0),
  ROW(Subs  , kF_DP   , C, 0x00500000u, ALT(kAltNeg, 0x4), 0),
  ROW(Teq   , kF_Cmp  , C, 0x01300000u, 0, 0),
  ROW(Tst   , kF_Cmp  , C, 0x01100000u, 0, 0),

  // Base - Multiply & Divide
  // ------------------------
  ROW(Mla   , kF_Regs , C, 0x00200090u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Mlas  , kF_Regs , C, 0x00300090u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Mls   , kF_Regs , C, 0x00600090u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Mul   , kF_Regs , C, 0x00000090u, REGS(3, 16, 0, 8), 0),
  ROW(Muls  , kF_Regs , C, 0x00100090u, REGS(3, 16, 0, 8), 0),
  ROW(Smlal , kF_Regs , C, 0x00E00090u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Smlals, kF_Regs , C, 0x00F00090u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Smull , kF_Regs , C, 0x00C00090u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Smulls, kF_Regs , C, 0x00D00090u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Umaal , kF_Regs , C, 0x00400090u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Umlal , kF_Regs , C, 0x00A00090u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Umlals, kF_Regs , C, 0x00B00090u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Umull , kF_Regs , C, 0x00800090u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Umulls, kF_Regs , C, 0x00900090u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Sdiv  , kF_Regs , C, 0x0710F010u, REGS(3, 16, 0, 8), 0),
  ROW(Udiv  , kF_Regs , C, 0x0730F010u, REGS(3, 16, 0, 8), 0),
  ROW(Smlabb, kF_Regs , C, 0x01000080u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smlabt, kF_Regs , C, 0x010000C0u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smlatb, kF_Regs , C, 0x010000A0u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smlatt, kF_Regs , C, 0x010000E0u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smlawb, kF_Regs , C, 0x01200080u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smlawt, kF_Regs , C, 0x012000C0u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smulbb, kF_Regs , C, 0x01600080u, REGS(3, 16, 0, 8), 0),
  ROW(Smulbt, kF_Regs , C, 0x016000C0u, REGS(3, 16, 0, 8), 0),
  ROW(Smultb, kF_Regs , C, 0x016000A0u, REGS(3, 16, 0, 8), 0),
  ROW(Smultt, kF_Regs , C, 0x016000E0u, REGS(3, 16, 0, 8), 0),
  ROW(Smulwb, kF_Regs , C, 0x012000A0u, REGS(3, 16, 0, 8), 0),
  ROW(Smulwt, kF_Regs , C, 0x012000E0u, REGS(3, 16, 0, 8), 0),
  ROW(Smlalbb, kF_Regs, C, 0x01400080u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Smlalbt, kF_Regs, C, 0x014000C0u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Smlaltb, kF_Regs, C, 0x014000A0u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Smlaltt, kF_Regs, C, 0x014000E0u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Smuad , kF_Regs , C, 0x0700F010u, REGS(3, 16, 0, 8), 0),
  ROW(Smuadx, kF_Regs , C, 0x0700F030u, REGS(3, 16, 0, 8), 0),
  ROW(Smusd , kF_Regs , C, 0x0700F050u, REGS(3, 16, 0, 8), 0),
  ROW(Smusdx, kF_Regs , C, 0x0700F070u, REGS(3, 16, 0, 8), 0),
  ROW(Smlad , kF_Regs , C, 0x07000010u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smladx, kF_Regs , C, 0x07000030u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smlsd , kF_Regs , C, 0x07000050u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smlsdx, kF_Regs , C, 0x07000070u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smlald, kF_Regs , C, 0x07400010u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Smlaldx, kF_Regs, C, 0x07400030u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Smlsld, kF_Regs , C, 0x07400050u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Smlsldx, kF_Regs, C, 0x07400070u, REGS(4, 12, 16, 0, 8), 0),
  ROW(Smmul , kF_Regs , C, 0x0750F010u, REGS(3, 16, 0, 8), 0),
  ROW(Smmulr, kF_Regs , C, 0x0750F030u, REGS(3, 16, 0, 8), 0),
  ROW(Smmla , kF_Regs , C, 0x07500010u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smmlar, kF_Regs , C, 0x07500030u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smmls , kF_Regs , C, 0x075000D0u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Smmlsr, kF_Regs , C, 0x075000F0u, REGS(4, 16, 0, 8, 12), 0),

  // Base - Saturating & Parallel
  // ----------------------------
  ROW(Qadd  , kF_Regs , C, 0x01000050u, REGS(3, 12, 0, 16), 0),
  ROW(Qsub  , kF_Regs , C, 0x01200050u, REGS(3, 12, 0, 16), 0),
  ROW(Qdadd , kF_Regs , C, 0x01400050u, REGS(3, 12, 0, 16), 0),
  ROW(Qdsub , kF_Regs , C, 0x01600050u, REGS(3, 12, 0, 16), 0),
  ROW(Ssat  , kF_Sat  , C, 0x06A00010u, 1, 0),
  ROW(Usat  , kF_Sat  , C, 0x06E00010u, 0, 0),
  ROW(Ssat16, kF_Sat16, C, 0x06A00F30u, 1, 0),
  ROW(Usat16, kF_Sat16, C, 0x06E00F30u, 0, 0),

#define PAR(Prefix, Op1) \
  ROW(Prefix##add16, kF_Regs, C, 0x06000F10u | (uint32_t(Op1) << 20), REGS(3, 12, 16, 0), 0), \
  ROW(Prefix##asx  , kF_Regs, C, 0x06000F30u | (uint32_t(Op1) << 20), REGS(3, 12, 16, 0), 0), \
  ROW(Prefix##sax  , kF_Regs, C, 0x06000F50u | (uint32_t(Op1) << 20), REGS(3, 12, 16, 0), 0), \
  ROW(Prefix##sub16, kF_Regs, C, 0x06000F70u | (uint32_t(Op1) << 20), REGS(3, 12, 16, 0), 0), \
  ROW(Prefix##add8 , kF_Regs, C, 0x06000F90u | (uint32_t(Op1) << 20), REGS(3, 12, 16, 0), 0), \
  ROW(Prefix##sub8 , kF_Regs, C, 0x06000FF0u | (uint32_t(Op1) << 20), REGS(3, 12, 16, 0), 0)

  PAR(S , 0x61),
  PAR(Q , 0x62),
  PAR(Sh, 0x63),
  PAR(U , 0x65),
  PAR(Uq, 0x66),
  PAR(Uh, 0x67),

#undef PAR

  // Base - Extend, Pack, Bitfield, Misc
  // -----------------------------------
  ROW(Sxtb  , kF_Ext  , C, 0x06AF0070u, 0, 0),
  ROW(Sxth  , kF_Ext  , C, 0x06BF0070u, 0, 0),
  ROW(Sxtb16, kF_Ext  , C, 0x068F0070u, 0, 0),
  ROW(Uxtb  , kF_Ext  , C, 0x06EF0070u, 0, 0),
  ROW(Uxth  , kF_Ext  , C, 0x06FF0070u, 0, 0),
  ROW(Uxtb16, kF_Ext  , C, 0x06CF0070u, 0, 0),
  ROW(Sxtab , kF_Ext  , C, 0x06A00070u, 1, 0),
  ROW(Sxtah , kF_Ext  , C, 0x06B00070u, 1, 0),
  ROW(Sxtab16, kF_Ext , C, 0x06800070u, 1, 0),
  ROW(Uxtab , kF_Ext  , C, 0x06E00070u, 1, 0),
  ROW(Uxtah , kF_Ext  , C, 0x06F00070u, 1, 0),
  ROW(Uxtab16, kF_Ext , C, 0x06C00070u, 1, 0),
  ROW(Pkhbt , kF_Pkh  , C, 0x06800010u, 0, 0),
  ROW(Pkhtb , kF_Pkh  , C, 0x06800050u, 1, 0),
  ROW(Sel   , kF_Regs , C, 0x06800FB0u, REGS(3, 12, 16, 0), 0),
  ROW(Rev   , kF_Regs , C, 0x06BF0F30u, REGS(2, 12, 0), 0),
  ROW(Rev16 , kF_Regs , C, 0x06BF0FB0u, REGS(2, 12, 0), 0),
  ROW(Revsh , kF_Regs , C, 0x06FF0FB0u, REGS(2, 12, 0), 0),
  ROW(Rbit  , kF_Regs , C, 0x06FF0F30u, REGS(2, 12, 0), 0),
  ROW(Clz   , kF_Regs , C, 0x016F0F10u, REGS(2, 12, 0), 0),
  ROW(Usad8 , kF_Regs , C, 0x0780F010u, REGS(3, 16, 0, 8), 0),
  ROW(Usada8, kF_Regs , C, 0x07800010u, REGS(4, 16, 0, 8, 12), 0),
  ROW(Bfc   , kF_Bfc  , C, 0x07C0001Fu, 0, 0),
  ROW(Bfi   , kF_Bfi  , C, 0x07C00010u, 0, 0),
  ROW(Sbfx  , kF_Bfx  , C, 0x07A00050u, 0, 0),
  ROW(Ubfx  , kF_Bfx  , C, 0x07E00050u, 0, 0),
  ROW(Crc32b, kF_Regs , C, 0x01000040u, REGS(3, 12, 16, 0), 0),
  ROW(Crc32h, kF_Regs , C, 0x01200040u, REGS(3, 12, 16, 0), 0),
  ROW(Crc32w, kF_Regs , C, 0x01400040u, REGS(3, 12, 16, 0), 0),
  ROW(Crc32cb, kF_Regs, C, 0x01000240u, REGS(3, 12, 16, 0), 0),
  ROW(Crc32ch, kF_Regs, C, 0x01200240u, REGS(3, 12, 16, 0), 0),
  ROW(Crc32cw, kF_Regs, C, 0x01400240u, REGS(3, 12, 16, 0), 0),

  // Base - Branch
  // -------------
  ROW(B     , kF_B    , C, 0x0A000000u, 0, 0),
  ROW(Bl    , kF_B    , C, 0x0B000000u, 0, 0),
  ROW(Blx   , kF_Regs , C, 0x012FFF30u, REGS(1, 0), 0),
  ROW(Blx   , kF_Blx  , U, 0xFA000000u, 0, 0),
  ROW(Bx    , kF_Regs , C, 0x012FFF10u, REGS(1, 0), 0),
  ROW(Bxj   , kF_Regs , C, 0x012FFF20u, REGS(1, 0), 0),

  // Base - Load & Store
  // -------------------
  ROW(Ldr   , kF_LdSt , C, 0x04100000u, 0, 0),
  ROW(Ldrb  , kF_LdSt , C, 0x04500000u, 0, 0),
  ROW(Ldrh  , kF_LdStX, C, 0x001000B0u, 0, 0),
  ROW(Ldrsb , kF_LdStX, C, 0x001000D0u, 0, 0),
  ROW(Ldrsh , kF_LdStX, C, 0x001000F0u, 0, 0),
  ROW(Ldrd  , kF_LdStX, C, 0x000000D0u, 2, 0),
  ROW(Str   , kF_LdSt , C, 0x04000000u, 0, 0),
  ROW(Strb  , kF_LdSt , C, 0x04400000u, 0, 0),
  ROW(Strh  , kF_LdStX, C, 0x000000B0u, 0, 0),
  ROW(Strd  , kF_LdStX, C, 0x000000F0u, 2, 0),
  ROW(Ldrt  , kF_LdSt , C, 0x04100000u, 1, 0),
  ROW(Ldrbt , kF_LdSt , C, 0x04500000u, 1, 0),
  ROW(Ldrht , kF_LdStX, C, 0x001000B0u, 1, 0),
  ROW(Ldrsbt, kF_LdStX, C, 0x001000D0u, 1, 0),
  ROW(Ldrsht, kF_LdStX, C, 0x001000F0u, 1, 0),
  ROW(Strt  , kF_LdSt , C, 0x04000000u, 1, 0),
  ROW(Strbt , kF_LdSt , C, 0x04400000u, 1, 0),
  ROW(Strht , kF_LdStX, C, 0x000000B0u, 1, 0),
  ROW(Ldm   , kF_Ldm  , C, 0x08900000u, 0, 0),
  ROW(Ldmib , kF_Ldm  , C, 0x09900000u, 0, 0),
  ROW(Ldmda , kF_Ldm  , C, 0x08100000u, 0, 0),
  ROW(Ldmdb , kF_Ldm  , C, 0x09100000u, 0, 0),
  ROW(Stm   , kF_Ldm  , C, 0x08800000u, 0, 0),
  ROW(Stmib , kF_Ldm  , C, 0x09800000u, 0, 0),
  ROW(Stmda , kF_Ldm  , C, 0x08000000u, 0, 0),
  ROW(Stmdb , kF_Ldm  , C, 0x09000000u, 0, 0),
  ROW(Push  , kF_PushPop, C, 0x092D0000u, 0, 0),
  ROW(Pop   , kF_PushPop, C, 0x08BD0000u, 1, 0),
  ROW(Ldrex , kF_MemRegs, C, 0x01900F9Fu, REGS(1, 12), 0),
  ROW(Ldrexb, kF_MemRegs, C, 0x01D00F9Fu, REGS(1, 12), 0),
  ROW(Ldrexh, kF_MemRegs, C, 0x01F00F9Fu, REGS(1, 12), 0),
  ROW(Ldrexd, kF_MemRegs, C, 0x01B00F9Fu, REGS(2, 12, kRegSkip), 0),
  ROW(Strex , kF_MemRegs, C, 0x01800F90u, REGS(2, 12, 0), 0),
  ROW(Strexb, kF_MemRegs, C, 0x01C00F90u, REGS(2, 12, 0), 0),
  ROW(Strexh, kF_MemRegs, C, 0x01E00F90u, REGS(2, 12, 0), 0),
  ROW(Strexd, kF_MemRegs, C, 0x01A00F90u, REGS(3, 12, 0, kRegSkip), 0),
  ROW(Lda   , kF_MemRegs, C, 0x01900C9Fu, REGS(1, 12), 0),
  ROW(Ldab  , kF_MemRegs, C, 0x01D00C9Fu, REGS(1, 12), 0),
  ROW(Ldah  , kF_MemRegs, C, 0x01F00C9Fu, REGS(1, 12), 0),
  ROW(Stl   , kF_MemRegs, C, 0x0180FC90u, REGS(1, 0), 0),
  ROW(Stlb  , kF_MemRegs, C, 0x01C0FC90u, REGS(1, 0), 0),
  ROW(Stlh  , kF_MemRegs, C, 0x01E0FC90u, REGS(1, 0), 0),
  ROW(Ldaex , kF_MemRegs, C, 0x01900E9Fu, REGS(1, 12), 0),
  ROW(Ldaexb, kF_MemRegs, C, 0x01D00E9Fu, REGS(1, 12), 0),
  ROW(Ldaexh, kF_MemRegs, C, 0x01F00E9Fu, REGS(1, 12), 0),
  ROW(Ldaexd, kF_MemRegs, C, 0x01B00E9Fu, REGS(2, 12, kRegSkip), 0),
  ROW(Stlex , kF_MemRegs, C, 0x01800E90u, REGS(2, 12, 0), 0),
  ROW(Stlexb, kF_MemRegs, C, 0x01C00E90u, REGS(2, 12, 0), 0),
  ROW(Stlexh, kF_MemRegs, C, 0x01E00E90u, REGS(2, 12, 0), 0),
  ROW(Stlexd, kF_MemRegs, C, 0x01A00E90u, REGS(3, 12, 0, kRegSkip), 0),
  ROW(Clrex , kF_None , U, 0xF57FF01Fu, 0, 0),

  // Base - Hints, Exceptions, Barriers, System
  // ------------------------------------------
  ROW(Nop   , kF_None , C, 0x0320F000u, 0, 0),
  ROW(Yield , kF_None , C, 0x0320F001u, 0, 0),
  ROW(Wfe   , kF_None , C, 0x0320F002u, 0, 0),
  ROW(Wfi   , kF_None , C, 0x0320F003u, 0, 0),
  ROW(Sev   , kF_None , C, 0x0320F004u, 0, 0),
  ROW(Sevl  , kF_None , C, 0x0320F005u, 0, 0),
  ROW(Csdb  , kF_None , C, 0x0320F014u, 0, 0),
  ROW(Dbg   , kF_Imm  , C, 0x0320F0F0u, 2, 0),
  ROW(Svc   , kF_Imm  , C, 0x0F000000u, 0, 0),
  ROW(Bkpt  , kF_Imm  , U, 0xE1200070u, 1, 0),
  ROW(Udf   , kF_Imm  , U, 0xE7F000F0u, 1, 0),
  ROW(Hvc   , kF_Imm  , U, 0xE1400070u, 1, 0),
  ROW(Smc   , kF_Imm  , C, 0x01600070u, 2, 0),
  ROW(Eret  , kF_None , C, 0x0160006Eu, 0, 0),
  ROW(Dmb   , kF_Barrier, U, 0xF57FF050u, 0, 0),
  ROW(Dsb   , kF_Barrier, U, 0xF57FF040u, 0, 0),
  ROW(Isb   , kF_Barrier, U, 0xF57FF060u, 0, 0),
  ROW(Mrs   , kF_Mrs  , C, 0x010F0000u, 0, 0),
  ROW(Msr   , kF_Msr  , C, 0x0120F000u, 0, 0),
  ROW(Cps   , kF_Cps  , U, 0xF1020000u, 0, 0),
  ROW(Cpsie , kF_Cps  , U, 0xF1080000u, 1, 0),
  ROW(Cpsid , kF_Cps  , U, 0xF10C0000u, 1, 0),
  ROW(Setend, kF_Setend, U, 0xF1010000u, 0, 0),
  ROW(Pld   , kF_Pld  , U, 0xF550F000u, 0, 0),
  ROW(Pldw  , kF_Pld  , U, 0xF510F000u, 1, 0),
  ROW(Pli   , kF_Pld  , U, 0xF450F000u, 0, 0),
  ROW(Mrc   , kF_Mcr  , C, 0x0E100010u, 0, 0),
  ROW(Mcr   , kF_Mcr  , C, 0x0E000010u, 0, 0),
  ROW(Mrrc  , kF_Mcrr , C, 0x0C500000u, 0, 0),
  ROW(Mcrr  , kF_Mcrr , C, 0x0C400000u, 0, 0),

  // VFP
  // ---
  ROW(Vabs  , kF_V2   , C, 0x0EB00AC0u, 0, kDtAny),
  ROW(Vabs  , kF_N2Misc, U, 0xF3B10300u, AUX_DT_SHAPE(kDtEncSize18, kShapeDQ), D1(kDtS8_32)),
  ROW(Vabs  , kF_N2Misc, U, 0xF3B90700u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vadd  , kF_V3   , C, 0x0E300A00u, 0, kDtAny),
  ROW(Vadd  , kF_N3Same, U, 0xF2000800u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtSU8_64)),
  ROW(Vadd  , kF_N3Same, U, 0xF2000D00u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vcmp  , kF_V2   , C, 0x0EB40A40u, 0, kDtAny),
  ROW(Vcmp  , kF_VCmp0, C, 0x0EB50A40u, 0, kDtAny),
  ROW(Vcmpe , kF_V2   , C, 0x0EB40AC0u, 0, kDtAny),
  ROW(Vcmpe , kF_VCmp0, C, 0x0EB50AC0u, 0, kDtAny),
  ROW(Vcvt  , kF_VCvt , C, 0x00000000u, 0, kDtAny),
  ROW(Vcvta , kF_VCvtRm, U, 0x00000000u, 0, kDtAny),
  ROW(Vcvtb , kF_VCvtBT, C, 0x0EB20A40u, 0, kDtAny),
  ROW(Vcvtm , kF_VCvtRm, U, 0x00000000u, 3, kDtAny),
  ROW(Vcvtn , kF_VCvtRm, U, 0x00000000u, 1, kDtAny),
  ROW(Vcvtp , kF_VCvtRm, U, 0x00000000u, 2, kDtAny),
  ROW(Vcvtr , kF_VCvt , C, 0x00000000u, 1, kDtAny),
  ROW(Vcvtt , kF_VCvtBT, C, 0x0EB20AC0u, 0, kDtAny),
  ROW(Vdiv  , kF_V3   , C, 0x0E800A00u, 0, kDtAny),
  ROW(Vfma  , kF_V3   , C, 0x0EA00A00u, 0, kDtAny),
  ROW(Vfma  , kF_N3Same, U, 0xF2000C10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vfms  , kF_V3   , C, 0x0EA00A40u, 0, kDtAny),
  ROW(Vfms  , kF_N3Same, U, 0xF2200C10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vfnma , kF_V3   , C, 0x0E900A40u, 0, kDtAny),
  ROW(Vfnms , kF_V3   , C, 0x0E900A00u, 0, kDtAny),
  ROW(Vldm  , kF_VLdm , C, 0x0C900A00u, 0, kDtAny),
  ROW(Vldmdb, kF_VLdm , C, 0x0D300A00u, 1, kDtAny),
  ROW(Vldr  , kF_VLdr , C, 0x0D100A00u, 0, kDtAny),
  ROW(Vmaxnm, kF_V3   , U, 0xFE800A00u, 0, kDtAny),
  ROW(Vmaxnm, kF_N3Same, U, 0xF3000F10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vminnm, kF_V3   , U, 0xFE800A40u, 0, kDtAny),
  ROW(Vminnm, kF_N3Same, U, 0xF3200F10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vmla  , kF_V3   , C, 0x0E000A00u, 0, kDtAny),
  ROW(Vmla  , kF_NScalar, U, 0xF2800040u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtI16 | kDtI32)),
  ROW(Vmla  , kF_NScalar, U, 0xF2A00140u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vmla  , kF_N3Same, U, 0xF2000900u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vmla  , kF_N3Same, U, 0xF2000D10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vmls  , kF_V3   , C, 0x0E000A40u, 0, kDtAny),
  ROW(Vmls  , kF_NScalar, U, 0xF2800440u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtI16 | kDtI32)),
  ROW(Vmls  , kF_NScalar, U, 0xF2A00540u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vmls  , kF_N3Same, U, 0xF3000900u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vmls  , kF_N3Same, U, 0xF2200D10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vmov  , kF_VMov , C, 0x00000000u, 0, kDtAny),
  ROW(Vmrs  , kF_VMrs , C, 0x0EF00A10u, 0, 0),
  ROW(Vmsr  , kF_VMsr , C, 0x0EE00A10u, 0, 0),
  ROW(Vmul  , kF_V3   , C, 0x0E200A00u, 0, kDtAny),
  ROW(Vmul  , kF_NScalar, U, 0xF2800840u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtI16 | kDtI32)),
  ROW(Vmul  , kF_NScalar, U, 0xF2A00940u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vmul  , kF_N3Same, U, 0xF2000910u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vmul  , kF_N3Same, U, 0xF3000910u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtP8)),
  ROW(Vmul  , kF_N3Same, U, 0xF3000D10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vneg  , kF_V2   , C, 0x0EB10A40u, 0, kDtAny),
  ROW(Vneg  , kF_N2Misc, U, 0xF3B10380u, AUX_DT_SHAPE(kDtEncSize18, kShapeDQ), D1(kDtS8_32)),
  ROW(Vneg  , kF_N2Misc, U, 0xF3B90780u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vnmla , kF_V3   , C, 0x0E100A40u, 0, kDtAny),
  ROW(Vnmls , kF_V3   , C, 0x0E100A00u, 0, kDtAny),
  ROW(Vnmul , kF_V3   , C, 0x0E200A40u, 0, kDtAny),
  ROW(Vpop  , kF_VPushPop, C, 0x0CBD0A00u, 0, kDtAny),
  ROW(Vpush , kF_VPushPop, C, 0x0D2D0A00u, 0, kDtAny),
  ROW(Vrinta, kF_V2   , U, 0xFEB80A40u, 0, kDtAny),
  ROW(Vrinta, kF_N2Misc, U, 0xF3BA0500u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vrintm, kF_V2   , U, 0xFEBB0A40u, 0, kDtAny),
  ROW(Vrintm, kF_N2Misc, U, 0xF3BA0680u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vrintn, kF_V2   , U, 0xFEB90A40u, 0, kDtAny),
  ROW(Vrintn, kF_N2Misc, U, 0xF3BA0400u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vrintp, kF_V2   , U, 0xFEBA0A40u, 0, kDtAny),
  ROW(Vrintp, kF_N2Misc, U, 0xF3BA0780u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vrintr, kF_V2   , C, 0x0EB60A40u, 0, kDtAny),
  ROW(Vrintx, kF_V2   , C, 0x0EB70A40u, 0, kDtAny),
  ROW(Vrintx, kF_N2Misc, U, 0xF3BA0480u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vrintz, kF_V2   , C, 0x0EB60AC0u, 0, kDtAny),
  ROW(Vrintz, kF_N2Misc, U, 0xF3BA0580u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vseleq, kF_V3   , U, 0xFE000A00u, 0, kDtAny),
  ROW(Vselge, kF_V3   , U, 0xFE200A00u, 0, kDtAny),
  ROW(Vselgt, kF_V3   , U, 0xFE300A00u, 0, kDtAny),
  ROW(Vselvs, kF_V3   , U, 0xFE100A00u, 0, kDtAny),
  ROW(Vsqrt , kF_V2   , C, 0x0EB10AC0u, 0, kDtAny),
  ROW(Vstm  , kF_VLdm , C, 0x0C800A00u, 0, kDtAny),
  ROW(Vstmdb, kF_VLdm , C, 0x0D200A00u, 1, kDtAny),
  ROW(Vstr  , kF_VLdr , C, 0x0D000A00u, 0, kDtAny),
  ROW(Vsub  , kF_V3   , C, 0x0E300A40u, 0, kDtAny),
  ROW(Vsub  , kF_N3Same, U, 0xF3000800u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtSU8_64)),
  ROW(Vsub  , kF_N3Same, U, 0xF2200D00u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),

  // Advanced SIMD
  // -------------
  ROW(Vaba  , kF_N3Same, U, 0xF2000710u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vabal , kF_N3Diff, U, 0xF2800500u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeL), D1(kDtSU8_32)),
  ROW(Vabd  , kF_N3Same, U, 0xF2000700u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vabd  , kF_N3Same, U, 0xF3200D00u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vabdl , kF_N3Diff, U, 0xF2800700u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeL), D1(kDtSU8_32)),
  ROW(Vacge , kF_N3Same, U, 0xF3000E10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vacgt , kF_N3Same, U, 0xF3200E10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vacle , kF_N3Same, U, 0xF3000E10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ) | kAuxSwapNM, D1(kDtF32)),
  ROW(Vaclt , kF_N3Same, U, 0xF3200E10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ) | kAuxSwapNM, D1(kDtF32)),
  ROW(Vaddhn, kF_N3Diff, U, 0xF2800400u, AUX_DT_SHAPE(kDtEncSizeN20, kShapeN), D1(kDtI16 | kDtI32 | kDtI64)),
  ROW(Vaddl , kF_N3Diff, U, 0xF2800000u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeL), D1(kDtSU8_32)),
  ROW(Vaddw , kF_N3Diff, U, 0xF2800100u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeW), D1(kDtSU8_32)),
  ROW(Vand  , kF_NImm , U, 0x00000000u, kNImmAnd, D1(kDtI16 | kDtI32)),
  ROW(Vand  , kF_N3Same, U, 0xF2000110u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), kDtAny),
  ROW(Vbic  , kF_NImm , U, 0x00000000u, kNImmBic, D1(kDtI16 | kDtI32)),
  ROW(Vbic  , kF_N3Same, U, 0xF2100110u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), kDtAny),
  ROW(Vbif  , kF_N3Same, U, 0xF3300110u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), kDtAny),
  ROW(Vbit  , kF_N3Same, U, 0xF3200110u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), kDtAny),
  ROW(Vbsl  , kF_N3Same, U, 0xF3100110u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), kDtAny),
  ROW(Vceq  , kF_N2Misc, U, 0xF3B10100u, AUX_DT_SHAPE(kDtEncSize18, kShapeZero), D1(kDtSU8_32)),
  ROW(Vceq  , kF_N2Misc, U, 0xF3B90500u, AUX_DT_SHAPE(kDtEncNone, kShapeZero), D1(kDtF32)),
  ROW(Vceq  , kF_N3Same, U, 0xF3000810u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vceq  , kF_N3Same, U, 0xF2000E00u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vcge  , kF_N2Misc, U, 0xF3B10080u, AUX_DT_SHAPE(kDtEncSize18, kShapeZero), D1(kDtS8_32)),
  ROW(Vcge  , kF_N2Misc, U, 0xF3B90480u, AUX_DT_SHAPE(kDtEncNone, kShapeZero), D1(kDtF32)),
  ROW(Vcge  , kF_N3Same, U, 0xF2000310u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vcge  , kF_N3Same, U, 0xF3000E00u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vcgt  , kF_N2Misc, U, 0xF3B10000u, AUX_DT_SHAPE(kDtEncSize18, kShapeZero), D1(kDtS8_32)),
  ROW(Vcgt  , kF_N2Misc, U, 0xF3B90400u, AUX_DT_SHAPE(kDtEncNone, kShapeZero), D1(kDtF32)),
  ROW(Vcgt  , kF_N3Same, U, 0xF2000300u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vcgt  , kF_N3Same, U, 0xF3200E00u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vcle  , kF_N2Misc, U, 0xF3B10180u, AUX_DT_SHAPE(kDtEncSize18, kShapeZero), D1(kDtS8_32)),
  ROW(Vcle  , kF_N2Misc, U, 0xF3B90580u, AUX_DT_SHAPE(kDtEncNone, kShapeZero), D1(kDtF32)),
  ROW(Vcle  , kF_N3Same, U, 0xF2000310u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ) | kAuxSwapNM, D1(kDtSU8_32)),
  ROW(Vcle  , kF_N3Same, U, 0xF3000E00u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ) | kAuxSwapNM, D1(kDtF32)),
  ROW(Vclt  , kF_N2Misc, U, 0xF3B10200u, AUX_DT_SHAPE(kDtEncSize18, kShapeZero), D1(kDtS8_32)),
  ROW(Vclt  , kF_N2Misc, U, 0xF3B90600u, AUX_DT_SHAPE(kDtEncNone, kShapeZero), D1(kDtF32)),
  ROW(Vclt  , kF_N3Same, U, 0xF2000300u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ) | kAuxSwapNM, D1(kDtSU8_32)),
  ROW(Vclt  , kF_N3Same, U, 0xF3200E00u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ) | kAuxSwapNM, D1(kDtF32)),
  ROW(Vcls  , kF_N2Misc, U, 0xF3B00400u, AUX_DT_SHAPE(kDtEncSize18, kShapeDQ), D1(kDtS8_32)),
  ROW(Vclz  , kF_N2Misc, U, 0xF3B00480u, AUX_DT_SHAPE(kDtEncSize18, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vcnt  , kF_N2Misc, U, 0xF3B00500u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtX8)),
  ROW(Vdup  , kF_NDup , U, 0xF3B00C00u, 0, D1(kDtX8 | kDtX16 | kDtX32)),
  ROW(Vdup  , kF_NDupGp, C, 0x0E800B10u, 0, D1(kDtX8 | kDtX16 | kDtX32)),
  ROW(Veor  , kF_N3Same, U, 0xF3000110u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), kDtAny),
  ROW(Vext  , kF_NExt , U, 0xF2B00000u, 0, D1(kDtNone | kDtX8 | kDtX16 | kDtX32 | kDtX64)),
  ROW(Vhadd , kF_N3Same, U, 0xF2000000u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vhsub , kF_N3Same, U, 0xF2000200u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vld1  , kF_NLdSt, U, 0xF4200000u, 1, D1(kDtX8 | kDtX16 | kDtX32 | kDtX64)),
  ROW(Vld2  , kF_NLdSt, U, 0xF4200000u, 2, D1(kDtX8 | kDtX16 | kDtX32)),
  ROW(Vld3  , kF_NLdSt, U, 0xF4200000u, 3, D1(kDtX8 | kDtX16 | kDtX32)),
  ROW(Vld4  , kF_NLdSt, U, 0xF4200000u, 4, D1(kDtX8 | kDtX16 | kDtX32)),
  ROW(Vmax  , kF_N3Same, U, 0xF2000600u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vmax  , kF_N3Same, U, 0xF2000F00u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vmin  , kF_N3Same, U, 0xF2000610u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vmin  , kF_N3Same, U, 0xF2200F00u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vmlal , kF_NScalar, U, 0xF2800240u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeL), D1(kDtI16 | kDtI32)),
  ROW(Vmlal , kF_N3Diff, U, 0xF2800800u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeL), D1(kDtSU8_32)),
  ROW(Vmlsl , kF_NScalar, U, 0xF2800640u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeL), D1(kDtI16 | kDtI32)),
  ROW(Vmlsl , kF_N3Diff, U, 0xF2800A00u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeL), D1(kDtSU8_32)),
  ROW(Vmovl , kF_NShift, U, 0xF2800A10u, AUX_DT_SHAPE(kDtEncU24, kShiftLL), D1(kDtSU8_32)),
  ROW(Vmovn , kF_N2Misc, U, 0xF3B20200u, AUX_DT_SHAPE(kDtEncSizeN18, kShapeN), D1(kDtI16 | kDtI32 | kDtI64)),
  ROW(Vmull , kF_NScalar, U, 0xF2800A40u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeL), D1(kDtI16 | kDtI32)),
  ROW(Vmull , kF_N3Diff, U, 0xF2800C00u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeL), D1(kDtSU8_32)),
  ROW(Vmull , kF_N3Diff, U, 0xF2800E00u, AUX_DT_SHAPE(kDtEncNone, kShapeL), D1(kDtP8)),
  ROW(Vmull , kF_N3Diff, U, 0xF2A00E00u, AUX_DT_SHAPE(kDtEncNone, kShapeL), D1(kDtP64)),
  ROW(Vmvn  , kF_NImm , U, 0x00000000u, kNImmMvn, D1(kDtI16 | kDtI32)),
  ROW(Vmvn  , kF_N2Misc, U, 0xF3B00580u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), kDtAny),
  ROW(Vorn  , kF_NImm , U, 0x00000000u, kNImmOrn, D1(kDtI16 | kDtI32)),
  ROW(Vorn  , kF_N3Same, U, 0xF2300110u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), kDtAny),
  ROW(Vorr  , kF_NImm , U, 0x00000000u, kNImmOrr, D1(kDtI16 | kDtI32)),
  ROW(Vorr  , kF_N3Same, U, 0xF2200110u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), kDtAny),
  ROW(Vpadal, kF_N2Misc, U, 0xF3B00600u, AUX_DT_SHAPE(kDtEncSize18U7, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vpadd , kF_N3Same, U, 0xF2000B10u, AUX_DT_SHAPE(kDtEncSize20, kShapeD), D1(kDtSU8_32)),
  ROW(Vpadd , kF_N3Same, U, 0xF3000D00u, AUX_DT_SHAPE(kDtEncNone, kShapeD), D1(kDtF32)),
  ROW(Vpaddl, kF_N2Misc, U, 0xF3B00200u, AUX_DT_SHAPE(kDtEncSize18U7, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vpmax , kF_N3Same, U, 0xF2000A00u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeD), D1(kDtSU8_32)),
  ROW(Vpmax , kF_N3Same, U, 0xF3000F00u, AUX_DT_SHAPE(kDtEncNone, kShapeD), D1(kDtF32)),
  ROW(Vpmin , kF_N3Same, U, 0xF2000A10u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeD), D1(kDtSU8_32)),
  ROW(Vpmin , kF_N3Same, U, 0xF3200F00u, AUX_DT_SHAPE(kDtEncNone, kShapeD), D1(kDtF32)),
  ROW(Vqabs , kF_N2Misc, U, 0xF3B00700u, AUX_DT_SHAPE(kDtEncSize18, kShapeDQ), D1(kDtS8_32)),
  ROW(Vqadd , kF_N3Same, U, 0xF2000010u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ), D1(kDtSU8_64)),
  ROW(Vqdmlal, kF_NScalar, U, 0xF2800340u, AUX_DT_SHAPE(kDtEncSize20, kShapeL), D1(kDtS16 | kDtS32)),
  ROW(Vqdmlal, kF_N3Diff, U, 0xF2800900u, AUX_DT_SHAPE(kDtEncSize20, kShapeL), D1(kDtS16 | kDtS32)),
  ROW(Vqdmlsl, kF_NScalar, U, 0xF2800740u, AUX_DT_SHAPE(kDtEncSize20, kShapeL), D1(kDtS16 | kDtS32)),
  ROW(Vqdmlsl, kF_N3Diff, U, 0xF2800B00u, AUX_DT_SHAPE(kDtEncSize20, kShapeL), D1(kDtS16 | kDtS32)),
  ROW(Vqdmulh, kF_NScalar, U, 0xF2800C40u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtS16 | kDtS32)),
  ROW(Vqdmulh, kF_N3Same, U, 0xF2000B00u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtS16 | kDtS32)),
  ROW(Vqdmull, kF_NScalar, U, 0xF2800B40u, AUX_DT_SHAPE(kDtEncSize20, kShapeL), D1(kDtS16 | kDtS32)),
  ROW(Vqdmull, kF_N3Diff, U, 0xF2800D00u, AUX_DT_SHAPE(kDtEncSize20, kShapeL), D1(kDtS16 | kDtS32)),
  ROW(Vqmovn, kF_N2Misc, U, 0xF3B20280u, AUX_DT_SHAPE(kDtEncSizeN18, kShapeN), D1(kDtS16 | kDtS32 | kDtS64)),
  ROW(Vqmovn, kF_N2Misc, U, 0xF3B202C0u, AUX_DT_SHAPE(kDtEncSizeN18, kShapeN), D1(kDtU16 | kDtU32 | kDtU64)),
  ROW(Vqmovun, kF_N2Misc, U, 0xF3B20240u, AUX_DT_SHAPE(kDtEncSizeN18, kShapeN), D1(kDtS16 | kDtS32 | kDtS64)),
  ROW(Vqneg , kF_N2Misc, U, 0xF3B00780u, AUX_DT_SHAPE(kDtEncSize18, kShapeDQ), D1(kDtS8_32)),
  ROW(Vqrdmulh, kF_NScalar, U, 0xF2800D40u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtS16 | kDtS32)),
  ROW(Vqrdmulh, kF_N3Same, U, 0xF3000B00u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtS16 | kDtS32)),
  ROW(Vqrshl, kF_N3Same, U, 0xF2000510u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ) | kAuxSwapNM, D1(kDtSU8_64)),
  ROW(Vqrshrn, kF_NShift, U, 0xF2800950u, AUX_DT_SHAPE(kDtEncU24, kShiftRN), D1(kDtI16 | kDtI32 | kDtI64)),
  ROW(Vqrshrun, kF_NShift, U, 0xF3800850u, AUX_DT_SHAPE(kDtEncNone, kShiftRN), D1(kDtS16 | kDtS32 | kDtS64)),
  ROW(Vqshl , kF_NShift, U, 0xF2800710u, AUX_DT_SHAPE(kDtEncU24, kShiftL), D1(kDtSU8_64)),
  ROW(Vqshl , kF_N3Same, U, 0xF2000410u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ) | kAuxSwapNM, D1(kDtSU8_64)),
  ROW(Vqshlu, kF_NShift, U, 0xF3800610u, AUX_DT_SHAPE(kDtEncNone, kShiftL), D1(kDtS8_64)),
  ROW(Vqshrn, kF_NShift, U, 0xF2800910u, AUX_DT_SHAPE(kDtEncU24, kShiftRN), D1(kDtI16 | kDtI32 | kDtI64)),
  ROW(Vqshrun, kF_NShift, U, 0xF3800810u, AUX_DT_SHAPE(kDtEncNone, kShiftRN), D1(kDtS16 | kDtS32 | kDtS64)),
  ROW(Vqsub , kF_N3Same, U, 0xF2000210u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ), D1(kDtSU8_64)),
  ROW(Vraddhn, kF_N3Diff, U, 0xF3800400u, AUX_DT_SHAPE(kDtEncSizeN20, kShapeN), D1(kDtI16 | kDtI32 | kDtI64)),
  ROW(Vrecpe, kF_N2Misc, U, 0xF3BB0400u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtU32)),
  ROW(Vrecpe, kF_N2Misc, U, 0xF3BB0500u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vrecps, kF_N3Same, U, 0xF2000F10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vrev16, kF_N2Misc, U, 0xF3B00100u, AUX_DT_SHAPE(kDtEncSize18, kShapeDQ), D1(kDtX8)),
  ROW(Vrev32, kF_N2Misc, U, 0xF3B00080u, AUX_DT_SHAPE(kDtEncSize18, kShapeDQ), D1(kDtX8 | kDtX16)),
  ROW(Vrev64, kF_N2Misc, U, 0xF3B00000u, AUX_DT_SHAPE(kDtEncSize18, kShapeDQ), D1(kDtX8 | kDtX16 | kDtX32)),
  ROW(Vrhadd, kF_N3Same, U, 0xF2000100u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ), D1(kDtSU8_32)),
  ROW(Vrshl , kF_N3Same, U, 0xF2000500u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ) | kAuxSwapNM, D1(kDtSU8_64)),
  ROW(Vrshr , kF_NShift, U, 0xF2800210u, AUX_DT_SHAPE(kDtEncU24, kShiftR), D1(kDtSU8_64)),
  ROW(Vrshrn, kF_NShift, U, 0xF2800850u, AUX_DT_SHAPE(kDtEncNone, kShiftRN), D1(kDtI16 | kDtI32 | kDtI64)),
  ROW(Vrsqrte, kF_N2Misc, U, 0xF3BB0480u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtU32)),
  ROW(Vrsqrte, kF_N2Misc, U, 0xF3BB0580u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vrsqrts, kF_N3Same, U, 0xF2200F10u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), D1(kDtF32)),
  ROW(Vrsra , kF_NShift, U, 0xF2800310u, AUX_DT_SHAPE(kDtEncU24, kShiftR), D1(kDtSU8_64)),
  ROW(Vrsubhn, kF_N3Diff, U, 0xF3800600u, AUX_DT_SHAPE(kDtEncSizeN20, kShapeN), D1(kDtI16 | kDtI32 | kDtI64)),
  ROW(Vshl  , kF_NShift, U, 0xF2800510u, AUX_DT_SHAPE(kDtEncNone, kShiftL), D1(kDtSU8_64)),
  ROW(Vshl  , kF_N3Same, U, 0xF2000400u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeDQ) | kAuxSwapNM, D1(kDtSU8_64)),
  ROW(Vshll , kF_NShift, U, 0xF2800A10u, AUX_DT_SHAPE(kDtEncU24, kShiftLL), D1(kDtSU8_32)),
  ROW(Vshll , kF_N2Misc, U, 0xF3B20300u, AUX_DT_SHAPE(kDtEncSize18, kShapeLImm), D1(kDtSU8_32)),
  ROW(Vshr  , kF_NShift, U, 0xF2800010u, AUX_DT_SHAPE(kDtEncU24, kShiftR), D1(kDtSU8_64)),
  ROW(Vshrn , kF_NShift, U, 0xF2800810u, AUX_DT_SHAPE(kDtEncNone, kShiftRN), D1(kDtI16 | kDtI32 | kDtI64)),
  ROW(Vsli  , kF_NShift, U, 0xF3800510u, AUX_DT_SHAPE(kDtEncNone, kShiftL), D1(kDtX8 | kDtX16 | kDtX32 | kDtX64)),
  ROW(Vsra  , kF_NShift, U, 0xF2800110u, AUX_DT_SHAPE(kDtEncU24, kShiftR), D1(kDtSU8_64)),
  ROW(Vsri  , kF_NShift, U, 0xF3800410u, AUX_DT_SHAPE(kDtEncNone, kShiftR), D1(kDtX8 | kDtX16 | kDtX32 | kDtX64)),
  ROW(Vst1  , kF_NLdSt, U, 0xF4000000u, 1, D1(kDtX8 | kDtX16 | kDtX32 | kDtX64)),
  ROW(Vst2  , kF_NLdSt, U, 0xF4000000u, 2, D1(kDtX8 | kDtX16 | kDtX32)),
  ROW(Vst3  , kF_NLdSt, U, 0xF4000000u, 3, D1(kDtX8 | kDtX16 | kDtX32)),
  ROW(Vst4  , kF_NLdSt, U, 0xF4000000u, 4, D1(kDtX8 | kDtX16 | kDtX32)),
  ROW(Vsubhn, kF_N3Diff, U, 0xF2800600u, AUX_DT_SHAPE(kDtEncSizeN20, kShapeN), D1(kDtI16 | kDtI32 | kDtI64)),
  ROW(Vsubl , kF_N3Diff, U, 0xF2800200u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeL), D1(kDtSU8_32)),
  ROW(Vsubw , kF_N3Diff, U, 0xF2800300u, AUX_DT_SHAPE(kDtEncSize20U24, kShapeW), D1(kDtSU8_32)),
  ROW(Vswp  , kF_N2Misc, U, 0xF3B20000u, AUX_DT_SHAPE(kDtEncNone, kShapeDQ), kDtAny),
  ROW(Vtbl  , kF_NTbl , U, 0xF3B00800u, 0, D1(kDtNone | kDtX8)),
  ROW(Vtbx  , kF_NTbl , U, 0xF3B00840u, 0, D1(kDtNone | kDtX8)),
  ROW(Vtrn  , kF_N2Misc, U, 0xF3B20080u, AUX_DT_SHAPE(kDtEncSize18, kShapeDQ), D1(kDtX8 | kDtX16 | kDtX32)),
  ROW(Vtst  , kF_N3Same, U, 0xF2000810u, AUX_DT_SHAPE(kDtEncSize20, kShapeDQ), D1(kDtX8 | kDtX16 | kDtX32)),
  ROW(Vuzp  , kF_N2Misc, U, 0xF3B20100u, AUX_DT_SHAPE(kDtEncSize18, kShapeDQ), D1(kDtX8 | kDtX16 | kDtX32)),
  ROW(Vzip  , kF_N2Misc, U, 0xF3B20180u, AUX_DT_SHAPE(kDtEncSize18, kShapeDQ), D1(kDtX8 | kDtX16 | kDtX32)),

  // Crypto
  // ------
  ROW(Aesd  , kF_N2Misc, U, 0xF3B00340u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX8)),
  ROW(Aese  , kF_N2Misc, U, 0xF3B00300u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX8)),
  ROW(Aesimc, kF_N2Misc, U, 0xF3B003C0u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX8)),
  ROW(Aesmc , kF_N2Misc, U, 0xF3B00380u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX8)),
  ROW(Sha1c , kF_N3Same, U, 0xF2000C40u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX32)),
  ROW(Sha1h , kF_N2Misc, U, 0xF3B902C0u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX32)),
  ROW(Sha1m , kF_N3Same, U, 0xF2200C40u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX32)),
  ROW(Sha1p , kF_N3Same, U, 0xF2100C40u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX32)),
  ROW(Sha1su0, kF_N3Same, U, 0xF2300C40u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX32)),
  ROW(Sha1su1, kF_N2Misc, U, 0xF3BA0380u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX32)),
  ROW(Sha256h, kF_N3Same, U, 0xF3000C40u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX32)),
  ROW(Sha256h2, kF_N3Same, U, 0xF3100C40u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX32)),
  ROW(Sha256su0, kF_N2Misc, U, 0xF3BA03C0u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX32)),
  ROW(Sha256su1, kF_N3Same, U, 0xF3200C40u, AUX_DT_SHAPE(kDtEncNone, kShapeQ), D1(kDtNone | kDtX32))
};

#undef ALT
#undef DPOP
#undef D2
#undef D1
#undef U
#undef C
#undef ROW

static constexpr uint32_t kRowCount = uint32_t(sizeof(rowTable) / sizeof(rowTable[0]));

// Row index - maps instruction id to the first row and number of rows.
struct RowIndex {
  uint16_t start[Inst::_kIdCount];
  uint8_t count[Inst::_kIdCount];
  bool valid;
};

static constexpr RowIndex makeRowIndex() noexcept {
  RowIndex index {};
  index.valid = true;

  for (uint32_t i = 0; i < Inst::_kIdCount; i++) {
    index.start[i] = 0;
    index.count[i] = 0;
  }

  for (uint32_t i = 0; i < kRowCount; i++) {
    uint32_t id = rowTable[i].instId;
    if (index.count[id] == 0) {
      index.start[id] = uint16_t(i);
    }
    else if (index.start[id] + index.count[id] != i) {
      // Rows of a single instruction must be consecutive.
      index.valid = false;
    }
    index.count[id]++;
  }

  // Each instruction (except kIdNone) must have at least one row.
  for (uint32_t i = 1; i < Inst::_kIdCount; i++) {
    if (index.count[i] == 0) {
      index.valid = false;
    }
  }

  return index;
}

static constexpr RowIndex rowIndex = makeRowIndex();
static_assert(rowIndex.valid, "AArch32 instruction table is invalid (missing rows or rows not consecutive)");

// Encoder Context
// ---------------

struct EncCtx {
  const Operand_* op[Globals::kMaxOpCount];
  uint32_t n;
  DataType dt;
  DataType dt2;

  // Output.
  uint32_t opcode;
  const Operand_* rel;
  OffsetFormat relFmt;
  //! Set by encoders that emit an unconditional encoding from a conditional row (like VMOV -> VORR).
  bool uncond;

  ASMJIT_INLINE const Operand_& o(uint32_t i) const noexcept { return *op[i]; }

  ASMJIT_INLINE void setRel(const Operand_& target, OffsetType type, uint32_t bitCount, uint32_t discardLsb) noexcept {
    rel = &target;
    relFmt.resetToImmValue(type, 4, 0, bitCount, discardLsb);
  }
};

} // {anonymous}

// a32::Assembler - Encoders (Base)
// ================================

namespace {

//! Encodes <op2> of a data-processing instruction starting at operand `i`. Doesn't handle immediates.
static Error encodeRegOp2(const EncCtx& c, uint32_t i, uint32_t* out) noexcept {
  if (i >= c.n || !isGp(c.o(i))) {
    return kNoMatch;
  }

  if (!isGpIdValid(c.o(i))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t bits = regId(c.o(i));

  if (i + 1 == c.n) {
    *out = bits;
    return kErrorOk;
  }

  if (i + 2 != c.n) {
    return kNoMatch;
  }

  const Operand_& sh = c.o(i + 1);
  if (isImm(sh)) {
    const Imm& imm = sh.as<Imm>();
    uint32_t amount;
    if (!immInRange(sh, 0, 32, &amount)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }

    uint32_t shBits;
    if (!encodeImmShift(ShiftOp(imm.predicate()), amount, &shBits)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }

    *out = bits | shBits;
    return kErrorOk;
  }

  if (isGp(sh)) {
    ShiftOp sop = sh.as<Gp>().shiftOp();
    if (uint32_t(sop) > uint32_t(ShiftOp::kROR)) {
      return DebugUtils::errored(kErrorInvalidInstruction);
    }

    if (!isGpIdValid(sh)) {
      return DebugUtils::errored(kErrorInvalidPhysId);
    }

    *out = bits | (regId(sh) << 8) | (uint32_t(sop) << 5) | (1u << 4);
    return kErrorOk;
  }

  return kNoMatch;
}

//! Encodes immediate <op2> (with an optional alternative opcode that uses negated / inverted immediate).
static Error encodeImmOp2(EncCtx& c, const Row& row, uint32_t i, uint32_t base) noexcept {
  if (i + 1 != c.n) {
    return kNoMatch;
  }

  uint32_t v;
  if (!immToU32(c.o(i), &v)) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  uint32_t enc;
  if (encodeModImm(v, &enc)) {
    c.opcode = base | (1u << 25) | enc;
    return kErrorOk;
  }

  uint32_t mode = row.aux & 0x3u;
  if (mode != kAltNone) {
    uint32_t alt = (mode == kAltNeg) ? uint32_t(0u - v) : ~v;
    if (encodeModImm(alt, &enc)) {
      c.opcode = (base & ~(0xFu << 21)) | (row.aux & (0xFu << 21)) | (1u << 25) | enc;
      return kErrorOk;
    }
  }

  return DebugUtils::errored(kErrorInvalidImmediate);
}

static Error encDP(EncCtx& c, const Row& row) noexcept {
  if (c.n < 2 || !isGp(c.o(0))) {
    return kNoMatch;
  }

  uint32_t rd = regId(c.o(0));
  uint32_t rn;
  uint32_t i;

  if (c.n >= 3) {
    if (!isGp(c.o(1))) {
      return kNoMatch;
    }
    rn = regId(c.o(1));
    i = 2;
  }
  else {
    // Shorthand: `op Rd, <op2>` is `op Rd, Rd, <op2>`.
    rn = rd;
    i = 1;
  }

  if (rd > 15u || rn > 15u) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t base = row.opcode | (rn << 16) | (rd << 12);
  if (isImm(c.o(i))) {
    return encodeImmOp2(c, row, i, base);
  }

  uint32_t op2;
  Error err = encodeRegOp2(c, i, &op2);
  if (err != kErrorOk) {
    return err;
  }

  c.opcode = base | op2;
  return kErrorOk;
}

static Error encMov(EncCtx& c, const Row& row) noexcept {
  if (c.n < 2 || !isGp(c.o(0))) {
    return kNoMatch;
  }

  uint32_t rd = regId(c.o(0));
  if (rd > 15u) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t base = row.opcode | (rd << 12);
  if (isImm(c.o(1))) {
    Error err = encodeImmOp2(c, row, 1, base);
    if (err == kErrorOk || err == kNoMatch) {
      return err;
    }

    // MOV Rd, #imm16 can be encoded as MOVW.
    uint32_t v;
    if (row.instId == Inst::kIdMov && immToU32(c.o(1), &v) && v <= 0xFFFFu) {
      c.opcode = 0x03000000u | ((v >> 12) << 16) | (rd << 12) | (v & 0xFFFu);
      return kErrorOk;
    }
    return err;
  }

  uint32_t op2;
  Error err = encodeRegOp2(c, 1, &op2);
  if (err != kErrorOk) {
    return err;
  }

  c.opcode = base | op2;
  return kErrorOk;
}

static Error encCmp(EncCtx& c, const Row& row) noexcept {
  if (c.n < 2 || !isGp(c.o(0))) {
    return kNoMatch;
  }

  uint32_t rn = regId(c.o(0));
  if (rn > 15u) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t base = row.opcode | (rn << 16);
  if (isImm(c.o(1))) {
    return encodeImmOp2(c, row, 1, base);
  }

  uint32_t op2;
  Error err = encodeRegOp2(c, 1, &op2);
  if (err != kErrorOk) {
    return err;
  }

  c.opcode = base | op2;
  return kErrorOk;
}

static Error encShift(EncCtx& c, const Row& row) noexcept {
  // Rd, Rm, #imm|Rs  (or shorthand Rd, #imm|Rs).
  if ((c.n != 2 && c.n != 3) || !isGp(c.o(0))) {
    return kNoMatch;
  }

  const Operand_& rdOp = c.o(0);
  const Operand_& rmOp = c.n == 3 ? c.o(1) : c.o(0);
  const Operand_& shOp = c.o(c.n - 1);

  if (!isGp(rmOp)) {
    return kNoMatch;
  }

  if (!isGpIdValid(rdOp) || !isGpIdValid(rmOp)) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t type = row.aux & 0x3u;
  uint32_t base = row.opcode | (regId(rdOp) << 12) | regId(rmOp);

  if (isImm(shOp)) {
    uint32_t amount;
    if (!immInRange(shOp, 0, 32, &amount)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }

    uint32_t shBits = 0;
    if (amount != 0 || type == 0) {
      if (!encodeImmShift(ShiftOp(type), amount, &shBits)) {
        return DebugUtils::errored(kErrorInvalidImmediate);
      }
    }
    else {
      // LSR/ASR/ROR #0 is invalid (ROR #0 would be RRX).
      return DebugUtils::errored(kErrorInvalidImmediate);
    }

    c.opcode = base | shBits;
    return kErrorOk;
  }

  if (isGp(shOp)) {
    if (!isGpIdValid(shOp)) {
      return DebugUtils::errored(kErrorInvalidPhysId);
    }
    c.opcode = base | (regId(shOp) << 8) | (type << 5) | (1u << 4);
    return kErrorOk;
  }

  return kNoMatch;
}

static Error encRrx(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2 || !isGp(c.o(0)) || !isGp(c.o(1))) {
    return kNoMatch;
  }

  if (!isGpIdValid(c.o(0)) || !isGpIdValid(c.o(1))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  c.opcode = row.opcode | (regId(c.o(0)) << 12) | regId(c.o(1));
  return kErrorOk;
}

static Error encMovW(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2 || !isGp(c.o(0)) || !isImm(c.o(1))) {
    return kNoMatch;
  }

  if (!isGpIdValid(c.o(0))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t v;
  if (!immInRange(c.o(1), 0, 0xFFFF, &v)) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  c.opcode = row.opcode | ((v >> 12) << 16) | (regId(c.o(0)) << 12) | (v & 0xFFFu);
  return kErrorOk;
}

//! Encodes registers described by `aux` (REGS) starting at operand 0. Returns the number of operands consumed.
static Error encodeRegFields(const EncCtx& c, uint32_t aux, uint32_t* opcode, uint32_t* consumed) noexcept {
  uint32_t count = regsCount(aux);
  uint32_t i = 0;
  uint32_t prevId = 0;

  for (uint32_t k = 0; k < count; k++) {
    uint32_t shift = regsShift(aux, k);

    if (shift == kRegSkip) {
      // Optional register that must be `prev + 1` (like Rt2 in LDREXD), the previous register must be even.
      if (i < c.n && isGp(c.o(i))) {
        if (regId(c.o(i)) != prevId + 1u) {
          return DebugUtils::errored(kErrorInvalidPhysId);
        }
        i++;
      }

      if ((prevId & 1u) != 0 || prevId == 14u) {
        return DebugUtils::errored(kErrorInvalidPhysId);
      }
      continue;
    }

    if (i >= c.n || !isGp(c.o(i))) {
      return kNoMatch;
    }

    if (!isGpIdValid(c.o(i))) {
      return DebugUtils::errored(kErrorInvalidPhysId);
    }

    prevId = regId(c.o(i));
    *opcode |= prevId << shift;
    i++;
  }

  *consumed = i;
  return kErrorOk;
}

static Error encRegs(EncCtx& c, const Row& row) noexcept {
  uint32_t opcode = row.opcode;
  uint32_t consumed;

  Error err = encodeRegFields(c, row.aux, &opcode, &consumed);
  if (err != kErrorOk) {
    return err;
  }

  if (consumed != c.n) {
    return kNoMatch;
  }

  c.opcode = opcode;
  return kErrorOk;
}

static Error encExt(EncCtx& c, const Row& row) noexcept {
  // Rd, [Rn,] Rm {, ROR #n}.
  bool hasRn = (row.aux & 1u) != 0;
  uint32_t nRegs = hasRn ? 3u : 2u;

  if (c.n < nRegs || c.n > nRegs + 1u) {
    return kNoMatch;
  }

  for (uint32_t i = 0; i < nRegs; i++) {
    if (!isGp(c.o(i))) {
      return kNoMatch;
    }
    if (!isGpIdValid(c.o(i))) {
      return DebugUtils::errored(kErrorInvalidPhysId);
    }
  }

  uint32_t opcode = row.opcode | (regId(c.o(0)) << 12) | regId(c.o(nRegs - 1u));
  if (hasRn) {
    opcode |= regId(c.o(1)) << 16;
  }

  if (c.n > nRegs) {
    const Operand_& rot = c.o(nRegs);
    if (!isImm(rot)) {
      return kNoMatch;
    }

    uint32_t sop = rot.as<Imm>().predicate();
    uint32_t v;
    if ((sop != uint32_t(ShiftOp::kROR) && sop != uint32_t(ShiftOp::kLSL)) || !immInRange(rot, 0, 24, &v) || (v & 7u) != 0) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
    opcode |= (v >> 3) << 10;
  }

  c.opcode = opcode;
  return kErrorOk;
}

static Error encSat(EncCtx& c, const Row& row) noexcept {
  // Rd, #sat, Rn {, LSL #n | ASR #n}.
  if (c.n < 3 || c.n > 4 || !isGp(c.o(0)) || !isImm(c.o(1)) || !isGp(c.o(2))) {
    return kNoMatch;
  }

  if (!isGpIdValid(c.o(0)) || !isGpIdValid(c.o(2))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  bool isSigned = (row.aux & 1u) != 0;
  uint32_t sat;
  if (!immInRange(c.o(1), isSigned ? 1 : 0, isSigned ? 32 : 31, &sat)) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  if (isSigned) {
    sat--;
  }

  uint32_t opcode = row.opcode | (sat << 16) | (regId(c.o(0)) << 12) | regId(c.o(2));

  if (c.n == 4) {
    const Operand_& sh = c.o(3);
    if (!isImm(sh)) {
      return kNoMatch;
    }

    uint32_t sop = sh.as<Imm>().predicate();
    uint32_t amount;
    if (sop == uint32_t(ShiftOp::kLSL)) {
      if (!immInRange(sh, 0, 31, &amount)) {
        return DebugUtils::errored(kErrorInvalidImmediate);
      }
      opcode |= amount << 7;
    }
    else if (sop == uint32_t(ShiftOp::kASR)) {
      if (!immInRange(sh, 1, 32, &amount)) {
        return DebugUtils::errored(kErrorInvalidImmediate);
      }
      opcode |= ((amount & 31u) << 7) | (1u << 6);
    }
    else {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
  }

  c.opcode = opcode;
  return kErrorOk;
}

static Error encSat16(EncCtx& c, const Row& row) noexcept {
  if (c.n != 3 || !isGp(c.o(0)) || !isImm(c.o(1)) || !isGp(c.o(2))) {
    return kNoMatch;
  }

  if (!isGpIdValid(c.o(0)) || !isGpIdValid(c.o(2))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  bool isSigned = (row.aux & 1u) != 0;
  uint32_t sat;
  if (!immInRange(c.o(1), isSigned ? 1 : 0, isSigned ? 16 : 15, &sat)) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  if (isSigned) {
    sat--;
  }

  c.opcode = row.opcode | (sat << 16) | (regId(c.o(0)) << 12) | regId(c.o(2));
  return kErrorOk;
}

static Error encPkh(EncCtx& c, const Row& row) noexcept {
  // Rd, Rn, Rm {, LSL #n (PKHBT) | ASR #n (PKHTB)}.
  if (c.n < 3 || c.n > 4 || !isGp(c.o(0)) || !isGp(c.o(1)) || !isGp(c.o(2))) {
    return kNoMatch;
  }

  if (!isGpIdValid(c.o(0)) || !isGpIdValid(c.o(1)) || !isGpIdValid(c.o(2))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  bool tb = (row.aux & 1u) != 0;
  uint32_t rd = regId(c.o(0));
  uint32_t rn = regId(c.o(1));
  uint32_t rm = regId(c.o(2));

  if (c.n == 3) {
    if (tb) {
      // PKHTB Rd, Rn, Rm without a shift is PKHBT Rd, Rm, Rn.
      c.opcode = (row.opcode & ~0x40u) | (rm << 16) | (rd << 12) | rn;
    }
    else {
      c.opcode = row.opcode | (rn << 16) | (rd << 12) | rm;
    }
    return kErrorOk;
  }

  const Operand_& sh = c.o(3);
  if (!isImm(sh)) {
    return kNoMatch;
  }

  uint32_t sop = sh.as<Imm>().predicate();
  uint32_t amount;

  if (!tb) {
    if (sop != uint32_t(ShiftOp::kLSL) || !immInRange(sh, 0, 31, &amount)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
  }
  else {
    if ((sop != uint32_t(ShiftOp::kASR) && sop != uint32_t(ShiftOp::kLSL)) || !immInRange(sh, 1, 32, &amount)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
    amount &= 31u;
  }

  c.opcode = row.opcode | (rn << 16) | (rd << 12) | (amount << 7) | rm;
  return kErrorOk;
}

static Error encBfc(EncCtx& c, const Row& row) noexcept {
  if (c.n != 3 || !isGp(c.o(0)) || !isImm(c.o(1)) || !isImm(c.o(2))) {
    return kNoMatch;
  }

  if (!isGpIdValid(c.o(0))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t lsb, width;
  if (!immInRange(c.o(1), 0, 31, &lsb) || !immInRange(c.o(2), 1, 32 - lsb, &width)) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  c.opcode = row.opcode | ((lsb + width - 1u) << 16) | (regId(c.o(0)) << 12) | (lsb << 7);
  return kErrorOk;
}

static Error encBfi(EncCtx& c, const Row& row) noexcept {
  if (c.n != 4 || !isGp(c.o(0)) || !isGp(c.o(1)) || !isImm(c.o(2)) || !isImm(c.o(3))) {
    return kNoMatch;
  }

  if (!isGpIdValid(c.o(0)) || !isGpIdValid(c.o(1))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t lsb, width;
  if (!immInRange(c.o(2), 0, 31, &lsb) || !immInRange(c.o(3), 1, 32 - lsb, &width)) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  c.opcode = row.opcode | ((lsb + width - 1u) << 16) | (regId(c.o(0)) << 12) | (lsb << 7) | regId(c.o(1));
  return kErrorOk;
}

static Error encBfx(EncCtx& c, const Row& row) noexcept {
  if (c.n != 4 || !isGp(c.o(0)) || !isGp(c.o(1)) || !isImm(c.o(2)) || !isImm(c.o(3))) {
    return kNoMatch;
  }

  if (!isGpIdValid(c.o(0)) || !isGpIdValid(c.o(1))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t lsb, width;
  if (!immInRange(c.o(2), 0, 31, &lsb) || !immInRange(c.o(3), 1, 32 - lsb, &width)) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  c.opcode = row.opcode | ((width - 1u) << 16) | (regId(c.o(0)) << 12) | (lsb << 7) | regId(c.o(1));
  return kErrorOk;
}

static ASMJIT_INLINE bool isRelTarget(const Operand_& op) noexcept {
  return isLabel(op) || isImm(op) || (isMem(op) && !op.as<Mem>().hasBaseReg() && !op.as<Mem>().hasIndex());
}

static Error encB(EncCtx& c, const Row& row) noexcept {
  if (c.n != 1 || !isRelTarget(c.o(0))) {
    return kNoMatch;
  }

  c.opcode = row.opcode;
  c.setRel(c.o(0), OffsetType::kSignedOffset, 24, 2);
  return kErrorOk;
}

static Error encBlx(EncCtx& c, const Row& row) noexcept {
  if (c.n != 1 || !isRelTarget(c.o(0))) {
    return kNoMatch;
  }

  c.opcode = row.opcode;
  c.setRel(c.o(0), OffsetType::kAArch32_1To24At0_0At24, 25, 1);
  return kErrorOk;
}

static Error encAdr(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2 || !isGp(c.o(0)) || !isRelTarget(c.o(1))) {
    return kNoMatch;
  }

  if (!isGpIdValid(c.o(0))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  c.opcode = row.opcode | (regId(c.o(0)) << 12);
  c.setRel(c.o(1), OffsetType::kAArch32_ADR, 32, 0);
  return kErrorOk;
}

//! Returns P (bit 24) and W (bit 21) bits of a memory operand based on its offset mode.
static ASMJIT_INLINE uint32_t memPW(const Mem& m) noexcept {
  switch (m.offsetMode()) {
    case OffsetMode::kPreIndex: return (1u << 24) | (1u << 21);
    case OffsetMode::kPostIndex: return 0u;
    default: return (1u << 24);
  }
}

static Error encLdSt(EncCtx& c, const Row& row) noexcept {
  // LDR|STR|LDRB|STRB Rt, mem (and LDRT|STRT|LDRBT|STRBT).
  if (c.n != 2 || !isGp(c.o(0))) {
    return kNoMatch;
  }

  bool isT = (row.aux & 1u) != 0;
  const Operand_& mOp = c.o(1);

  if (!isGpIdValid(c.o(0))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t opcode = row.opcode | (regId(c.o(0)) << 12);

  if (isRelTarget(mOp) && !isImm(mOp)) {
    // Literal (PC relative).
    if (isT || (isMem(mOp) && !mOp.as<Mem>().isFixedOffset())) {
      return DebugUtils::errored(kErrorInvalidAddress);
    }

    c.opcode = opcode | (1u << 24) | (15u << 16);
    c.setRel(mOp, OffsetType::kAArch32_U23_SignedOffset, 12, 0);
    return kErrorOk;
  }

  if (!isMem(mOp)) {
    return kNoMatch;
  }

  const Mem& m = mOp.as<Mem>();
  if (!m.hasBaseReg() || m.baseType() != RegType::kGp32 || m.baseId() > 15u) {
    return DebugUtils::errored(kErrorInvalidAddress);
  }

  uint32_t pw = memPW(m);
  if (isT) {
    // T variants are always post-indexed (P=0, W=1), `[Rn]` is accepted as `[Rn], #0`.
    if (m.isPreIndex() || (m.isFixedOffset() && (m.hasIndex() || m.offsetLo32() != 0))) {
      return DebugUtils::errored(kErrorInvalidAddress);
    }
    pw = (1u << 21);
  }

  opcode |= pw | (m.baseId() << 16);

  if (m.hasIndex()) {
    if (m.indexType() != RegType::kGp32 || m.indexId() > 15u) {
      return DebugUtils::errored(kErrorInvalidAddress);
    }

    uint32_t shBits;
    if (!encodeImmShift(m.shiftOp(), m.shift(), &shBits)) {
      return DebugUtils::errored(kErrorInvalidAddressScale);
    }

    opcode |= (1u << 25) | (uint32_t(!m.isNegIndex()) << 23) | shBits | m.indexId();
  }
  else {
    int32_t off = m.offsetLo32();
    uint32_t absOff = off < 0 ? uint32_t(0) - uint32_t(off) : uint32_t(off);

    if (absOff > 4095u) {
      return DebugUtils::errored(kErrorInvalidDisplacement);
    }

    opcode |= (uint32_t(off >= 0) << 23) | absOff;
  }

  c.opcode = opcode;
  return kErrorOk;
}

static Error encLdStX(EncCtx& c, const Row& row) noexcept {
  // LDRH|STRH|LDRSB|LDRSH|LDRD|STRD Rt, [Rt2,] mem (and T variants).
  bool isT = (row.aux & 1u) != 0;
  bool isDual = (row.aux & 2u) != 0;

  if (c.n < 2 || !isGp(c.o(0))) {
    return kNoMatch;
  }

  uint32_t mIndex = 1;
  if (isDual && c.n == 3) {
    if (!isGp(c.o(1))) {
      return kNoMatch;
    }
    if (regId(c.o(1)) != regId(c.o(0)) + 1u) {
      return DebugUtils::errored(kErrorInvalidPhysId);
    }
    mIndex = 2;
  }

  if (c.n != mIndex + 1u) {
    return kNoMatch;
  }

  uint32_t rt = regId(c.o(0));
  if (rt > 15u || (isDual && ((rt & 1u) != 0 || rt == 14u))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  const Operand_& mOp = c.o(mIndex);
  uint32_t opcode = row.opcode | (rt << 12);

  if (isRelTarget(mOp) && !isImm(mOp)) {
    if (isT || (isMem(mOp) && !mOp.as<Mem>().isFixedOffset())) {
      return DebugUtils::errored(kErrorInvalidAddress);
    }

    c.opcode = opcode | (1u << 24) | (1u << 22) | (15u << 16);
    c.setRel(mOp, OffsetType::kAArch32_U23_0To3At0_4To7At8, 8, 0);
    return kErrorOk;
  }

  if (!isMem(mOp)) {
    return kNoMatch;
  }

  const Mem& m = mOp.as<Mem>();
  if (!m.hasBaseReg() || m.baseType() != RegType::kGp32 || m.baseId() > 15u) {
    return DebugUtils::errored(kErrorInvalidAddress);
  }

  uint32_t pw = memPW(m);
  if (isT) {
    if (m.isPreIndex() || (m.isFixedOffset() && (m.hasIndex() || m.offsetLo32() != 0))) {
      return DebugUtils::errored(kErrorInvalidAddress);
    }
    pw = (1u << 21);
  }

  opcode |= pw | (m.baseId() << 16);

  if (m.hasIndex()) {
    if (m.indexType() != RegType::kGp32 || m.indexId() > 15u) {
      return DebugUtils::errored(kErrorInvalidAddress);
    }

    if (m.hasShift()) {
      return DebugUtils::errored(kErrorInvalidAddressScale);
    }

    opcode |= (uint32_t(!m.isNegIndex()) << 23) | m.indexId();
  }
  else {
    int32_t off = m.offsetLo32();
    uint32_t absOff = off < 0 ? uint32_t(0) - uint32_t(off) : uint32_t(off);

    if (absOff > 255u) {
      return DebugUtils::errored(kErrorInvalidDisplacement);
    }

    opcode |= (1u << 22) | (uint32_t(off >= 0) << 23) | ((absOff >> 4) << 8) | (absOff & 0xFu);
  }

  c.opcode = opcode;
  return kErrorOk;
}

//! Decodes a base register of LDM/STM/VLDM/VSTM - either a plain GP register or a memory operand with write-back.
static Error decodeMultipleBase(const Operand_& op, uint32_t* rn, bool* wb) noexcept {
  if (isGp(op)) {
    if (!isGpIdValid(op)) {
      return DebugUtils::errored(kErrorInvalidPhysId);
    }
    *rn = regId(op);
    *wb = false;
    return kErrorOk;
  }

  if (isMem(op)) {
    const Mem& m = op.as<Mem>();
    if (!m.hasBaseReg() || m.baseType() != RegType::kGp32 || m.baseId() > 15u || m.hasIndex() || m.offsetLo32() != 0) {
      return DebugUtils::errored(kErrorInvalidAddress);
    }
    *rn = m.baseId();
    *wb = m.isPreOrPost();
    return kErrorOk;
  }

  return kNoMatch;
}

//! Decodes a list of GP registers from operands [i, n).
static Error decodeGpList(const EncCtx& c, uint32_t i, uint32_t* maskOut) noexcept {
  if (i >= c.n) {
    return kNoMatch;
  }

  uint32_t mask = 0;
  if (i + 1u == c.n && c.o(i).isRegList()) {
    const BaseRegList& list = c.o(i).as<BaseRegList>();
    if (!list.isType(RegType::kGp32)) {
      return kNoMatch;
    }
    mask = list.list();
  }
  else {
    for (uint32_t k = i; k < c.n; k++) {
      if (!isGp(c.o(k))) {
        return kNoMatch;
      }
      if (!isGpIdValid(c.o(k))) {
        return DebugUtils::errored(kErrorInvalidPhysId);
      }
      mask |= 1u << regId(c.o(k));
    }
  }

  if (mask == 0 || mask > 0xFFFFu) {
    return DebugUtils::errored(kErrorInvalidRegType);
  }

  *maskOut = mask;
  return kErrorOk;
}

static Error encLdm(EncCtx& c, const Row& row) noexcept {
  if (c.n < 2) {
    return kNoMatch;
  }

  uint32_t rn;
  bool wb;
  Error err = decodeMultipleBase(c.o(0), &rn, &wb);
  if (err != kErrorOk) {
    return err;
  }

  uint32_t mask;
  err = decodeGpList(c, 1, &mask);
  if (err != kErrorOk) {
    return err;
  }

  c.opcode = row.opcode | (uint32_t(wb) << 21) | (rn << 16) | mask;
  return kErrorOk;
}

static Error encPushPop(EncCtx& c, const Row& row) noexcept {
  uint32_t mask;
  Error err = decodeGpList(c, 0, &mask);
  if (err != kErrorOk) {
    return err;
  }

  bool isPop = (row.aux & 1u) != 0;
  if (Support::isPowerOf2(mask)) {
    // Single register - encoded as STR Rt, [SP, #-4]! or LDR Rt, [SP], #4.
    uint32_t rt = Support::ctz(mask);
    c.opcode = (isPop ? 0x049D0004u : 0x052D0004u) | (rt << 12);
    return kErrorOk;
  }

  c.opcode = row.opcode | mask;
  return kErrorOk;
}

static Error encMemRegs(EncCtx& c, const Row& row) noexcept {
  if (c.n < 2 || !isMem(c.o(c.n - 1u))) {
    return kNoMatch;
  }

  uint32_t opcode = row.opcode;
  uint32_t consumed;

  EncCtx tmp = c;
  tmp.n = c.n - 1u;

  Error err = encodeRegFields(tmp, row.aux, &opcode, &consumed);
  if (err != kErrorOk) {
    return err;
  }

  if (consumed != tmp.n) {
    return kNoMatch;
  }

  const Mem& m = c.o(c.n - 1u).as<Mem>();
  if (!m.hasBaseReg() || m.baseType() != RegType::kGp32 || m.baseId() > 15u || m.hasIndex() || !m.isFixedOffset()) {
    return DebugUtils::errored(kErrorInvalidAddress);
  }

  if (m.offsetLo32() != 0) {
    return DebugUtils::errored(kErrorInvalidDisplacement);
  }

  c.opcode = opcode | (m.baseId() << 16);
  return kErrorOk;
}

static Error encNone(EncCtx& c, const Row& row) noexcept {
  if (c.n != 0) {
    return kNoMatch;
  }

  c.opcode = row.opcode;
  return kErrorOk;
}

static Error encBarrier(EncCtx& c, const Row& row) noexcept {
  uint32_t option = 0xFu;

  if (c.n == 1) {
    if (!isImm(c.o(0))) {
      return kNoMatch;
    }
    if (!immInRange(c.o(0), 0, 15, &option)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
  }
  else if (c.n != 0) {
    return kNoMatch;
  }

  c.opcode = row.opcode | option;
  return kErrorOk;
}

static Error encImm(EncCtx& c, const Row& row) noexcept {
  uint32_t v = 0;

  if (c.n == 1) {
    if (!isImm(c.o(0))) {
      return kNoMatch;
    }

    int64_t hi = row.aux == 0 ? 0xFFFFFF : row.aux == 1 ? 0xFFFF : 0xF;
    if (!immInRange(c.o(0), 0, hi, &v)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
  }
  else if (c.n != 0) {
    return kNoMatch;
  }

  if (row.aux == 1) {
    v = ((v >> 4) << 8) | (v & 0xFu);
  }

  c.opcode = row.opcode | v;
  return kErrorOk;
}

static Error encPld(EncCtx& c, const Row& row) noexcept {
  if (c.n != 1) {
    return kNoMatch;
  }

  const Operand_& mOp = c.o(0);
  bool isPldw = (row.aux & 1u) != 0;

  if (isRelTarget(mOp) && !isImm(mOp)) {
    if (isPldw) {
      return DebugUtils::errored(kErrorInvalidAddress);
    }

    c.opcode = row.opcode | (15u << 16);
    c.setRel(mOp, OffsetType::kAArch32_U23_SignedOffset, 12, 0);
    return kErrorOk;
  }

  if (!isMem(mOp)) {
    return kNoMatch;
  }

  const Mem& m = mOp.as<Mem>();
  if (!m.hasBaseReg() || m.baseType() != RegType::kGp32 || m.baseId() > 15u || !m.isFixedOffset()) {
    return DebugUtils::errored(kErrorInvalidAddress);
  }

  uint32_t opcode = row.opcode | (m.baseId() << 16);

  if (m.hasIndex()) {
    if (m.indexType() != RegType::kGp32 || m.indexId() > 15u) {
      return DebugUtils::errored(kErrorInvalidAddress);
    }

    uint32_t shBits;
    if (!encodeImmShift(m.shiftOp(), m.shift(), &shBits)) {
      return DebugUtils::errored(kErrorInvalidAddressScale);
    }

    opcode |= (1u << 25) | (uint32_t(!m.isNegIndex()) << 23) | shBits | m.indexId();
  }
  else {
    int32_t off = m.offsetLo32();
    uint32_t absOff = off < 0 ? uint32_t(0) - uint32_t(off) : uint32_t(off);

    if (absOff > 4095u) {
      return DebugUtils::errored(kErrorInvalidDisplacement);
    }

    opcode |= (uint32_t(off >= 0) << 23) | absOff;
  }

  c.opcode = opcode;
  return kErrorOk;
}

static Error encMrs(EncCtx& c, const Row& row) noexcept {
  if (c.n < 1 || c.n > 2 || !isGp(c.o(0))) {
    return kNoMatch;
  }

  uint32_t psr = 0;
  if (c.n == 2) {
    if (!isImm(c.o(1))) {
      return kNoMatch;
    }
    if (!immInRange(c.o(1), 0, 0x1F, &psr)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
  }

  if (!isGpIdValid(c.o(0))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  c.opcode = row.opcode | (((psr >> 4) & 1u) << 22) | (regId(c.o(0)) << 12);
  return kErrorOk;
}

static Error encMsr(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2 || !isImm(c.o(0))) {
    return kNoMatch;
  }

  uint32_t psr;
  if (!immInRange(c.o(0), 0, 0x1F, &psr) || (psr & 0xFu) == 0) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  uint32_t opcode = row.opcode | (((psr >> 4) & 1u) << 22) | ((psr & 0xFu) << 16);

  if (isGp(c.o(1))) {
    if (!isGpIdValid(c.o(1))) {
      return DebugUtils::errored(kErrorInvalidPhysId);
    }
    c.opcode = opcode | regId(c.o(1));
    return kErrorOk;
  }

  if (isImm(c.o(1))) {
    uint32_t v, enc;
    if (!immToU32(c.o(1), &v) || !encodeModImm(v, &enc)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
    c.opcode = opcode | (1u << 25) | enc;
    return kErrorOk;
  }

  return kNoMatch;
}

static Error encCps(EncCtx& c, const Row& row) noexcept {
  bool hasFlags = row.aux != 0;

  if (!hasFlags) {
    // CPS #mode.
    uint32_t mode;
    if (c.n != 1 || !isImm(c.o(0))) {
      return kNoMatch;
    }
    if (!immInRange(c.o(0), 0, 31, &mode)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
    c.opcode = row.opcode | mode;
    return kErrorOk;
  }

  // CPSIE|CPSID #flags {, #mode}.
  if (c.n < 1 || c.n > 2 || !isImm(c.o(0))) {
    return kNoMatch;
  }

  uint32_t flags;
  if (!immInRange(c.o(0), 1, 7, &flags)) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  uint32_t opcode = row.opcode | (flags << 6);
  if (c.n == 2) {
    uint32_t mode;
    if (!isImm(c.o(1))) {
      return kNoMatch;
    }
    if (!immInRange(c.o(1), 0, 31, &mode)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
    opcode |= (1u << 17) | mode;
  }

  c.opcode = opcode;
  return kErrorOk;
}

static Error encSetend(EncCtx& c, const Row& row) noexcept {
  uint32_t e;
  if (c.n != 1 || !isImm(c.o(0))) {
    return kNoMatch;
  }
  if (!immInRange(c.o(0), 0, 1, &e)) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }
  c.opcode = row.opcode | (e << 9);
  return kErrorOk;
}

static Error encMcr(EncCtx& c, const Row& row) noexcept {
  // #coproc, #opc1, Rt, #CRn, #CRm {, #opc2}.
  if (c.n < 5 || c.n > 6 || !isImm(c.o(0)) || !isImm(c.o(1)) || !isGp(c.o(2)) || !isImm(c.o(3)) || !isImm(c.o(4))) {
    return kNoMatch;
  }

  uint32_t cp, opc1, crn, crm, opc2 = 0;
  if (!immInRange(c.o(0), 0, 15, &cp) ||
      !immInRange(c.o(1), 0, 7, &opc1) ||
      !immInRange(c.o(3), 0, 15, &crn) ||
      !immInRange(c.o(4), 0, 15, &crm)) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  if (c.n == 6) {
    if (!isImm(c.o(5))) {
      return kNoMatch;
    }
    if (!immInRange(c.o(5), 0, 7, &opc2)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
  }

  if (!isGpIdValid(c.o(2))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  c.opcode = row.opcode | (opc1 << 21) | (crn << 16) | (regId(c.o(2)) << 12) | (cp << 8) | (opc2 << 5) | crm;
  return kErrorOk;
}

static Error encMcrr(EncCtx& c, const Row& row) noexcept {
  // #coproc, #opc1, Rt, Rt2, #CRm.
  if (c.n != 5 || !isImm(c.o(0)) || !isImm(c.o(1)) || !isGp(c.o(2)) || !isGp(c.o(3)) || !isImm(c.o(4))) {
    return kNoMatch;
  }

  uint32_t cp, opc1, crm;
  if (!immInRange(c.o(0), 0, 15, &cp) ||
      !immInRange(c.o(1), 0, 15, &opc1) ||
      !immInRange(c.o(4), 0, 15, &crm)) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  if (!isGpIdValid(c.o(2)) || !isGpIdValid(c.o(3))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  c.opcode = row.opcode | (regId(c.o(3)) << 16) | (regId(c.o(2)) << 12) | (cp << 8) | (opc1 << 4) | crm;
  return kErrorOk;
}

} // {anonymous}

// a32::Assembler - Encoders (VFP)
// ===============================

namespace {

static ASMJIT_INLINE bool dtIsNoneOr(DataType dt, DataType a) noexcept { return dt == DataType::kNone || dt == a; }

static Error encV3(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2 && c.n != 3) {
    return kNoMatch;
  }

  const Operand_& d = c.o(0);
  const Operand_& n = c.n == 3 ? c.o(1) : c.o(0);
  const Operand_& m = c.o(c.n - 1u);

  uint32_t sz;
  if (isPlainS(d) && isPlainS(n) && isPlainS(m)) {
    if (!dtIsNoneOr(c.dt, DataType::kF32)) {
      return kNoMatch;
    }
    sz = 0;
  }
  else if (isPlainD(d) && isPlainD(n) && isPlainD(m)) {
    if (!dtIsNoneOr(c.dt, DataType::kF64)) {
      return kNoMatch;
    }
    sz = 1;
  }
  else {
    return kNoMatch;
  }

  if (c.dt2 != DataType::kNone) {
    return kNoMatch;
  }

  c.opcode = row.opcode | (sz << 8) | encVd(d) | encVn(n) | encVm(m);
  return kErrorOk;
}

static Error encV2(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2) {
    return kNoMatch;
  }

  const Operand_& d = c.o(0);
  const Operand_& m = c.o(1);

  uint32_t sz;
  if (isPlainS(d) && isPlainS(m)) {
    if (!dtIsNoneOr(c.dt, DataType::kF32)) {
      return kNoMatch;
    }
    sz = 0;
  }
  else if (isPlainD(d) && isPlainD(m)) {
    if (!dtIsNoneOr(c.dt, DataType::kF64)) {
      return kNoMatch;
    }
    sz = 1;
  }
  else {
    return kNoMatch;
  }

  if (c.dt2 != DataType::kNone) {
    return kNoMatch;
  }

  c.opcode = row.opcode | (sz << 8) | encVd(d) | encVm(m);
  return kErrorOk;
}

static Error encVCmp0(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2 || !isImm(c.o(1))) {
    return kNoMatch;
  }

  const Operand_& d = c.o(0);
  uint32_t sz;

  if (isPlainS(d) && dtIsNoneOr(c.dt, DataType::kF32)) {
    sz = 0;
  }
  else if (isPlainD(d) && dtIsNoneOr(c.dt, DataType::kF64)) {
    sz = 1;
  }
  else {
    return kNoMatch;
  }

  if (immAsDouble(c.o(1)) != 0.0) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  c.opcode = row.opcode | (sz << 8) | encVd(d);
  return kErrorOk;
}

static ASMJIT_INLINE bool dtIsI32(DataType dt) noexcept { return dt == DataType::kS32 || dt == DataType::kU32; }
static ASMJIT_INLINE bool dtIsI16or32(DataType dt) noexcept {
  return dt == DataType::kS16 || dt == DataType::kU16 || dt == DataType::kS32 || dt == DataType::kU32;
}
static ASMJIT_INLINE bool dtIsF32or64(DataType dt) noexcept { return dt == DataType::kF32 || dt == DataType::kF64; }

//! Tests whether `op` is a plain VFP register matching the floating point `dt` (S for F32, D for F64).
static ASMJIT_INLINE bool isVfpRegOf(const Operand_& op, DataType dt) noexcept {
  return dt == DataType::kF64 ? isPlainD(op) : isPlainS(op);
}

static Error encVCvt(EncCtx& c, const Row& row) noexcept {
  bool isR = row.aux != 0;
  DataType dst = c.dt;
  DataType src = c.dt2;

  if (c.n == 2) {
    const Operand_& d = c.o(0);
    const Operand_& m = c.o(1);

    // VFP.
    if (!isR && dst == DataType::kF64 && src == DataType::kF32 && isPlainD(d) && isPlainS(m)) {
      c.opcode = 0x0EB70AC0u | encVd(d) | encVm(m);
      return kErrorOk;
    }

    if (!isR && dst == DataType::kF32 && src == DataType::kF64 && isPlainS(d) && isPlainD(m)) {
      c.opcode = 0x0EB70BC0u | encVd(d) | encVm(m);
      return kErrorOk;
    }

    if (!isR && dtIsF32or64(dst) && dtIsI32(src) && isVfpRegOf(d, dst) && isPlainS(m)) {
      c.opcode = 0x0EB80A40u | (uint32_t(dst == DataType::kF64) << 8) | (uint32_t(src == DataType::kS32) << 7) | encVd(d) | encVm(m);
      return kErrorOk;
    }

    if (dtIsI32(dst) && dtIsF32or64(src) && isPlainS(d) && isVfpRegOf(m, src)) {
      c.opcode = 0x0EBC0A40u | (uint32_t(dst == DataType::kS32) << 16) | (uint32_t(src == DataType::kF64) << 8) | (uint32_t(!isR) << 7) | encVd(d) | encVm(m);
      return kErrorOk;
    }

    if (isR) {
      return kNoMatch;
    }

    // ASIMD.
    if ((isPlainD(d) && isPlainD(m)) || (isPlainQ(d) && isPlainQ(m))) {
      uint32_t op;
      if (dst == DataType::kF32 && src == DataType::kS32) op = 0;
      else if (dst == DataType::kF32 && src == DataType::kU32) op = 1;
      else if (dst == DataType::kS32 && src == DataType::kF32) op = 2;
      else if (dst == DataType::kU32 && src == DataType::kF32) op = 3;
      else return kNoMatch;

      c.uncond = true;
      c.opcode = 0xF3BB0600u | (op << 7) | qBit(d) | encVd(d) | encVm(m);
      return kErrorOk;
    }

    if (dst == DataType::kF16 && src == DataType::kF32 && isPlainD(d) && isPlainQ(m)) {
      c.uncond = true;
      c.opcode = 0xF3B60600u | encVd(d) | encVm(m);
      return kErrorOk;
    }

    if (dst == DataType::kF32 && src == DataType::kF16 && isPlainQ(d) && isPlainD(m)) {
      c.uncond = true;
      c.opcode = 0xF3B60700u | encVd(d) | encVm(m);
      return kErrorOk;
    }

    return kNoMatch;
  }

  if (c.n == 3 && !isR && isImm(c.o(2))) {
    const Operand_& d = c.o(0);
    const Operand_& m = c.o(1);

    // VFP fixed-point conversion (Vd and Vm must be the same register).
    bool toFixed = dtIsI16or32(dst) && dtIsF32or64(src);
    bool toFloat = dtIsF32or64(dst) && dtIsI16or32(src);

    if ((toFixed || toFloat) && ((isPlainS(d) && isPlainS(m)) || (isPlainD(d) && isPlainD(m))) && regId(d) == regId(m)) {
      DataType fdt = toFixed ? src : dst;
      DataType idt = toFixed ? dst : src;

      if (!isVfpRegOf(d, fdt)) {
        return kNoMatch;
      }

      uint32_t size = dtSizeBits(idt);
      uint32_t fbits;
      if (!immInRange(c.o(2), size == 16 ? 0 : 1, size, &fbits)) {
        return DebugUtils::errored(kErrorInvalidImmediate);
      }

      uint32_t imm = size - fbits;
      c.opcode = (toFixed ? 0x0EBE0A40u : 0x0EBA0A40u) |
                 (uint32_t(dtIsUnsigned(idt)) << 16) |
                 (uint32_t(fdt == DataType::kF64) << 8) |
                 (uint32_t(size == 32) << 7) |
                 ((imm & 1u) << 5) | (imm >> 1) | encVd(d);
      return kErrorOk;
    }

    // ASIMD fixed-point conversion.
    if ((isPlainD(d) && isPlainD(m)) || (isPlainQ(d) && isPlainQ(m))) {
      uint32_t op, u;
      if (dst == DataType::kF32 && dtIsI32(src)) {
        op = 0;
        u = uint32_t(src == DataType::kU32);
      }
      else if (dtIsI32(dst) && src == DataType::kF32) {
        op = 1;
        u = uint32_t(dst == DataType::kU32);
      }
      else {
        return kNoMatch;
      }

      uint32_t fbits;
      if (!immInRange(c.o(2), 1, 32, &fbits)) {
        return DebugUtils::errored(kErrorInvalidImmediate);
      }

      c.uncond = true;
      c.opcode = 0xF2800E10u | (u << 24) | ((64u - fbits) << 16) | (op << 8) | qBit(d) | encVd(d) | encVm(m);
      return kErrorOk;
    }
  }

  return kNoMatch;
}

static Error encVCvtRm(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2) {
    return kNoMatch;
  }

  const Operand_& d = c.o(0);
  const Operand_& m = c.o(1);

  uint32_t rm = row.aux & 0x3u;
  DataType dst = c.dt;
  DataType src = c.dt2;

  if (!dtIsI32(dst)) {
    return kNoMatch;
  }

  if (dtIsF32or64(src) && isPlainS(d) && isVfpRegOf(m, src)) {
    c.opcode = 0xFEBC0A40u | (rm << 16) | (uint32_t(src == DataType::kF64) << 8) | (uint32_t(dst == DataType::kS32) << 7) | encVd(d) | encVm(m);
    return kErrorOk;
  }

  if (src == DataType::kF32 && ((isPlainD(d) && isPlainD(m)) || (isPlainQ(d) && isPlainQ(m)))) {
    c.opcode = 0xF3BB0000u | (rm << 8) | (uint32_t(dst == DataType::kU32) << 7) | qBit(d) | encVd(d) | encVm(m);
    return kErrorOk;
  }

  return kNoMatch;
}

static Error encVCvtBT(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2) {
    return kNoMatch;
  }

  const Operand_& d = c.o(0);
  const Operand_& m = c.o(1);

  DataType dst = c.dt;
  DataType src = c.dt2;

  if (src == DataType::kF16 && dtIsF32or64(dst) && isVfpRegOf(d, dst) && isPlainS(m)) {
    c.opcode = row.opcode | (uint32_t(dst == DataType::kF64) << 8) | encVd(d) | encVm(m);
    return kErrorOk;
  }

  if (dst == DataType::kF16 && dtIsF32or64(src) && isPlainS(d) && isVfpRegOf(m, src)) {
    c.opcode = row.opcode | (1u << 16) | (uint32_t(src == DataType::kF64) << 8) | encVd(d) | encVm(m);
    return kErrorOk;
  }

  return kNoMatch;
}

// ASIMD modified immediate.
static bool encodeNImmValue(uint32_t esize, uint64_t v, uint32_t op, bool orrBic, uint32_t* cmodeOut, uint32_t* imm8Out) noexcept {
  switch (esize) {
    case 8: {
      if (op != 0 || orrBic || (v & ~uint64_t(0xFF)) != 0) {
        return false;
      }
      *cmodeOut = 0xEu;
      *imm8Out = uint32_t(v);
      return true;
    }

    case 16: {
      v &= 0xFFFFu;
      for (uint32_t k = 0; k < 2; k++) {
        if ((v & ~(uint64_t(0xFF) << (k * 8u))) == 0) {
          *cmodeOut = 0x8u | (k << 1) | uint32_t(orrBic);
          *imm8Out = uint32_t(v >> (k * 8u)) & 0xFFu;
          return true;
        }
      }
      return false;
    }

    case 32: {
      v &= 0xFFFFFFFFu;
      for (uint32_t k = 0; k < 4; k++) {
        if ((v & ~(uint64_t(0xFF) << (k * 8u))) == 0) {
          *cmodeOut = (k << 1) | uint32_t(orrBic);
          *imm8Out = uint32_t(v >> (k * 8u)) & 0xFFu;
          return true;
        }
      }

      if (!orrBic) {
        if ((v & ~uint64_t(0xFF00)) == 0xFFu) {
          *cmodeOut = 0xCu;
          *imm8Out = uint32_t(v >> 8) & 0xFFu;
          return true;
        }

        if ((v & ~uint64_t(0xFF0000)) == 0xFFFFu) {
          *cmodeOut = 0xDu;
          *imm8Out = uint32_t(v >> 16) & 0xFFu;
          return true;
        }
      }
      return false;
    }

    case 64: {
      if (op != 1 || orrBic) {
        return false;
      }

      uint32_t imm8 = 0;
      for (uint32_t i = 0; i < 8; i++) {
        uint32_t b = uint32_t(v >> (i * 8u)) & 0xFFu;
        if (b == 0xFFu) {
          imm8 |= 1u << i;
        }
        else if (b != 0) {
          return false;
        }
      }

      *cmodeOut = 0xEu;
      *imm8Out = imm8;
      return true;
    }

    default:
      return false;
  }
}

static ASMJIT_INLINE uint32_t makeNImmOpcode(uint32_t cmode, uint32_t op, uint32_t imm8, const Operand_& d) noexcept {
  return 0xF2800010u |
         (((imm8 >> 7) & 1u) << 24) |
         (((imm8 >> 4) & 7u) << 16) |
         (imm8 & 0xFu) |
         (cmode << 8) |
         (op << 5) |
         qBit(d) |
         encVd(d);
}

//! Encodes VMOV|VMVN|VORR|VBIC|VAND|VORN (immediate).
static Error encodeNImm(EncCtx& c, uint32_t kind, const Operand_& d, const Operand_& immOp) noexcept {
  if (!isPlainDQ(d) || !isImm(immOp) || c.dt2 != DataType::kNone) {
    return kNoMatch;
  }

  DataType dt = c.dt;
  uint32_t cmode, imm8;

  if (dt == DataType::kF32) {
    if (kind != kNImmMov) {
      return kNoMatch;
    }

    if (!encodeFP32Imm8(float(immAsDouble(immOp)), &imm8)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }

    c.opcode = makeNImmOpcode(0xFu, 0, imm8, d);
    return kErrorOk;
  }

  if (immOp.as<Imm>().isDouble()) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  uint32_t esize;
  if (dt == DataType::kS8 || dt == DataType::kU8) esize = 8;
  else if (dt == DataType::kS16 || dt == DataType::kU16) esize = 16;
  else if (dt == DataType::kS32 || dt == DataType::kU32) esize = 32;
  else if (dt == DataType::kS64 || dt == DataType::kU64) esize = 64;
  else return kNoMatch;

  uint64_t v = uint64_t(immOp.as<Imm>().value());
  uint64_t sizeMask = esize == 64 ? ~uint64_t(0) : (uint64_t(1) << esize) - 1u;

  // Accept both signed and unsigned representation of the value.
  if (esize < 64) {
    uint64_t hi = v & ~sizeMask;
    if (hi != 0 && hi != (~uint64_t(0) & ~sizeMask)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
    v &= sizeMask;
  }

  switch (kind) {
    case kNImmAnd:
      v = ~v & sizeMask;
      kind = kNImmBic;
      break;

    case kNImmOrn:
      v = ~v & sizeMask;
      kind = kNImmOrr;
      break;

    default:
      break;
  }

  if (kind == kNImmOrr || kind == kNImmBic) {
    if (esize != 16 && esize != 32) {
      return kNoMatch;
    }

    if (!encodeNImmValue(esize, v, 0, true, &cmode, &imm8)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }

    c.opcode = makeNImmOpcode(cmode, uint32_t(kind == kNImmBic), imm8, d);
    return kErrorOk;
  }

  uint32_t op = uint32_t(kind == kNImmMvn);

  if (esize == 64) {
    if (op != 0) {
      return kNoMatch;
    }

    if (!encodeNImmValue(64, v, 1, false, &cmode, &imm8)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }

    c.opcode = makeNImmOpcode(cmode, 1, imm8, d);
    return kErrorOk;
  }

  if (esize == 8) {
    if (op != 0) {
      return kNoMatch;
    }

    if (!encodeNImmValue(8, v, 0, false, &cmode, &imm8)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }

    c.opcode = makeNImmOpcode(cmode, 0, imm8, d);
    return kErrorOk;
  }

  // I16 / I32 - try direct encoding first.
  if (encodeNImmValue(esize, v, op, false, &cmode, &imm8)) {
    c.opcode = makeNImmOpcode(cmode, op, imm8, d);
    return kErrorOk;
  }

  // Try the value as a byte splat (VMOV.I8).
  uint64_t inv = ~v & sizeMask;
  uint64_t movValue = op ? inv : v;

  uint32_t b0 = uint32_t(movValue & 0xFFu);
  bool isSplat = true;
  for (uint32_t i = 8; i < esize; i += 8) {
    if (uint32_t((movValue >> i) & 0xFFu) != b0) {
      isSplat = false;
      break;
    }
  }

  if (isSplat) {
    c.opcode = makeNImmOpcode(0xEu, 0, b0, d);
    return kErrorOk;
  }

  // Try the inverted operation (VMOV <-> VMVN).
  if (encodeNImmValue(esize, inv, op ^ 1u, false, &cmode, &imm8)) {
    c.opcode = makeNImmOpcode(cmode, op ^ 1u, imm8, d);
    return kErrorOk;
  }

  return DebugUtils::errored(kErrorInvalidImmediate);
}

static Error encVMov(EncCtx& c, const Row& row) noexcept {
  DebugUtils::unused(row);

  DataType dt = c.dt;
  if (c.dt2 != DataType::kNone) {
    return kNoMatch;
  }

  if (c.n == 2) {
    const Operand_& a = c.o(0);
    const Operand_& b = c.o(1);

    // VMOV Sd, Sm | VMOV.F64 Dd, Dm.
    if (isPlainS(a) && isPlainS(b) && dtIsNoneOr(dt, DataType::kF32)) {
      c.opcode = 0x0EB00A40u | encVd(a) | encVm(b);
      return kErrorOk;
    }

    if (isPlainD(a) && isPlainD(b) && dt == DataType::kF64) {
      c.opcode = 0x0EB00B40u | encVd(a) | encVm(b);
      return kErrorOk;
    }

    // VMOV Dd, Dm | VMOV Qd, Qm (alias of VORR Vd, Vm, Vm).
    if ((isPlainD(a) && isPlainD(b)) || (isPlainQ(a) && isPlainQ(b))) {
      c.uncond = true;
      c.opcode = 0xF2200110u | qBit(a) | encVd(a) | encVn(b) | encVm(b);
      return kErrorOk;
    }

    // VMOV Sn, Rt | VMOV Rt, Sn.
    if (isPlainS(a) && isGp(b)) {
      if (!isGpIdValid(b)) return DebugUtils::errored(kErrorInvalidPhysId);
      c.opcode = 0x0E000A10u | encVn(a) | (regId(b) << 12);
      return kErrorOk;
    }

    if (isGp(a) && isPlainS(b)) {
      if (!isGpIdValid(a)) return DebugUtils::errored(kErrorInvalidPhysId);
      c.opcode = 0x0E100A10u | encVn(b) | (regId(a) << 12);
      return kErrorOk;
    }

    // VMOV.<size> Dd[x], Rt.
    if (isScalarD(a) && isGp(b)) {
      if (!isGpIdValid(b)) return DebugUtils::errored(kErrorInvalidPhysId);

      uint32_t x = a.as<Vec>().elementIndex();
      uint32_t sizeLog2 = dt == DataType::kNone ? 2u : dtSizeLog2(dt);
      uint32_t fields;

      if (sizeLog2 == 0 && x < 8) fields = (1u << 22) | ((x >> 2) << 21) | ((x & 3u) << 5);
      else if (sizeLog2 == 1 && x < 4) fields = ((x >> 1) << 21) | ((((x & 1u) << 1) | 1u) << 5);
      else if (sizeLog2 == 2 && x < 2) fields = (x << 21);
      else return DebugUtils::errored(kErrorInvalidElementIndex);

      c.opcode = 0x0E000B10u | fields | encVn(a) | (regId(b) << 12);
      return kErrorOk;
    }

    // VMOV.<dt> Rt, Dn[x].
    if (isGp(a) && isScalarD(b)) {
      if (!isGpIdValid(a)) return DebugUtils::errored(kErrorInvalidPhysId);

      uint32_t x = b.as<Vec>().elementIndex();
      uint32_t sizeLog2 = dt == DataType::kNone ? 2u : dtSizeLog2(dt);
      uint32_t u = uint32_t(dtIsUnsigned(dt) && sizeLog2 < 2);
      uint32_t fields;

      if (sizeLog2 == 0 && x < 8) fields = (1u << 22) | ((x >> 2) << 21) | ((x & 3u) << 5);
      else if (sizeLog2 == 1 && x < 4) fields = ((x >> 1) << 21) | ((((x & 1u) << 1) | 1u) << 5);
      else if (sizeLog2 == 2 && x < 2) fields = (x << 21);
      else return DebugUtils::errored(kErrorInvalidElementIndex);

      c.opcode = 0x0E100B10u | (u << 23) | fields | encVn(b) | (regId(a) << 12);
      return kErrorOk;
    }

    // VMOV Vd, #imm.
    if (isImm(b)) {
      if (isPlainS(a) && dtIsNoneOr(dt, DataType::kF32)) {
        uint32_t imm8;
        if (!encodeFP32Imm8(float(immAsDouble(b)), &imm8)) {
          return DebugUtils::errored(kErrorInvalidImmediate);
        }
        c.opcode = 0x0EB00A00u | ((imm8 >> 4) << 16) | (imm8 & 0xFu) | encVd(a);
        return kErrorOk;
      }

      if (isPlainD(a) && (dt == DataType::kF64 || (dt == DataType::kNone && b.as<Imm>().isDouble()))) {
        uint32_t imm8;
        if (!encodeFP64Imm8(immAsDouble(b), &imm8)) {
          return DebugUtils::errored(kErrorInvalidImmediate);
        }
        c.opcode = 0x0EB00B00u | ((imm8 >> 4) << 16) | (imm8 & 0xFu) | encVd(a);
        return kErrorOk;
      }

      if (isPlainDQ(a) && dt != DataType::kNone) {
        c.uncond = true;
        return encodeNImm(c, kNImmMov, a, b);
      }
    }

    return kNoMatch;
  }

  if (c.n == 3) {
    // VMOV Dm, Rt, Rt2 | VMOV Rt, Rt2, Dm.
    if (isPlainD(c.o(0)) && isGp(c.o(1)) && isGp(c.o(2))) {
      if (!isGpIdValid(c.o(1)) || !isGpIdValid(c.o(2))) return DebugUtils::errored(kErrorInvalidPhysId);
      c.opcode = 0x0C400B10u | (regId(c.o(2)) << 16) | (regId(c.o(1)) << 12) | encVm(c.o(0));
      return kErrorOk;
    }

    if (isGp(c.o(0)) && isGp(c.o(1)) && isPlainD(c.o(2))) {
      if (!isGpIdValid(c.o(0)) || !isGpIdValid(c.o(1))) return DebugUtils::errored(kErrorInvalidPhysId);
      c.opcode = 0x0C500B10u | (regId(c.o(1)) << 16) | (regId(c.o(0)) << 12) | encVm(c.o(2));
      return kErrorOk;
    }

    return kNoMatch;
  }

  if (c.n == 4) {
    // VMOV Sm, Sm1, Rt, Rt2 | VMOV Rt, Rt2, Sm, Sm1.
    if (isPlainS(c.o(0)) && isPlainS(c.o(1)) && isGp(c.o(2)) && isGp(c.o(3))) {
      if (regId(c.o(1)) != regId(c.o(0)) + 1u) return DebugUtils::errored(kErrorInvalidPhysId);
      if (!isGpIdValid(c.o(2)) || !isGpIdValid(c.o(3))) return DebugUtils::errored(kErrorInvalidPhysId);
      c.opcode = 0x0C400A10u | (regId(c.o(3)) << 16) | (regId(c.o(2)) << 12) | encVm(c.o(0));
      return kErrorOk;
    }

    if (isGp(c.o(0)) && isGp(c.o(1)) && isPlainS(c.o(2)) && isPlainS(c.o(3))) {
      if (regId(c.o(3)) != regId(c.o(2)) + 1u) return DebugUtils::errored(kErrorInvalidPhysId);
      if (!isGpIdValid(c.o(0)) || !isGpIdValid(c.o(1))) return DebugUtils::errored(kErrorInvalidPhysId);
      c.opcode = 0x0C500A10u | (regId(c.o(1)) << 16) | (regId(c.o(0)) << 12) | encVm(c.o(2));
      return kErrorOk;
    }

    return kNoMatch;
  }

  return kNoMatch;
}

static Error encVLdr(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2) {
    return kNoMatch;
  }

  const Operand_& d = c.o(0);
  uint32_t sz;

  if (isPlainS(d) && dtIsNoneOr(c.dt, DataType::kF32)) {
    sz = 0;
  }
  else if (isPlainD(d) && (c.dt == DataType::kNone || dtSizeLog2(c.dt) == 3)) {
    sz = 1;
  }
  else {
    return kNoMatch;
  }

  uint32_t opcode = row.opcode | (sz << 8) | encVd(d);
  const Operand_& mOp = c.o(1);

  if (isRelTarget(mOp) && !isImm(mOp)) {
    if (isMem(mOp) && !mOp.as<Mem>().isFixedOffset()) {
      return DebugUtils::errored(kErrorInvalidAddress);
    }

    c.opcode = opcode | (15u << 16);
    c.setRel(mOp, OffsetType::kAArch32_U23_SignedOffset, 8, 2);
    return kErrorOk;
  }

  if (!isMem(mOp)) {
    return kNoMatch;
  }

  const Mem& m = mOp.as<Mem>();
  if (!m.hasBaseReg() || m.baseType() != RegType::kGp32 || m.baseId() > 15u || m.hasIndex() || !m.isFixedOffset()) {
    return DebugUtils::errored(kErrorInvalidAddress);
  }

  int32_t off = m.offsetLo32();
  uint32_t absOff = off < 0 ? uint32_t(0) - uint32_t(off) : uint32_t(off);

  if ((absOff & 3u) != 0 || absOff > 1020u) {
    return DebugUtils::errored(kErrorInvalidDisplacement);
  }

  c.opcode = opcode | (uint32_t(off >= 0) << 23) | (m.baseId() << 16) | (absOff >> 2);
  return kErrorOk;
}

//! Decodes a list of consecutive S or D registers (Q registers are converted to D pairs) from operands [i, n).
static Error decodeVfpList(const EncCtx& c, uint32_t i, bool* isDouble, uint32_t* first, uint32_t* count) noexcept {
  if (i >= c.n) {
    return kNoMatch;
  }

  if (i + 1u == c.n && c.o(i).isRegList()) {
    const BaseRegList& list = c.o(i).as<BaseRegList>();
    RegType type = list.regType();
    uint32_t mask = list.list();

    if (type != RegType::kVec32 && type != RegType::kVec64 && type != RegType::kVec128) {
      return kNoMatch;
    }

    if (mask == 0) {
      return DebugUtils::errored(kErrorInvalidRegType);
    }

    uint32_t lo = Support::ctz(mask);
    uint32_t cnt = Support::popcnt(mask);

    // Must be consecutive.
    if ((mask >> lo) != Support::lsbMask<uint32_t>(cnt)) {
      return DebugUtils::errored(kErrorInvalidPhysId);
    }

    if (type == RegType::kVec128) {
      if (lo + cnt > 16u) {
        return DebugUtils::errored(kErrorInvalidPhysId);
      }
      *isDouble = true;
      *first = lo * 2u;
      *count = cnt * 2u;
    }
    else {
      *isDouble = type == RegType::kVec64;
      *first = lo;
      *count = cnt;
    }
    return kErrorOk;
  }

  const Operand_& f = c.o(i);
  if (!isVecAny(f) || !isPlainVec(f)) {
    return kNoMatch;
  }

  RegType type = f.as<Reg>().regType();
  uint32_t mult = type == RegType::kVec128 ? 2u : 1u;
  uint32_t base = regId(f) * mult;
  uint32_t cnt = 0;

  for (uint32_t k = i; k < c.n; k++) {
    const Operand_& r = c.o(k);
    if (!r.isReg(type) || !isPlainVec(r)) {
      return kNoMatch;
    }

    if (regId(r) * mult != base + cnt) {
      return DebugUtils::errored(kErrorInvalidPhysId);
    }
    cnt += mult;
  }

  *isDouble = type != RegType::kVec32;
  *first = base;
  *count = cnt;
  return kErrorOk;
}

static ASMJIT_INLINE uint32_t encVfpListFirst(bool isDouble, uint32_t first) noexcept {
  return isDouble ? (((first & 0xFu) << 12) | ((first >> 4) << 22) | (1u << 8))
                  : (((first >> 1) << 12) | ((first & 1u) << 22));
}

static Error encVLdm(EncCtx& c, const Row& row) noexcept {
  if (c.n < 2) {
    return kNoMatch;
  }

  uint32_t rn;
  bool wb;
  Error err = decodeMultipleBase(c.o(0), &rn, &wb);
  if (err != kErrorOk) {
    return err;
  }

  bool isDB = (row.aux & 1u) != 0;
  if (isDB && !wb) {
    return DebugUtils::errored(kErrorInvalidAddress);
  }

  bool isDouble;
  uint32_t first, count;
  err = decodeVfpList(c, 1, &isDouble, &first, &count);
  if (err != kErrorOk) {
    return err;
  }

  if (first + count > 32u || (isDouble && count > 16u)) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t imm8 = isDouble ? count * 2u : count;
  c.opcode = row.opcode | (uint32_t(wb) << 21) | (rn << 16) | encVfpListFirst(isDouble, first) | imm8;
  return kErrorOk;
}

static Error encVPushPop(EncCtx& c, const Row& row) noexcept {
  bool isDouble;
  uint32_t first, count;

  Error err = decodeVfpList(c, 0, &isDouble, &first, &count);
  if (err != kErrorOk) {
    return err;
  }

  if (first + count > 32u || (isDouble && count > 16u)) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t imm8 = isDouble ? count * 2u : count;
  c.opcode = row.opcode | encVfpListFirst(isDouble, first) | imm8;
  return kErrorOk;
}

static Error encVMrs(EncCtx& c, const Row& row) noexcept {
  if (c.n < 1 || c.n > 2 || !isGp(c.o(0))) {
    return kNoMatch;
  }

  uint32_t reg = uint32_t(FpSysReg::kFPSCR);
  if (c.n == 2) {
    if (!isImm(c.o(1))) {
      return kNoMatch;
    }
    if (!immInRange(c.o(1), 0, 15, &reg)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
  }

  if (!isGpIdValid(c.o(0))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  c.opcode = row.opcode | (reg << 16) | (regId(c.o(0)) << 12);
  return kErrorOk;
}

static Error encVMsr(EncCtx& c, const Row& row) noexcept {
  uint32_t reg = uint32_t(FpSysReg::kFPSCR);
  const Operand_* rt;

  if (c.n == 1 && isGp(c.o(0))) {
    rt = &c.o(0);
  }
  else if (c.n == 2 && isImm(c.o(0)) && isGp(c.o(1))) {
    if (!immInRange(c.o(0), 0, 15, &reg)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }
    rt = &c.o(1);
  }
  else {
    return kNoMatch;
  }

  if (!isGpIdValid(*rt)) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  c.opcode = row.opcode | (reg << 16) | (regId(*rt) << 12);
  return kErrorOk;
}

} // {anonymous}

// a32::Assembler - Encoders (ASIMD)
// =================================

namespace {

//! Applies data type encoding `dtEnc` to an opcode.
static ASMJIT_INLINE uint32_t applyDtEnc(uint32_t dtEnc, DataType dt) noexcept {
  uint32_t sizeLog2 = dtSizeLog2(dt);
  uint32_t u = uint32_t(dtIsUnsigned(dt));

  switch (dtEnc) {
    case kDtEncSize20: return sizeLog2 << 20;
    case kDtEncSize20U24: return (sizeLog2 << 20) | (u << 24);
    case kDtEncSizeN20: return (sizeLog2 - 1u) << 20;
    case kDtEncSize18: return sizeLog2 << 18;
    case kDtEncSizeN18: return (sizeLog2 - 1u) << 18;
    case kDtEncSize18U7: return (sizeLog2 << 18) | (u << 7);
    case kDtEncU24: return u << 24;
    default: return 0;
  }
}

static ASMJIT_INLINE bool matchesDQShape(uint32_t shape, const Operand_& a, const Operand_& b) noexcept {
  if (shape == kShapeD) return isPlainD(a) && isPlainD(b);
  if (shape == kShapeQ) return isPlainQ(a) && isPlainQ(b);
  return (isPlainD(a) && isPlainD(b)) || (isPlainQ(a) && isPlainQ(b));
}

static Error encN3Same(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2 && c.n != 3) {
    return kNoMatch;
  }

  const Operand_* d = &c.o(0);
  const Operand_* n = c.n == 3 ? &c.o(1) : &c.o(0);
  const Operand_* m = &c.o(c.n - 1u);

  uint32_t shape = auxShape(row.aux);
  if (!matchesDQShape(shape, *d, *n) || !matchesDQShape(shape, *d, *m)) {
    return kNoMatch;
  }

  if (row.aux & kAuxSwapNM) {
    std::swap(n, m);
  }

  c.opcode = row.opcode | applyDtEnc(auxDtEnc(row.aux), c.dt) | qBit(*d) | encVd(*d) | encVn(*n) | encVm(*m);
  return kErrorOk;
}

static Error encN3Diff(EncCtx& c, const Row& row) noexcept {
  if (c.n != 3) {
    return kNoMatch;
  }

  const Operand_& d = c.o(0);
  const Operand_& n = c.o(1);
  const Operand_& m = c.o(2);

  switch (auxShape(row.aux)) {
    case kShapeL: if (!isPlainQ(d) || !isPlainD(n) || !isPlainD(m)) return kNoMatch; break;
    case kShapeW: if (!isPlainQ(d) || !isPlainQ(n) || !isPlainD(m)) return kNoMatch; break;
    case kShapeN: if (!isPlainD(d) || !isPlainQ(n) || !isPlainQ(m)) return kNoMatch; break;
    default: return kNoMatch;
  }

  c.opcode = row.opcode | applyDtEnc(auxDtEnc(row.aux), c.dt) | encVd(d) | encVn(n) | encVm(m);
  return kErrorOk;
}

static Error encNScalar(EncCtx& c, const Row& row) noexcept {
  if (c.n != 3 || !isScalarD(c.o(2))) {
    return kNoMatch;
  }

  const Operand_& d = c.o(0);
  const Operand_& n = c.o(1);
  const Operand_& m = c.o(2);

  uint32_t shape = auxShape(row.aux);
  uint32_t q = 0;

  if (shape == kShapeL) {
    if (!isPlainQ(d) || !isPlainD(n)) {
      return kNoMatch;
    }
  }
  else {
    if (!((isPlainD(d) && isPlainD(n)) || (isPlainQ(d) && isPlainQ(n)))) {
      return kNoMatch;
    }
    q = uint32_t(isVecQ(d)) << 24;
  }

  uint32_t esize = dtSizeBits(c.dt);
  uint32_t mId = regId(m);
  uint32_t x = m.as<Vec>().elementIndex();
  uint32_t mBits;

  if (esize == 16) {
    if (mId > 7u || x > 3u) {
      return DebugUtils::errored(mId > 7u ? kErrorInvalidPhysId : kErrorInvalidElementIndex);
    }
    mBits = mId | ((x & 1u) << 3) | ((x >> 1) << 5);
  }
  else if (esize == 32) {
    if (mId > 15u || x > 1u) {
      return DebugUtils::errored(mId > 15u ? kErrorInvalidPhysId : kErrorInvalidElementIndex);
    }
    mBits = mId | (x << 5);
  }
  else {
    return kNoMatch;
  }

  c.opcode = row.opcode | q | applyDtEnc(auxDtEnc(row.aux), c.dt) | encVd(d) | encVn(n) | mBits;
  return kErrorOk;
}

static Error encNShift(EncCtx& c, const Row& row) noexcept {
  uint32_t kind = auxShape(row.aux);
  uint32_t esize = dtSizeBits(c.dt);

  const Operand_& d = c.o(0);
  const Operand_& m = c.o(1);

  if (kind == kShiftLL && c.n == 2) {
    // VMOVL Qd, Dm (VSHLL #0).
    if (!isPlainQ(d) || !isPlainD(m)) {
      return kNoMatch;
    }
    c.opcode = row.opcode | applyDtEnc(auxDtEnc(row.aux), c.dt) | ((esize) << 16) | encVd(d) | encVm(m);
    return kErrorOk;
  }

  if (c.n != 3 || !isImm(c.o(2))) {
    return kNoMatch;
  }

  uint32_t q = 0;
  switch (kind) {
    case kShiftR:
    case kShiftL:
      if (!((isPlainD(d) && isPlainD(m)) || (isPlainQ(d) && isPlainQ(m)))) {
        return kNoMatch;
      }
      q = qBit(d);
      break;

    case kShiftRN:
      if (!isPlainD(d) || !isPlainQ(m)) {
        return kNoMatch;
      }
      esize >>= 1;
      break;

    case kShiftLL:
      if (!isPlainQ(d) || !isPlainD(m)) {
        return kNoMatch;
      }
      break;

    default:
      return kNoMatch;
  }

  uint32_t imm;
  uint32_t imm6;
  uint32_t lBit = 0;

  if (kind == kShiftR || kind == kShiftRN) {
    if (!immInRange(c.o(2), 1, esize, &imm)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }

    if (esize == 64) {
      lBit = 1;
      imm6 = 64u - imm;
    }
    else {
      imm6 = esize * 2u - imm;
    }
  }
  else {
    if (kind == kShiftLL && c.o(2).as<Imm>().value() == int64_t(esize)) {
      // VSHLL with maximum shift uses a different encoding.
      return kNoMatch;
    }

    if (!immInRange(c.o(2), 0, esize - 1u, &imm)) {
      return DebugUtils::errored(kErrorInvalidImmediate);
    }

    if (esize == 64) {
      lBit = 1;
      imm6 = imm;
    }
    else {
      imm6 = esize + imm;
    }
  }

  c.opcode = row.opcode | applyDtEnc(auxDtEnc(row.aux), c.dt) | (imm6 << 16) | (lBit << 7) | q | encVd(d) | encVm(m);
  return kErrorOk;
}

static Error encN2Misc(EncCtx& c, const Row& row) noexcept {
  uint32_t shape = auxShape(row.aux);

  if (c.n < 2) {
    return kNoMatch;
  }

  const Operand_& d = c.o(0);
  const Operand_& m = c.o(1);

  switch (shape) {
    case kShapeDQ:
    case kShapeD:
    case kShapeQ:
      if (c.n != 2 || !matchesDQShape(shape, d, m)) {
        return kNoMatch;
      }
      break;

    case kShapeN:
      if (c.n != 2 || !isPlainD(d) || !isPlainQ(m)) {
        return kNoMatch;
      }
      break;

    case kShapeLImm:
      if (c.n != 3 || !isPlainQ(d) || !isPlainD(m) || !isImm(c.o(2))) {
        return kNoMatch;
      }
      if (c.o(2).as<Imm>().value() != int64_t(dtSizeBits(c.dt))) {
        return DebugUtils::errored(kErrorInvalidImmediate);
      }
      break;

    case kShapeZero:
      if (c.n != 3 || !matchesDQShape(kShapeDQ, d, m) || !isImm(c.o(2))) {
        return kNoMatch;
      }
      if (immAsDouble(c.o(2)) != 0.0) {
        return DebugUtils::errored(kErrorInvalidImmediate);
      }
      break;

    default:
      return kNoMatch;
  }

  // Q bit is only encoded by D/Q forms - Q-only forms (crypto) and narrowing / widening forms have it fixed.
  uint32_t q = (shape == kShapeDQ || shape == kShapeZero) ? qBit(d) : 0u;
  c.opcode = row.opcode | applyDtEnc(auxDtEnc(row.aux), c.dt) | q | encVd(d) | encVm(m);
  return kErrorOk;
}

static Error encNImm(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2) {
    return kNoMatch;
  }
  return encodeNImm(c, row.aux, c.o(0), c.o(1));
}

static Error encNExt(EncCtx& c, const Row& row) noexcept {
  if (c.n != 4 || !isImm(c.o(3))) {
    return kNoMatch;
  }

  const Operand_& d = c.o(0);
  const Operand_& n = c.o(1);
  const Operand_& m = c.o(2);

  if (!matchesDQShape(kShapeDQ, d, n) || !matchesDQShape(kShapeDQ, d, m)) {
    return kNoMatch;
  }

  uint32_t bytes = c.dt == DataType::kNone ? 1u : (1u << dtSizeLog2(c.dt));
  uint32_t maxBytes = isVecQ(d) ? 15u : 7u;
  uint32_t imm;

  if (!immInRange(c.o(3), 0, maxBytes / bytes, &imm)) {
    return DebugUtils::errored(kErrorInvalidImmediate);
  }

  c.opcode = row.opcode | ((imm * bytes) << 8) | qBit(d) | encVd(d) | encVn(n) | encVm(m);
  return kErrorOk;
}

static Error encNTbl(EncCtx& c, const Row& row) noexcept {
  if (c.n < 3 || !isPlainD(c.o(0)) || !isPlainD(c.o(c.n - 1u))) {
    return kNoMatch;
  }

  EncCtx tmp = c;
  tmp.n = c.n - 1u;

  bool isDouble;
  uint32_t first, count;
  Error err = decodeVfpList(tmp, 1, &isDouble, &first, &count);
  if (err != kErrorOk) {
    return err;
  }

  if (!isDouble || count < 1u || count > 4u || first + count > 32u) {
    return DebugUtils::errored(kErrorInvalidRegType);
  }

  uint32_t n = ((first & 0xFu) << 16) | ((first >> 4) << 7);
  c.opcode = row.opcode | ((count - 1u) << 8) | encVd(c.o(0)) | n | encVm(c.o(c.n - 1u));
  return kErrorOk;
}

static Error encNDup(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2 || !isPlainDQ(c.o(0)) || !isScalarD(c.o(1))) {
    return kNoMatch;
  }

  uint32_t sizeLog2 = dtSizeLog2(c.dt);
  uint32_t x = c.o(1).as<Vec>().elementIndex();

  if (x >= (8u >> sizeLog2)) {
    return DebugUtils::errored(kErrorInvalidElementIndex);
  }

  uint32_t imm4 = (x << (sizeLog2 + 1u)) | (1u << sizeLog2);
  c.opcode = row.opcode | (imm4 << 16) | qBit(c.o(0)) | encVd(c.o(0)) | encVm(c.o(1));
  return kErrorOk;
}

static Error encNDupGp(EncCtx& c, const Row& row) noexcept {
  if (c.n != 2 || !isPlainDQ(c.o(0)) || !isGp(c.o(1))) {
    return kNoMatch;
  }

  if (!isGpIdValid(c.o(1))) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  uint32_t sizeLog2 = dtSizeLog2(c.dt);
  uint32_t be = sizeLog2 == 0 ? (1u << 22) : sizeLog2 == 1 ? (1u << 5) : 0u;
  uint32_t q = uint32_t(isVecQ(c.o(0))) << 21;

  c.opcode = row.opcode | be | q | encVn(c.o(0)) | (regId(c.o(1)) << 12);
  return kErrorOk;
}

//! Register list used by VLDn/VSTn.
struct NList {
  uint32_t first;     // First D register.
  uint32_t count;     // Number of D registers.
  uint32_t spacing;   // 1 or 2.
  uint32_t lane;      // 0 = multiple structures, 1 = single lane, 2 = all lanes.
  uint32_t index;     // Lane index (single lane only).
};

static Error decodeNList(const EncCtx& c, uint32_t end, NList* out) noexcept {
  uint32_t regs[8];
  uint32_t count = 0;
  uint32_t lane = 0xFFFFFFFFu;
  uint32_t index = 0;

  auto addReg = [&](uint32_t id, uint32_t l, uint32_t x) noexcept -> bool {
    if (count >= 8u) return false;
    if (lane == 0xFFFFFFFFu) {
      lane = l;
      index = x;
    }
    else if (lane != l || index != x) {
      return false;
    }
    regs[count++] = id;
    return true;
  };

  if (end == 1u && c.o(0).isRegList()) {
    const BaseRegList& list = c.o(0).as<BaseRegList>();
    RegType type = list.regType();
    uint32_t mask = list.list();

    if (type != RegType::kVec64 && type != RegType::kVec128) {
      return kNoMatch;
    }

    Support::BitWordIterator<uint32_t> it(mask);
    while (it.hasNext()) {
      uint32_t id = it.next();
      if (type == RegType::kVec128) {
        if (!addReg(id * 2u, 0, 0) || !addReg(id * 2u + 1u, 0, 0)) return DebugUtils::errored(kErrorInvalidRegType);
      }
      else {
        if (!addReg(id, 0, 0)) return DebugUtils::errored(kErrorInvalidRegType);
      }
    }
  }
  else {
    for (uint32_t i = 0; i < end; i++) {
      const Operand_& r = c.o(i);
      if (isVecQ(r) && isPlainVec(r)) {
        if (!addReg(regId(r) * 2u, 0, 0) || !addReg(regId(r) * 2u + 1u, 0, 0)) return DebugUtils::errored(kErrorInvalidRegType);
      }
      else if (isVecD(r)) {
        const Vec& v = r.as<Vec>();
        uint32_t l = v.isAllLanes() ? 2u : v.hasElementIndex() ? 1u : 0u;
        if (!addReg(regId(r), l, l == 1u ? v.elementIndex() : 0u)) return DebugUtils::errored(kErrorInvalidRegType);
      }
      else {
        return kNoMatch;
      }
    }
  }

  if (count == 0 || count > 4u) {
    return DebugUtils::errored(kErrorInvalidRegType);
  }

  uint32_t spacing = 1;
  if (count > 1u) {
    spacing = regs[1] - regs[0];
    if (spacing != 1u && spacing != 2u) {
      return DebugUtils::errored(kErrorInvalidPhysId);
    }
    for (uint32_t i = 2; i < count; i++) {
      if (regs[i] - regs[i - 1u] != spacing) {
        return DebugUtils::errored(kErrorInvalidPhysId);
      }
    }
  }

  if (regs[0] + (count - 1u) * spacing > 31u) {
    return DebugUtils::errored(kErrorInvalidPhysId);
  }

  out->first = regs[0];
  out->count = count;
  out->spacing = spacing;
  out->lane = lane;
  out->index = index;
  return kErrorOk;
}

static Error encNLdSt(EncCtx& c, const Row& row) noexcept {
  if (c.n < 2 || !isMem(c.o(c.n - 1u))) {
    return kNoMatch;
  }

  uint32_t nStruct = row.aux;
  bool isLoad = (row.opcode & (1u << 21)) != 0;

  NList list;
  Error err = decodeNList(c, c.n - 1u, &list);
  if (err != kErrorOk) {
    return err;
  }

  const Mem& m = c.o(c.n - 1u).as<Mem>();
  if (!m.hasBaseReg() || m.baseType() != RegType::kGp32 || m.baseId() > 15u || m.offsetLo32() != 0 || m.hasShift() || m.isNegIndex()) {
    return DebugUtils::errored(kErrorInvalidAddress);
  }

  uint32_t rm;
  if (m.hasIndex()) {
    if (!m.isPostIndex() || m.indexType() != RegType::kGp32 || m.indexId() > 12u) {
      return DebugUtils::errored(kErrorInvalidAddress);
    }
    rm = m.indexId();
  }
  else {
    rm = m.isPreOrPost() ? 13u : 15u;
  }

  uint32_t align = memAlignFieldToBits(m.alignmentField());
  uint32_t sizeLog2 = dtSizeLog2(c.dt);
  uint32_t esize = 8u << sizeLog2;
  uint32_t d = ((list.first & 0xFu) << 12) | ((list.first >> 4) << 22);
  uint32_t base = (m.baseId() << 16) | d | rm;

  if (list.lane == 0) {
    // Multiple structures.
    uint32_t type;
    switch (nStruct) {
      case 1:
        if (list.spacing != 1u && list.count > 1u) return DebugUtils::errored(kErrorInvalidPhysId);
        type = list.count == 1u ? 0x7u : list.count == 2u ? 0xAu : list.count == 3u ? 0x6u : 0x2u;
        break;
      case 2:
        if (list.count == 2u) type = list.spacing == 1u ? 0x8u : 0x9u;
        else if (list.count == 4u && list.spacing == 1u) type = 0x3u;
        else return DebugUtils::errored(kErrorInvalidRegType);
        break;
      case 3:
        if (list.count != 3u) return DebugUtils::errored(kErrorInvalidRegType);
        type = list.spacing == 1u ? 0x4u : 0x5u;
        break;
      default:
        if (list.count != 4u) return DebugUtils::errored(kErrorInvalidRegType);
        type = list.spacing == 1u ? 0x0u : 0x1u;
        break;
    }

    if (nStruct > 1u && sizeLog2 == 3u) {
      return kNoMatch;
    }

    uint32_t alignBits;
    switch (align) {
      case 0: alignBits = 0; break;
      case 64: alignBits = 1; break;
      case 128: alignBits = 2; break;
      case 256: alignBits = 3; break;
      default: return DebugUtils::errored(kErrorInvalidAddress);
    }

    c.opcode = row.opcode | base | (type << 8) | (sizeLog2 << 6) | (alignBits << 4);
    return kErrorOk;
  }

  if (sizeLog2 > 2u) {
    return DebugUtils::errored(kErrorInvalidRegType);
  }

  // VLD1 to all lanes can use one or two registers, all other lane variants must match the number of structures.
  if (list.count != nStruct && !(nStruct == 1u && list.lane == 2u && list.count == 2u)) {
    return DebugUtils::errored(kErrorInvalidRegType);
  }

  uint32_t spacingBit = uint32_t(list.spacing == 2u);

  if (list.lane == 1) {
    // Single structure to/from one lane.
    if (list.index >= (8u >> sizeLog2)) {
      return DebugUtils::errored(kErrorInvalidElementIndex);
    }

    uint32_t alignAmount = align == 0 ? 0u : esize * nStruct;
    if (nStruct == 3u && align != 0) return DebugUtils::errored(kErrorInvalidAddress);
    if (nStruct == 4u && sizeLog2 == 2u && align == 128u) alignAmount = 128u;

    uint32_t ia;
    if (sizeLog2 == 0) {
      if (nStruct > 1u && list.spacing != 1u) return DebugUtils::errored(kErrorInvalidPhysId);
      ia = list.index << 1;
      if (align != 0) {
        if (nStruct == 1u || align != alignAmount) return DebugUtils::errored(kErrorInvalidAddress);
        ia |= 1u;
      }
    }
    else if (sizeLog2 == 1) {
      ia = (list.index << 2) | (spacingBit << 1);
      if (align != 0) {
        if (align != alignAmount) return DebugUtils::errored(kErrorInvalidAddress);
        ia |= 1u;
      }
    }
    else {
      ia = (list.index << 3) | (spacingBit << 2);
      if (align != 0) {
        if (nStruct == 1u) {
          if (align != 32u) return DebugUtils::errored(kErrorInvalidAddress);
          ia |= 3u;
        }
        else if (nStruct == 2u) {
          if (align != 64u) return DebugUtils::errored(kErrorInvalidAddress);
          ia |= 1u;
        }
        else {
          if (align == 64u) ia |= 1u;
          else if (align == 128u) ia |= 2u;
          else return DebugUtils::errored(kErrorInvalidAddress);
        }
      }
    }

    c.opcode = (row.opcode | 0x00800000u) | base | (sizeLog2 << 10) | ((nStruct - 1u) << 8) | (ia << 4);
    return kErrorOk;
  }

  // Single structure to all lanes (load only).
  if (!isLoad) {
    return DebugUtils::errored(kErrorInvalidInstruction);
  }

  uint32_t t;
  uint32_t a = 0;
  uint32_t size = sizeLog2;

  if (nStruct == 1u) {
    if (list.count > 2u || list.spacing != 1u) {
      return DebugUtils::errored(kErrorInvalidRegType);
    }
    t = list.count - 1u;
    if (align != 0) {
      if (sizeLog2 == 0 || align != esize) return DebugUtils::errored(kErrorInvalidAddress);
      a = 1;
    }
  }
  else {
    t = spacingBit;
    if (align != 0) {
      if (nStruct == 2u) {
        if (align != esize * 2u) return DebugUtils::errored(kErrorInvalidAddress);
        a = 1;
      }
      else if (nStruct == 4u) {
        if (sizeLog2 == 2u && align == 128u) {
          size = 3;
        }
        else if (align != (sizeLog2 == 2u ? 64u : esize * 4u)) {
          return DebugUtils::errored(kErrorInvalidAddress);
        }
        a = 1;
      }
      else {
        return DebugUtils::errored(kErrorInvalidAddress);
      }
    }
  }

  c.opcode = (row.opcode | 0x00800000u) | 0x00000C00u | base | ((nStruct - 1u) << 8) | (size << 6) | (t << 5) | (a << 4);
  return kErrorOk;
}

// Encoder Dispatch
// ----------------

static Error encodeRow(EncCtx& c, const Row& row) noexcept {
  switch (row.form) {
    case kF_DP: return encDP(c, row);
    case kF_Mov: return encMov(c, row);
    case kF_Cmp: return encCmp(c, row);
    case kF_Shift: return encShift(c, row);
    case kF_Rrx: return encRrx(c, row);
    case kF_MovW: return encMovW(c, row);
    case kF_Regs: return encRegs(c, row);
    case kF_Ext: return encExt(c, row);
    case kF_Sat: return encSat(c, row);
    case kF_Sat16: return encSat16(c, row);
    case kF_Pkh: return encPkh(c, row);
    case kF_Bfc: return encBfc(c, row);
    case kF_Bfi: return encBfi(c, row);
    case kF_Bfx: return encBfx(c, row);
    case kF_B: return encB(c, row);
    case kF_Blx: return encBlx(c, row);
    case kF_Adr: return encAdr(c, row);
    case kF_LdSt: return encLdSt(c, row);
    case kF_LdStX: return encLdStX(c, row);
    case kF_Ldm: return encLdm(c, row);
    case kF_PushPop: return encPushPop(c, row);
    case kF_MemRegs: return encMemRegs(c, row);
    case kF_None: return encNone(c, row);
    case kF_Barrier: return encBarrier(c, row);
    case kF_Imm: return encImm(c, row);
    case kF_Pld: return encPld(c, row);
    case kF_Mrs: return encMrs(c, row);
    case kF_Msr: return encMsr(c, row);
    case kF_Cps: return encCps(c, row);
    case kF_Setend: return encSetend(c, row);
    case kF_Mcr: return encMcr(c, row);
    case kF_Mcrr: return encMcrr(c, row);

    case kF_V3: return encV3(c, row);
    case kF_V2: return encV2(c, row);
    case kF_VCmp0: return encVCmp0(c, row);
    case kF_VCvt: return encVCvt(c, row);
    case kF_VCvtRm: return encVCvtRm(c, row);
    case kF_VCvtBT: return encVCvtBT(c, row);
    case kF_VMov: return encVMov(c, row);
    case kF_VLdr: return encVLdr(c, row);
    case kF_VLdm: return encVLdm(c, row);
    case kF_VPushPop: return encVPushPop(c, row);
    case kF_VMrs: return encVMrs(c, row);
    case kF_VMsr: return encVMsr(c, row);

    case kF_N3Same: return encN3Same(c, row);
    case kF_N3Diff: return encN3Diff(c, row);
    case kF_NScalar: return encNScalar(c, row);
    case kF_NShift: return encNShift(c, row);
    case kF_N2Misc: return encN2Misc(c, row);
    case kF_NImm: return encNImm(c, row);
    case kF_NExt: return encNExt(c, row);
    case kF_NTbl: return encNTbl(c, row);
    case kF_NDup: return encNDup(c, row);
    case kF_NDupGp: return encNDupGp(c, row);
    case kF_NLdSt: return encNLdSt(c, row);

    default:
      return kNoMatch;
  }
}

static ASMJIT_INLINE bool rowAcceptsDataType(const Row& row, DataType dt, DataType dt2) noexcept {
  uint32_t mask = row.dtMask;
  if (mask == kDtAny) {
    return true;
  }

  if (mask == 0) {
    mask = kDtNone | kDt2None;
  }

  return ((mask >> uint32_t(dt)) & 1u) != 0 && ((mask >> (uint32_t(dt2) + 16u)) & 1u) != 0;
}

//! Validates that all register operands have valid ids (and are not virtual registers).
static ASMJIT_INLINE bool validateRegIds(const EncCtx& c) noexcept {
  for (uint32_t i = 0; i < c.n; i++) {
    const Operand_& op = c.o(i);
    if (op.isReg()) {
      if (isGp(op)) {
        if (!isGpIdValid(op)) return false;
      }
      else if (isVecAny(op)) {
        if (!isVecIdValid(op)) return false;
      }
      else {
        return false;
      }
    }
  }
  return true;
}

} // {anonymous}

// a32::Assembler - Emitter Functions
// ==================================

static Error ASMJIT_CDECL Emitter_emitProlog(BaseEmitter* emitter, const FuncFrame& frame) {
  DebugUtils::unused(emitter, frame);
  return DebugUtils::errored(kErrorFeatureNotEnabled);
}

static Error ASMJIT_CDECL Emitter_emitEpilog(BaseEmitter* emitter, const FuncFrame& frame) {
  DebugUtils::unused(emitter, frame);
  return DebugUtils::errored(kErrorFeatureNotEnabled);
}

static Error ASMJIT_CDECL Emitter_emitArgsAssignment(BaseEmitter* emitter, const FuncFrame& frame, const FuncArgsAssignment& args) {
  DebugUtils::unused(emitter, frame, args);
  return DebugUtils::errored(kErrorFeatureNotEnabled);
}

static void initEmitterFuncs(BaseEmitter* emitter) noexcept {
  emitter->_funcs.emitProlog = Emitter_emitProlog;
  emitter->_funcs.emitEpilog = Emitter_emitEpilog;
  emitter->_funcs.emitArgsAssignment = Emitter_emitArgsAssignment;

#ifndef ASMJIT_NO_LOGGING
  emitter->_funcs.formatInstruction = InstInternal::formatInstruction;
#endif

#ifndef ASMJIT_NO_VALIDATION
  emitter->_funcs.validate = InstInternal::validate;
#endif
}

// a32::Assembler - Construction & Destruction
// ===========================================

Assembler::Assembler(CodeHolder* code) noexcept : BaseAssembler() {
  _archMask = uint64_t(1) << uint32_t(Arch::kARM);
  initEmitterFuncs(this);

  if (code) {
    code->attach(this);
  }
}

Assembler::~Assembler() noexcept {}

// a32::Assembler - Emit
// =====================

Error Assembler::_emit(InstId instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* opExt) {
  constexpr InstOptions kRequiresSpecialHandling = InstOptions::kReserved;

  Error err;
  CodeWriter writer(this);

  InstOptions options = InstOptions((instId & uint32_t(InstIdParts::kRealId)) - 1u >= Inst::_kIdCount - 1u) |
                        InstOptions((size_t)(_bufferEnd - writer.cursor()) < 4) |
                        instOptions() |
                        forcedInstOptions();

  CondCode instCC = BaseInst::extractARMCondCode(instId);
  DataType dt = DataType((instId & uint32_t(InstIdParts::kA32_DT)) >> Support::ConstCTZ<uint32_t(InstIdParts::kA32_DT)>::value);
  DataType dt2 = DataType((instId & uint32_t(InstIdParts::kA32_DT2)) >> Support::ConstCTZ<uint32_t(InstIdParts::kA32_DT2)>::value);
  InstId fullInstId = instId;
  DebugUtils::unused(fullInstId);

  instId = instId & uint32_t(InstIdParts::kRealId);
  if (instId >= Inst::_kIdCount) {
    instId = 0;
  }

  EncCtx c;
  c.op[0] = &o0;
  c.op[1] = &o1;
  c.op[2] = &o2;
  c.op[3] = &opExt[EmitterUtils::kOp3];
  c.op[4] = &opExt[EmitterUtils::kOp4];
  c.op[5] = &opExt[EmitterUtils::kOp5];
  c.dt = dt;
  c.dt2 = dt2;
  c.opcode = 0;
  c.rel = nullptr;
  c.uncond = false;

  uint32_t n = Globals::kMaxOpCount;
  while (n > 0 && c.op[n - 1u]->isNone()) {
    n--;
  }
  c.n = n;

  const Row* matched = nullptr;
  uint64_t offsetValue = 0;

  if (ASMJIT_UNLIKELY(Support::test(options, kRequiresSpecialHandling))) {
    if (ASMJIT_UNLIKELY(!_code)) {
      return reportError(DebugUtils::errored(kErrorNotInitialized));
    }

    // Unknown instruction.
    if (ASMJIT_UNLIKELY(instId == 0)) {
      goto InvalidInstruction;
    }

    // Grow request, happens rarely.
    err = writer.ensureSpace(this, 4);
    if (ASMJIT_UNLIKELY(err)) {
      goto Failed;
    }

#ifndef ASMJIT_NO_VALIDATION
    // Strict validation.
    if (hasDiagnosticOption(DiagnosticOptions::kValidateAssembler)) {
      Operand_ opArray[Globals::kMaxOpCount];
      EmitterUtils::opArrayFromEmitArgs(opArray, o0, o1, o2, opExt);

      err = _funcs.validate(BaseInst(fullInstId, options, _extraReg), opArray, Globals::kMaxOpCount, ValidationFlags::kNone);
      if (ASMJIT_UNLIKELY(err)) {
        goto Failed;
      }
    }
#endif
  }

  if (ASMJIT_UNLIKELY(instCC == CondCode::kNA)) {
    goto InvalidInstruction;
  }

  if (ASMJIT_UNLIKELY(!validateRegIds(c))) {
    err = DebugUtils::errored(kErrorInvalidPhysId);
    goto Failed;
  }

  // Find the first encoding row that matches the given operands.
  {
    uint32_t rowStart = rowIndex.start[instId];
    uint32_t rowCount = rowIndex.count[instId];
    Error firstErr = kErrorOk;

    for (uint32_t i = 0; i < rowCount; i++) {
      const Row& row = rowTable[rowStart + i];
      if (!rowAcceptsDataType(row, dt, dt2)) {
        continue;
      }

      c.opcode = 0;
      c.rel = nullptr;
      c.uncond = false;

      Error e = encodeRow(c, row);
      if (e == kErrorOk) {
        matched = &row;
        break;
      }

      if (e != kNoMatch && firstErr == kErrorOk) {
        firstErr = e;
      }
    }

    if (!matched) {
      if (firstErr != kErrorOk) {
        err = firstErr;
        goto Failed;
      }
      goto InvalidInstruction;
    }
  }

  // Condition code.
  if ((matched->flags & kRowCond) && !c.uncond) {
    c.opcode |= condCodeToOpcodeCond(instCC) << 28;
  }
  else if (instCC != CondCode::kAL) {
    goto InvalidInstruction;
  }

  if (c.rel) {
    goto EmitOp_Rel;
  }

  goto EmitOp;

  // --------------------------------------------------------------------------
  // [EmitOp - PC Relative]
  // --------------------------------------------------------------------------

EmitOp_Rel:
  {
    // AArch32 PC reads as the address of the current instruction + 8.
    constexpr int64_t kPcBias = 8;
    const Operand_* rmRel = c.rel;

    if (rmRel->isLabel() || (rmRel->isMem() && rmRel->as<Mem>().hasBaseLabel())) {
      uint32_t labelId;
      int64_t labelOffset = 0;

      if (rmRel->isLabel()) {
        labelId = rmRel->as<Label>().id();
      }
      else {
        labelId = rmRel->as<Mem>().baseId();
        labelOffset = rmRel->as<Mem>().offset();
      }

      if (ASMJIT_UNLIKELY(!_code->isLabelValid(labelId))) {
        goto InvalidLabel;
      }

      LabelEntry& le = _code->labelEntry(labelId);

      if (le.isBoundTo(_section)) {
        // Label bound to the current section.
        offsetValue = le.offset() - uint64_t(offset()) + uint64_t(labelOffset - kPcBias);
        goto EmitOp_DispImm;
      }
      else {
        // Create a fixup referencing a non-bound label.
        size_t codeOffset = writer.offsetFrom(_bufferData);
        Fixup* fixup = _code->newFixup(le, _section->sectionId(), codeOffset, intptr_t(labelOffset - kPcBias), c.relFmt);

        if (ASMJIT_UNLIKELY(!fixup)) {
          goto OutOfMemory;
        }

        goto EmitOp;
      }
    }

    uint64_t targetOffset;
    if (rmRel->isImm()) {
      targetOffset = rmRel->as<Imm>().valueAs<uint64_t>();
    }
    else if (rmRel->isMem()) {
      targetOffset = uint64_t(rmRel->as<Mem>().offset());
    }
    else {
      goto InvalidInstruction;
    }

    uint64_t baseAddress = _code->baseAddress();
    size_t codeOffset = writer.offsetFrom(_bufferData);

    if (baseAddress == Globals::kNoBaseAddress || _section->sectionId() != 0) {
      // Create a new RelocEntry as we cannot calculate the offset right now.
      RelocEntry* re;
      err = _code->newRelocEntry(&re, RelocType::kAbsToRel);
      if (err) {
        goto Failed;
      }

      re->_sourceSectionId = _section->sectionId();
      re->_sourceOffset = codeOffset;
      re->_format = c.relFmt;
      // AbsToRel computes `payload - (address + regionSize)`, the region is 4 bytes and PC bias is 8.
      re->_payload = targetOffset + 4u - uint64_t(kPcBias);
      goto EmitOp;
    }
    else {
      uint64_t pc = baseAddress + codeOffset;
      offsetValue = targetOffset - pc - uint64_t(kPcBias);
      goto EmitOp_DispImm;
    }
  }

EmitOp_DispImm:
  {
    int64_t disp = int64_t(offsetValue);

    // AArch32 is a 32-bit architecture - wrap around the address space if the base address is known.
    if (disp > int64_t(0x7FFFFFFF) || disp < -int64_t(0x80000000)) {
      disp = int64_t(int32_t(uint32_t(uint64_t(disp))));
    }

    uint32_t mask;
    if (!CodeWriterUtils::encodeOffset32(&mask, disp, c.relFmt)) {
      goto InvalidDisplacement;
    }

    c.opcode |= mask;
    goto EmitOp;
  }

  // --------------------------------------------------------------------------
  // [EmitOp - Opcode]
  // --------------------------------------------------------------------------

EmitOp:
  writer.emit32uLE(c.opcode);
  goto EmitDone;

  // --------------------------------------------------------------------------
  // [Done]
  // --------------------------------------------------------------------------

EmitDone:
  if (Support::test(options, InstOptions::kReserved)) {
#ifndef ASMJIT_NO_LOGGING
    if (_logger) {
      EmitterUtils::logInstructionEmitted(this, fullInstId, options, o0, o1, o2, opExt, 0, 0, writer.cursor());
    }
#endif
  }

  resetState();

  writer.done(this);
  return kErrorOk;

  // --------------------------------------------------------------------------
  // [Error Handler]
  // --------------------------------------------------------------------------

#define ERROR_HANDLER(ERR) ERR: err = DebugUtils::errored(kError##ERR); goto Failed;
  ERROR_HANDLER(OutOfMemory)
  ERROR_HANDLER(InvalidDisplacement)
  ERROR_HANDLER(InvalidLabel)
  ERROR_HANDLER(InvalidInstruction)
#undef ERROR_HANDLER

Failed:
#ifndef ASMJIT_NO_LOGGING
  return EmitterUtils::logInstructionFailed(this, err, fullInstId, options, o0, o1, o2, opExt);
#else
  resetState();
  return reportError(err);
#endif
}

// a32::Assembler - Align
// ======================

Error Assembler::align(AlignMode alignMode, uint32_t alignment) {
  constexpr uint32_t kNopA32 = 0xE320F000u;

  if (ASMJIT_UNLIKELY(!_code)) {
    return reportError(DebugUtils::errored(kErrorNotInitialized));
  }

  if (ASMJIT_UNLIKELY(uint32_t(alignMode) > uint32_t(AlignMode::kMaxValue))) {
    return reportError(DebugUtils::errored(kErrorInvalidArgument));
  }

  if (alignment <= 1) {
    return kErrorOk;
  }

  if (ASMJIT_UNLIKELY(!Support::isPowerOf2UpTo(alignment, Globals::kMaxAlignment))) {
    return reportError(DebugUtils::errored(kErrorInvalidArgument));
  }

  uint32_t i = uint32_t(Support::alignUpDiff<size_t>(offset(), alignment));
  if (i == 0) {
    return kErrorOk;
  }

  CodeWriter writer(this);
  ASMJIT_PROPAGATE(writer.ensureSpace(this, i));

  switch (alignMode) {
    case AlignMode::kCode: {
      if (ASMJIT_UNLIKELY(offset() & 0x3u)) {
        return DebugUtils::errored(kErrorInvalidState);
      }

      while (i >= 4) {
        writer.emit32uLE(kNopA32);
        i -= 4;
      }

      ASMJIT_ASSERT(i == 0);
      break;
    }

    case AlignMode::kData:
    case AlignMode::kZero:
      writer.emitZeros(i);
      break;
  }

  writer.done(this);

#ifndef ASMJIT_NO_LOGGING
  if (_logger) {
    StringTmp<128> sb;
    sb.appendChars(' ', _logger->indentation(FormatIndentationGroup::kCode));
    sb.appendFormat("align %u\n", alignment);
    _logger->log(sb);
  }
#endif

  return kErrorOk;
}

// a32::Assembler - Events
// =======================

Error Assembler::onAttach(CodeHolder& code) noexcept {
  ASMJIT_PROPAGATE(Base::onAttach(code));
  _instructionAlignment = uint8_t(4);
  return kErrorOk;
}

Error Assembler::onDetach(CodeHolder& code) noexcept {
  return Base::onDetach(code);
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH32
