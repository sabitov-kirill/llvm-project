add_llvm_component_library(LLVMSystemSDesc
  SystemSMCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  Support
  SystemSInfo

  ADD_TO_COMPONENT
  SystemS
 )