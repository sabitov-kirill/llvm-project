add_llvm_component_group(SystemS)

set(LLVM_TARGET_DEFINITIONS SystemS.td)
tablegen(LLVM SystemSGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM SystemSGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM SystemSGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(SystemSCommonTableGen)

add_llvm_target(SystemSCodeGen
  SystemSTargetMachine.cpp
  SystemSRegisterInfo.cpp
  SystemSInstrInfo.cpp
  SystemSSubtarget.cpp

  LINK_COMPONENTS
  CodeGen
  Core
  Support
  Target
  SystemSInfo

  ADD_TO_COMPONENT
  SystemS
  )

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)