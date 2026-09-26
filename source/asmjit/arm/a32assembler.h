// This file is based on part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A32ASSEMBLER_H_INCLUDED
#define ASMJIT_ARM_A32ASSEMBLER_H_INCLUDED

#include "../core/assembler.h"
#include "../arm/a32emitter.h"
#include "../arm/a32operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

//! \addtogroup asmjit_a32
//! \{

//! AArch32 assembler implementation (A32 instruction set, no Thumb).
//!
//! Example:
//!
//! ```
//! using namespace asmjit;
//!
//! Environment env(Arch::kARM);
//! CodeHolder code;
//! code.init(env, 0x10000);             // Base address (used to resolve absolute branch targets).
//!
//! a32::Assembler a(&code);
//! Label L_Loop = a.newLabel();
//!
//! a.mov(a32::r0, 0);
//! a.bind(L_Loop);
//! a.add(a32::r0, a32::r0, 1);
//! a.cmp(a32::r0, 10);
//! a.b(CondCode::kNE, L_Loop);           // or a.b_ne(L_Loop)
//! a.vadd(a32::s0, a32::s1, a32::s2);    // vadd.f32 s0, s1, s2
//! a.bx(a32::lr);
//! ```
class ASMJIT_VIRTAPI Assembler
  : public BaseAssembler,
    public EmitterExplicitT<Assembler> {

public:
  using Base = BaseAssembler;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API Assembler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API ~Assembler() noexcept override;

  //! \}

  //! \name Emit
  //! \{

  ASMJIT_API Error _emit(InstId instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* opExt) override;

  //! \}

  //! \name Align
  //! \{

  ASMJIT_API Error align(AlignMode alignMode, uint32_t alignment) override;

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error onAttach(CodeHolder& code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder& code) noexcept override;

  //! \}
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A32ASSEMBLER_H_INCLUDED
