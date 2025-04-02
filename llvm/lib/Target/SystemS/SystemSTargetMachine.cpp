#include <memory>
#include <optional>

#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"

#include "SystemSTargetMachine.h"
#include "TargetInfo/SystemSTargetInfo.h"

using namespace llvm;

namespace {

std::string computeDataLayout(const Triple &TT, StringRef CPU,
                              const TargetOptions &Options, bool IsLittle) {
  std::string Ret = "e-m:e-p:32:32-i8:8:32-i16:16:32-i64:64-n32";
  return Ret;
}

Reloc::Model getEffectiveRelocModel(bool JIT, std::optional<Reloc::Model> RM) {
  if (!RM || JIT) {
    return Reloc::Static;
  }
  return *RM;
}

class SystemSPassConfig : public TargetPassConfig {
public:
  SystemSPassConfig(SystemSTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  bool addInstSelector() override { return false; }
};

} // namespace

SystemSTargetMachine::SystemSTargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           std::optional<Reloc::Model> RM,
                                           std::optional<CodeModel::Model> CM,
                                           CodeGenOptLevel OL, bool JIT,
                                           bool IsLittle)
    : CodeGenTargetMachineImpl(T, computeDataLayout(TT, CPU, Options, IsLittle),
                               TT, CPU, FS, Options,
                               getEffectiveRelocModel(JIT, RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
}

SystemSTargetMachine::SystemSTargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           std::optional<Reloc::Model> RM,
                                           std::optional<CodeModel::Model> CM,
                                           CodeGenOptLevel OL, bool JIT)
    : SystemSTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT, true) {}

TargetPassConfig *SystemSTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new SystemSPassConfig(*this, PM);
}

TargetLoweringObjectFile *SystemSTargetMachine::getObjFileLowering() const {
  return TLOF.get();
}

const llvm::SystemSSubtarget *
llvm::SystemSTargetMachine::getSubtargetImpl(const Function &) const {
  return &Subtarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSystemSTarget() {
  RegisterTargetMachine<SystemSTargetMachine> A(getTheSystemSTarget());
}