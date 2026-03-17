; RUN: opt %loadNPMPolly '-passes=polly<no-default-opts;no-end2end>' -polly-profile-scops -S < %s | FileCheck %s
;
; Verify that ScopProfiler passes a features array to __cas_scop_start.
; The first element (index 0 = TripCount) is i64; without barvinok it is -1.
; Either way __cas_scop_start must receive (ptr, ptr, i64) arguments.
;
; void f(long *A, long N) {
;   for (long i = 0; i < N; ++i)
;     A[i] = i;
; }

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"
target triple = "x86_64-unknown-linux-gnu"

define void @f(ptr %A, i64 %N) {
entry:
  br label %for.i

for.i:
  %i = phi i64 [ 0, %entry ], [ %i.next, %for.i ]
  %ptr = getelementptr inbounds i64, ptr %A, i64 %i
  store i64 %i, ptr %ptr, align 8
  %i.next = add nsw i64 %i, 1
  %exitcond = icmp eq i64 %i.next, %N
  br i1 %exitcond, label %return, label %for.i

return:
  ret void
}

; CHECK: %scop_feats{{[0-9]*}} = alloca [10 x i64]
; CHECK: call void @__cas_scop_start(ptr {{.*}}, ptr {{.*}}, i64 10)
; CHECK: declare void @__cas_scop_start(ptr, ptr, i64)
