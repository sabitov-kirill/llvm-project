//===--- ScopFeaturesLoop.cpp - Loop-structure SCoP features --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ScopFeaturesImpl.h"
#include "polly/ScopInfo.h"

using namespace polly;

int64_t polly::features::stmtCount(const Scop &S) {
  return (int64_t)S.getSize();
}

int64_t polly::features::maxLoopDepth(const Scop &S) {
  return (int64_t)S.getMaxLoopDepth();
}

int64_t polly::features::numParams(const Scop &S) {
  return (int64_t)S.getNumParams();
}
