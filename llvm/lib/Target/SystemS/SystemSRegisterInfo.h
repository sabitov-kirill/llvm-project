#ifndef LLVM_LIB_TARGET_SYSTEMS_SYSTEMSREGISTERINFO_H
#define LLVM_LIB_TARGET_SYSTEMS_SYSTEMSREGISTERINFO_H

#define GET_REGINFO_HEADER
#include "SystemSGenRegisterInfo.inc"

namespace llvm {

struct SystemSRegisterInfo : public SystemSGenRegisterInfo {
public: /* Constructors and destructors */
  SystemSRegisterInfo();

public: /* Public interface */
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;
  Register getFrameRegister(const MachineFunction &MF) const override;

  bool requiresRegisterScavenging(const MachineFunction &MF) const override;
  bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SYSTEMS_SYSTEMSREGISTERINFO_H