
#ifndef LLVM_LIB_TARGET_SYSTEMS_SYSTEMSTARGETMACHINE_H
#define LLVM_LIB_TARGET_SYSTEMS_SYSTEMSTARGETMACHINE_H

#include <optional>

#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"

namespace llvm {

extern Target TheSystemSTarget;

class SystemSTargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;

public:
  SystemSTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT, bool isLittle);

  SystemSTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT);

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};
} // end namespace llvm

#endif // LLVM_LIB_TARGET_SYSTEMS_SYSTEMSTARGETMACHINE_H