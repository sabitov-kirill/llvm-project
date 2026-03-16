//===--- ScopFeaturesMemory.cpp - Memory-access SCoP features -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ScopFeaturesImpl.h"
#include "polly/ScopInfo.h"

using namespace polly;

int64_t polly::features::numArrays(const Scop &S) {
  int64_t N = 0;
  for (auto &SAI : S.arrays())
    (void)SAI, ++N;
  return N;
}

int64_t polly::features::numDimensions(const Scop &S) {
  int64_t N = 0;
  for (auto &SAI : S.arrays())
    N += (int64_t)SAI->getNumberOfDimensions();
  return N;
}

int64_t polly::features::numReads(const Scop &S) {
  int64_t N = 0;
  for (auto &Stmt : S)
    for (auto *MA : Stmt)
      if (MA->isRead())
        ++N;
  return N;
}

int64_t polly::features::numWrites(const Scop &S) {
  int64_t N = 0;
  for (auto &Stmt : S)
    for (auto *MA : Stmt)
      if (MA->isWrite())
        ++N;
  return N;
}

int64_t polly::features::numReductions(const Scop &S) {
  int64_t N = 0;
  for (auto &Stmt : S)
    for (auto *MA : Stmt)
      if (MA->isReductionLike())
        ++N;
  return N;
}
