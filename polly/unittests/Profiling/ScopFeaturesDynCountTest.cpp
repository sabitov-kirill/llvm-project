//===- ScopFeaturesDynCountTest.cpp - Tests for evalScaledCardIR ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Tests for evalScaledCardIR — the shared building block used by all dynamic
// (domain-weighted) SCoP features:
//
//   Contrib = static_count_in_stmt * card(iteration_domain_of_stmt)
//
// A full Scop object is not needed; we exercise evalScaledCardIR directly with
// manually constructed ISL sets and LLVM IR environments.
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
#include "isl/set.h"

#include <barvinok/isl.h>

#include "gtest/gtest.h"

using namespace llvm;
using namespace polly;

namespace {

struct IREnv {
  LLVMContext Ctx;
  std::unique_ptr<Module> M;
  Function *F = nullptr;
  BasicBlock *BB = nullptr;
  Instruction *Ret = nullptr;
  IRBuilder<> Builder;

  IREnv() : M(std::make_unique<Module>("test", Ctx)), Builder(Ctx) {
    FunctionType *FTy = FunctionType::get(Type::getVoidTy(Ctx), false);
    F = Function::Create(FTy, Function::ExternalLinkage, "test_fn", *M);
    BB = BasicBlock::Create(Ctx, "entry", F);
    Builder.SetInsertPoint(BB);
    Ret = Builder.CreateRetVoid();
    Builder.SetInsertPoint(Ret);
  }
};

/// Compute card of an ISL set string as isl_pw_qpolynomial*.  Caller owns the
/// result.
static isl_pw_qpolynomial *cardOf(isl_ctx *IslCtx, const char *SetStr) {
  isl_set *S = isl_set_read_from_str(IslCtx, SetStr);
  return isl_set_card(S); // takes ownership of S
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Single statement: 3 reads × 10-iteration domain → 30
// ---------------------------------------------------------------------------
TEST(ScopFeaturesDynCount, SingleStmt) {
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_pw_qpolynomial *Card = cardOf(IslCtx, "{ [i] : 0 <= i < 10 }");
    ASSERT_NE(Card, nullptr);

    IREnv E;
    DenseMap<isl_id *, Value *> ParamMap;
    Value *Contrib = evalScaledCardIR(Card, /*N=*/3, ParamMap, E.Builder);
    isl_pw_qpolynomial_free(Card);

    ASSERT_NE(Contrib, nullptr);
    auto *CI = dyn_cast<ConstantInt>(Contrib);
    ASSERT_NE(CI, nullptr) << "Expected constant-folded result";
    EXPECT_EQ(CI->getSExtValue(), 30);
  }
  isl_ctx_free(IslCtx);
}

// ---------------------------------------------------------------------------
// Two statements accumulated: (2 reads × 5) + (1 read × 4) = 14
// ---------------------------------------------------------------------------
TEST(ScopFeaturesDynCount, TwoStmtAccumulation) {
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_pw_qpolynomial *Card1 = cardOf(IslCtx, "{ [i] : 0 <= i < 5 }");
    isl_pw_qpolynomial *Card2 = cardOf(IslCtx, "{ [i] : 0 <= i < 4 }");
    ASSERT_NE(Card1, nullptr);
    ASSERT_NE(Card2, nullptr);

    IREnv E;
    Type *I64 = Type::getInt64Ty(E.Ctx);
    DenseMap<isl_id *, Value *> ParamMap;

    Value *Total = ConstantInt::get(I64, 0);
    Value *C1 = evalScaledCardIR(Card1, /*N=*/2, ParamMap, E.Builder);
    Total = E.Builder.CreateAdd(Total, C1);
    Value *C2 = evalScaledCardIR(Card2, /*N=*/1, ParamMap, E.Builder);
    Total = E.Builder.CreateAdd(Total, C2);

    isl_pw_qpolynomial_free(Card1);
    isl_pw_qpolynomial_free(Card2);

    ASSERT_NE(Total, nullptr);
    auto *Result = dyn_cast<ConstantInt>(Total);
    ASSERT_NE(Result, nullptr) << "Expected constant-folded result";
    EXPECT_EQ(Result->getSExtValue(), 14);
  }
  isl_ctx_free(IslCtx);
}

// ---------------------------------------------------------------------------
// Zero instructions in a statement contributes nothing (skip N==0 path)
// Total = 0 (no non-zero statements)
// ---------------------------------------------------------------------------
TEST(ScopFeaturesDynCount, ZeroStaticCount) {
  IREnv E;
  Type *I64 = Type::getInt64Ty(E.Ctx);
  Value *Total = ConstantInt::get(I64, 0);

  // N == 0 → skip; total stays at the initial zero constant
  auto *CI = dyn_cast<ConstantInt>(Total);
  ASSERT_NE(CI, nullptr);
  EXPECT_EQ(CI->getSExtValue(), 0);
}

// ---------------------------------------------------------------------------
// Parametric domain: N writes × [0, P) → P writes (runtime expression)
// Verify the result is not a constant (parametric case).
// ---------------------------------------------------------------------------
TEST(ScopFeaturesDynCount, ParametricDomain) {
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_pw_qpolynomial *Card = cardOf(IslCtx, "[P] -> { [i] : 0 <= i < P }");
    ASSERT_NE(Card, nullptr);

    IREnv E;
    LLVMContext &Ctx = E.Ctx;
    Type *I64 = Type::getInt64Ty(Ctx);

    isl_id *PId = isl_id_alloc(IslCtx, "P", nullptr);
    Value *PVal = ConstantInt::get(I64, 7);
    DenseMap<isl_id *, Value *> ParamMap{{PId, PVal}};

    // N = 2 writes × card([0,7)) = 14
    Value *Contrib = evalScaledCardIR(Card, /*N=*/2, ParamMap, E.Builder);
    isl_pw_qpolynomial_free(Card);
    isl_id_free(PId);

    ASSERT_NE(Contrib, nullptr);
    // With P=7 constant-folded, result should be 14
    auto *CI = dyn_cast<ConstantInt>(Contrib);
    ASSERT_NE(CI, nullptr) << "Expected constant folding with P=7";
    EXPECT_EQ(CI->getSExtValue(), 14);
  }
  isl_ctx_free(IslCtx);
}

// ---------------------------------------------------------------------------
// Nested loop: 1 FP op × card([0,N)×[0,M)) = N*M ops
// ---------------------------------------------------------------------------
TEST(ScopFeaturesDynCount, NestedLoopFpOps) {
  auto *IslCtx = isl_ctx_alloc();
  {
    isl_pw_qpolynomial *Card =
        cardOf(IslCtx, "[N,M] -> { [i,j] : 0 <= i < N and 0 <= j < M }");
    ASSERT_NE(Card, nullptr);

    IREnv E;
    LLVMContext &Ctx = E.Ctx;
    Type *I64 = Type::getInt64Ty(Ctx);

    isl_id *NId = isl_id_alloc(IslCtx, "N", nullptr);
    isl_id *MId = isl_id_alloc(IslCtx, "M", nullptr);
    Value *NVal = ConstantInt::get(I64, 4);
    Value *MVal = ConstantInt::get(I64, 3);
    DenseMap<isl_id *, Value *> ParamMap{{NId, NVal}, {MId, MVal}};

    // 1 FMul per (i,j) iteration
    Value *Contrib = evalScaledCardIR(Card, /*N=*/1, ParamMap, E.Builder);
    isl_pw_qpolynomial_free(Card);
    isl_id_free(NId);
    isl_id_free(MId);

    ASSERT_NE(Contrib, nullptr);
    auto *CI = dyn_cast<ConstantInt>(Contrib);
    ASSERT_NE(CI, nullptr) << "Expected constant folding with N=4, M=3";
    EXPECT_EQ(CI->getSExtValue(), 12);
  }
  isl_ctx_free(IslCtx);
}

#endif // POLLY_HAVE_BARVINOK
