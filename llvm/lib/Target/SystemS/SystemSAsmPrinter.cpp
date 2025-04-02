#include "SystemS.h"
#include "TargetInfo/SystemSTargetInfo.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "SystemS-asm-printer"

namespace {

class SystemSAsmPrinter : public AsmPrinter {
  const MCSubtargetInfo *STI;

public:
  explicit SystemSAsmPrinter(TargetMachine &TM,
                             std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)), STI(TM.getMCSubtargetInfo()) {}

  StringRef getPassName() const override { return "SystemS Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override;
  bool lowerPseudoInstExpansion(const MachineInstr *MI, MCInst &Inst);
  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp) const;
};

} // end anonymous namespace

#include "SystemSGenMCPseudoLowering.inc"

void SystemSAsmPrinter::emitInstruction(const MachineInstr *MI) {
  if (MCInst OutInst; lowerPseudoInstExpansion(MI, OutInst)) {
    EmitToStreamer(*OutStreamer, OutInst);
    return;
  }

  MCInst TmpInst;
  if (!lowerSystemSMachineInstrToMCInst(MI, TmpInst, *this)) {
    EmitToStreamer(*OutStreamer, TmpInst);
  }
}

bool SystemSAsmPrinter::lowerOperand(const MachineOperand &MO,
                                     MCOperand &MCOp) const {
  return LowerSystemSMachineOperandToMCOperand(MO, MCOp, *this);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSystemSAsmPrinter() {
  RegisterAsmPrinter<SystemSAsmPrinter> X(getTheSystemSTarget());
}