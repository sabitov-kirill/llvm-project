//===--- ScopFeaturesFootprint.cpp - Memory footprint SCoP feature --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements footprintBytesIR() (internal).
//
// The footprint is the number of distinct bytes touched by all array accesses
// in the SCoP.  For each ScopArrayInfo of kind Array, the access ranges of all
// statements are unioned, the cardinality of the resulting set is computed via
// barvinok, and the result is multiplied by the element's byte size.  Sums
// across all arrays yield the total footprint.
//
// Without barvinok the feature returns -1.
//
//===----------------------------------------------------------------------===//

#include "ScopFeaturesImpl.h"
#include "polly/Profiling/IslPolyEval.h"
#include "polly/ScopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"

#include <cstdio>

#ifdef POLLY_HAVE_BARVINOK
#include <barvinok/isl.h>
#endif

using namespace llvm;
using namespace polly;

namespace {

// Build the element-index extent for an array: 0 <= idx_i < size_i (if known).
static isl::set buildArrayExtent(const ScopArrayInfo &SAI) {
  isl::set Extent = isl::set::universe(SAI.getSpace());
  const unsigned Dims = SAI.getNumberOfDimensions();
  const isl::id TupleId = SAI.getBasePtrId();

  for (unsigned I = 0; I < Dims; ++I) {
    Extent = Extent.lower_bound_si(isl::dim::set, I, 0);

    const SCEV *DimSize = SAI.getDimensionSize(I);
    if (!DimSize)
      continue;

    isl::pw_aff Size = SAI.getDimensionSizePw(I);
    if (Size.is_null())
      continue;

    Size = Size.add_dims(isl::dim::in, Dims);
    Size = Size.set_tuple_id(isl::dim::in, TupleId);

    isl::local_space LS(Extent.get_space());
    isl::pw_aff Idx = isl::pw_aff::var_on_domain(LS, isl::dim::set, I);
    Extent = Extent.intersect(Idx.lt_set(Size));
  }

  return Extent;
}

} // namespace

Value *polly::features::footprintBytesIR(const Scop &S, IRBuilder<> &B,
                                         Instruction * /*InsertBefore*/) {
  Type *I64 = B.getInt64Ty();
  Value *MinusOne = ConstantInt::get(I64, (uint64_t)-1LL);

#ifdef POLLY_HAVE_BARVINOK
  // Build param id → Value map (same pattern as tripCountIR).
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

  const DataLayout &DL = S.getFunction().getParent()->getDataLayout();
  Value *TotalBytes = ConstantInt::get(I64, 0);
  bool AnyFailed = false;

  for (ScopArrayInfo *SAI : S.arrays()) {
    if (!SAI->isArrayKind())
      continue;

    // Union ranges of statement-executed accesses touching this array.
    isl::union_set Accessed;
    for (auto &Stmt : S) {
      for (auto *MA : Stmt) {
        if (MA->getLatestScopArrayInfo() != SAI)
          continue;

        isl::map Rel = MA->getLatestAccessRelation();
        if (Rel.is_null())
          continue;

        Rel = Rel.intersect_domain(Stmt.getDomain());
        isl::union_set Range = isl::union_set(Rel.range());
        Accessed = Accessed.is_null() ? Range : Accessed.unite(Range);
      }
    }

    if (Accessed.is_null())
      continue;

    isl::set Extent = buildArrayExtent(*SAI);
    Extent = Extent.intersect_params(S.getContext());
    Accessed = Accessed.intersect(isl::union_set(Extent));
    Accessed = Accessed.intersect_params(S.getContext());

    if (bool(Accessed.is_empty()))
      continue;

    // Cardinality of the accessed set (barvinok); takes ownership of Accessed.
    isl_union_pw_qpolynomial *Card = isl_union_set_card(Accessed.release());
    if (!Card) {
      AnyFailed = true;
      continue;
    }

    Value *ElemCard = evalUnionPwQpolynomialIR(Card, IdToValue, B);
    isl_union_pw_qpolynomial_free(Card);

    if (ElemCard == MinusOne) {
      AnyFailed = true;
      continue;
    }

    uint64_t ElemBytes = DL.getTypeStoreSize(SAI->getElementType());
    Value *ArrayBytes =
        B.CreateMul(ElemCard, ConstantInt::get(I64, ElemBytes), "array_fp");
    TotalBytes = B.CreateAdd(TotalBytes, ArrayBytes, "total_fp");
  }

  for (auto &KV : IdToValue)
    isl_id_free(KV.first);

  return AnyFailed ? MinusOne : TotalBytes;
#else
  (void)S;
  return MinusOne;
#endif
}
