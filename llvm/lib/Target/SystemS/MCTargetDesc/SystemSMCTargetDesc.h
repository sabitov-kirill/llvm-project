#ifndef LLVM_LIB_TARGET_SYSTEMS_MCTARGETDESC_SYSTEMSMCTARGETDESC_H
#define LLVM_LIB_TARGET_SYSTEMS_MCTARGETDESC_SYSTEMSMCTARGETDESC_H

#define GET_REGINFO_ENUM
#include "SystemSGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "SystemSGenInstrInfo.inc"

#include <memory>

namespace llvm {

class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCAsmBackend;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;

MCCodeEmitter *createSystemSMCCodeEmitter(const MCInstrInfo &MCII,
                                          MCContext &Ctx);
MCAsmBackend *createSystemSAsmBackend(const Target &T,
                                      const MCSubtargetInfo &STI,
                                      const MCRegisterInfo &MRI,
                                      const MCTargetOptions &Options);
std::unique_ptr<MCObjectTargetWriter>
createSystemSELFObjectWriter(bool Is64Bit, uint8_t OSABI);

} // namespace llvm

#endif // LLVM_LIB_TARGET_SYSTEMS_MCTARGETDESC_SYSTEMSMCTARGETDESC_H