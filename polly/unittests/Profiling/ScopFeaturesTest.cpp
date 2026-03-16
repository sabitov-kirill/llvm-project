//===- ScopFeaturesTest.cpp - Unit tests for ScopFeatures -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Tests for evalPwQpolynomialIR (barvinok path).
//
//===----------------------------------------------------------------------===//

#ifdef POLLY_HAVE_BARVINOK

#include "polly/Profiling/ScopFeatures.h"
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
#include "isl/set.h"

#include <barvinok/isl.h>

#include "gtest/gtest.h"

using namespace llvm;
using namespace polly;

// ---------------------------------------------------------------------------
// Helper: build a minimal LLVM IR function and return (BB, RetVoid, Builder).
// The builder is positioned before the RetVoid terminator.
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
      : M(std::make_unique<Module>("test", Ctx)),
        Builder(Ctx) {
    FunctionType *FTy = FunctionType::get(Type::getVoidTy(Ctx), false);
    F = Function::Create(FTy, Function::ExternalLinkage, "test_fn", *M);
    BB = BasicBlock::Create(Ctx, "entry", F);
    Builder.SetInsertPoint(BB);
    Ret = Builder.CreateRetVoid();
    // Position before the return so inserted IR precedes it.
    Builder.SetInsertPoint(Ret);
  }
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// Helper: compute card of an ISL set string → isl_pw_qpolynomial*.
// Caller owns the returned pointer.
// ---------------------------------------------------------------------------
static isl_pw_qpolynomial *cardOf(isl_ctx *Ctx, const char *SetStr) {
  isl_set *S = isl_set_read_from_str(Ctx, SetStr);
  return isl_set_card(S); // barvinok; takes ownership of S
}

TEST(ScopFeatures, FixedLoop10) {
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_pw_qpolynomial *PwQp = cardOf(IslCtx, "{ [i] : 0 <= i < 10 }");
    ASSERT_NE(PwQp, nullptr);

    IRTestEnv E;
    DenseMap<isl_id *, Value *> ParamMap; // empty — no params
    Value *V = evalPwQpolynomialIR(PwQp, ParamMap, E.Builder);
    isl_pw_qpolynomial_free(PwQp);

    ASSERT_NE(V, nullptr);
    auto *CI = dyn_cast<ConstantInt>(V);
    ASSERT_NE(CI, nullptr) << "Expected constant-folded result";
    EXPECT_EQ(CI->getSExtValue(), 10);
  }
  isl_ctx_free(IslCtx);
}

TEST(ScopFeatures, FixedNested) {
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_pw_qpolynomial *PwQp =
        cardOf(IslCtx, "{ [i,j] : 0 <= i < 4 and 0 <= j < 3 }");
    ASSERT_NE(PwQp, nullptr);

    IRTestEnv E;
    DenseMap<isl_id *, Value *> ParamMap;
    Value *V = evalPwQpolynomialIR(PwQp, ParamMap, E.Builder);
    isl_pw_qpolynomial_free(PwQp);

    ASSERT_NE(V, nullptr);
    auto *CI = dyn_cast<ConstantInt>(V);
    ASSERT_NE(CI, nullptr);
    EXPECT_EQ(CI->getSExtValue(), 12);
  }
  isl_ctx_free(IslCtx);
}

TEST(ScopFeatures, EmptyDomain) {
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_pw_qpolynomial *PwQp = cardOf(IslCtx, "{ [i] : false }");
    ASSERT_NE(PwQp, nullptr);

    IRTestEnv E;
    DenseMap<isl_id *, Value *> ParamMap;
    Value *V = evalPwQpolynomialIR(PwQp, ParamMap, E.Builder);
    isl_pw_qpolynomial_free(PwQp);

    ASSERT_NE(V, nullptr);
    auto *CI = dyn_cast<ConstantInt>(V);
    ASSERT_NE(CI, nullptr);
    EXPECT_EQ(CI->getSExtValue(), 0);
  }
  isl_ctx_free(IslCtx);
}

TEST(ScopFeatures, ParametricLoop) {
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_pw_qpolynomial *PwQp =
        cardOf(IslCtx, "[N] -> { [i] : 0 <= i < N }");
    ASSERT_NE(PwQp, nullptr);

    IRTestEnv E;
    LLVMContext &Ctx = E.Ctx;
    Type *I64 = Type::getInt64Ty(Ctx);

    // Map the ISL param "N" to a constant 7.
    isl_id *NId = isl_id_alloc(IslCtx, "N", nullptr);
    Value *NVal = ConstantInt::get(I64, 7);
    DenseMap<isl_id *, Value *> ParamMap{{NId, NVal}};

    Value *V = evalPwQpolynomialIR(PwQp, ParamMap, E.Builder);
    isl_pw_qpolynomial_free(PwQp);
    isl_id_free(NId);

    ASSERT_NE(V, nullptr);
    auto *CI = dyn_cast<ConstantInt>(V);
    ASSERT_NE(CI, nullptr) << "Expected constant folding with N=7";
    EXPECT_EQ(CI->getSExtValue(), 7);
  }
  isl_ctx_free(IslCtx);
}

TEST(ScopFeatures, ParametricNested) {
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_pw_qpolynomial *PwQp =
        cardOf(IslCtx, "[N,M] -> { [i,j] : 0 <= i < N and 0 <= j < M }");
    ASSERT_NE(PwQp, nullptr);

    IRTestEnv E;
    LLVMContext &Ctx = E.Ctx;
    Type *I64 = Type::getInt64Ty(Ctx);

    isl_id *NId = isl_id_alloc(IslCtx, "N", nullptr);
    isl_id *MId = isl_id_alloc(IslCtx, "M", nullptr);
    Value *NVal = ConstantInt::get(I64, 5);
    Value *MVal = ConstantInt::get(I64, 3);
    DenseMap<isl_id *, Value *> ParamMap{{NId, NVal}, {MId, MVal}};

    Value *V = evalPwQpolynomialIR(PwQp, ParamMap, E.Builder);
    isl_pw_qpolynomial_free(PwQp);
    isl_id_free(NId);
    isl_id_free(MId);

    ASSERT_NE(V, nullptr);
    auto *CI = dyn_cast<ConstantInt>(V);
    ASSERT_NE(CI, nullptr) << "Expected constant folding with N=5,M=3";
    EXPECT_EQ(CI->getSExtValue(), 15);
  }
  isl_ctx_free(IslCtx);
}

#endif // POLLY_HAVE_BARVINOK
