//===--- ScopFeaturesUtils.cpp - Shared helpers for SCoP features ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ScopFeaturesImpl.h"
#include "polly/ScopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

using namespace llvm;
using namespace polly;

#ifdef POLLY_HAVE_BARVINOK

DenseMap<isl_id *, Value *> polly::features::buildParamMap(const Scop &S) {
  DenseMap<isl_id *, Value *> IdToValue;
  for (const SCEV *P : S.parameters()) {
    isl::id Id = S.getIdForParam(P);
    if (Id.is_null())
      continue;
    const auto *SU = dyn_cast<SCEVUnknown>(P);
    if (!SU)
      continue;
    IdToValue[Id.release()] = SU->getValue();
  }
  return IdToValue;
}

#endif // POLLY_HAVE_BARVINOK
