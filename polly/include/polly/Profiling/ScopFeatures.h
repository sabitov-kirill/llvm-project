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

enum class FeatureID : unsigned {
  // clang-format off
  TripCount         = 0,  // Ehrhart polynomial via barvinok; -1 if unavailable
  MemFootprintBytes = 1,  // union of all array access ranges * element size; -1 if unavailable
  StmtCount         = 2,  // S.getSize()
  MaxLoopDepth      = 3,  // S.getMaxLoopDepth()
  NumParams         = 4,  // S.getNumParams()
  NumArrays         = 5,  // count(S.arrays())
  NumDimensions     = 6,  // sum of SAI->getNumberOfDimensions() over all arrays
  NumReads          = 7,  // sum_stmt(reads_in_stmt * card(domain)); -1 if unavailable
  NumWrites         = 8,  // sum_stmt(writes_in_stmt * card(domain)); -1 if unavailable
  NumReductions     = 9,  // sum_stmt(reductions_in_stmt * card(domain)); -1 if unavailable
  NumAluOps         = 10, // sum_stmt(alu_insts * card(domain)); -1 if unavailable
  NumMulDivOps      = 11, // sum_stmt(muldiv_insts * card(domain)); -1 if unavailable
  NumFpOps          = 12, // sum_stmt(fp_insts * card(domain)); -1 if unavailable
  NumCfOps          = 13, // sum_stmt(cf_insts * card(domain)); -1 if unavailable
  NUM_FEATURES      = 14
  // clang-format on
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
computeScopFeaturesIR(const Scop &S, llvm::IRBuilder<> &B);

} // namespace polly

#endif // POLLY_PROFILING_SCOPFEATURES_H
