#include "SystemS.h"

#include "clang/Basic/Builtins.h"
#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetBuiltins.h"

using namespace llvm;
using namespace clang;
using namespace clang::targets;

namespace {

static constexpr Builtin::Info BuiltinInfo[] = {

#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::NO_HEADER, ALL_LANGUAGES},
#include "clang/Basic/BuiltinsSystemS.def"

};

} // namespace

SystemSTargetInfo::SystemSTargetInfo(const llvm::Triple &Triple,
                                     const TargetOptions &Opts)
    : TargetInfo(Triple) {
  NoAsmVariants = true;
  LongLongAlign = 32;
  SuitableAlign = 32;
  DoubleAlign = LongDoubleAlign = 32;
  SizeType = UnsignedInt;
  PtrDiffType = SignedInt;
  IntPtrType = SignedInt;
  WCharType = UnsignedChar;
  WIntType = UnsignedInt;

  resetDataLayout("e-m:e-p:32:32-i8:8:32-i16:16:32-i64:64-n32");
}

bool SystemSTargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &Info) const {
  return false;
}

void SystemSTargetInfo::getTargetDefines(const LangOptions &Opts,
                                         MacroBuilder &Builder) const {
  Builder.defineMacro("__systems_");
}

ArrayRef<Builtin::Info> SystemSTargetInfo::getTargetBuiltins() const {
  return ArrayRef(BuiltinInfo,
                  SystemS::LastTSBuiltin - Builtin::FirstTSBuiltin);
}

TargetInfo::BuiltinVaListKind SystemSTargetInfo::getBuiltinVaListKind() const {
  return TargetInfo::VoidPtrBuiltinVaList;
}

std::string_view SystemSTargetInfo::getClobbers() const { return ""; }

ArrayRef<const char *> SystemSTargetInfo::getGCCRegNames() const {
  static const char *const GCCRegNames[] = {
      "r0", "r1", "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};
  return ArrayRef(GCCRegNames);
}

ArrayRef<TargetInfo::GCCRegAlias> SystemSTargetInfo::getGCCRegAliases() const {
  return std::nullopt;
}
