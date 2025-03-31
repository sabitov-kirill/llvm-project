#include "SystemSRegisterInfo.h"
#include "MCTargetDesc/SystemSMCTargetDesc.h"
#include "SystemSFrameLowering.h"

#include "llvm/CodeGen/TargetInstrInfo.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "SystemSGenRegisterInfo.inc"

SystemSRegisterInfo::SystemSRegisterInfo()
    : SystemSGenRegisterInfo(SystemS::R0) {}