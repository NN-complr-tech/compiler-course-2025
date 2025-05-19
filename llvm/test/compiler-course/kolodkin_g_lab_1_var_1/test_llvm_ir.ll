; RUN: opt -load-pass-plugin %llvmshlibdir/LabPass_KolodkinGrigorii_FIIT3_LLVM_IR%pluginext\
; RUN: -passes=add-and-mul-fma -S %s | FileCheck %s

; CHECK: define dso_local noundef double @_Z3fmaddd(double noundef %a, double noundef %b, double noundef %c) local_unnamed_addr {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-NEXT: ret double %0
; CHECK-NEXT: }

; double fma(double a, double b, double c) {
;     return a*b+c;
; }

define dso_local noundef double @_Z3fmaddd(double noundef %a, double noundef %b, double noundef %c) local_unnamed_addr {
entry:
  %mul = fmul double %a, %b
  %add = fadd double %mul, %c
  ret double %add
}

; CHECK: define dso_local noundef float @_Z3barfff(float noundef %a1, float noundef %b1, float noundef %c1) local_unnamed_addr {
; CHECK-NEXT: entry:
; CHECK-NEXT: %mul = fmul float %a1, %b1
; CHECK-NEXT: %add = fadd float %mul, %c1
; CHECK-NEXT: %div = fdiv float %add, %mul
; CHECK-NEXT: %conv3 = fadd float %div, 2.000000e+00
; CHECK-NEXT: ret float %conv3
; CHECK-NEXT: }

; float bar(float a1, float b1, float c1) {
;     return (a1 * b1 + c1) / (a1 * b1) + 2.0;
; }

define dso_local noundef float @_Z3barfff(float noundef %a1, float noundef %b1, float noundef %c1) local_unnamed_addr {
entry:
  %mul = fmul float %a1, %b1
  %add = fadd float %mul, %c1
  %div = fdiv float %add, %mul
  %conv3 = fadd float %div, 2.000000e+00
  ret float %conv3
}

; CHECK: define dso_local noundef float @_Z7fma_subfff(float noundef %x, float noundef %y, float noundef %z) local_unnamed_addr {
; CHECK-NEXT: entry:
; CHECK-NEXT: %mul = fmul float %x, %y
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float %x, float %y, float %z)
; CHECK-NEXT: %sub = fsub float %0, %mul
; CHECK-NEXT: ret float %sub
; CHECK-NEXT: }

; float fma_sub(float x, float y, float z) {
;     float t1 = x * y;
;     return t1 + z - t1;
; }

define dso_local noundef float @_Z7fma_subfff(float noundef %x, float noundef %y, float noundef %z) local_unnamed_addr #0 {
entry:
  %mul = fmul float %x, %y
  %add = fadd float %mul, %z
  %sub = fsub float %add, %mul
  ret float %sub
}
