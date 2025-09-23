; RUN: opt -load-pass-plugin %llvmshlibdir/FmaPass_Agafeev_Sergey_FIIT3_LLVM_IR%pluginext \
; RUN: -passes="FmulFaddMergePass" -S %s | FileCheck %s

; CHECK-LABEL: @fmaDouble
; CHECK: call double @llvm.fmuladd.f64(double %A, double %B, double %C)
; CHECK-NOT: fmul double
; CHECK-NOT: fadd double
define dso_local noundef double @fmaDouble(double %A, double %B, double %C) {
entry:
  %mul = fmul double %A, %B
  %add = fadd double %mul, %C
  ret double %add
}

; CHECK-LABEL: @fmaFloat
; CHECK: call float @llvm.fmuladd.f32(float %A, float %B, float %C)
; CHECK-NOT: fmul float
; CHECK-NOT: fadd float
define dso_local noundef float @fmaFloat(float %A, float %B, float %C) {
entry:
  %mul = fmul float %A, %B
  %add = fadd float %mul, %C
  ret float %add
}

; CHECK-LABEL: @recursiveTest
; CHECK: fadd float %B, %C
; CHECK: fadd float %A, %B
; CHECK: call float @llvm.fmuladd.f32(float %add1, float %B, float %add)
define dso_local noundef float @recursiveTest(float %A, float %B, float %C) {
entry:
  %add = fadd float %B, %C
  %add1 = fadd float %A, %B
  %mul = fmul float %add1, %B
  %add2 = fadd float %add, %mul
  ret float %add2
}

; CHECK-LABEL: @func
; CHECK: call float @llvm.fmuladd.f32(float %A, float %B, float %C)
; CHECK: call float @llvm.fmuladd.f32(float %A, float %B, float 1.000000e+00)
; CHECK: fdiv float %0, %1
; CHECK-NOT: fmul float
; CHECK-NOT: fadd float
define float @func(float %A, float %B, float %C) {
entry:
  %mul = fmul float %A, %B
  %add = fadd float %mul, %C
  %add1 = fadd float %mul, 1.000000e+00
  %div = fdiv float %add, %add1
  ret float %div
}
