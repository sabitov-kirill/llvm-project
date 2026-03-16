; RUN: opt %loadNPMPolly '-passes=polly<no-default-opts>' -polly-profile-scops -S < %s | FileCheck %s
;
; Verify that ScopProfiler instruments BOTH the optimized and the original
; fallback paths produced by executeScopConditionally() when code generation
; is enabled.  The pass should:
;   - emit two global string constants: one with suffix ":opt", one with ":orig"
;   - in the optimized path (polly.start): call __cas_scop_start with ":opt" ID
;   - in polly.exiting:                   call __cas_scop_end  with ":opt" ID
;   - in the fallback entry block (%next): call __cas_scop_start with ":orig" ID
;   - in the unique fallback exiting block: call __cas_scop_end with ":orig" ID
;   - declare (but not define) __cas_scop_start / __cas_scop_end

; void f(long *A, long N) {
;   if (true)
;     for (i = 0; i < N; ++i)
;       A[i] = i;
; }

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"
target triple = "x86_64-unknown-linux-gnu"

define void @f(ptr %A, i64 %N) nounwind {
entry:
  fence seq_cst
  br label %next

next:
  br i1 true, label %for.i, label %return

for.i:
  %indvar = phi i64 [ 0, %next], [ %indvar.next, %for.i ]
  %scevgep = getelementptr i64, ptr %A, i64 %indvar
  store i64 %indvar, ptr %scevgep
  %indvar.next = add nsw i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %N
  br i1 %exitcond, label %return, label %for.i

return:
  fence seq_cst
  ret void
}

; CHECK: @llvm.global_ctors = appending constant [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 10, ptr @__cas_scop_profiler_init, ptr null }]

; Anchor past globals (SCoP ID constants contain "next:" as a substring).
; Fallback blocks appear before polly.* blocks in the output IR.
; CHECK-LABEL: define void @f
; CHECK-LABEL: next:
; CHECK:         call void @__cas_scop_start(ptr @"__cas_scop_id_f_%next_%polly.merge_new_and_old_orig", i64 {{.*}})
; CHECK-LABEL: return.region_exiting:
; CHECK-NEXT:    call void @__cas_scop_end(ptr @"__cas_scop_id_f_%next_%polly.merge_new_and_old_orig")
; CHECK-LABEL: polly.start:
; CHECK:         call void @__cas_scop_start(ptr @"__cas_scop_id_f_%next_%polly.merge_new_and_old_opt", i64 {{.*}})
; CHECK-LABEL: polly.exiting:
; CHECK-NEXT:    call void @__cas_scop_end(ptr @"__cas_scop_id_f_%next_%polly.merge_new_and_old_opt")

; Init wrapper emitted once even though two ScopProfiler instances ran.
; CHECK-LABEL: define weak_odr void @__cas_scop_profiler_init()
; CHECK-NEXT:  entry:
; CHECK-NEXT:    call void @__cas_scop_init()
; CHECK-NEXT:    ret void

; CHECK: declare void @__cas_scop_init()
; CHECK: declare void @__cas_scop_start(ptr, i64)
; CHECK: declare void @__cas_scop_end(ptr)
