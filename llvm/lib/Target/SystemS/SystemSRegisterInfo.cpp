#include "SystemSRegisterInfo.h"
#include "MCTargetDesc/SystemSMCTargetDesc.h"
#include "SystemSFrameLowering.h"

#include "llvm/CodeGen/TargetInstrInfo.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "SystemSGenRegisterInfo.inc"

SystemSRegisterInfo::SystemSRegisterInfo()
    : SystemSGenRegisterInfo(SystemS::R0) {}

const MCPhysReg *
SystemSRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SystemS_SaveList;
}

BitVector
SystemSRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  SystemSFrameLowering const *TFI = getFrameLowering(MF);
  BitVector Reserved(getNumRegs());

  Reserved.set(SystemS::R1);
  if (TFI->hasFP(MF)) {
    Reserved.set(SystemS::R2);
  }

  return Reserved;
}

bool SystemSRegisterInfo::requiresRegisterScavenging(
    const MachineFunction &MF) const {
  return false;
}

bool SystemSRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                              int SPAdj, unsigned FIOperandNum,
                                              RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected non-zero SPAdj value");

  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  DebugLoc DL = MI.getDebugLoc();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  Register FrameReg;

  int Offset = getFrameLowering(MF)
                   ->getFrameIndexReference(MF, FrameIndex, FrameReg)
                   .getFixed();

  Offset += MI.getOperand(FIOperandNum + 1).getImm();
  if (!isInt<16>(Offset)) {
    llvm_unreachable("incorrect offset");
  }
  MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, false, false, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);

  return false;
}

Register
SystemSRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = getFrameLowering(MF);
  return TFI->hasFP(MF) ? SystemS::R2 : SystemS::R1;
}

const uint32_t *
SystemSRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                          CallingConv::ID CC) const {
  return CSR_SystemS_RegMask;
}