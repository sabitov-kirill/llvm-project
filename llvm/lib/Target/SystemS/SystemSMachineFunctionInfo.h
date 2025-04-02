#ifndef LLVM_LIB_TARGET_SIM_SIMMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_SIM_SIMMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

class SystemSFunctionInfo : public MachineFunctionInfo {
public: /* Constructors and destructor */
  SystemSFunctionInfo() {}
  explicit SystemSFunctionInfo(MachineFunction &MF) {}

  ~SystemSFunctionInfo() {}

public: /* Public interface */
  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Off) { VarArgsFrameIndex = Off; }

  int getVarArgsSaveSize() const { return VarArgsSaveSize; }
  void setVarArgsSaveSize(int Size) { VarArgsSaveSize = Size; }

  unsigned getCalleeSavedStackSize() const { return CalleeSavedStackSize; }
  void setCalleeSavedStackSize(unsigned Size) { CalleeSavedStackSize = Size; }

  unsigned getReturnStackOffset() const {
    assert(ReturnStackOffsetSet && "Return stack offset not set");
    return ReturnStackOffset;
  }
  void setReturnStackOffset(unsigned Off) {
    assert(!ReturnStackOffsetSet && "Return stack offset set twice");
    ReturnStackOffset = Off;
    ReturnStackOffsetSet = true;
  }

private: /* Private interface */
  virtual void anchor();

private: /* Private data */
  bool ReturnStackOffsetSet = false;
  unsigned ReturnStackOffset = -1U;
  int VarArgsFrameIndex = 0;
  int VarArgsSaveSize = 0;
  unsigned CalleeSavedStackSize = 0;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SIM_SIMMACHINEFUNCTIONINFO_H