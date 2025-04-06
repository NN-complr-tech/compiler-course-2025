; RUN: opt -load-pass-plugin %llvmshlibdir/MulAddPass_Baranov_Aleksey_FIIT1_LLVM_IR%pluginext -passes=FMulAddPass -S %s | FileCheck %s

; === Case 1: Multiplication reused twice (should only replace in one place) ===
; CHECK-LABEL: define double @reused_mul(double %a, double %b, double %c) {
; CHECK: %mul = fmul double %a, %b
; CHECK: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK: %add2 = fadd double %0, %mul
define double @reused_mul(double %a, double %b, double %c) {
entry:
  %mul = fmul double %a, %b
  %add1 = fadd double %mul, %c
  %add2 = fadd double %add1, %mul
  ret double %add2
}

; === Case 2: FMA pattern inside loop ===
; CHECK-LABEL: define double @in_loop(double %a, double %b, double %c) {
; CHECK: call double @llvm.fmuladd.f64(double %a, double %b, double %c)
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
; CHECK: call double @llvm.fmuladd.f64(double %a, double %b, double %c)
define double @reverse_order(double %a, double %b, double %c) {
entry:
  %mul = fmul double %a, %b
  %add = fadd double %c, %mul
  ret double %add
}

; === Case 4: Constant folding possibility (folded instead of fma) ===
; CHECK-LABEL: define double @const_fold(double %a) {
; CHECK: %0 = call double @llvm.fmuladd.f64(double %a, double 2.000000e+00, double 3.000000e+00)
define double @const_fold(double %a) {
entry:
  %mul = fmul double %a, 2.0
  %add = fadd double %mul, 3.0
  ret double %add
}

; === Case 5: Float vs Double mixed ===
; CHECK-LABEL: define float @float_case(float %a, float %b, float %c) {
; CHECK: call float @llvm.fmuladd.f32(float %a, float %b, float %c)
define float @float_case(float %a, float %b, float %c) {
entry:
  %mul = fmul float %a, %b
  %add = fadd float %mul, %c
  ret float %add
}

; === Case 6: Negative test (should not optimize subtract) ===
; CHECK-LABEL: define double @no_fma_sub(double %a, double %b, double %c) {
; CHECK: %mul = fmul double %a, %b
; CHECK: %sub = fsub double %mul, %c
define double @no_fma_sub(double %a, double %b, double %c) {
entry:
  %mul = fmul double %a, %b
  %sub = fsub double %mul, %c
  ret double %sub
}