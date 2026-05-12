//===--- ScopFeaturesOpsCount.cpp - Per group operations count features --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// All dynamic SCoP features that weight a per-statement instruction count by
// the cardinality of the statement's iteration domain:
//
//   Feature = sum_stmt( static_count_in_stmt * card(domain_stmt) )
//
// Without barvinok all seven features return -1.
//
// Instruction groups:
//   Memory-access ops — Reads, Writes         (memory ops)
//   ALU    — Add Sub Shl LShr AShr And Or Xor (cheap integer ops)
//   MulDiv — Mul SDiv UDiv SRem URem          (expensive integer ops)
//   FP     — FAdd FSub FMul FDiv FRem         (floating-point ops)
//   CF     — ICmp FCmp Br Switch Select       (control-flow ops)
//
//===----------------------------------------------------------------------===//

#include "ScopFeaturesImpl.h"
#include "polly/Profiling/IslPolyEval.h"
#include "polly/ScopInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"

#ifdef POLLY_HAVE_BARVINOK
#include "isl/polynomial.h"
#include <barvinok/isl.h>
#endif

using namespace llvm;
using namespace polly;

namespace {

#ifdef POLLY_HAVE_BARVINOK

/// Emit IR for sum_stmt( Count(stmt) * card(domain_stmt) ).
/// Returns ConstantInt(-1) on any barvinok failure.
template <typename CountFn>
Value *weightedCountIR(const Scop &S, IRBuilder<> &B, CountFn Count) {
#ifdef POLLY_HAVE_BARVINOK
  Type *I64 = B.getInt64Ty();
  Value *MinusOne = ConstantInt::get(I64, (uint64_t)-1LL);

  DenseMap<isl_id *, Value *> IdToValue = features::buildParamMap(S);
  Value *Total = ConstantInt::get(I64, 0);
  bool Failed = false;

  for (auto &Stmt : S) {
    int64_t N = Count(Stmt);
    if (N == 0)
      continue;

    isl_pw_qpolynomial *Card = isl_set_card(Stmt.getDomain().release());
    if (!Card) {
      Failed = true;
      break;
    }

    Value *Contrib = evalScaledCardIR(Card, N, IdToValue, B);
    isl_pw_qpolynomial_free(Card);

    if (Contrib == MinusOne) {
      Failed = true;
      break;
    }
    Total = B.CreateAdd(Total, Contrib);
  }

  for (auto &KV : IdToValue)
    isl_id_free(KV.first);

  return Failed ? MinusOne : Total;
#else
  (void)S;
  return ConstantInt::get(B.getInt64Ty(), (uint64_t)-1LL);
#endif
}

int64_t countReads(const ScopStmt &Stmt) {
  return llvm::count_if(Stmt,
                        [](const MemoryAccess *MA) { return MA->isRead(); });
}

int64_t countWrites(const ScopStmt &Stmt) {
  return llvm::count_if(Stmt,
                        [](const MemoryAccess *MA) { return MA->isWrite(); });
}

int64_t countReductions(const ScopStmt &Stmt) {
  return llvm::count_if(
      Stmt, [](const MemoryAccess *MA) { return MA->isReductionLike(); });
}

int64_t countAluOps(const ScopStmt &Stmt) {
  return llvm::count_if(Stmt.insts(), [](const Instruction *I) {
    switch (I->getOpcode()) {
    case Instruction::Add:
    case Instruction::Sub:
    case Instruction::Shl:
    case Instruction::LShr:
    case Instruction::AShr:
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Xor:
      return true;
    default:
      return false;
    }
  });
}

int64_t countMulDivOps(const ScopStmt &Stmt) {
  return llvm::count_if(Stmt.insts(), [](const Instruction *I) {
    switch (I->getOpcode()) {
    case Instruction::Mul:
    case Instruction::SDiv:
    case Instruction::UDiv:
    case Instruction::SRem:
    case Instruction::URem:
      return true;
    default:
      return false;
    }
  });
}

int64_t countFpOps(const ScopStmt &Stmt) {
  return llvm::count_if(Stmt.insts(), [](const Instruction *I) {
    switch (I->getOpcode()) {
    case Instruction::FAdd:
    case Instruction::FSub:
    case Instruction::FMul:
    case Instruction::FDiv:
    case Instruction::FRem:
      return true;
    default:
      return false;
    }
  });
}

int64_t countCfOps(const ScopStmt &Stmt) {
  return llvm::count_if(Stmt.insts(), [](const Instruction *I) {
    switch (I->getOpcode()) {
    case Instruction::ICmp:
    case Instruction::FCmp:
    case Instruction::Br:
    case Instruction::Switch:
    case Instruction::Select:
      return true;
    default:
      return false;
    }
  });
}

#endif // POLLY_HAVE_BARVINOK

} // namespace

Value *polly::features::numReadsIR(const Scop &S, IRBuilder<> &B) {
  return weightedCountIR(S, B, countReads);
}

Value *polly::features::numWritesIR(const Scop &S, IRBuilder<> &B) {
  return weightedCountIR(S, B, countWrites);
}

Value *polly::features::numReductionsIR(const Scop &S, IRBuilder<> &B) {
  return weightedCountIR(S, B, countReductions);
}

Value *polly::features::numAluOpsIR(const Scop &S, IRBuilder<> &B) {
  return weightedCountIR(S, B, countAluOps);
}

Value *polly::features::numMulDivOpsIR(const Scop &S, IRBuilder<> &B) {
  return weightedCountIR(S, B, countMulDivOps);
}

Value *polly::features::numFpOpsIR(const Scop &S, IRBuilder<> &B) {
  return weightedCountIR(S, B, countFpOps);
}

Value *polly::features::numCfOpsIR(const Scop &S, IRBuilder<> &B) {
  return weightedCountIR(S, B, countCfOps);
}
