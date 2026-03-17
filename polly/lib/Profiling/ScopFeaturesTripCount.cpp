//===--- ScopFeaturesTripCount.cpp - Ehrhart trip-count IR codegen --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ScopFeaturesImpl.h"
#include "polly/Profiling/IslPolyEval.h"
#include "polly/ScopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Constants.h"

#include "isl/polynomial.h"

#ifdef POLLY_HAVE_BARVINOK
#include <barvinok/isl.h>
#endif

using namespace llvm;
using namespace polly;

Value *polly::features::tripCountIR(const Scop &S, IRBuilder<> &B,
                                    Instruction * /*InsertBefore*/) {
  Type *I64 = B.getInt64Ty();
  Value *MinusOne = ConstantInt::get(I64, (uint64_t)-1LL);

#ifdef POLLY_HAVE_BARVINOK
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

  isl_union_pw_qpolynomial *Upwqp =
      isl_union_set_card(S.getDomains().release());
  if (!Upwqp) {
    for (auto &KV : IdToValue)
      isl_id_free(KV.first);
    return MinusOne;
  }

  Value *Result = evalUnionPwQpolynomialIR(Upwqp, IdToValue, B);
  isl_union_pw_qpolynomial_free(Upwqp);

  for (auto &KV : IdToValue)
    isl_id_free(KV.first);

  return Result;
#else
  (void)S;
  return MinusOne;
#endif
}
