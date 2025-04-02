#include "SystemS.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/ErrorHandling.h"

#include <cassert>
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "SystemS-code-emitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");

namespace {

class SystemSMCCodeEmitter : public MCCodeEmitter {
  MCContext &Ctx;

public:
  SystemSMCCodeEmitter(const MCInstrInfo &, MCContext &ctx) : Ctx(ctx) {}
  SystemSMCCodeEmitter(const SystemSMCCodeEmitter &) = delete;
  SystemSMCCodeEmitter &operator=(const SystemSMCCodeEmitter &) = delete;
  ~SystemSMCCodeEmitter() override = default;

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// getMachineOpValue - Return binary encoding of operand. If the machine
  /// operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  unsigned getSImm16OpValue(const MCInst &MI, unsigned OpNo,
                            SmallVectorImpl<MCFixup> &Fixups,
                            const MCSubtargetInfo &STI) const;
};

} // end anonymous namespace

void SystemSMCCodeEmitter::encodeInstruction(const MCInst &MI,
                                             SmallVectorImpl<char> &CB,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  unsigned Bits = getBinaryCodeForInstr(MI, Fixups, STI);
  support::endian::write(CB, Bits, llvm::endianness::little);

  ++MCNumEmitted; // Keep track of the # of mi's emitted.
}

unsigned
SystemSMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  if (MO.isReg()) {
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
  }
  if (MO.isImm()) {
    return MO.getImm();
  }

  assert(MO.isExpr());
  const MCExpr *Expr = MO.getExpr();

  int64_t Res;
  if (Expr->evaluateAsAbsolute(Res)) {
    return Res;
  }

  llvm_unreachable("Unhandled expression!");
  return 0;
}

unsigned
SystemSMCCodeEmitter::getSImm16OpValue(const MCInst &MI, unsigned OpNo,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isImm()) {
    return MO.getImm();
  }

  assert(MO.isExpr() &&
         "getSImm16OpValue expects only expressions or an immediate");

  const MCExpr *Expr = MO.getExpr();

  // Constant value, no fixup is needed
  if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr)) {
    return CE->getValue();
  }
  return 0;
}

#include "SystemSGenMCCodeEmitter.inc"

MCCodeEmitter *llvm::createSystemSMCCodeEmitter(const MCInstrInfo &MCII,
                                                MCContext &Ctx) {
  return new SystemSMCCodeEmitter(MCII, Ctx);
}