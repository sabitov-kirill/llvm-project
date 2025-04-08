; RUN: llc < %s -mtriple=systems | FileCheck %s

define void @test_log(ptr %0) nounwind {
; CHECK-LABEL: test_log:
; CHECK: log r{{[0-9]+}}
  call void @llvm.systems.log(ptr %0)
  ret void
}

define i32 @test_deltatime() nounwind {
; CHECK-LABEL: test_deltatime:
; CHECK: deltatime r{{[0-9]+}}
  %1 = call i32 @llvm.systems.deltatime()
  ret i32 %1
}
