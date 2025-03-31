; RUN: opt -load-pass-plugin %llvmshlibdir/FMAPass_Kurakin_Matvey_FIIT1_LLVM_IR%pluginext -passes=kurakin_fma -S %s | FileCheck %s

; CHECK: define dso_local noundef double @fma(double noundef %a, double noundef %b, double noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-NEXT: ret double %0
; CHECK-NEXT: }

; double fma(double a, double b, double c) {
;     return a * b + c;
; }

define dso_local noundef double @fma(double noundef %a, double noundef %b, double noundef %c) {
entry:
  %mul = fmul double %a, %b
  %add = fadd double %mul, %c
  ret double %add
}

; CHECK: define dso_local noundef float @fma_floata(float noundef %a, float noundef %b, float noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT: ret float %0
; CHECK-NEXT: }

; float fma_float(float a, float b, float c) {
;     return a * b + c;
; }

define dso_local noundef float @fma_floata(float noundef %a, float noundef %b, float noundef %c) {
entry:
  %mul = fmul float %a, %b
  %add = fadd float %mul, %c
  ret float %add
}


; CHECK: define dso_local noundef double @fam(double noundef %a, double noundef %b, double noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %b, double %c, double %a)
; CHECK-NEXT: ret double %0
; CHECK-NEXT: }

; double fam(double a, double b, double c) {
;     return a + b * c;
; }

define dso_local noundef double @fam(double noundef %a, double noundef %b, double noundef %c) {
entry:
  %mul = fmul double %b, %c
  %add = fadd double %mul, %a
  ret double %add
}


; CHECK: define dso_local noundef double @fmaam(double noundef %a, double noundef %b, double noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-NEXT: %1 = call double @llvm.fmuladd.f64(double %a, double %b, double %0)
; CHECK-NEXT: ret double %1
; CHECK-NEXT: }

; double fmaam(double a, double b, double c) {
;     return a * b + c + a * b;
; }

define dso_local noundef double @fmaam(double noundef %a, double noundef %b, double noundef %c) {
entry:
  %mul = fmul double %a, %b
  %add = fadd double %mul, %c
  %add2 = fadd double %mul, %add
  ret double %add2
}


; CHECK: define dso_local noundef double @fmama(double noundef %a, double noundef %b, double noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %mul = fmul double %a, %b
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %mul)
; CHECK-NEXT: %add2 = fadd double %0, %c
; CHECK-NEXT: ret double %add2
; CHECK-NEXT: }

; double fmama(double a, double b, double c) {
;     return (a * b) + (a * b) + c ;
; }

define dso_local noundef double @fmama(double noundef %a, double noundef %b, double noundef %c) {
entry:
  %mul = fmul double %a, %b
  %add = fadd double %mul, %mul
  %add2 = fadd double %add, %c
  ret double %add2
}


; CHECK: define dso_local noundef double @fmama_br(double noundef %a, double noundef %b, double noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-NEXT: %1 = call double @llvm.fmuladd.f64(double %a, double %b, double %0)
; CHECK-NEXT: ret double %1
; CHECK-NEXT: }

; double fmama_br(double a, double b, double c) {
;     return (a * b) + ((a * b) + c) ;
; }

define dso_local noundef double @fmama_br(double noundef %a, double noundef %b, double noundef %c) {
entry:
  %mul = fmul double %a, %b
  %add = fadd double %mul, %c
  %add2 = fadd double %mul, %add
  ret double %add2
}

; CHECK: define dso_local noundef double @fmasm(double noundef %a, double noundef %b, double noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %mul = fmul double %a, %b
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-NEXT: %sub = fsub double %0, %mul
; CHECK-NEXT: ret double %sub
; CHECK-NEXT: }

; double fmasm(double a, double b, double c) {
;     double x = a * b;
;     return x + c - x ;
; }

define dso_local noundef double @fmasm(double noundef %a, double noundef %b, double noundef %c) {
entry:
  %mul = fmul double %a, %b
  %add = fadd double %mul, %c
  %sub = fsub double %add, %mul
  ret double %sub
}

; CHECK: define dso_local noundef float @foo(float noundef %a, float noundef %b, float noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %mul = fmul float %a, %b
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT: %div = fdiv float %0, %mul
; CHECK-NEXT: %add1 = fadd float %div, 1.000000e+00
; CHECK-NEXT: ret float %add1
; CHECK-NEXT: }

; float foo(float a, float b, float c) {
;     float t1 = a * b;
;     float t2 = t1 + c;
;     float t3 = t2 / t1 + 1;
;     return t3;
; }

define dso_local noundef float @foo(float noundef %a, float noundef %b, float noundef %c) {
entry:
  %mul = fmul float %a, %b
  %add = fadd float %mul, %c
  %div = fdiv float %add, %mul
  %add1 = fadd float %div, 1.000000e+00
  ret float %add1
}

; CHECK: define dso_local noundef float @faddmul(float noundef %a, float noundef %b, float noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT: ret float %0
; CHECK-NEXT: }

; float faddmul(float a, float b, float c) {
;   double x = a * b;
;   return c + x;
; }

define dso_local noundef float @faddmul(float noundef %a, float noundef %b, float noundef %c) {
entry:
  %mul = fmul float %a, %b
  %conv2 = fadd float %c, %mul
  ret float %conv2
}
