//===--- IslPolyEval.h - ISL polynomial → LLVM IR evaluator ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Reusable helpers that lower ISL piecewise quasi-polynomials to LLVM IR
// arithmetic.  Used by ScopFeaturesTripCount and ScopFeaturesFootprint.
//
// Only available when Polly is built with barvinok support
// (POLLY_HAVE_BARVINOK).
//
//===----------------------------------------------------------------------===//

#ifndef POLLY_PROFILING_ISLPOLYEVAL_H
#define POLLY_PROFILING_ISLPOLYEVAL_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/IRBuilder.h"

struct isl_id;
struct isl_pw_qpolynomial;
struct isl_union_pw_qpolynomial;

namespace llvm {
class Value;
} // namespace llvm

namespace polly {

#ifdef POLLY_HAVE_BARVINOK

/// Evaluate \p PwQp — a piecewise quasi-polynomial — at the parameter values
/// given in \p ParamMap (isl_id* → llvm::Value*), inserting IR into \p B.
///
/// Returns an i64 Value.  Returns ConstantInt(-1) on any failure.
/// Does NOT take ownership of \p PwQp.
llvm::Value *
evalPwQpolynomialIR(isl_pw_qpolynomial *PwQp,
                    llvm::DenseMap<isl_id *, llvm::Value *> &ParamMap,
                    llvm::IRBuilder<> &B);

/// Evaluate \p Upwqp — a union of piecewise quasi-polynomials — by summing
/// each piece via evalPwQpolynomialIR.
///
/// Returns an i64 Value.  Returns ConstantInt(-1) on any failure.
/// Does NOT take ownership of \p Upwqp.
llvm::Value *
evalUnionPwQpolynomialIR(isl_union_pw_qpolynomial *Upwqp,
                         llvm::DenseMap<isl_id *, llvm::Value *> &ParamMap,
                         llvm::IRBuilder<> &B);

#endif // POLLY_HAVE_BARVINOK

} // namespace polly

#endif // POLLY_PROFILING_ISLPOLYEVAL_H
