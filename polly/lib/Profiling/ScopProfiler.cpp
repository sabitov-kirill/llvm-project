//===------ ScopProfiler.cpp - Instrument SCoPs for dataset collection ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "polly/Profiling/ScopProfiler.h"
#include "polly/Profiling/ScopFeatures.h"
#include "polly/ScopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "polly/Support/PollyDebug.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Module.h"

using namespace llvm;
using namespace polly;

#define DEBUG_TYPE "polly-scop-profiling"

bool polly::ScopProfilingEnabled = false;

ScopProfiler::ScopProfiler(const Scop &S, Module *M, StringRef Suffix)
    : M(M), Builder(M->getContext()), S(S), Suffix(Suffix.str()) {}

void ScopProfiler::initialize() {
  std::string EntryStr, ExitStr;
  std::tie(EntryStr, ExitStr) = S.getEntryExitStr();

  ScopIDStr = (Twine(S.getFunction().getName()) + ":" + EntryStr + ":" +
               ExitStr + (Suffix.empty() ? "" : ":" + Suffix))
                  .str();

  std::string GlobalName =
      (Twine("__cas_scop_id_") + S.getFunction().getName() + "_" + EntryStr +
       "_" + ExitStr + (Suffix.empty() ? "" : "_" + Suffix))
          .str();

  ScopIDGlobal = Builder.CreateGlobalString(ScopIDStr, GlobalName, 0, M);

  insertGlobalInit();

  POLLY_DEBUG(dbgs() << "[ScopProfiler] Initialized SCoP ID: " << ScopIDStr
                     << "\n");
}

// ---------------------------------------------------------------------------
// Runtime function declarations
// ---------------------------------------------------------------------------

Function *ScopProfiler::getScopInitFn() {
  const char *Name = "__cas_scop_init";
  Function *F = M->getFunction(Name);
  if (!F) {
    FunctionType *Ty = FunctionType::get(Builder.getVoidTy(), {}, false);
    F = Function::Create(Ty, Function::ExternalLinkage, Name, M);
  }
  return F;
}

Function *ScopProfiler::getScopStartFn() {
  const char *Name = "__cas_scop_start";
  Function *F = M->getFunction(Name);
  if (!F) {
    // (ptr scop_id, ptr features, i64 num_features) -> void
    FunctionType *Ty = FunctionType::get(
        Builder.getVoidTy(),
        {Builder.getPtrTy(), Builder.getPtrTy(), Builder.getInt64Ty()}, false);
    F = Function::Create(Ty, Function::ExternalLinkage, Name, M);
  }
  return F;
}

Function *ScopProfiler::getScopEndFn() {
  const char *Name = "__cas_scop_end";
  Function *F = M->getFunction(Name);
  if (!F) {
    FunctionType *Ty =
        FunctionType::get(Builder.getVoidTy(), {Builder.getPtrTy()}, false);
    F = Function::Create(Ty, Function::ExternalLinkage, Name, M);
  }
  return F;
}

// ---------------------------------------------------------------------------
// Global constructor wiring (calls __cas_scop_init once at program startup)
// ---------------------------------------------------------------------------

// Tracks whether we already inserted the global-ctor entry for this module.
// Same pattern as PerfMonitor's static FinalReporting pointer.
static Function *InitFnInserted = nullptr;

void ScopProfiler::addToGlobalConstructors(Function *Fn) {
  const char *Name = "llvm.global_ctors";
  GlobalVariable *GV = M->getGlobalVariable(Name);
  std::vector<Constant *> V;

  if (GV) {
    Constant *Array = GV->getInitializer();
    for (Value *X : Array->operand_values())
      V.push_back(cast<Constant>(X));
    GV->eraseFromParent();
  }

  StructType *ST =
      StructType::get(Builder.getInt32Ty(), Fn->getType(), Builder.getPtrTy());
  V.push_back(
      ConstantStruct::get(ST, Builder.getInt32(10), Fn,
                          ConstantPointerNull::get(Builder.getPtrTy())));
  ArrayType *Ty = ArrayType::get(ST, V.size());
  new GlobalVariable(*M, Ty, true, GlobalValue::AppendingLinkage,
                     ConstantArray::get(Ty, V), Name, nullptr,
                     GlobalVariable::NotThreadLocal);
}

void ScopProfiler::insertGlobalInit() {
  if (InitFnInserted)
    return;

  // WeakODR linkage: if multiple TUs are compiled with profiling and then
  // linked, the linker keeps exactly one copy of this wrapper.
  FunctionType *Ty = FunctionType::get(Builder.getVoidTy(), {}, false);
  Function *Fn = Function::Create(Ty, Function::WeakODRLinkage,
                                  "__cas_scop_profiler_init", M);
  BasicBlock *BB = BasicBlock::Create(M->getContext(), "entry", Fn);
  Builder.SetInsertPoint(BB);
  Builder.CreateCall(getScopInitFn(), {});
  Builder.CreateRetVoid();

  addToGlobalConstructors(Fn);
  InitFnInserted = Fn;
}

// ---------------------------------------------------------------------------
// Instrumentation
// ---------------------------------------------------------------------------

void ScopProfiler::insertScopStart(Instruction *InsertBefore) {
  assert(ScopIDGlobal && "Call initialize() before insertScopStart()");
  POLLY_DEBUG(dbgs() << "[ScopProfiler] insertScopStart for " << ScopIDStr
                     << "\n");

  Builder.SetInsertPoint(InsertBefore->getIterator());
  auto FVals = computeScopFeaturesIR(S, Builder, InsertBefore);

  // Alloca at function entry so the array doesn't grow on every loop iteration.
  IRBuilder<> AllocaB(
      &*InsertBefore->getFunction()->getEntryBlock().getFirstInsertionPt());
  auto *ArrTy = ArrayType::get(Builder.getInt64Ty(), NumScopFeatures);
  auto *Arr = AllocaB.CreateAlloca(ArrTy, nullptr, "scop_feats");

  // Store each feature into the array (insert point is before InsertBefore).
  Builder.SetInsertPoint(InsertBefore->getIterator());
  for (unsigned I = 0; I < NumScopFeatures; ++I)
    Builder.CreateStore(FVals[I],
                        Builder.CreateConstGEP2_32(ArrTy, Arr, 0, I));

  // Arr is already a ptr to the first element in LLVM's opaque-pointer model.
  Builder.CreateCall(getScopStartFn(),
                     {ScopIDGlobal, Arr, Builder.getInt64(NumScopFeatures)});
}

void ScopProfiler::insertScopEnd(Instruction *InsertBefore) {
  assert(ScopIDGlobal && "Call initialize() before insertScopEnd()");
  POLLY_DEBUG(dbgs() << "[ScopProfiler] insertScopEnd for " << ScopIDStr
                     << "\n");
  Builder.SetInsertPoint(InsertBefore->getIterator());
  Builder.CreateCall(getScopEndFn(), {ScopIDGlobal});
}

void polly::runScopProfiling(Scop &S, Module &M) {
  POLLY_DEBUG(dbgs() << "[ScopProfiler] Instrumenting original SCoP: "
                     << S.getNameStr() << "\n");

  ScopProfiler P(S, &M);
  P.initialize();

  BasicBlock *EntryBB = S.getEntry();
  P.insertScopStart(&*EntryBB->getFirstNonPHIIt());

  // getExitingBlock() returns nullptr for multi-exit SCoPs; skip end call.
  BasicBlock *ExitingBB = S.getExitingBlock();
  if (!ExitingBB) {
    POLLY_DEBUG(dbgs() << "[ScopProfiler] Skipping multi-exit SCoP: "
                       << S.getNameStr() << "\n");
    return;
  }
  P.insertScopEnd(ExitingBB->getTerminator());
}
