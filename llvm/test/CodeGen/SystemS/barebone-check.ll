; RUN: llc -march=systems < %s | FileCheck %s
define void @test() {
  ret void
}
; CHECK: test: