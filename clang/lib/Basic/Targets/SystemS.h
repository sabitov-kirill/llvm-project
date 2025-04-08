#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_SYSTEMS_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_SYSTEMS_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"

#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"

namespace clang {

namespace targets {

class LLVM_LIBRARY_VISIBILITY SystemSTargetInfo : public TargetInfo {
public: /* Constructors and destructors */
  SystemSTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts);

public: /* Public API */
  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override;

public: /* Data getters and setters */
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
  ArrayRef<Builtin::Info> getTargetBuiltins() const override;
  BuiltinVaListKind getBuiltinVaListKind() const override;
  std::string_view getClobbers() const override;
  ArrayRef<const char *> getGCCRegNames() const override;
  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override;
};

} // namespace targets

} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_SYSTEMS_H