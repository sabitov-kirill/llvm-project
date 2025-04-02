#ifndef LLVM_LIB_TARGET_SYSTEMS_INSTPRINTER_SYSTEMSINSTPRINTER_H
#define LLVM_LIB_TARGET_SYSTEMS_INSTPRINTER_SYSTEMSINSTPRINTER_H

#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCRegister.h"

namespace llvm {

class SystemSInstPrinter : public MCInstPrinter {
public: /* Constructors and destructures */
  SystemSInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                     const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

public: /* Public interface */
  std::pair<const char *, uint64_t>
  getMnemonic(const MCInst &MI) const override;
  static const char *getRegisterName(MCRegister Reg);

  void printRegName(raw_ostream &O, MCRegister Reg) override;
  void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &O) override;

  void printInstruction(const MCInst *MI, uint64_t Address, raw_ostream &O);
  void printOperand(const MCInst *MI, int OpNo, raw_ostream &OS);
  static void printOperand(const MCOperand &MO, raw_ostream &O);
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SYSTEMS_INSTPRINTER_SYSTEMSINSTPRINTER_H