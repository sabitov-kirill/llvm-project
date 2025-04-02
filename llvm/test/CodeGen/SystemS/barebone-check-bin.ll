; RUN: llc -march=systems < %s | FileCheck %s
define void @test-asm() {
  ret void
}
; CHECK: "test-asm"
