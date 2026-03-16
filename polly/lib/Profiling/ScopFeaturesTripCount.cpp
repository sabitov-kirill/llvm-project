//===--- ScopFeaturesTripCount.cpp - Ehrhart trip-count IR codegen --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements tripCountIR() (internal) and the public evalPwQpolynomialIR()
// (re-exported under POLLY_HAVE_BARVINOK for unit tests).
//
//===----------------------------------------------------------------------===//

#include "ScopFeaturesImpl.h"
#include "polly/Profiling/ScopFeatures.h"
#include "polly/ScopInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Type.h"

#include "isl/polynomial.h"
#include "isl/space.h"
#include "isl/val.h"

#ifdef POLLY_HAVE_BARVINOK
#include <barvinok/isl.h>
#endif

using namespace llvm;
using namespace polly;

#ifdef POLLY_HAVE_BARVINOK

// ---------------------------------------------------------------------------
// Term-level IR generation
// ---------------------------------------------------------------------------

namespace {

struct TermCtx {
  IRBuilder<> &B;
  const SmallVectorImpl<Value *>
      &PVals; // positional: PVals[i] = Value for param i
  Value *Acc;
  bool Failed;
};

/// ISL callback: accumulate one monomial term into Ctx->Acc.
static isl_stat evalTermCB(isl_term *Term, void *User) {
  auto *Ctx = static_cast<TermCtx *>(User);
  IRBuilder<> &B = Ctx->B;
  Type *I64 = B.getInt64Ty();

  isl_val *CoeffV = isl_term_get_coefficient_val(Term);
  long Coeff = isl_val_get_num_si(CoeffV);
  isl_val_free(CoeffV);

  Value *TermVal = ConstantInt::get(I64, (int64_t)Coeff);

  isl_size NPar = isl_term_dim(Term, isl_dim_param);
  for (int I = 0; I < NPar; I++) {
    isl_size Exp = isl_term_get_exp(Term, isl_dim_param, I);
    if (Exp == 0)
      continue;

    if (I >= (int)Ctx->PVals.size() || !Ctx->PVals[I]) {
      isl_term_free(Term);
      Ctx->Failed = true;
      return isl_stat_error;
    }

    Value *P = B.CreateSExtOrTrunc(Ctx->PVals[I], I64, "param");
    for (isl_size E = 0; E < Exp; E++)
      TermVal = B.CreateMul(TermVal, P, "pow");
  }

  Ctx->Acc = B.CreateAdd(Ctx->Acc, TermVal, "term_sum");
  isl_term_free(Term);
  return isl_stat_ok;
}

// ---------------------------------------------------------------------------
// Piece-level IR generation
// ---------------------------------------------------------------------------

struct PieceCtx {
  IRBuilder<> &B;
  const SmallVectorImpl<Value *> &PVals;
  Value *Total;
  bool Failed;
};

/// ISL callback: sum one piece of a pw_qpolynomial into Ctx->Total.
static isl_stat evalPieceCB(isl_set *Set, isl_qpolynomial *Qp, void *User) {
  // Domain condition is ignored (simplified: assume all pieces are valid at the
  // SCoP entry where parameters are live).
  isl_set_free(Set);

  auto *Ctx = static_cast<PieceCtx *>(User);
  IRBuilder<> &B = Ctx->B;
  Type *I64 = B.getInt64Ty();

  TermCtx TCtx{B, Ctx->PVals, ConstantInt::get(I64, 0), false};
  isl_stat S = isl_qpolynomial_foreach_term(Qp, evalTermCB, &TCtx);
  isl_qpolynomial_free(Qp);

  if (S != isl_stat_ok || TCtx.Failed) {
    Ctx->Failed = true;
    return isl_stat_error;
  }

  Ctx->Total = B.CreateAdd(Ctx->Total, TCtx.Acc, "piece_sum");
  return isl_stat_ok;
}

// ---------------------------------------------------------------------------
// pw_qpolynomial-level IR generation
// ---------------------------------------------------------------------------

struct UnionPieceCtx {
  IRBuilder<> &B;
  DenseMap<isl_id *, Value *> &IdToValue;
  Value *Total;
  bool Failed;
};

/// ISL callback: evaluate one pw_qpolynomial piece and add to Ctx->Total.
static isl_stat evalUnionPieceCB(isl_pw_qpolynomial *PwQp, void *User) {
  auto *Ctx = static_cast<UnionPieceCtx *>(User);
  Value *V = evalPwQpolynomialIR(PwQp, Ctx->IdToValue, Ctx->B);
  isl_pw_qpolynomial_free(PwQp);

  Type *I64 = Ctx->B.getInt64Ty();
  Value *MinusOne = ConstantInt::get(I64, (uint64_t)-1LL);
  if (V == MinusOne) {
    Ctx->Failed = true;
    return isl_stat_error;
  }

  Ctx->Total = Ctx->B.CreateAdd(Ctx->Total, V, "union_sum");
  return isl_stat_ok;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public: evalPwQpolynomialIR
// ---------------------------------------------------------------------------

Value *polly::evalPwQpolynomialIR(isl_pw_qpolynomial *PwQp,
                                  DenseMap<isl_id *, Value *> &ParamMap,
                                  IRBuilder<> &B) {
  Type *I64 = B.getInt64Ty();

  // Build positional Value array from the pw_qpolynomial's param space.
  isl_space *Space = isl_pw_qpolynomial_get_domain_space(PwQp);
  int NPar = isl_space_dim(Space, isl_dim_param);
  SmallVector<Value *, 8> PVals(NPar, nullptr);
  for (int I = 0; I < NPar; I++) {
    isl_id *Id = isl_space_get_dim_id(Space, isl_dim_param, I);
    auto It = ParamMap.find(Id);
    if (It != ParamMap.end())
      PVals[I] = It->second;
    isl_id_free(Id);
  }
  isl_space_free(Space);

  PieceCtx Ctx{B, PVals, ConstantInt::get(I64, 0), false};
  isl_stat S = isl_pw_qpolynomial_foreach_piece(PwQp, evalPieceCB, &Ctx);

  if (S != isl_stat_ok || Ctx.Failed)
    return ConstantInt::get(I64, (uint64_t)-1LL);

  return Ctx.Total;
}

#endif // POLLY_HAVE_BARVINOK

// ---------------------------------------------------------------------------
// Internal: tripCountIR
// ---------------------------------------------------------------------------

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

  UnionPieceCtx Ctx{B, IdToValue, ConstantInt::get(I64, 0), false};
  isl_stat Stat = isl_union_pw_qpolynomial_foreach_pw_qpolynomial(
      Upwqp, evalUnionPieceCB, &Ctx);
  isl_union_pw_qpolynomial_free(Upwqp);

  for (auto &KV : IdToValue)
    isl_id_free(KV.first);

  if (Stat != isl_stat_ok || Ctx.Failed)
    return MinusOne;

  return Ctx.Total;
#else
  (void)S;
  return MinusOne;
#endif
}
