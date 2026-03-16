//===--- ScopFeatures.h - Compile-time polyhedral feature extraction ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef POLLY_PROFILING_SCOPFEATURES_H
#define POLLY_PROFILING_SCOPFEATURES_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/IRBuilder.h"

struct isl_id;
struct isl_pw_qpolynomial;

namespace llvm {
class Instruction;
class Value;
} // namespace llvm

namespace polly {

class Scop;

/// Fixed ABI: append only.  Index == position in the features[] array passed
/// to __cas_scop_start at runtime.
enum class FeatureID : unsigned {
  TripCount    = 0, // Ehrhart polynomial via barvinok; -1 if unavailable
  StmtCount    = 1, // S.getSize()
  MaxLoopDepth = 2, // S.getMaxLoopDepth()
  NumParams    = 3, // S.getNumParams()
  NumArrays    = 4, // count(S.arrays())
  NumDimensions = 5, // sum of SAI->getNumberOfDimensions() over all arrays
  NumReads     = 6, // count MA->isRead() across all stmts
  NumWrites    = 7, // count MA->isWrite() across all stmts
  NumReductions = 8, // count MA->isReductionLike() across all stmts
  NUM_FEATURES  = 9
};

constexpr unsigned NumScopFeatures =
    static_cast<unsigned>(FeatureID::NUM_FEATURES);

/// Compute all SCoP features as LLVM IR Values (i64 each).
///
/// Returns a SmallVector of exactly NumScopFeatures values, in FeatureID order.
/// Compile-time-constant features are emitted as ConstantInt.  TripCount may
/// be a runtime expression when barvinok is available and the SCoP has
/// symbolic parameters.
///
/// The caller must set B's insert point before calling this function.
llvm::SmallVector<llvm::Value *, NumScopFeatures>
computeScopFeaturesIR(const Scop &S, llvm::IRBuilder<> &B,
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
