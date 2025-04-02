#ifndef LLVM_LIB_TARGET_SYSTEMS_SIMISELLOWERING_H
#define LLVM_LIB_TARGET_SYSTEMS_SIMISELLOWERING_H

#include "SystemS.h"

#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class SystemSSubtarget;
class SystemSTargetMachine;

namespace SystemSISD {

enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RET,
  CALL,
  BR_CC,
};

} // namespace SystemSISD

class SystemSTargetLowering : public TargetLowering {
public: /* Constructors and destructors */
  explicit SystemSTargetLowering(const TargetMachine &TM,
                                 const SystemSSubtarget &STI);

public: /* Public interface */
  const char *getTargetNodeName(unsigned Opcode) const override;
  SystemSSubtarget const &getSubtarget() const;

  bool isLegalAddressingMode(const DataLayout &DL, const AddrMode &AM, Type *Ty,
                             unsigned AS,
                             Instruction *I = nullptr) const override;

private: /* Private interface */
  void ReplaceNodeResults(SDNode *N, SmallVectorImpl<SDValue> &Results,
                          SelectionDAG &DAG) const override;

  SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;

  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                      bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &ArgsFlags,
                      LLVMContext &Context, const Type *RetTy) const override;

private: /* Private data */
  const SystemSSubtarget &STI;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SYSTEMS_SIMISELLOWERING_H