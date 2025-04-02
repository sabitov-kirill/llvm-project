; RUN: llc -march=systems -filetype=obj < %s | FileCheck %s
define void @test-asm() {
  ret void
}
; CHECK: "test-asm"
