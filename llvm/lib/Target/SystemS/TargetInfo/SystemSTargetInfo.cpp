#include "llvm/MC/TargetRegistry.h"

#include "SystemSTargetInfo.h"

using namespace llvm;

Target &llvm::getTheSystemSTarget() {
  static Target TheSystemSTarget;
  return TheSystemSTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSystemSTargetInfo() {
  RegisterTarget<Triple::systems> X(getTheSystemSTarget(), "systems",
                                    "System Simulator", "SYSTEMS");
}