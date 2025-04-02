#include "SystemSMCTargetDesc.h"
#include "SystemSInfo.h"
#include "SystemSInstPrinter.h"
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

MCInstPrinter *createSystemSMCInstPrinter(const Triple &T,
                                          unsigned SyntaxVariant,
                                          const MCAsmInfo &MAI,
                                          const MCInstrInfo &MII,
                                          const MCRegisterInfo &MRI) {
  return new SystemSInstPrinter(MAI, MII, MRI);
}

} // namespace

extern "C" void LLVMInitializeSystemSTargetMC() {
  Target &TheSystemSTarget = getTheSystemSTarget();

  RegisterMCAsmInfoFn X(TheSystemSTarget, createSystemSMCAsmInfo);

  TargetRegistry::RegisterMCRegInfo(TheSystemSTarget,
                                    createSystemSMCRegisterInfo);
  TargetRegistry::RegisterMCInstrInfo(TheSystemSTarget,
                                      createSystemSMCInstrInfo);
  TargetRegistry::RegisterMCSubtargetInfo(TheSystemSTarget,
                                          createSystemSMCSubtargetInfo);
  TargetRegistry::RegisterMCInstPrinter(TheSystemSTarget,
                                        createSystemSMCInstPrinter);
  TargetRegistry::RegisterMCCodeEmitter(TheSystemSTarget,
                                        createSystemSMCCodeEmitter);
  TargetRegistry::RegisterMCAsmBackend(TheSystemSTarget, createSystemSAsmBackend);
}