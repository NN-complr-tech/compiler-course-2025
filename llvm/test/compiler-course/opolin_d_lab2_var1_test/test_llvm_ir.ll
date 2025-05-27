; RUN: opt -load-pass-plugin %llvmshlibdir/LlvmFmaPass_Opolin_Dmitry_FIIT2_LLVM_IR%pluginext\
; RUN: -passes=LlvmFmaPass -S %s | FileCheck %s

; a * b + c
; CHECK-LABEL: @basic_case
; CHECK: call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-NOT: fmul double
; CHECK-NOT: fadd double
define double @basic_case(double %a, double %b, double %c) {
  %mul = fmul double %a, %b
  %add = fadd double %mul, %c
  ret double %add
}

; c + (a * b)
; CHECK-LABEL: @reverse_order
; CHECK: call double @llvm.fmuladd.f64(double %a, double %b, double %c)
define double @reverse_order(double %a, double %b, double %c) {
  %mul = fmul double %a, %b
  %add = fadd double %c, %mul
  ret double %add
}

; (a * 2.0) + 3.0
; CHECK-LABEL: @with_constants
; CHECK: call double @llvm.fmuladd.f64(double %a, double 2.0{{[0+e+]*}}, double 3.0{{[0+e+]*}})
define double @with_constants(double %a) {
  %mul = fmul double %a, 2.0
  %add = fadd double %mul, 3.0
  ret double %add
}

; (a * b) + (-1.0)
; CHECK-LABEL: @negative_constant
; CHECK: call double @llvm.fmuladd.f64(double %a, double %b, double -1.0{{[0+e+]*}})
define double @negative_constant(double %a, double %b) {
  %mul = fmul double %a, %b
  %add = fadd double %mul, -1.0
  ret double %add
}

; (a * b) + c
; CHECK-LABEL: @float_type
; CHECK: call float @llvm.fmuladd.f32(float %a, float %b, float %c)
define float @float_type(float %a, float %b, float %c) {
  %mul = fmul float %a, %b
  %add = fadd float %mul, %c
  ret float %add
}

; (a * b) + c (a * b) + d
; CHECK-LABEL: @multi_use
; CHECK-DAG: call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-DAG: call double @llvm.fmuladd.f64(double %a, double %b, double %d)
; CHECK-NOT: fmul double
; CHECK-NOT: fadd double
define double @multi_use(double %a, double %b, double %c, double %d) {
  %mul = fmul double %a, %b
  %add1 = fadd double %mul, %c
  %add2 = fadd double %mul, %d
  ret double %add2
}

; a + b
; CHECK-LABEL: @no_change
; CHECK: fadd double %a, %b
; CHECK-NOT: call double @llvm.fmuladd.f64
define double @no_change(double %a, double %b) {
  %add = fadd double %a, %b
  ret double %add
}

; (a / b) + c
; CHECK-LABEL: @division
; CHECK: fdiv double %a, %b
; CHECK: fadd double %div, %c
define double @division(double %a, double %b, double %c) {
  %div = fdiv double %a, %b
  %add = fadd double %div, %c
  ret double %div
}

; CHECK-LABEL: @multi_use2
; CHECK: %mul = fmul float %a, %b
; CHECK: %add1 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK: %add2 = fdiv float %add1, %mul
; CHECK-NOT: fadd float
define float @multi_use2(float %a, float %b, float %c, float %d) {
  %mul = fmul float %a, %b
  %add1 = fadd float %mul, %c
  %add2 = fdiv float %add1, %mul
  ret float %add2
}
