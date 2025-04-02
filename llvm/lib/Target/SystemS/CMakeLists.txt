add_llvm_component_group(SystemS)

set(LLVM_TARGET_DEFINITIONS SystemS.td)
tablegen(LLVM SystemSGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM SystemSGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM SystemSGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM SystemSGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM SystemSGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM SystemSGenCallingConv.inc -gen-callingconv)
tablegen(LLVM SystemSGenAsmWriter.inc -gen-asm-writer)
add_public_tablegen_target(SystemSCommonTableGen)

add_llvm_target(SystemSCodeGen
  SystemSTargetMachine.cpp
  SystemSRegisterInfo.cpp
  SystemSInstrInfo.cpp
  SystemSSubtarget.cpp
  SystemSAsmPrinter.cpp
  SystemSISelDagToDag.cpp
  SystemSISelLowering.cpp
  SystemSMCInstLower.cpp
  
  LINK_COMPONENTS
  CodeGen
  Core
  Support
  Target
  SystemSInfo
  AsmPrinter
  SelectionDAG

  ADD_TO_COMPONENT
  SystemS
  )

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)