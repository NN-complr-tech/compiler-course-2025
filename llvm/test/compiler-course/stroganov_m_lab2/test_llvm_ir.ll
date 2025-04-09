; RUN: opt -load-pass-plugin %llvmshlibdir/FmuladdPass_Stroganov_Mikhail_FIIT2_LLVM_IR%pluginext \
; RUN: -passes=FmuladdPass -S %s | FileCheck %s

; a * b + c
; CHECK-LABEL: @_test1
; CHECK-NEXT: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double %2)
; CHECK-NEXT: ret double %fma
; CHECK-NOT: fmul
; CHECK-NOT: fadd

define dso_local noundef double @_test1(double noundef %0, double noundef %1, double noundef %2) {
  %mul = fmul double %0, %1
  %add = fadd double %mul, %2
  ret double %add
}

; a * b + 1.0
; CHECK-LABEL: @_test2
; CHECK-NEXT: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double 1.000000e+00)
; CHECK-NEXT: ret double %fma
; CHECK-NOT: fmul
; CHECK-NOT: fadd

define dso_local noundef double @_test2(double noundef %0, double noundef %1) {
  %mul = fmul double %0, %1
  %add = fadd double %mul, 1.0
  ret double %add
}

; (a * b + c) + b;
; CHECK-LABEL: @_test3
; CHECK-NEXT: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double %2)
; CHECK-NEXT: %add2 = fadd double %fma, %1
; CHECK-NEXT: ret double %add2
; CHECK-NOT: fmul
; CHECK-NOT: %add1 = fadd

define dso_local noundef double @_test3(double noundef %0, double noundef %1, double noundef %2) {
  %mul1 = fmul double %0, %1
  %add1 = fadd double %mul1, %2
  %add2 = fadd double %add1, %1
  ret double %add2
}

; a * b + c + d
; CHECK-LABEL: @_test4
; CHECK-NEXT: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double %2)
; CHECK-NEXT: %add2 = fadd double %fma, %3
; CHECK-NEXT: ret double %add2
; CHECK-NOT: fmul
; CHECK-NOT: %add1 = fadd

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

; CHECK-LABEL: @_test6
; CHECK-NEXT: %mul = fmul double %0, %1
; CHECK-NEXT: %add1 = fadd double %mul, %2
; CHECK-NEXT: %add2 = fadd double %mul, %3
; CHECK-NEXT: ret double %add2
; CHECK-NOT: call double @llvm.fmuladd.f64

define dso_local noundef double @_test6(double noundef %0, double noundef %1, double noundef %2, double noundef %3) {
  %mul = fmul double %0, %1
  %add1 = fadd double %mul, %2
  %add2 = fadd double %mul, %3
  ret double %add2
}

; CHECK-LABEL: @_test7
; CHECK-NEXT: %x = fmul double %0, %1
; CHECK-NEXT: %y = fadd double %x, %2
; CHECK-NEXT: %z = fadd double %x, %3
; CHECK-NEXT: ret double %z
; CHECK-NOT: call double @llvm.fmuladd.f64

define dso_local noundef double @_test7(double noundef %0, double noundef %1, double noundef %2, double noundef %3) {
  %x = fmul double %0, %1
  %y = fadd double %x, %2
  %z = fadd double %x, %3
  ret double %z
}

; CHECK-LABEL: @_test8_float
; CHECK-NEXT: %fma = call float @llvm.fmuladd.f32(float %0, float %1, float %2)
; CHECK-NEXT: ret float %fma
; CHECK-NOT: = fmul
; CHECK-NOT: fadd

define dso_local noundef float @_test8_float(float noundef %0, float noundef %1, float noundef %2) {
  %mul = fmul float %0, %1
  %add = fadd float %mul, %2
  ret float %add
}

; CHECK-LABEL: @_test9_float
; CHECK-NEXT: %fma = call float @llvm.fmuladd.f32(float %0, float %1, float %2)
; CHECK-NEXT: %add2 = fadd float %fma, %3
; CHECK-NEXT: ret float %add2
; CHECK-NOT: = fmul
; CHECK-NOT: %add1 = fadd

define dso_local noundef float @_test9_float(float noundef %0, float noundef %1, float noundef %2, float noundef %3) {
  %mul1 = fmul float %0, %1
  %add1 = fadd float %mul1, %2
  %add2 = fadd float %add1, %3
  ret float %add2
}

