; RUN: llc < %s -mtriple=systems | FileCheck %s

define i32 @test_fpsqrt(i32 %a) nounwind {
; CHECK-LABEL: test_fpsqrt:
; CHECK: fpsqrt r{{[0-9]+}}, r{{[0-9]+}}
  %1 = call i32 @llvm.systems.fpsqrt(i32 %a)
  ret i32 %1
}

define i32 @test_fpsin(i32 %a) nounwind {
; CHECK-LABEL: test_fpsin:
; CHECK: fpsin r{{[0-9]+}}, r{{[0-9]+}}
  %1 = call i32 @llvm.systems.fpsin(i32 %a)
  ret i32 %1
}

define i32 @test_fpcos(i32 %a) nounwind {
; CHECK-LABEL: test_fpcos:
; CHECK: fpcos r{{[0-9]+}}, r{{[0-9]+}}
  %1 = call i32 @llvm.systems.fpcos(i32 %a)
  ret i32 %1
}

define i32 @test_fpatan2(i32 %a, i32 %b) nounwind {
; CHECK-LABEL: test_fpatan2:
; CHECK: atan2 r{{[0-9]+}}, r{{[0-9]+}}, r{{[0-9]+}}
  %1 = call i32 @llvm.systems.fpatan2(i32 %a, i32 %b)
  ret i32 %1
}