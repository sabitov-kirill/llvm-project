#include "SystemS.h"

#include "clang/Basic/Builtins.h"
#include "clang/Basic/MacroBuilder.h"

using namespace clang;
using namespace clang::targets;

clang::targets::SystemSTargetInfo::SystemSTargetInfo(const llvm::Triple &Triple,
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

bool clang::targets::SystemSTargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &Info) const {
  return false;
}

void SystemSTargetInfo::getTargetDefines(const LangOptions &Opts,

                                         MacroBuilder &Builder) const {
  Builder.defineMacro("__systems_");
}

ArrayRef<Builtin::Info> SystemSTargetInfo::getTargetBuiltins() const {
  return std::nullopt;
}

clang::TargetInfo::BuiltinVaListKind
clang::targets::SystemSTargetInfo::getBuiltinVaListKind() const {
  return TargetInfo::VoidPtrBuiltinVaList;
}

std::string_view clang::targets::SystemSTargetInfo::getClobbers() const {
  return "";
}

ArrayRef<const char *>
clang::targets::SystemSTargetInfo::getGCCRegNames() const {
  static const char *const GCCRegNames[] = {
      "r0", "r1", "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};
  return llvm::ArrayRef(GCCRegNames);
}

ArrayRef<TargetInfo::GCCRegAlias>
clang::targets::SystemSTargetInfo::getGCCRegAliases() const {
  return std::nullopt;
}
