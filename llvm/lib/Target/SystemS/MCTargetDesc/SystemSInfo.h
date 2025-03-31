#ifndef LLVM_LIB_TARGET_SYSTEMS_MCTARGETDESC_SYSTEMSINFO_H
#define LLVM_LIB_TARGET_SYSTEMS_MCTARGETDESC_SYSTEMSINFO_H

#include "llvm/MC/MCInstrDesc.h"

namespace llvm {

namespace SystemSOp {

enum OperandType : unsigned {
  OPERAND_SYSTEMSM16 = MCOI::OPERAND_FIRST_TARGET,
};

} // namespace SystemSOp

} // end namespace llvm

#endif