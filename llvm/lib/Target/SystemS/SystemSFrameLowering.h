
#ifndef LLVM_LIB_TARGET_SYSTEMS_SYSTEMSFRAMELOWERING_H
#define LLVM_LIB_TARGET_SYSTEMS_SYSTEMSFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class SystemSFrameLowering : public TargetFrameLowering {

public:
  explicit SystemSFrameLowering()
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), 0) {
  }

  void emitPrologue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}

  void emitEpilogue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}

  bool hasFPImpl(const MachineFunction &MF) const override { return false; }
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_SYSTEMS_SYSTEMSFRAMELOWERING_H