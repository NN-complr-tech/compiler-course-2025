; RUN: opt -load-pass-plugin %llvmshlibdir/FmulFaddMergePass_LopatinIlya_FIIT3_LLVM_IR%pluginext \
; RUN: -passes="FmulFaddMergePass" -S %s | FileCheck %s

; CHECK: call double @llvm.fmuladd.f64
; CHECK-NOT: fmul double
; CHECK-NOT: fadd double

define dso_local noundef double @fmaDouble(double %a, double %b, double %c) {
entry:
  %mul = fmul double %a, %b
  %add = fadd double %mul, %c
  ret double %add
}

define dso_local noundef float @fmaFloat(float %a, float %b, float %c) {
entry:
  %mul = fmul float %a, %b
  %add = fadd float %mul, %c
  ret float %add
}
