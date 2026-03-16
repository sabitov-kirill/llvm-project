; RUN: opt %loadNPMPolly '-passes=polly<no-default-opts;no-end2end>' -polly-profile-scops -S < %s | FileCheck %s
;
; Array copy loop — verifies NumReads=1, NumWrites=1, NumArrays=2.
;
; void copy(long *A, long *B, long N) {
;   for (long i = 0; i < N; ++i)
;     B[i] = A[i];
; }
;
; Expected feature vector:
;   [0] TripCount    = runtime (N is a parameter)
;   [1] StmtCount    = 1
;   [2] MaxLoopDepth = 1
;   [3] NumParams    = 1   (N)
;   [4] NumArrays    = 2   (A, B)
;   [5] NumDimensions = 2  (1 per array)
;   [6] NumReads     = 1   (load from A[i])
;   [7] NumWrites    = 1   (store to B[i])
;   [8] NumReductions = 0

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"
target triple = "x86_64-unknown-linux-gnu"

define void @copy(ptr %A, ptr %B, i64 %N) {
entry:
  br label %for.i

for.i:
  %i = phi i64 [ 0, %entry ], [ %i.next, %for.i ]
  %src = getelementptr inbounds i64, ptr %A, i64 %i
  %val = load i64, ptr %src, align 8
  %dst = getelementptr inbounds i64, ptr %B, i64 %i
  store i64 %val, ptr %dst, align 8
  %i.next = add nsw i64 %i, 1
  %exitcond = icmp eq i64 %i.next, %N
  br i1 %exitcond, label %return, label %for.i

return:
  ret void
}

; CHECK-LABEL: define void @copy
; CHECK:  %scop_feats{{[0-9]*}} = alloca [9 x i64]

; Slot 0: runtime TripCount.
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 2, ptr {{.*}}
; CHECK:  store i64 2, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 0, ptr {{.*}}

; CHECK:  call void @__cas_scop_start(ptr {{.*}}, ptr {{.*}}, i64 9)
