#ifndef LLVM_LIB_TARGET_SYSTEMS_SYSTEMSINSTRINFO_H
#define LLVM_LIB_TARGET_SYSTEMS_SYSTEMSINSTRINFO_H

#include "MCTargetDesc/SystemSInfo.h"
#include "SystemSRegisterInfo.h"

#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "SystemSGenInstrInfo.inc"

namespace llvm {

class SystemSSubtarget;

class SystemSInstrInfo : public SystemSGenInstrInfo {
public: /* Constructors and destructors */
  SystemSInstrInfo();
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SYSTEMS_SYSTEMSINSTRINFO_H