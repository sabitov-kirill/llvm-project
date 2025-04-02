#include "SystemS.h"
#include "SystemSSubtarget.h"

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

static MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym,
                                    const AsmPrinter &AP) {
  MCContext &Ctx = AP.OutContext;

  const MCExpr *ME =
      MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, Ctx);

  if (!MO.isJTI() && !MO.isMBB() && MO.getOffset()) {
    ME = MCBinaryExpr::createAdd(
        ME, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);
  }

  return MCOperand::createExpr(ME);
}

bool llvm::LowerSystemSMachineOperandToMCOperand(const MachineOperand &MO,
                                                 MCOperand &MCOp,
                                                 const AsmPrinter &AP) {

  switch (MO.getType()) {
  default:
    report_fatal_error(
        "LowerSystemSMachineInstrToMCInst: unknown operand type");

  case MachineOperand::MO_Register:
    if (MO.isImplicit()) {
      return false;
    }

    MCOp = MCOperand::createReg(MO.getReg());
    break;

  case MachineOperand::MO_RegisterMask:
    return false;

  case MachineOperand::MO_Immediate:
    MCOp = MCOperand::createImm(MO.getImm());
    break;

  case MachineOperand::MO_MachineBasicBlock:
    MCOp = lowerSymbolOperand(MO, MO.getMBB()->getSymbol(), AP);
    break;

  case MachineOperand::MO_GlobalAddress:
    MCOp = lowerSymbolOperand(MO, AP.getSymbolPreferLocal(*MO.getGlobal()), AP);
    break;

  case MachineOperand::MO_BlockAddress:
    MCOp = lowerSymbolOperand(
        MO, AP.GetBlockAddressSymbol(MO.getBlockAddress()), AP);
    break;

  case MachineOperand::MO_ExternalSymbol:
    MCOp = lowerSymbolOperand(
        MO, AP.GetExternalSymbolSymbol(MO.getSymbolName()), AP);
    break;

  case MachineOperand::MO_ConstantPoolIndex:
    MCOp = lowerSymbolOperand(MO, AP.GetCPISymbol(MO.getIndex()), AP);
    break;

  case MachineOperand::MO_JumpTableIndex:
    MCOp = lowerSymbolOperand(MO, AP.GetJTISymbol(MO.getIndex()), AP);
    break;
  }

  return true;
}

bool llvm::lowerSystemSMachineInstrToMCInst(const MachineInstr *MI,
                                            MCInst &OutMI, AsmPrinter &AP) {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    if (LowerSystemSMachineOperandToMCOperand(MO, MCOp, AP)) {
      OutMI.addOperand(MCOp);
    }
  }

  return false;
}