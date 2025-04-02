#include "SystemSSubtarget.h"
#include "SystemS.h"

#include "llvm/Target/TargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "SystemS-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "SystemSGenSubtargetInfo.inc"

SystemSSubtarget::SystemSSubtarget(const Triple &TT, const std::string &CPU,
                                   const std::string &FS,
                                   const TargetMachine &TM)
    : SystemSGenSubtargetInfo(TT, CPU, CPU, FS), TLInfo(TM, *this),
      FrameLowering(*this), InstrInfo() {}

const llvm::SystemSTargetLowering *
llvm::SystemSSubtarget::getTargetLowering() const {
  return &TLInfo;
}

const llvm::SystemSFrameLowering *
llvm::SystemSSubtarget::getFrameLowering() const {
  return &FrameLowering;
}

const llvm::SystemSRegisterInfo *
llvm::SystemSSubtarget::getRegisterInfo() const {
  return &RegInfo;
}

const llvm::SystemSInstrInfo *llvm::SystemSSubtarget::getInstrInfo() const {
  return &InstrInfo;
}

const llvm::SelectionDAGTargetInfo *
llvm::SystemSSubtarget::getSelectionDAGInfo() const {
  return &TSInfo;
}
