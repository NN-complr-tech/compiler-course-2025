; RUN: opt -load-pass-plugin %llvmshlibdir/FmuladdPass_Stroganov_Mikhail_FIIT2_LLVM_IR%pluginext\
; RUN: -passes=FmuladdPass -S %s | FileCheck %s

; a * b + c
; CHECK-LABEL: @_test1
; CHECK-NEXT: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double %2)
; CHECK-NEXT: ret double %fma

define dso_local noundef double @_test1(double noundef %0, double noundef %1, double noundef %2) {
  %mul = fmul double %0, %1
  %add = fadd double %mul, %2
  ret double %add
}

; a * b + 1.0
; CHECK-LABEL: @_test2
; CHECK-NEXT: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double 1.000000e+00)
; CHECK-NEXT: ret double %fma


define dso_local noundef double @_test2(double noundef %0, double noundef %1) {
  %mul = fmul double %0, %1
  %add = fadd double %mul, 1.0
  ret double %add
}

; (a * b + c) + b;
; CHECK-LABEL: @_test3
; CHECK: call double @llvm.fmuladd.f64
; CHECK: fadd
; CHECK: ret

define dso_local noundef double @_test3(double noundef %0, double noundef %1, double noundef %2) {
  %mul1 = fmul double %0, %1
  %add1 = fadd double %mul1, %2
  %add2 = fadd double %add1, %1
  ret double %add2
}

; a * b + c + d
; CHECK-LABEL: @_test4
; CHECK: call double @llvm.fmuladd.f64
; CHECK: fadd
; CHECK: ret

define dso_local noundef double @_test4(double noundef %0, double noundef %1, double noundef %2, double noundef %3) {
  %mul1 = fmul double %0, %1
  %add1 = fadd double %mul1, %2
  %add2 = fadd double %add1, %3
  ret double %add2
}

; a * b + a * c
; CHECK-LABEL: @_test5
; CHECK: call double @llvm.fmuladd.f64
; CHECK: ret

define dso_local noundef double @_test5(double noundef %0, double noundef %1, double noundef %2) {
  %mul1 = fmul double %0, %1
  %mul2 = fmul double %0, %2
  %add = fadd double %mul1, %mul2
  ret double %add
}


