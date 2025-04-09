; RUN: llc < %s -mtriple=systems | FileCheck %s

define i32 @test_flush() nounwind {
; CHECK-LABEL: test_flush:
; CHECK: flush {{.*}}
  %1 = call i32 @llvm.systems.flush()
  ret i32 %1
}

define void @test_putpixel(i32 %x, i32 %y, i32 %color) nounwind {
; CHECK-LABEL: test_putpixel:
; CHECK: putpixel {{.*}}, {{.*}}, {{.*}}
  call void @llvm.systems.putpixel(i32 %x, i32 %y, i32 %color)
  ret void
}
