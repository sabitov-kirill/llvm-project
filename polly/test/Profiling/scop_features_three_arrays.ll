; RUN: opt %loadNPMPolly '-passes=polly<no-default-opts;no-end2end>' -polly-profile-scops -S < %s | FileCheck %s
;
; Three-array loop: C[i] = A[i] + B[i] — verifies NumArrays=3, NumReads=2.
;
; void saxpy(long *A, long *B, long *C, long N) {
;   for (long i = 0; i < N; ++i)
;     C[i] = A[i] + B[i];
; }
;
; Expected feature vector:
;   [0] TripCount    = runtime
;   [1] StmtCount    = 1
;   [2] MaxLoopDepth = 1
;   [3] NumParams    = 1   (N)
;   [4] NumArrays    = 3   (A, B, C)
;   [5] NumDimensions = 3  (1 per array)
;   [6] NumReads     = 2   (load A[i], load B[i])
;   [7] NumWrites    = 1   (store to C[i])
;   [8] NumReductions = 0
;   [9] MemFootprintBytes = runtime (3×N elements × 8 B) or -1

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"
target triple = "x86_64-unknown-linux-gnu"

define void @saxpy(ptr %A, ptr %B, ptr %C, i64 %N) {
entry:
  br label %for.i

for.i:
  %i = phi i64 [ 0, %entry ], [ %i.next, %for.i ]
  %pa = getelementptr inbounds i64, ptr %A, i64 %i
  %va = load i64, ptr %pa, align 8
  %pb = getelementptr inbounds i64, ptr %B, i64 %i
  %vb = load i64, ptr %pb, align 8
  %sum = add nsw i64 %va, %vb
  %pc = getelementptr inbounds i64, ptr %C, i64 %i
  store i64 %sum, ptr %pc, align 8
  %i.next = add nsw i64 %i, 1
  %exitcond = icmp eq i64 %i.next, %N
  br i1 %exitcond, label %return, label %for.i

return:
  ret void
}

; CHECK-LABEL: define void @saxpy
; CHECK:  %scop_feats{{[0-9]*}} = alloca [10 x i64]

; Slot 0: runtime TripCount.
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 3, ptr {{.*}}
; CHECK:  store i64 3, ptr {{.*}}
; CHECK:  store i64 2, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 0, ptr {{.*}}

; Slot 9: MemFootprintBytes — runtime or -1.
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; CHECK:  call void @__cas_scop_start(ptr {{.*}}, ptr {{.*}}, i64 10)
