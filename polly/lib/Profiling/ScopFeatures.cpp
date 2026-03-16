//===--- ScopFeatures.cpp - Compile-time polyhedral feature extraction ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Thin orchestrator: collects all per-category feature Values into a single
// SmallVector.  Each category is implemented in its own .cpp file.
//
//===----------------------------------------------------------------------===//

#include "polly/Profiling/ScopFeatures.h"
#include "ScopFeaturesImpl.h"
#include "polly/ScopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Type.h"

using namespace llvm;
using namespace polly;

SmallVector<Value *, NumScopFeatures>
polly::computeScopFeaturesIR(const Scop &S, IRBuilder<> &B,
                             Instruction *InsertBefore) {
  using namespace polly::features;
  Type *I64 = B.getInt64Ty();
  auto C = [&](int64_t v) -> Value * { return ConstantInt::get(I64, v); };

  return {
    /* TripCount     */ tripCountIR(S, B, InsertBefore),
    /* StmtCount     */ C(stmtCount(S)),
    /* MaxLoopDepth  */ C(maxLoopDepth(S)),
    /* NumParams     */ C(numParams(S)),
    /* NumArrays     */ C(numArrays(S)),
    /* NumDimensions */ C(numDimensions(S)),
    /* NumReads      */ C(numReads(S)),
    /* NumWrites     */ C(numWrites(S)),
    /* NumReductions */ C(numReductions(S)),
  };
}
