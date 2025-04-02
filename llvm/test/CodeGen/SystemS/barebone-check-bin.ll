; RUN: llc -march=systems -filetype=asm < %s | FileCheck %s --check-prefix=ASM
; RUN: llc -march=systems -filetype=obj < %s | llvm-readelf --sections - | FileCheck %s --check-prefix=OBJ

define i32 @test_immediate() {
  ret i32 42
}

define void @test_return() {
  ret void
}

define void @test_branch(i32 %addr) {
  %ptr = inttoptr i32 %addr to i8*
  indirectbr i8* %ptr, []
  ret void
}

; ASM-LABEL: test_immediate:
; ASM:       li r{{[0-9]+}} 42
; ASM:       br r{{[0-9]+}}

; ASM-LABEL: test_return:
; ASM:       br r{{[0-9]+}}

; ASM-LABEL: test_branch:
; ASM:       br r{{[0-9]+}}


; OBJ-DAG: .text
; OBJ-DAG: .symtab
; OBJ-DAG: .strtab