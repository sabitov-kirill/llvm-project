#ifndef LLVM_LIB_TARGET_SYSTEMS_SYSTEMSTARGETMACHINE_H
#define LLVM_LIB_TARGET_SYSTEMS_SYSTEMSTARGETMACHINE_H

#include "SystemSSubtarget.h"

#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"

#include <optional>

namespace llvm {

extern Target TheSystemSTarget;

class SystemSTargetMachine : public CodeGenTargetMachineImpl {
public: /* Constructos and destructors */
  SystemSTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT, bool IsLittle);

  SystemSTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT);

public: /* Public interface */
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override;
  const SystemSSubtarget *getSubtargetImpl(const Function &) const override;

private: /* Private data */
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  SystemSSubtarget Subtarget;
};
} // end namespace llvm

#endif // LLVM_LIB_TARGET_SYSTEMS_SYSTEMSTARGETMACHINE_H