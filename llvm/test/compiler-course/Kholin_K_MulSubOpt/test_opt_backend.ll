; RUN: opt -load-pass-plugin %llvmshlibdir/MulSubPass_KholinKirill_FIIT3_BACKEND%pluginext -passes=mulsub-opt-pass -S %s | FileCheck %s

; CHECK-LABEL: define dso_local noundef float @_Z20classic_mul_sub_testfff(float noundef %a, float noundef %b, float noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = fneg float %a
; CHECK-NEXT: %1 = call float @llvm.fma.f32(float %0, float %b, float %c)
; CHECK-NEXT: ret float %1

define dso_local noundef float @_Z20classic_mul_sub_testfff(float noundef %a, float noundef %b, float noundef %c) {
entry:
  %mul = fmul float %a, %b
  %sub = fsub float %c, %mul
  ret float %sub
}

; float classic_mul_sub_test(float a, float b, float c) {
;   float tmp = a * b;
;   float d = c - tmp;
;   return d;
; }

; CHECK-LABEL: define dso_local noundef float @_Z28reverse_classic_mul_sub_testfff(float noundef %a, float noundef %b, float noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = fneg float %a
; CHECK-NEXT: %1 = call float @llvm.fma.f32(float %0, float %b, float %c)
; CHECK-NEXT: ret float %1

define dso_local noundef float @_Z28reverse_classic_mul_sub_testfff(float noundef %a, float noundef %b, float noundef %c) {
entry:
  %0 = fmul float %a, %b
  %add = fsub float %c, %0
  ret float %add
}

; float reverse_classic_mul_sub_test(float a, float b, float c) {
;   float tmp = a * b;
;   float d = -tmp + c;
;   return d;
; }

; CHECK-LABEL: define dso_local noundef float @_Z19return_mul_sub_testfff(float noundef %a, float noundef %b, float noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = fneg float %a
; CHECK-NEXT: %1 = call float @llvm.fma.f32(float %0, float %b, float %c)
; CHECK-NEXT: ret float %1


define dso_local noundef float @_Z19return_mul_sub_testfff(float noundef %a, float noundef %b, float noundef %c) {
entry:
  %mul = fmul float %a, %b
  %sub = fsub float %c, %mul
  ret float %sub
}

; float return_mul_sub_test(float a, float b, float c) {
;   float tmp = b * a;
;   return c - tmp;
; }

; CHECK-LABEL: define dso_local noundef float @_Z21multiple_mul_sub_testfff(float noundef %x, float noundef %y, float noundef %z) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = fneg float %x
; CHECK-NEXT: %1 = call float @llvm.fma.f32(float %0, float %y, float %z)
; CHECK-NEXT: %2 = fneg float %y
; CHECK-NEXT: %3 = call float @llvm.fma.f32(float %2, float %z, float %x)
; CHECK-NEXT: fadd float %1, %3
; CHECK-NEXT: ret float %add

define dso_local noundef float @_Z21multiple_mul_sub_testfff(float noundef %x, float noundef %y, float noundef %z) {
entry:
  %mul = fmul float %x, %y
  %sub = fsub float %z, %mul
  %mul1 = fmul float %y, %z
  %sub2 = fsub float %x, %mul1
  %add = fadd float %sub, %sub2
  ret float %add
}

; float multiple_mul_sub_test(float x, float y, float z) {
;   float t1 = x * y;
;   float res1 = z - t1;
;
;   float t2 = y * z;
;   float res2 = x - t2;
;
;   return res1 + res2;
; }

; CHECK-LABEL: define dso_local noundef double @_Z27classic_double_mul_sub_testddd(double noundef %a, double noundef %b, double noundef %c) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = fneg double %a
; CHECK-NEXT: %1 = call double @llvm.fma.f64(double %0, double %b, double %c)
; CHECK-NEXT: ret double %1

define dso_local noundef double @_Z27classic_double_mul_sub_testddd(double noundef %a, double noundef %b, double noundef %c) {
entry:
  %mul = fmul double %a, %b
  %sub = fsub double %c, %mul
  ret double %sub
}

; double classic_double_mul_sub_test(double a, double b, double c) {
;   double tmp = a * b;
;   double d = c - tmp;
;   return d;
; }

; CHECK-LABEL: define dso_local noundef float @_Z19nested_mul_sub_testffff(float noundef %a, float noundef %b, float noundef %c, float noundef %d) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = fneg float %a
; CHECK-NEXT: %1 = call float @llvm.fma.f32(float %0, float %b, float %c)
; CHECK-NEXT: %mul1 = fmul float %1, %d
; CHECK-NEXT: ret float %mul1

define dso_local noundef float @_Z19nested_mul_sub_testffff(float noundef %a, float noundef %b, float noundef %c, float noundef %d) {
entry:
  %mul = fmul float %a, %b
  %sub = fsub float %c, %mul
  %mul1 = fmul float %sub, %d
  ret float %mul1
}

; float nested_mul_sub_test(float a, float b, float c, float d) {
;  float tmp1 = a * b;
;  float tmp2 = c - tmp1;
;  float tmp3 = tmp2 * d;
;  return tmp3;
; }
