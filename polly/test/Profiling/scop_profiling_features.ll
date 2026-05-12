; RUN: opt %loadNPMPolly '-passes=polly<no-default-opts;no-end2end>' -polly-profile-scops -S < %s | FileCheck %s
;
; Verify per-slot feature values for: for (i = 0; i < N; ++i) A[i] = i;
;
; Expected feature vector (FeatureID order):
;   [0] TripCount    = N   (barvinok runtime expr; -1 without barvinok)
;   [1] MemFootprintBytes = runtime (N elements × 8 B) or -1
;   [2] StmtCount    = 1
;   [3] MaxLoopDepth = 1
;   [4] NumParams    = 1   (N is a symbolic parameter)
;   [5] NumArrays    = 1   (A)
;   [6] NumDimensions = 1  (A is 1-D)
;   [7] DynNumReads     = runtime or -1   (no array loads, but barvinok still needed)
;   [8] DynNumWrites    = runtime or -1   (1 write × card(domain))
;   [9] DynNumReductions = runtime or -1
;  [10] DynNumAluOps    = runtime or -1
;  [11] DynNumMulDivOps = runtime or -1
;  [12] DynNumFpOps     = runtime or -1
;  [13] DynNumCfOps     = runtime or -1

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

; CHECK-LABEL: define void @f
; CHECK:  %scop_feats{{[0-9]*}} = alloca [14 x i64]

; Slot 0: TripCount — runtime expression (barvinok) or -1; just verify a store exists.
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; Slot 1: MemFootprintBytes — runtime or -1.
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; Slots 2-6: compile-time constants.
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}

; Slots 7-9: dynamic memory-access counts — runtime or -1.
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; Slots 10-13: dynamic instruction-group counts — runtime or -1.
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; CHECK:  call void @__cas_scop_start(ptr {{.*}}, ptr {{.*}}, i64 14)
