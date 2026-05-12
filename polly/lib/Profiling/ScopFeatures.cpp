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

using namespace llvm;
using namespace polly;

SmallVector<Value *, NumScopFeatures>
polly::computeScopFeaturesIR(const Scop &S, IRBuilder<> &B) {
  using namespace polly::features;
  Type *I64 = B.getInt64Ty();
  auto C = [&](int64_t V) { return ConstantInt::get(I64, V); };

  SmallVector<Value *, NumScopFeatures> Features(NumScopFeatures);
  auto At = [&](FeatureID ID) -> Value *& {
    return Features[static_cast<unsigned>(ID)];
  };

  At(FeatureID::TripCount) = tripCountIR(S, B);
  At(FeatureID::MemFootprintBytes) = footprintBytesIR(S, B);
  At(FeatureID::StmtCount) = C(stmtCount(S));
  At(FeatureID::MaxLoopDepth) = C(maxLoopDepth(S));
  At(FeatureID::NumParams) = C(numParams(S));
  At(FeatureID::NumArrays) = C(numArrays(S));
  At(FeatureID::NumDimensions) = C(numDimensions(S));
  At(FeatureID::NumReads) = numReadsIR(S, B);
  At(FeatureID::NumWrites) = numWritesIR(S, B);
  At(FeatureID::NumReductions) = numReductionsIR(S, B);
  At(FeatureID::NumAluOps) = numAluOpsIR(S, B);
  At(FeatureID::NumMulDivOps) = numMulDivOpsIR(S, B);
  At(FeatureID::NumFpOps) = numFpOpsIR(S, B);
  At(FeatureID::NumCfOps) = numCfOpsIR(S, B);
  return Features;
}
