#ifndef LLVM_LIB_TARGET_SYSTEMS_SYSTEMSREGISTERINFO_H
#define LLVM_LIB_TARGET_SYSTEMS_SYSTEMSREGISTERINFO_H

#define GET_REGINFO_HEADER
#include "SystemSGenRegisterInfo.inc"

namespace llvm {

struct SystemSRegisterInfo : public SystemSGenRegisterInfo {

public:
  SystemSRegisterInfo();
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SYSTEMS_SYSTEMSREGISTERINFO_H