; RUN: opt %loadNPMPolly '-passes=polly<no-default-opts>' -polly-profile-scops -S < %s | FileCheck %s
;
; Regression test: when the SCoP region's entry block IS the outermost loop
; header (i.e., it has a back-edge), the orig-path instrumentation must NOT be
; placed inside the loop.  Specifically:
;
;   - __cas_scop_start (":orig") must appear in the dedicated pre-entry block
;     (%for.i.pre_entry_bb), which is visited exactly ONCE per SCoP execution,
;     NOT in %for.i which is visited once per loop iteration.
;
;   - __cas_scop_end (":orig") must appear in polly.orig.region_exiting, a
;     split block on the exit edge of the loop, visited exactly ONCE,
;     NOT in %for.i whose terminator fires every iteration.
;
; This pattern arises in polybench correlation (and other benchmarks) where
; Polly creates a SCoP region whose entry is the outermost loop header.
;
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

; CHECK-LABEL: define void @f

; Orig start must be in the pre-entry block (visited once), not in %for.i.
; CHECK-LABEL: for.i.pre_entry_bb:
; CHECK:         call void @__cas_scop_start(ptr @"__cas_scop_id_f_%for.i_%polly.merge_new_and_old_orig", ptr {{.*}}, i64 14)

; %for.i is the loop header/latch — the orig start/end must NOT appear here.
; CHECK-LABEL: for.i:
; CHECK-NOT:    call void @__cas_scop_start(ptr @"__cas_scop_id_f_%for.i_%polly.merge_new_and_old_orig"
; CHECK-NOT:    call void @__cas_scop_end(ptr @"__cas_scop_id_f_%for.i_%polly.merge_new_and_old_orig"

; Orig end must be in the split exit block (visited once), on the edge
; for.i → polly.merge_new_and_old.
; CHECK-LABEL: polly.orig.region_exiting:
; CHECK-NEXT:   call void @__cas_scop_end(ptr @"__cas_scop_id_f_%for.i_%polly.merge_new_and_old_orig")

; Opt path is bracketed by polly.start / polly.exiting as usual.
; CHECK-LABEL: polly.start:
; CHECK:         call void @__cas_scop_start(ptr @"__cas_scop_id_f_%for.i_%polly.merge_new_and_old_opt"
; CHECK-LABEL: polly.exiting:
; CHECK-NEXT:   call void @__cas_scop_end(ptr @"__cas_scop_id_f_%for.i_%polly.merge_new_and_old_opt")
