; RUN: opt -load-pass-plugin %llvmshlibdir/FmulFaddMergePass_LopatinIlya_FIIT3_LLVM_IR%pluginext \
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
; CHECK: call float @llvm.fmuladd.f32
define dso_local noundef float @recursiveTest(float %A, float %B, float %C) {
entry:
  %add = fadd float %B, %C
  %add1 = fadd float %A, %B
  %mul = fmul float %add1, %B
  %add2 = fadd float %add, %mul
  ret float %add2
}

; CHECK-LABEL: @noFusionTest
; CHECK: fmul float %x, %y
; CHECK: fmul float %y, %z
; CHECK: fadd float %mul1, %mul2
; CHECK-NOT: call float @llvm.fmuladd.f32
define float @noFusionTest(float %x, float %y, float %z) {
entry:
  %mul1 = fmul float %x, %y
  %mul2 = fmul float %y, %z
  %sum = fadd float %mul1, %mul2
  ret float %sum
}
             