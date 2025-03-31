#include "SystemSMCTargetDesc.h"
#include "SystemSInfo.h"
#include "SystemSMCAsmInfo.h"
#include "SystemSMCTargetDesc.h"

#include "TargetInfo/SystemSTargetInfo.h"

#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

#define GET_REGINFO_MC_DESC
#include "SystemSGenRegisterInfo.inc"

#define GET_INSTRINFO_MC_DESC
#include "SystemSGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "SystemSGenSubtargetInfo.inc"

namespace {

MCRegisterInfo *createSystemSMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();

  InitSystemSMCRegisterInfo(X, SystemS::R0);

  return X;
}

MCInstrInfo *createSystemSMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();

  InitSystemSMCInstrInfo(X);

  return X;
}

MCSubtargetInfo *createSystemSMCSubtargetInfo(const Triple &TT, StringRef CPU,
                                              StringRef FS) {
  return createSystemSMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

MCAsmInfo *createSystemSMCAsmInfo(const MCRegisterInfo &MRI, const Triple &TT,
                                  const MCTargetOptions &Options) {

  MCAsmInfo *MAI = new SystemSELFMCAsmInfo(TT);
  unsigned SP = MRI.getDwarfRegNum(SystemS::R1, true);
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(nullptr, SP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

} // namespace

extern "C" void LLVMInitializeSystemSTargetMC() {
  Target &TheSystemSTarget = getTheSystemSTarget();
  TargetRegistry::RegisterMCRegInfo(TheSystemSTarget,
                                    createSystemSMCRegisterInfo);
  TargetRegistry::RegisterMCInstrInfo(TheSystemSTarget,
                                      createSystemSMCInstrInfo);
  TargetRegistry::RegisterMCSubtargetInfo(TheSystemSTarget,
                                          createSystemSMCSubtargetInfo);
  RegisterMCAsmInfoFn X(TheSystemSTarget, createSystemSMCAsmInfo);
}