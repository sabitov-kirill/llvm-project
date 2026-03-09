//===------ ScopProfiler.h - Instrument SCoPs for dataset collection ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// ScopProfiler inserts runtime calls around SCoP region boundaries to measure
// execution time and (eventually) dump polyhedral code features for each SCoP
// invocation. It is part of the compiler-assisted-scheduling dataset pipeline.
//
// Differences from PerfMonitor:
//
//   PerfMonitor:
//     - Only usable when code generation is enabled (instruments generated
//     code)
//     - Accumulates totals in globals and prints a summary on program exit
//     - x86_64 only (uses RDTSCP intrinsic)
//
//   ScopProfiler:
//     - Can instrument BOTH the original (unoptimized) and the codegen'd
//       (optimized) code paths, controlled by the same pair of methods;
//       the caller selects the appropriate insertion point for each case
//     - Calls a runtime library function on each SCoP invocation, enabling
//       per-invocation dataset collection instead of a final summary
//     - Designed to eventually emit polyhedral features (trip counts, working
//       set sizes, reuse distance histograms) alongside timing
//
// ---- Insertion points ----
//
// For the ORIGINAL (unoptimized) SCoP (called from PhaseManager after ScopInfo,
// before any optimization or code generation):
//
//   ScopProfiler P(S, M);
//   P.initialize();
//   // Start: first non-PHI instruction of the SCoP entry block.
//   // PHI nodes at the entry must not be split; getFirstNonPHIIt() skips them.
//   P.insertScopStart(&*S.getEntry()->getFirstNonPHIIt());
//   // End: terminator of the unique exiting block (nullptr → multi-exit,
//   skip). if (BasicBlock *ExitingBB = S.getExitingBlock())
//     P.insertScopEnd(ExitingBB->getTerminator());
//
// For the CODEGEN'd SCoP (called from CodeGeneration.cpp after
// executeScopConditionally()), each path is instrumented separately so the
// runtime can record which variant executed:
//
//   Module *Mod = EnteringBB->getParent()->getParent();
//   // Optimized path
//   {
//     ScopProfiler P(S, Mod, "opt");
//     P.initialize();
//     P.insertScopStart(&*StartBlock->getFirstNonPHIIt());
//     P.insertScopEnd(ExitBlock->getTerminator());
//   }
//   // Original fallback path (skip for multi-exit SCoPs)
//   if (BasicBlock *ExitingBB = S.getExitingBlock()) {
//     ScopProfiler P(S, Mod, "orig");
//     P.initialize();
//     P.insertScopStart(&*S.getEntry()->getFirstNonPHIIt());
//     P.insertScopEnd(ExitingBB->getTerminator());
//   }
//
//===----------------------------------------------------------------------===//

#ifndef POLLY_PROFILING_SCOPPROFILE_H
#define POLLY_PROFILING_SCOPPROFILE_H

#include "llvm/IR/IRBuilder.h"
#include <string>

namespace llvm {
class Function;
class Instruction;
class Module;
class Value;
} // namespace llvm

namespace polly {

class Scop;
class ScopDetection;
class ScopInfo;

/// Instruments a single SCoP with calls to the CAS (compiler-assisted-
/// scheduling) profiling runtime library.
///
/// One instance of ScopProfiler is created per SCoP per compilation mode.
/// It emits a global string constant identifying the SCoP and inserts calls
/// to __cas_scop_start / __cas_scop_end around the region boundaries.
///
/// The caller is responsible for choosing the insertion points — see the file
/// header for the two supported usage patterns (original vs. codegen'd SCoP).
class ScopProfiler {
public:
  /// \param S       The SCoP to instrument.
  /// \param M       The LLVM module being compiled (used to declare runtime
  ///                function prototypes and to create global constants).
  /// \param Suffix  Optional path suffix appended to the SCoP ID string,
  ///                e.g. "opt" or "orig" to distinguish the two code paths
  ///                produced by executeScopConditionally().
  ScopProfiler(const Scop &S, llvm::Module *M, llvm::StringRef Suffix = "");

  void initialize();
  void insertScopStart(llvm::Instruction *InsertBefore);
  void insertScopEnd(llvm::Instruction *InsertBefore);

private:
  llvm::Module *M;
  llvm::IRBuilder<> Builder;
  const Scop &S;
  std::string Suffix;

  /// Identifies this SCoP in the runtime output: "<func>:<entry>:<exit>"
  /// or "<func>:<entry>:<exit>:<suffix>" when Suffix is non-empty.
  /// Set by initialize().
  std::string ScopIDStr;

  /// Global i8* constant holding ScopIDStr. Set by initialize().
  llvm::Value *ScopIDGlobal = nullptr;

  llvm::Function *getScopInitFn();
  llvm::Function *getScopStartFn();
  llvm::Function *getScopEndFn();

  /// Create a WeakODR wrapper that calls __cas_scop_init() and register it in
  /// llvm.global_ctors. Idempotent — only adds the entry once per module.
  void insertGlobalInit();
  void addToGlobalConstructors(llvm::Function *Fn);
};

/// True when SCoP profiling instrumentation is enabled (set by the
/// -polly-profile-scops command-line flag). Used by CodeGeneration.cpp to
/// instrument the codegen'd (optimized) code path in addition to the original
/// SCoP instrumentation driven by PassPhase::ScopProfiling.
extern bool ScopProfilingEnabled;

/// Instrument \p S with start/end profiling calls around the ORIGINAL
/// (pre-optimization) SCoP region boundaries. Called from PhaseManager for
/// each max-region SCoP when PassPhase::ScopProfiling is enabled.
void runScopProfiling(Scop &S, llvm::Module &M);

} // namespace polly

#endif // POLLY_PROFILING_SCOPPROFILE_H
