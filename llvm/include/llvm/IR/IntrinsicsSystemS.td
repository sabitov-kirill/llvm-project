let TargetPrefix = "systems" in {  // All intrinsics start with "llvm.systems.".

  // Fixed-point math intrinsics
  def int_systems_fpsqrt  : ClangBuiltin<"systemSFPSqrt">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty], 
              [IntrNoMem]>;
  def int_systems_fpsin   : ClangBuiltin<"systemSFPSin">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty], 
              [IntrNoMem]>;
  def int_systems_fpcos   : ClangBuiltin<"systemSFPCos">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty], 
              [IntrNoMem]>;
  def int_systems_fpatan2 : ClangBuiltin<"systemSFPAtan2">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty], 
              [IntrNoMem]>;
  
  // Graphics intrinsics
  def int_systems_putpixel : ClangBuiltin<"systemSPutPixel">,
    Intrinsic<[], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], 
              [IntrHasSideEffects]>;
  def int_systems_flush    : ClangBuiltin<"systemSFlush">,
    Intrinsic<[llvm_i32_ty], [], 
              [IntrHasSideEffects]>;
  
  // Utility intrinsics 
  def int_systems_log       : ClangBuiltin<"systemSLog">,
    Intrinsic<[], [llvm_ptr_ty], 
              [IntrHasSideEffects]>;
  def int_systems_deltatime : ClangBuiltin<"systemSDeltaTime">,
    Intrinsic<[llvm_i32_ty], [], 
              [IntrNoMem]>;

} // TargetPrefix = "systems"