; RUN: opt %loadNPMPolly '-passes=polly<no-default-opts;no-end2end>' -polly-profile-scops -S < %s | FileCheck %s
;
; Verify that the four instruction-group features (slots 10-13) are present in
; the feature vector emitted by ScopProfiler.
;
; Two functions exercise different group mixes:
;
; saxpy_int: C[i] = A[i] + B[i]  — one ADD per iteration (ALU group)
; saxpy_fp:  C[i] = A[i] + B[i]  — one FADD per iteration (FP group)
;
; In both cases the feature vector must have exactly 14 slots, and stores for
; all four instruction-group slots (10-13) must be present.  We use {{.*}} for
; dynamic (barvinok-dependent) values and verify only the structural invariants.

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128"
target triple = "x86_64-unknown-linux-gnu"

; ---------------------------------------------------------------------------
; Integer saxpy: C[i] = A[i] + B[i]
; ---------------------------------------------------------------------------
define void @saxpy_int(ptr %A, ptr %B, ptr %C, i64 %N) {
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

; ---------------------------------------------------------------------------
; FP saxpy: C[i] = A[i] + B[i]  (double)
; ---------------------------------------------------------------------------
define void @saxpy_fp(ptr %A, ptr %B, ptr %C, i64 %N) {
entry:
  br label %for.i

for.i:
  %i = phi i64 [ 0, %entry ], [ %i.next, %for.i ]
  %pa = getelementptr inbounds double, ptr %A, i64 %i
  %va = load double, ptr %pa, align 8
  %pb = getelementptr inbounds double, ptr %B, i64 %i
  %vb = load double, ptr %pb, align 8
  %sum = fadd double %va, %vb
  %pc = getelementptr inbounds double, ptr %C, i64 %i
  store double %sum, ptr %pc, align 8
  %i.next = add nsw i64 %i, 1
  %exitcond = icmp eq i64 %i.next, %N
  br i1 %exitcond, label %return, label %for.i

return:
  ret void
}

; ---------------------------------------------------------------------------
; CHECK-LABEL: define void @saxpy_int
; CHECK:  %scop_feats{{[0-9]*}} = alloca [14 x i64]

; Slots 0-6: trip-count + footprint + static counts.
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 3, ptr {{.*}}
; CHECK:  store i64 3, ptr {{.*}}

; Slots 7-9: dynamic memory — all runtime.
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; Slots 10-13: instruction groups — all present (runtime or -1).
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; CHECK:  call void @__cas_scop_start(ptr {{.*}}, ptr {{.*}}, i64 14)

; ---------------------------------------------------------------------------
; CHECK-LABEL: define void @saxpy_fp
; CHECK:  %scop_feats{{[0-9]*}} = alloca [14 x i64]

; Slots 0-6.
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 1, ptr {{.*}}
; CHECK:  store i64 3, ptr {{.*}}
; CHECK:  store i64 3, ptr {{.*}}

; Slots 7-9.
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; Slots 10-13.
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}
; CHECK:  store i64 {{.*}}, ptr {{.*}}

; CHECK:  call void @__cas_scop_start(ptr {{.*}}, ptr {{.*}}, i64 14)
