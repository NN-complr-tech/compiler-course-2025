; RUN: opt -load-pass-plugin %llvmshlibdir/FmuladdPass_Chistov_Alexey_FIIT1_LLVM_IR%pluginext -passes=FmuladdPass -S %s | FileCheck %s

; a+b (test checks that no transformation is applied for a simple addition operation)
; CHECK-LABEL: define dso_local noundef double @simple_sum(double noundef %0, double noundef %1) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %add = fadd double %0, %1
; CHECK-NEXT: ret double %add
; CHECK-NEXT: }

define dso_local noundef double @simple_sum(double noundef %0, double noundef %1) {
entry:
  %add = fadd double %0, %1
  ret double %add
}

; a * b(separately) + c (test where optimization is not applied)
; CHECK-LABEL: define dso_local noundef double @no_fma_separate(double noundef %a, double noundef %b, double noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %mul = fmul double %a, %b
; CHECK-NEXT: %add = fadd double %c, %mul
; CHECK-NEXT: ret double %add
; CHECK-NEXT: }

define dso_local noundef double @no_fma_separate(double noundef %a, double noundef %b, double noundef %c) {
entry:
  %mul = fmul double %a, %b
  %add = fadd double %c, %mul
  ret double %add
}

; a * b + c
; CHECK-LABEL: define dso_local noundef double @example(double noundef %0, double noundef %1, double noundef %2) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %3 = call double @llvm.fmuladd.f64(double %0, double %1, double %2)
; CHECK-NEXT: ret double %3
; CHECK-NEXT: }

define dso_local noundef double @example(double noundef %0, double noundef %1, double noundef %2) {
entry:
  %mul = fmul double %0, %1
  %add = fadd double %mul, %2
  ret double %add
}

; a * 5.0 + 2.0
; CHECK-LABEL: define dso_local noundef double @const(double noundef %a) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double 5.000000e+00, double 2.000000e+00)
; CHECK-NEXT: ret double %0
; CHECK-NEXT: }

define dso_local noundef double @const(double noundef %a) {
entry:
  %mul = fmul double %a, 5.0
  %add = fadd double %mul, 2.0
  ret double %add
}

; a * b + c + d
; CHECK-LABEL:define dso_local noundef double @chain(double noundef %a, double noundef %b, double noundef %c, double noundef %d) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-NEXT: %add2 = fadd double %0, %d
; CHECK-NEXT: ret double %add2
; CHECK-NEXT: }

define dso_local noundef double @chain(double noundef %a, double noundef %b, double noundef %c, double noundef %d) {
entry:
  %mul = fmul double %a, %b
  %add1 = fadd double %mul, %c
  %add2 = fadd double %add1, %d
  ret double %add2
}

; (a * b + c)+ (x * y + z)
; CHECK-LABEL: define dso_local noundef double @several(double noundef %a, double noundef %b, double noundef %c, double noundef %x, double noundef %y, double noundef %z) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-NEXT: %1 = call double @llvm.fmuladd.f64(double %x, double %y, double %z)
; CHECK-NEXT: %sum = fadd double %0, %1
; CHECK-NEXT: ret double %sum
; CHECK-NEXT: }

define dso_local noundef double @several(double noundef %a, double noundef %b, double noundef %c, double noundef %x, double noundef %y, double noundef %z) {
entry:
  %mul1 = fmul double %a, %b
  %add1 = fadd double %mul1, %c
  %mul2 = fmul double %x, %y
  %add2 = fadd double %mul2, %z
  %sum = fadd double %add1, %add2
  ret double %sum
}
