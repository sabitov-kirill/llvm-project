add_llvm_component_library(LLVMSystemSInfo
  SystemSTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  SystemS
  )