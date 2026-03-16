//===--- ScopFeatures.h - Compile-time polyhedral feature extraction ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#ifndef POLLY_PROFILING_SCOPFEATURES_H
#define POLLY_PROFILING_SCOPFEATURES_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/IRBuilder.h"

struct isl_id;
struct isl_pw_qpolynomial;

namespace llvm {
class Instruction;
class Value;
} // namespace llvm

namespace polly {

class Scop;

/// Generate LLVM IR that evaluates the Ehrhart polynomial of \p S's iteration
/// domain at the live SCoP parameter values, inserting IR before \p B's
/// current insert point.
///
/// Requires barvinok (POLLY_HAVE_BARVINOK) for parametric SCoPs. Without it,
/// always returns ConstantInt(-1, i64).
///
/// The caller must set B's insert point before calling this function.
/// Returns an llvm::Value* of type i64. Never returns nullptr.
llvm::Value *computeTripCountIR(const Scop &S, llvm::IRBuilder<> &B,
                                llvm::Instruction *InsertBefore);

#ifdef POLLY_HAVE_BARVINOK
/// Evaluate \p PwQp — a piecewise quasi-polynomial — at the parameter values
/// given in \p ParamMap (isl_id* → llvm::Value*), inserting IR into \p B.
///
/// Used directly by unit tests (which can construct pwqp without a real Scop).
/// Returns an i64 Value; returns ConstantInt(-1) on any failure.
llvm::Value *
evalPwQpolynomialIR(isl_pw_qpolynomial *PwQp,
                    llvm::DenseMap<isl_id *, llvm::Value *> &ParamMap,
                    llvm::IRBuilder<> &B);
#endif // POLLY_HAVE_BARVINOK

} // namespace polly

#endif // POLLY_PROFILING_SCOPFEATURES_H
