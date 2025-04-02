; RUN: llc -march=systems -filetype=asm < %s | FileCheck %s
define void @test-asm() {
  ret void
}
; CHECK: "test-asm"
