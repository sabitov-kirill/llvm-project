//===--- ScopFeaturesImpl.h - Internal per-category feature functions -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Internal declarations for per-category SCoP feature compute functions.
// Not installed; used only by ScopFeatures.cpp and the implementation files.
//
//===----------------------------------------------------------------------===//

#ifndef POLLY_PROFILING_SCOPFEATURESIMPL_H
#define POLLY_PROFILING_SCOPFEATURESIMPL_H

#include <cstdint>

// IRBuilder forward declaration requires matching the primary template exactly;
// include instead to avoid fragility.
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/DenseMap.h"

struct isl_id;

namespace polly {

class Scop;

namespace features {

/// Build an isl_id* → Value* map for every SCEVUnknown parameter in \p S.
/// Caller must call isl_id_free() on every key when done.
/// Only has a definition when POLLY_HAVE_BARVINOK is set; call sites must be
/// guarded accordingly.
llvm::DenseMap<isl_id *, llvm::Value *> buildParamMap(const Scop &S);

// Loop-structure features (ScopFeaturesLoop.cpp)
int64_t stmtCount(const Scop &S);
int64_t maxLoopDepth(const Scop &S);
int64_t numParams(const Scop &S);
int64_t numArrays(const Scop &S);
int64_t numDimensions(const Scop &S);

// Trip-count IR codegen (ScopFeaturesTripCount.cpp)
llvm::Value *tripCountIR(const Scop &S, llvm::IRBuilder<> &B);

// Memory footprint IR codegen (ScopFeaturesFootprint.cpp)
llvm::Value *footprintBytesIR(const Scop &S, llvm::IRBuilder<> &B);

// Instruction-group counts IR codegen (ScopFeaturesOpsCount.cpp)
llvm::Value *numReadsIR(const Scop &S, llvm::IRBuilder<> &B);
llvm::Value *numWritesIR(const Scop &S, llvm::IRBuilder<> &B);
llvm::Value *numReductionsIR(const Scop &S, llvm::IRBuilder<> &B);
llvm::Value *numAluOpsIR(const Scop &S, llvm::IRBuilder<> &B);
llvm::Value *numMulDivOpsIR(const Scop &S, llvm::IRBuilder<> &B);
llvm::Value *numFpOpsIR(const Scop &S, llvm::IRBuilder<> &B);
llvm::Value *numCfOpsIR(const Scop &S, llvm::IRBuilder<> &B);

} // namespace features
} // namespace polly

#endif // POLLY_PROFILING_SCOPFEATURESIMPL_H
