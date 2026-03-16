; RUN: opt %loadNPMPolly '-passes=polly<no-default-opts;no-end2end>' -polly-profile-scops -S < %s | FileCheck %s
;
; Nested 2-level loop — verifies MaxLoopDepth=2, NumParams=2.
;
; void nested(long *A, long N, long M) {
;   for (long i = 0; i < N; ++i)
;     for (long j = 0; j < M; ++j)
;       A[i + j] = 0;
; }
;
; Expected feature vector:
;   [0] TripCount    = runtime (N*M + overlap terms)
;   [1] StmtCount    = 1
;   [2] MaxLoopDepth = 2   (two loop levels)
;   [3] NumParams    = 2   (N, M)
;   [4] NumArrays    = 1   (A)
;   [5] NumDimensions = 1
;   [6] NumReads     = 0   (no array loads)
;   [7] NumWrites    = 1   (store to A[i+j])
;   [8] NumReductions = 0

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"
target triple = "x86_64-unknown-linux-gnu"

define void @nested(ptr %A, i64 %N, i64 %M) {
entry:
  br label %for.i

for.i:
  %i = phi i64 [ 0, %entry ], [ %i.next, %for.i.inc ]
  br label %for.j

for.j:
  %j = phi i64 [ 0, %for.i ], [ %j.next, %for.j ]
  %idx = add nsw i64 %i, %j
  %ptr = getelementptr inbounds i64, ptr %A, i64 %idx
  store i64 0, ptr %ptr, align 8
  %j.next = add nsw i64 %j, 1
  %exitcond.j = icmp eq i64 %j.next, %M
  br i1 %exitcond.j, label %for.i.inc, label %for.j

for.i.inc:
  %i.next = add nsw i64 %i, 1
  %exitcond.i = icmp eq i64 %i.next, %N
  br i1 %exitcond.i, label %return, label %for.i

return:
  ret void
}

; CHECK-LABEL: define void @nested
; CHECK:  %scop_feats{{[0-9]*}} = alloca [9 x i64]

; Slot 0: runtime TripCount.
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 2, ptr {{.*}}
; CHECK:  store i64 2, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 0, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 0, ptr {{.*}}

; CHECK:  call void @__cas_scop_start(ptr {{.*}}, ptr {{.*}}, i64 9)
