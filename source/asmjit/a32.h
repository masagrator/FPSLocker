// This file is based on part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_A32_H_INCLUDED
#define ASMJIT_A32_H_INCLUDED

//! \addtogroup asmjit_a32
//!
//! ### Emitters
//!
//!   - \ref a32::Assembler - AArch32 assembler (A32 instruction set, Thumb is not supported).
//!   - \ref a32::Emitter - AArch32 emitter (abstract).
//!
//! ### Supported Instructions
//!
//!   - \ref a32::EmitterExplicitT - Provides all instructions that use explicit operands:
//!     - Base A32 instruction set (ARMv8-A AArch32 state) including CRC32 extension.
//!     - VFP (FP-ARMv8) - VFPv4 + ARMv8 additions (VRINT*, VCVT{A,N,P,M}, VSEL*, VMAXNM, VMINNM).
//!     - Advanced SIMD (NEON) including ARMv8 additions.
//!     - Crypto extension (AES, SHA1, SHA256, VMULL.P64).
//!   - \ref a32::Inst::Id - instruction identifiers.
//!
//! ### Register Operands
//!
//!   - \ref a32::Gp - 32-bit general purpose register (r0-r15).
//!   - \ref a32::Vec - Vector (VFP / SIMD) register (s0-s31, d0-d31, q0-q15).
//!   - \ref a32::GpList, \ref a32::VecList - register lists.
//!
//! ### Memory Operands
//!
//!   - \ref a32::Mem - AArch32 memory operand.

#include "./arm.h"

#include "asmjit-scope-begin.h"
#include "arm/a32assembler.h"
#include "arm/a32emitter.h"
#include "arm/a32globals.h"
#include "arm/a32operand.h"
#include "asmjit-scope-end.h"

#endif // ASMJIT_A32_H_INCLUDED
