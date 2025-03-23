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
