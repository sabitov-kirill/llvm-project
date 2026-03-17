; RUN: opt %loadNPMPolly '-passes=polly<no-default-opts;no-end2end>' -polly-profile-scops -S < %s | FileCheck %s
;
; SCoP with fixed (non-symbolic) loop bound — NumParams must be 0.
; TripCount is a compile-time constant (100) when barvinok is available.
;
; void f(long *A) {
;   for (long i = 0; i < 100; ++i)
;     A[i] = i;
; }
;
; Expected feature vector:
;   [0] TripCount    = 100  (barvinok evaluates to constant) or -1
;   [1] StmtCount    = 1
;   [2] MaxLoopDepth = 1
;   [3] NumParams    = 0    (no symbolic parameters — fixed bound 100)
;   [4] NumArrays    = 1
;   [5] NumDimensions = 1
;   [6] NumReads     = 0
;   [7] NumWrites    = 1
;   [8] NumReductions = 0
;   [9] MemFootprintBytes = 800 (barvinok: 100 elem × 8 B) or -1

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"
target triple = "x86_64-unknown-linux-gnu"

define void @f(ptr %A) {
entry:
  br label %for.i

for.i:
  %i = phi i64 [ 0, %entry ], [ %i.next, %for.i ]
  %ptr = getelementptr inbounds i64, ptr %A, i64 %i
  store i64 %i, ptr %ptr, align 8
  %i.next = add nsw i64 %i, 1
  %exitcond = icmp eq i64 %i.next, 100
  br i1 %exitcond, label %return, label %for.i

return:
  ret void
}

; CHECK-LABEL: define void @f
; CHECK:  %scop_feats{{[0-9]*}} = alloca [10 x i64]

; Slot 0: TripCount — 100 constant (barvinok) or -1.
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 0, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 0, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 0, ptr {{.*}}

; Slot 9: MemFootprintBytes — barvinok constant or -1.
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; CHECK:  call void @__cas_scop_start(ptr {{.*}}, ptr {{.*}}, i64 10)
