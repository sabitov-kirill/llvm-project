#ifndef LLVM_LIB_TARGET_SYSTEMS_MCTARGETDESC_SYSTEMSMCASMINFO_H
#define LLVM_LIB_TARGET_SYSTEMS_MCTARGETDESC_SYSTEMSMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {

class Triple;

class SystemSELFMCAsmInfo : public MCAsmInfoELF {
public: /* Constructors and destructures */
  explicit SystemSELFMCAsmInfo(const Triple &TheTriple);
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SYSTEMS_MCTARGETDESC_SYSTEMSMCASMINFO_H