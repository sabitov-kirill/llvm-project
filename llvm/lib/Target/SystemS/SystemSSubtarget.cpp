#include "SystemSSubtarget.h"
#include "SystemS.h"

#include "llvm/Target/TargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "SystemS-subtarget"
#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "SystemSGenSubtargetInfo.inc"

SystemSSubtarget::SystemSSubtarget(const StringRef &CPU,
                                   const StringRef &TuneCPU,
                                   const StringRef &FS, const TargetMachine &TM)
    : SystemSGenSubtargetInfo(TM.getTargetTriple(), CPU, TuneCPU, FS) {
}