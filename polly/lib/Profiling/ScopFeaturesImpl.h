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

namespace polly {

class Scop;

namespace features {

// Loop-structure features (ScopFeaturesLoop.cpp)
int64_t stmtCount(const Scop &S);
int64_t maxLoopDepth(const Scop &S);
int64_t numParams(const Scop &S);

// Memory-access features (ScopFeaturesMemory.cpp)
int64_t numArrays(const Scop &S);
int64_t numDimensions(const Scop &S);
int64_t numReads(const Scop &S);
int64_t numWrites(const Scop &S);
int64_t numReductions(const Scop &S);

// Trip-count IR codegen (ScopFeaturesTripCount.cpp)
llvm::Value *tripCountIR(const Scop &S, llvm::IRBuilder<> &B,
                         llvm::Instruction *InsertBefore);

// Memory footprint IR codegen (ScopFeaturesFootprint.cpp)
llvm::Value *footprintBytesIR(const Scop &S, llvm::IRBuilder<> &B,
                               llvm::Instruction *InsertBefore);

} // namespace features
} // namespace polly

#endif // POLLY_PROFILING_SCOPFEATURESIMPL_H
