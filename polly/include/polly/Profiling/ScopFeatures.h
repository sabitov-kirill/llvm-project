//===--- ScopFeatures.h - Compile-time polyhedral feature extraction ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef POLLY_PROFILING_SCOPFEATURES_H
#define POLLY_PROFILING_SCOPFEATURES_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/IRBuilder.h"


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
  NumReductions     = 8, // count MA->isReductionLike() across all stmts
  MemFootprintBytes = 9, // union of all array access ranges × element size; -1 if unavailable
  NUM_FEATURES      = 10
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


} // namespace polly

#endif // POLLY_PROFILING_SCOPFEATURES_H
