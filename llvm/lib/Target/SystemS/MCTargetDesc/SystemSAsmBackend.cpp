#include "MCTargetDesc/SystemSMCTargetDesc.h"

#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;

namespace {
class SystemSAsmBackend : public MCAsmBackend {
protected:
  const Target &TheTarget;

public:
  SystemSAsmBackend(const Target &T)
      : MCAsmBackend(llvm::endianness::little), TheTarget(T) {}

  unsigned getNumFixupKinds() const override { return 0; }

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override {
    // Cannot emit NOP with size not multiple of 32 bits.
    if (Count % 4 != 0)
      return false;

    uint64_t NumNops = Count / 4;
    for (uint64_t i = 0; i != NumNops; ++i)
      support::endian::write<uint32_t>(OS, 0x01000000, Endian);

    return true;
  }
};

class ELFSystemSAsmBackend : public SystemSAsmBackend {
public:
  ELFSystemSAsmBackend(const Target &T, Triple::OSType OSType)
      : SystemSAsmBackend(T), OSType(OSType) {}

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override {
    return;
  }

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(OSType);
    return createSystemSELFObjectWriter(false, OSABI);
  }

private: /* Private data */
  [[maybe_unused]]
  Triple::OSType OSType;
};

} // end anonymous namespace

MCAsmBackend *llvm::createSystemSAsmBackend(const Target &T,
                                            const MCSubtargetInfo &STI,
                                            const MCRegisterInfo &MRI,
                                            const MCTargetOptions &Options) {
  return new ELFSystemSAsmBackend(T, STI.getTargetTriple().getOS());
}