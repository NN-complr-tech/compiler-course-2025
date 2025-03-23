; RUN: opt -load-pass-plugin %llvmshlibdir/FmulFaddMergePass_LopatinIlya_FIIT3_LLVM_IR%pluginext \
; RUN: -passes="FmulFaddMergePass" -S %s | FileCheck %s

; CHECK: call double @llvm.fmuladd.f64
; CHECK: call float @llvm.fmuladd.f32
; CHECK-NOT: fmul double
; CHECK-NOT: fadd double
; CHECK-NOT: fadd float

; double fmaDouble(double A, double B, double C) {
;     double Res1 = A * B + C;
;     return Res1;
; }

define dso_local noundef double @fmaDouble(double %A, double %B, double %C) {
entry:
  %mul = fmul double %A, %B
  %add = fadd double %mul, %C
  ret double %add
}

; float fmaFloatFirst(float A, float B, float C) {
;     float Res1 = A * B + C;
;     return Res1;
; }

define dso_local noundef float @fmaFloatFisrt(float %A, float %B, float %C) {
entry:
  %mul = fmul float %A, %B
  %add = fadd float %mul, %C
  ret float %add
}

; float fmaFloatSecond(float A, float B, float C) {
;     float Res1 = A * B;
;     float Res2 = B * C;
;     float Res3 = Res1 + Res2;
;     return Res3;
; }

define dso_local noundef float @fmaFloatSecond(float %A, float %B, float %C) {
entry:
  %mul = fmul float %A, %B
  %mul1 = fmul float %B, %C
  %add = fadd float %mul, %mul1
  ret float %add
}

; CHECK-LABEL: @negativeTest
; CHECK: fmul float
; CHECK: fadd float
; CHECK-NOT: call float @llvm.fmuladd.f32

; float negativeTest(float A, float B, float C) {
;     float Res1 = B + C;
;     float Res2 = A + B;
;     return Res1 + Res2 * B;
; }

define dso_local noundef float @negativeTest(float %A, float %B, float %C) {
entry:
  %add = fadd float %B, %C
  %add1 = fadd float %A, %B
  %mul = fmul float %add1, %B
  %add2 = fadd float %add, %mul
  ret float %add2
}
