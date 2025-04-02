
#ifndef LLVM_LIB_TARGET_SYSTEMS_SYSTEMSFRAMELOWERING_H
#define LLVM_LIB_TARGET_SYSTEMS_SYSTEMSFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class SystemSSubtarget;

class SystemSFrameLowering : public TargetFrameLowering {
public: /* Constructors and destructors */
  explicit SystemSFrameLowering(const SystemSSubtarget &STI)
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), 0),
        STI(STI) {}

public: /* Public interface */
  void emitPrologue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}

  void emitEpilogue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}

  bool hasFPImpl(const MachineFunction &MF) const override { return false; }

private: /* Private data */
  [[maybe_unused]]
  const SystemSSubtarget &STI;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_SYSTEMS_SYSTEMSFRAMELOWERING_H