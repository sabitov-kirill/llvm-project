add_llvm_component_group(SystemS)

add_llvm_target(SystemSCodeGen
  SystemSTargetMachine.cpp

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