; RUN: opt %loadNPMPolly '-passes=polly<no-default-opts;no-end2end>' -polly-profile-scops -S < %s | FileCheck %s
;
; Verify that ScopProfiler instruments the ORIGINAL (unoptimized) SCoP when
; code generation is disabled.  The pass should:
;   - emit a global string constant with the SCoP ID (no suffix)
;   - call __cas_scop_start before the first non-PHI of the entry block
;   - call __cas_scop_end  before the terminator of the unique exiting block
;   - declare (but not define) __cas_scop_start / __cas_scop_end

; void f(long *A, long N) {
;   for (long i = 0; i < N; ++i)
;     A[i] = i;
; }

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"
target triple = "x86_64-unknown-linux-gnu"

define void @f(ptr %A, i64 %N) nounwind {
entry:
  fence seq_cst
  br label %for.i

for.i:
  %i = phi i64 [ 0, %entry ], [ %i.next, %for.i ]
  %p = getelementptr i64, ptr %A, i64 %i
  store i64 %i, ptr %p
  %i.next = add nsw i64 %i, 1
  %exitcond = icmp eq i64 %i.next, %N
  br i1 %exitcond, label %return, label %for.i

return:
  fence seq_cst
  ret void
}

; CHECK: @llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 10, ptr @__cas_scop_profiler_init, ptr null }]

; Anchor past globals (SCoP ID constant names contain "for.i:" as a substring).
; CHECK-LABEL: define void @f
; CHECK-LABEL: for.i:
; CHECK-NEXT:    %i = phi
; CHECK:         call void @__cas_scop_start(ptr @"__cas_scop_id_f_%for.i_%return", i64 {{.*}})
; CHECK:         call void @__cas_scop_end(ptr @"__cas_scop_id_f_%for.i_%return")
; CHECK-NEXT:    br i1 %exitcond

; CHECK-LABEL: define weak_odr void @__cas_scop_profiler_init()
; CHECK-NEXT:  entry:
; CHECK-NEXT:    call void @__cas_scop_init()
; CHECK-NEXT:    ret void

; CHECK: declare void @__cas_scop_init()
; CHECK: declare void @__cas_scop_start(ptr, i64)
; CHECK: declare void @__cas_scop_end(ptr)
