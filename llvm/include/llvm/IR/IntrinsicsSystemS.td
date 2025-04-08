let TargetPrefix = "systems" in {  // All intrinsics start with "llvm.systems.".

  // Fixed-point math intrinsics
  def int_systems_fpsqrt  : Intrinsic<[llvm_i32_ty], [llvm_i32_ty], 
                                      [IntrNoMem]>;
  def int_systems_fpsin   : Intrinsic<[llvm_i32_ty], [llvm_i32_ty], 
                                      [IntrNoMem]>;
  def int_systems_fpcos   : Intrinsic<[llvm_i32_ty], [llvm_i32_ty], 
                                      [IntrNoMem]>;
  def int_systems_fpatan2 : Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty], 
                                      [IntrNoMem]>;
  
  // Graphics intrinsics
  def int_systems_putpixel : Intrinsic<[], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty], 
                                       [IntrHasSideEffects]>;
  def int_systems_flush    : Intrinsic<[llvm_i32_ty], [], 
                                       [IntrHasSideEffects]>;
  
  // Utility intrinsics 
  def int_systems_log       : Intrinsic<[], [llvm_anyptr_ty], 
                                        [IntrHasSideEffects]>;
  def int_systems_deltatime : Intrinsic<[llvm_i32_ty], [], 
                                        [IntrNoMem]>;

} // TargetPrefix = "systems"