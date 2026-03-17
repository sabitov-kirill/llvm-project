//===- ScopFeaturesFootprintTest.cpp - Unit tests for evalUnionPwQpolynomialIR ===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Tests for evalUnionPwQpolynomialIR (barvinok path), the primitive used by
// footprintBytesIR to evaluate the cardinality of a union of accessed sets.
//
//===----------------------------------------------------------------------===//

#ifdef POLLY_HAVE_BARVINOK

#include "polly/Profiling/IslPolyEval.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

#include "isl/ctx.h"
#include "isl/id.h"
#include "isl/union_set.h"

#include <barvinok/isl.h>

#include "gtest/gtest.h"

using namespace llvm;
using namespace polly;

// ---------------------------------------------------------------------------
// Helper: minimal LLVM IR environment for IR-generating tests.
// ---------------------------------------------------------------------------
namespace {

struct IRTestEnv {
  LLVMContext Ctx;
  std::unique_ptr<Module> M;
  Function *F = nullptr;
  BasicBlock *BB = nullptr;
  Instruction *Ret = nullptr;
  IRBuilder<> Builder;

  IRTestEnv()
      : M(std::make_unique<Module>("test", Ctx)), Builder(Ctx) {
    FunctionType *FTy = FunctionType::get(Type::getVoidTy(Ctx), false);
    F = Function::Create(FTy, Function::ExternalLinkage, "test_fn", *M);
    BB = BasicBlock::Create(Ctx, "entry", F);
    Builder.SetInsertPoint(BB);
    Ret = Builder.CreateRetVoid();
    Builder.SetInsertPoint(Ret);
  }
};

/// Build isl_union_pw_qpolynomial* = card(union_set_from_string).
/// Caller owns the returned pointer.
static isl_union_pw_qpolynomial *unionCardOf(isl_ctx *Ctx,
                                              const char *SetStr) {
  isl_union_set *US = isl_union_set_read_from_str(Ctx, SetStr);
  return isl_union_set_card(US); // barvinok; takes ownership of US
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// evalUnionPwQpolynomialIR tests
// ---------------------------------------------------------------------------

TEST(ScopFeaturesFootprint, FixedSet10) {
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_union_pw_qpolynomial *Upwqp =
        unionCardOf(IslCtx, "{ [i] : 0 <= i < 10 }");
    ASSERT_NE(Upwqp, nullptr);

    IRTestEnv E;
    DenseMap<isl_id *, Value *> ParamMap;
    Value *V = evalUnionPwQpolynomialIR(Upwqp, ParamMap, E.Builder);
    isl_union_pw_qpolynomial_free(Upwqp);

    ASSERT_NE(V, nullptr);
    auto *CI = dyn_cast<ConstantInt>(V);
    ASSERT_NE(CI, nullptr) << "Expected constant for fixed-bound set";
    EXPECT_EQ(CI->getSExtValue(), 10);
  }
  isl_ctx_free(IslCtx);
}

TEST(ScopFeaturesFootprint, TwoDisjointPieces) {
  // Union of two disjoint ranges — simulates two statements accessing
  // non-overlapping parts of the same array.
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_union_pw_qpolynomial *Upwqp =
        unionCardOf(IslCtx, "{ [i] : 0 <= i < 4 or 10 <= i < 16 }");
    ASSERT_NE(Upwqp, nullptr);

    IRTestEnv E;
    DenseMap<isl_id *, Value *> ParamMap;
    Value *V = evalUnionPwQpolynomialIR(Upwqp, ParamMap, E.Builder);
    isl_union_pw_qpolynomial_free(Upwqp);

    ASSERT_NE(V, nullptr);
    auto *CI = dyn_cast<ConstantInt>(V);
    ASSERT_NE(CI, nullptr);
    EXPECT_EQ(CI->getSExtValue(), 10); // 4 + 6
  }
  isl_ctx_free(IslCtx);
}

TEST(ScopFeaturesFootprint, ParametricSet) {
  // Parametric access range — cardinality depends on runtime parameter N.
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_union_pw_qpolynomial *Upwqp =
        unionCardOf(IslCtx, "[N] -> { [i] : 0 <= i < N }");
    ASSERT_NE(Upwqp, nullptr);

    IRTestEnv E;
    LLVMContext &Ctx = E.Ctx;
    Type *I64 = Type::getInt64Ty(Ctx);

    isl_id *NId = isl_id_alloc(IslCtx, "N", nullptr);
    Value *NVal = ConstantInt::get(I64, 8);
    DenseMap<isl_id *, Value *> ParamMap{{NId, NVal}};

    Value *V = evalUnionPwQpolynomialIR(Upwqp, ParamMap, E.Builder);
    isl_union_pw_qpolynomial_free(Upwqp);
    isl_id_free(NId);

    ASSERT_NE(V, nullptr);
    auto *CI = dyn_cast<ConstantInt>(V);
    ASSERT_NE(CI, nullptr) << "Expected constant folding with N=8";
    EXPECT_EQ(CI->getSExtValue(), 8);
  }
  isl_ctx_free(IslCtx);
}

TEST(ScopFeaturesFootprint, EmptySet) {
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_union_pw_qpolynomial *Upwqp =
        unionCardOf(IslCtx, "{ [i] : false }");
    ASSERT_NE(Upwqp, nullptr);

    IRTestEnv E;
    DenseMap<isl_id *, Value *> ParamMap;
    Value *V = evalUnionPwQpolynomialIR(Upwqp, ParamMap, E.Builder);
    isl_union_pw_qpolynomial_free(Upwqp);

    ASSERT_NE(V, nullptr);
    auto *CI = dyn_cast<ConstantInt>(V);
    ASSERT_NE(CI, nullptr);
    EXPECT_EQ(CI->getSExtValue(), 0);
  }
  isl_ctx_free(IslCtx);
}

TEST(ScopFeaturesFootprint, TwoDimensionalAccess) {
  // 2D access range — simulates a 2D array accessed at (i, j).
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_union_pw_qpolynomial *Upwqp =
        unionCardOf(IslCtx, "{ [i, j] : 0 <= i < 4 and 0 <= j < 5 }");
    ASSERT_NE(Upwqp, nullptr);

    IRTestEnv E;
    DenseMap<isl_id *, Value *> ParamMap;
    Value *V = evalUnionPwQpolynomialIR(Upwqp, ParamMap, E.Builder);
    isl_union_pw_qpolynomial_free(Upwqp);

    ASSERT_NE(V, nullptr);
    auto *CI = dyn_cast<ConstantInt>(V);
    ASSERT_NE(CI, nullptr);
    EXPECT_EQ(CI->getSExtValue(), 20); // 4 × 5
  }
  isl_ctx_free(IslCtx);
}

#endif // POLLY_HAVE_BARVINOK
