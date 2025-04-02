add_llvm_component_library(LLVMSystemSDesc
  SystemSMCTargetDesc.cpp
  SystemSMCAsmInfo.cpp
  SystemSInstPrinter.cpp
  SystemSMCCodeEmiter.cpp
  SystemSAsmBackend.cpp
  SystemSELFObjectWriter.cpp
  
  LINK_COMPONENTS
  MC
  Support
  SystemSInfo

  ADD_TO_COMPONENT
  SystemS
 )