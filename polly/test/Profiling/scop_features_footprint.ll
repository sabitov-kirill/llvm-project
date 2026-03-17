; RUN: opt %loadNPMPolly '-passes=polly<no-default-opts;no-end2end>' -polly-profile-scops -S < %s | FileCheck %s
;
; Fixed-size copy loop — verifies MemFootprintBytes slot is present.
;
; void copy_fixed(long *A, long *B) {
;   for (int i = 0; i < 16; ++i)
;     B[i] = A[i];
; }
;
; Expected feature vector:
;   [0] TripCount         = 16  (barvinok constant) or -1
;   [1] StmtCount         = 1
;   [2] MaxLoopDepth      = 1
;   [3] NumParams         = 0   (fixed bound — no symbolic parameters)
;   [4] NumArrays         = 2   (A, B)
;   [5] NumDimensions     = 2   (1 per array)
;   [6] NumReads          = 1   (load from A[i])
;   [7] NumWrites         = 1   (store to B[i])
;   [8] NumReductions     = 0
;   [9] MemFootprintBytes = 256 (barvinok: 2 arrays × 16 elem × 8 B) or -1

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"
target triple = "x86_64-unknown-linux-gnu"

define void @copy_fixed(ptr %A, ptr %B) {
entry:
  br label %for.i

for.i:
  %i = phi i64 [ 0, %entry ], [ %i.next, %for.i ]
  %src = getelementptr inbounds i64, ptr %A, i64 %i
  %val = load i64, ptr %src, align 8
  %dst = getelementptr inbounds i64, ptr %B, i64 %i
  store i64 %val, ptr %dst, align 8
  %i.next = add nsw i64 %i, 1
  %exitcond = icmp eq i64 %i.next, 16
  br i1 %exitcond, label %return, label %for.i

return:
  ret void
}

; CHECK-LABEL: define void @copy_fixed
; CHECK:  %scop_feats{{[0-9]*}} = alloca [10 x i64]

; Slot 0: TripCount — 16 constant (barvinok) or -1.
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 0, ptr {{.*}}
; CHECK:  store i64 2, ptr {{.*}}
; CHECK:  store i64 2, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 0, ptr {{.*}}

; Slot 9: MemFootprintBytes — 256 (barvinok) or -1.
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; CHECK:  call void @__cas_scop_start(ptr {{.*}}, ptr {{.*}}, i64 10)
