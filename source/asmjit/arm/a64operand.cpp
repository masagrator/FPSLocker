// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_AARCH64)

#include "../core/misc_p.h"
#include "../arm/a64operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

// a64::Operand - Tests
// ====================

#if defined(ASMJIT_TEST)
UNIT(a64_operand) {
  INFO("Checking if a64::reg(...) matches built-in IDs");
  EXPECT_EQ(w(5), w5);
  EXPECT_EQ(x(5), x5);

  INFO("Checking Gp register properties");
  EXPECT_TRUE(Gp().isReg());
  EXPECT_TRUE(w0.isReg());
  EXPECT_TRUE(x0.isReg());
  EXPECT_EQ(w0.id(), 0u);
  EXPECT_EQ(x0.id(), 0u);
  EXPECT_EQ(wzr.id(), Gp::kIdZr);
  EXPECT_EQ(xzr.id(), Gp::kIdZr);
  EXPECT_EQ(wsp.id(), Gp::kIdSp);
  EXPECT_EQ(sp.id(), Gp::kIdSp);
  EXPECT_EQ(w0.size(), 4u);
  EXPECT_EQ(x0.size(), 8u);
  EXPECT_EQ(w0.regType(), RegType::kGp32);
  EXPECT_EQ(x0.regType(), RegType::kGp64);
  EXPECT_EQ(w0.regGroup(), RegGroup::kGp);
  EXPECT_EQ(x0.regGroup(), RegGroup::kGp);

  INFO("Checking Vec register properties");
  EXPECT_EQ(v0.regType(), RegType::kVec128);
  EXPECT_EQ(d0.regType(), RegType::kVec64);
  EXPECT_EQ(s0.regType(), RegType::kVec32);
  EXPECT_EQ(h0.regType(), RegType::kVec16);
  EXPECT_EQ(b0.regType(), RegType::kVec8);

  EXPECT_EQ(v0.regGroup(), RegGroup::kVec);
  EXPECT_EQ(d0.regGroup(), RegGroup::kVec);
  EXPECT_EQ(s0.regGroup(), RegGroup::kVec);
  EXPECT_EQ(h0.regGroup(), RegGroup::kVec);
  EXPECT_EQ(b0.regGroup(), RegGroup::kVec);

  INFO("Checking Vec register element[] access");
  Vec vd_1 = v15.d(1);
  EXPECT_EQ(vd_1.regType(), RegType::kVec128);
  EXPECT_EQ(vd_1.regGroup(), RegGroup::kVec);
  EXPECT_EQ(vd_1.id(), 15u);
  EXPECT_TRUE(vd_1.isVecD2());
  EXPECT_EQ(vd_1.elementType(), VecElementType::kD);
  EXPECT_TRUE(vd_1.hasElementIndex());
  EXPECT_EQ(vd_1.elementIndex(), 1u);

  Vec vs_3 = v15.s(3);
  EXPECT_EQ(vs_3.regType(), RegType::kVec128);
  EXPECT_EQ(vs_3.regGroup(), RegGroup::kVec);
  EXPECT_EQ(vs_3.id(), 15u);
  EXPECT_TRUE(vs_3.isVecS4());
  EXPECT_EQ(vs_3.elementType(), VecElementType::kS);
  EXPECT_TRUE(vs_3.hasElementIndex());
  EXPECT_EQ(vs_3.elementIndex(), 3u);

  Vec vb_4 = v15.b4(3);
  EXPECT_EQ(vb_4.regType(), RegType::kVec128);
  EXPECT_EQ(vb_4.regGroup(), RegGroup::kVec);
  EXPECT_EQ(vb_4.id(), 15u);
  EXPECT_TRUE(vb_4.isVecB4x4());
  EXPECT_EQ(vb_4.elementType(), VecElementType::kB4);
  EXPECT_TRUE(vb_4.hasElementIndex());
  EXPECT_EQ(vb_4.elementIndex(), 3u);
}
#endif

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH64
