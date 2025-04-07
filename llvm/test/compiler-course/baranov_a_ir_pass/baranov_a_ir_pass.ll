; RUN: opt -load-pass-plugin %llvmshlibdir/MulAddPass_Baranov_Aleksey_FIIT1_LLVM_IR%pluginext -passes=FMulAddPass -S %s | FileCheck %s

; === Case 1: Multiplication reused twice (should only replace in one place) ===
; CHECK-LABEL: define double @reused_mul(double %a, double %b, double %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %mul = fmul double %a, %b
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-NEXT: %add2 = fadd double %0, %mul
; CHECK-NEXT: ret double %add2
define double @reused_mul(double %a, double %b, double %c) {
entry:
  %mul = fmul double %a, %b
  %add1 = fadd double %mul, %c
  %add2 = fadd double %add1, %mul
  ret double %add2
}

; === Case 2: FMA pattern inside loop ===
; CHECK-LABEL: define double @in_loop(double %a, double %b, double %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: br label %loop
; CHECK: loop:
; CHECK-NEXT: %phi = phi double [ 0.000000e+00, %entry ], [ %sum, %loop ]
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-NEXT: %sum = fadd double %phi, %0
; CHECK-NEXT: %cond = fcmp ult double %sum, 1.000000e+02
; CHECK-NEXT: br i1 %cond, label %loop, label %exit
; CHECK: exit:
; CHECK-NEXT: ret double %sum
define double @in_loop(double %a, double %b, double %c) {
entry:
  br label %loop
loop:
  %phi = phi double [ 0.0, %entry ], [ %sum, %loop ]
  %mul = fmul double %a, %b
  %add = fadd double %mul, %c
  %sum = fadd double %phi, %add
  %cond = fcmp ult double %sum, 100.0
  br i1 %cond, label %loop, label %exit
exit:
  ret double %sum
}

; === Case 3: Operand order changed (c + a * b) ===
; CHECK-LABEL: define double @reverse_order(double %a, double %b, double %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-NEXT: ret double %0
define double @reverse_order(double %a, double %b, double %c) {
entry:
  %mul = fmul double %a, %b
  %add = fadd double %c, %mul
  ret double %add
}

; === Case 4: Constant folding possibility (folded instead of fma) ===
; CHECK-LABEL: define double @const_fold(double %a) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double 2.000000e+00, double 3.000000e+00)
; CHECK-NEXT: ret double %0
define double @const_fold(double %a) {
entry:
  %mul = fmul double %a, 2.0
  %add = fadd double %mul, 3.0
  ret double %add
}

; === Case 5: Float vs Double mixed ===
; CHECK-LABEL: define float @float_case(float %a, float %b, float %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT: ret float %0
define float @float_case(float %a, float %b, float %c) {
entry:
  %mul = fmul float %a, %b
  %add = fadd float %mul, %c
  ret float %add
}

; === Case 6: Negative test (should not optimize subtract) ===
; CHECK-LABEL: define double @no_fma_sub(double %a, double %b, double %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %mul = fmul double %a, %b
; CHECK-NEXT: %sub = fsub double %mul, %c
; CHECK-NEXT: ret double %sub
define double @no_fma_sub(double %a, double %b, double %c) {
entry:
  %mul = fmul double %a, %b
  %sub = fsub double %mul, %c
  ret double %sub
}

; === Case 7: FMul reused in extra use (should not optimize) ===
; CHECK-LABEL: define double @multi_use_no_opt(double %a, double %b, double %c, double %d) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %mul = fmul double %a, %b
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double 0.000000e+00)
; CHECK-NEXT: %add1 = fadd double %mul, %c
; CHECK-NEXT: %add2 = fadd double %mul, %d
; CHECK-NEXT: %sum = fadd double %add1, %add2
; CHECK-NEXT: ret double %sum
define double @multi_use_no_opt(double %a, double %b, double %c, double %d) {
entry:
  %mul = fmul double %a, %b
  %x = fadd double %mul, 0.0
  %add1 = fadd double %mul, %c
  %add2 = fadd double %mul, %d
  %sum = fadd double %add1, %add2
  ret double %sum
}

; === Case 7: Type mismatch (no optimization due to type difference) ===
; CHECK-LABEL: define double @type_mismatch_no_opt(float %a, double %b, double %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %a_ext = fpext float %a to double
; CHECK-NEXT: %mul = fmul double %a_ext, %b
; CHECK-NEXT: %add = fadd double %mul, %c
; CHECK-NEXT: ret double %add
define double @type_mismatch_no_opt(float %a, double %b, double %c) {
entry:
  %a_ext = fpext float %a to double
  %mul = fmul double %a_ext, %b
  %add = fadd double %mul, %c
  ret double %add
}