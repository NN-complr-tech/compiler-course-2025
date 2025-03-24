; RUN: opt -load-pass-plugin %llvmshlibdir/FMAPass_Ionova_Ekaterina_FIIT1_LLVM_IR%pluginext \
; RUN: -passes="fmapass" -S %s | FileCheck %s

; double f1(double a, double b, double c) {
;     return a * b + c;
; }

define dso_local noundef double @_Z2f1ddd(double noundef %0, double noundef %1, double noundef %2) local_unnamed_addr {
  %4 = fmul double %0, %1
  %5 = fadd double %4, %2
  ret double %5
}

; CHECK-LABEL: @_Z2f1ddd
; CHECK: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double %2)
; CHECK-NEXT: ret double %fma



; double f2(double a, double b, double c, double d, double e) {
;     double result1 = a * b + c;
;     double result2 = d * e + result1;
;     return result2;
; }

define dso_local noundef double @_Z2f2ddddd(double noundef %0, double noundef %1, double noundef %2, double noundef %3, double noundef %4) local_unnamed_addr {
  %6 = fmul double %0, %1
  %7 = fadd double %6, %2
  %8 = fmul double %3, %4
  %9 = fadd double %7, %8
  ret double %9
}

; CHECK-LABEL: @_Z2f2ddddd
; CHECK: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double %2)
; CHECK: %fma1 = call double @llvm.fmuladd.f64(double %3, double %4, double %fma)
; CHECK-NEXT: ret double %fma1


; double f3(double a, double b, double c, double d) {
;     double result1 = a * b;
;     double result3 = result1 + c;
;     double result4 = result1 + d;
;     return result3 + result4;
; }

define dso_local noundef double @_Z2f3dddd(double noundef %0, double noundef %1, double noundef %2, double noundef %3) local_unnamed_addr {
  %5 = fmul double %0, %1
  %6 = fadd double %5, %2
  %7 = fadd double %5, %3
  %8 = fadd double %6, %7
  ret double %8
}

; CHECK-LABEL: @_Z2f3dddd
; CHECK: %5 = fmul double %0, %1
; CHECK-NEXT: %6 = fadd double %5, %2
; CHECK-NEXT: %7 = fadd double %5, %3
; CHECK-NEXT: %8 = fadd double %6, %7
; CHECK-NEXT: ret double %8


; double f4(double a, double b, double c) {
;     double result1 = a * b;
;     double result2 = c + 1.0;
;     return result1 + result2;
; }

define dso_local noundef double @_Z2f4ddd(double noundef %0, double noundef %1, double noundef %2) local_unnamed_addr {
  %4 = fmul double %0, %1
  %5 = fadd double %2, 1.000000e+00
  %6 = fadd double %4, %5
  ret double %6
}

; CHECK-LABEL: @_Z2f4ddd
; CHECK: %4 = fadd double %2, 1.000000e+00
; CHECK: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double %4)
; CHECK-NEXT: ret double %fma


; double f5(double a, double b, double c, double d) {
;     double result1 = a * b + c;
;     double result2 = result1 * d + c;
;     return result2;
; }

define dso_local noundef double @_Z2f5dddd(double noundef %0, double noundef %1, double noundef %2, double noundef %3) local_unnamed_addr {
  %5 = fmul double %0, %1
  %6 = fadd double %5, %2
  %7 = fmul double %6, %3
  %8 = fadd double %7, %2
  ret double %8
}

; CHECK-LABEL: @_Z2f5dddd
; CHECK: %fma = call double @llvm.fmuladd.f64(double %0, double %1, double %2)
; CHECK: %fma1 = call double @llvm.fmuladd.f64(double %fma, double %3, double %2)
; CHECK-NEXT: ret double %fma1


; float f6(float a, float b, float c) {
;     return a * b + c;
; }

define dso_local noundef float @_Z2f6fff(float noundef %0, float noundef %1, float noundef %2) local_unnamed_addr {
  %4 = fmul float %0, %1
  %5 = fadd float %4, %2
  ret float %5
}

; CHECK-LABEL: @_Z2f6fff
; CHECK: %fma = call float @llvm.fmuladd.f32(float %0, float %1, float %2)
; CHECK-NEXT: ret float %fma


; float f7(float a, float b, float c, float d, float e) {
;     float result1 = a * b + c;
;     float result2 = d * e + result1;
;     return result2;
; }

define dso_local noundef float @_Z2f7fffff(float noundef %0, float noundef %1, float noundef %2, float noundef %3, float noundef %4) local_unnamed_addr {
  %6 = fmul float %0, %1
  %7 = fadd float %6, %2
  %8 = fmul float %3, %4
  %9 = fadd float %7, %8
  ret float %9
}

; CHECK-LABEL: @_Z2f7fffff
; CHECK: %fma = call float @llvm.fmuladd.f32(float %0, float %1, float %2)
; CHECK: %fma1 = call float @llvm.fmuladd.f32(float %3, float %4, float %fma)
; CHECK-NEXT: ret float %fma1


; int f8(int a, int b, int c) {
;     return a * b + c;
; }

define dso_local noundef i32 @_Z2f8iii(i32 noundef %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr {
  %4 = mul nsw i32 %0, %1
  %5 = add nsw i32 %4, %2
  ret i32 %5
}

; CHECK-LABEL: @_Z2f8iii
; CHECK: %4 = mul nsw i32 %0, %1
; CHECK-NEXT: %5 = add nsw i32 %4, %2
; CHECK-NEXT: ret i32 %5


; int f9(int a, int b, int c, int d, int e) {
;     int result1 = a * b + c;
;     int result2 = d * e + result1;
;     return result2;
; }

define dso_local noundef i32 @_Z2f9iiiii(i32 noundef %0, i32 noundef %1, i32 noundef %2, i32 noundef %3, i32 noundef %4) local_unnamed_addr {
  %6 = mul nsw i32 %0, %1
  %7 = add nsw i32 %6, %2
  %8 = mul nsw i32 %3, %4
  %9 = add nsw i32 %7, %8
  ret i32 %9
}

; CHECK-LABEL: @_Z2f9iiiii
; CHECK: %6 = mul nsw i32 %0, %1
; CHECK-NEXT: %7 = add nsw i32 %6, %2
; CHECK-NEXT: %8 = mul nsw i32 %3, %4
; CHECK-NEXT: %9 = add nsw i32 %7, %8
; CHECK-NEXT: ret i32 %9
