; RUN: opt -load-pass-plugin %llvmshlibdir/FmuladdMergePass_MarkinIvan_FIIT2_LLVM_IR%pluginext \
; RUN: -passes="FmulFaddMergePass" -S %s | FileCheck %s

; CHECK-LABEL: @fmaDouble
; CHECK: call double @llvm.fmuladd.f64(double %A, double %B, double %C)
; CHECK-NOT: fmul
; CHECK-NOT: fadd
define dso_local noundef double @fmaDouble(double %A, double %B, double %C) {
entry:
  %mul = fmul contract double %A, %B
  %add = fadd contract double %mul, %C
  ret double %add
}

; CHECK-LABEL: @fmaFloat
; CHECK: call float @llvm.fmuladd.f32(float %A, float %B, float %C)
; CHECK-NOT: fmul
; CHECK-NOT: fadd
define dso_local noundef float @fmaFloat(float %A, float %B, float %C) {
entry:
  %mul = fmul contract float %A, %B
  %add = fadd contract float %mul, %C
  ret float %add
}

; CHECK-LABEL: @recursiveTest
; CHECK: call float @llvm.fmuladd.f32(float %add1, float %B, float %add)
; CHECK-NOT: fmul
; CHECK-NOT: fadd
define dso_local noundef float @recursiveTest(float %A, float %B, float %C) {
entry:
  %add = fadd contract float %B, %C
  %add1 = fadd contract float %A, %B
  %mul = fmul contract float %add1, %B
  %add2 = fadd contract float %add, %mul
  ret float %add2
}

; CHECK-LABEL: @foo
; CHECK:   %0 = call float @llvm.fmuladd.f32(float %A, float %B, float %C)
; CHECK:   %1 = call float @llvm.fmuladd.f32(float %A, float %B, float 1.000000e+00)
; CHECK:   %div = fdiv float %0, %1
; CHECK-NOT: %mul = fmul contract float %A, %B
; CHECK-NOT: fadd 
define float @foo(float %A, float %B, float %C) {
entry:
  %mul = fmul contract float %A, %B
  %add = fadd contract float %mul, %C
  %add1 = fadd contract float %mul, 1.000000e+00
  %div = fdiv float %add, %add1
  ret float %div
}
; CHECK-LABEL: @noContract
; CHECK: fmul float %A, %B
; CHECK: fadd float %mul, %C
define float @noContract(float %A, float %B, float %C) {
entry:
  %mul = fmul float %A, %B
  %add = fadd float %mul, %C
  ret float %add
}
; CHECK-LABEL: @multipleFMA
; CHECK: call float @llvm.fmuladd.f32(float %A, float %B, float %C)
; CHECK: call float @llvm.fmuladd.f32(float %D, float %E, float %F)
define float @multipleFMA(float %A, float %B, float %C, float %D, float %E, float %F) {
entry:
  %mul1 = fmul contract float %A, %B
  %add1 = fadd contract float %mul1, %C
  %mul2 = fmul contract float %D, %E
  %add2 = fadd contract float %mul2, %F
  ret float %add2
}
; CHECK-LABEL: @differentArduments
; CHECK: fmul contract float %A, %B
; CHECK: fadd contract float %B, %D
define float @differentArduments(float %A, float %B, float %D) {
entry:
  %mul = fmul contract float %A, %B
  %add = fadd contract float %B, %D
  ret float %add
}
; CHECK-LABEL: @differentInstructions
; CHECK: fadd contract float %A, %B
; CHECK: fadd contract float %add1, %D
define float @differentInstructions(float %A, float %B, float %D) {
entry:
  %add1 = fadd contract float %A, %B
  %add = fadd contract float %add1, %D
  ret float %add
}
