#ifndef LLVM_LIB_TARGET_SYSTEMS_SYSTEMS_H
#define LLVM_LIB_TARGET_SYSTEMS_SYSTEMS_H

#include "MCTargetDesc/SystemSMCTargetDesc.h"

#include "llvm/Target/TargetMachine.h"

namespace llvm {

class SystemSTargetMachine;
class FunctionPass;
class SystemSSubtarget;
class AsmPrinter;
class InstructionSelector;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineOperand;
class PassRegistry;

bool lowerSystemSMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                      AsmPrinter &AP);
bool LowerSystemSMachineOperandToMCOperand(const MachineOperand &MO,
                                           MCOperand &MCOp,
                                           const AsmPrinter &AP);

FunctionPass *createSystemSISelDag(SystemSTargetMachine &TM,
                                   CodeGenOptLevel OptLevel);

} // namespace llvm

#endif // LLVM_LIB_TARGET_SYSTEMS_SYSTEMS_H