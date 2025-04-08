; RUN: opt -load-pass-plugin %llvmshlibdir/FmulFaddPass_Sozonov_Ilya_FIIT3_LLVM_IR%pluginext -passes=FmulFaddPass -S %s | FileCheck %s

; Test 1: (a * b) + c - should be replaced with llvm.fmuladd(a, b, c)
; CHECK-LABEL: @fmul_fadd_ab_c
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT: ret float %0
define float @fmul_fadd_ab_c(float %a, float %b, float %c) {
entry:
  %mul = fmul float %a, %b
  %add = fadd float %mul, %c
  ret float %add
}

; Test 2: c + (a * b) - should also be replaced with llvm.fmuladd(a, b, c)
; CHECK-LABEL: @fadd_c_fmul_ab
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT: ret float %0
define float @fadd_c_fmul_ab(float %a, float %b, float %c) {
entry:
  %mul = fmul float %a, %b
  %add = fadd float %c, %mul
  ret float %add
}

; Test 3: a * (c + b) - addition first (no matching pattern)
; CHECK-LABEL: @no_match_fadd_then_fmul
; CHECK-NEXT: entry:
; CHECK-NEXT: %add = fadd float %c, %b
; CHECK-NEXT: %mul = fmul float %a, %add
; CHECK-NEXT: ret float %mul
define float @no_match_fadd_then_fmul(float %a, float %b, float %c) {
entry:
  %add = fadd float %c, %b
  %mul = fmul float %a, %add
  ret float %mul
}

; Test 4: (a + b) * c — addition comes first (should not match)
; CHECK-LABEL: @no_match_add_then_mul
; CHECK-NEXT: entry:
; CHECK-NEXT: %add = fadd float %a, %b
; CHECK-NEXT: %mul = fmul float %add, %c
; CHECK-NEXT: ret float %mul
define float @no_match_add_then_mul(float %a, float %b, float %c) {
entry:
  %add = fadd float %a, %b
  %mul = fmul float %add, %c
  ret float %mul
}

; Test 5: (a * a) + a - same operand used in multiple places (should still replace)
; CHECK-LABEL: @fmul_fadd_reuse
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float %a, float %a, float %a)
; CHECK-NEXT: ret float %0
define float @fmul_fadd_reuse(float %a) {
entry:
  %mul = fmul float %a, %a
  %add = fadd float %mul, %a
  ret float %add
}

; Test 6: ((a * b) + c) + d - nested expression (should replace inner only)
; CHECK-LABEL: @nested_fmul_fadd_then_add
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT: %add2 = fadd float %0, %d
; CHECK-NEXT: ret float %add2
define float @nested_fmul_fadd_then_add(float %a, float %b, float %c, float %d) {
entry:
  %mul = fmul float %a, %b
  %add1 = fadd float %mul, %c
  %add2 = fadd float %add1, %d
  ret float %add2
}

; Test 7: (a * b) + c — double precision (should replace)
; CHECK-LABEL: @fmul_fadd_double
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %a, double %b, double %c)
; CHECK-NEXT: ret double %0
define double @fmul_fadd_double(double %a, double %b, double %c) {
entry:
  %mul = fmul double %a, %b
  %add = fadd double %mul, %c
  ret double %add
}

; Test 8: (2.0 * a) + 1.0 - used constants (should replace)
; CHECK-LABEL: @fmul_fadd_with_constants
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float 2.000000e+00, float %a, float 1.000000e+00)
; CHECK-NEXT: ret float %0
define float @fmul_fadd_with_constants(float %a) {
entry:
  %mul = fmul float 2.0, %a
  %add = fadd float %mul, 1.0
  ret float %add
}

; Test 9: (x * x) + y - one operand used multiple times (should replace)
; CHECK-LABEL: @fmul_fadd_duplicate_operand
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float %x, float %x, float %y)
; CHECK-NEXT: ret float %0
define float @fmul_fadd_duplicate_operand(float %x, float %y) {
entry:
  %mul = fmul float %x, %x
  %add = fadd float %mul, %y
  ret float %add
}

; Test 10: (x = a * b; y = x + c; return x + y) - use of intermediate result
; CHECK-LABEL: @fmul_used_multiple_times
; CHECK-NEXT: entry:
; CHECK-NEXT: %mul = fmul float %a, %b
; CHECK-NEXT: %add = fadd float %mul, %c
; CHECK-NEXT: %add2 = fadd float %mul, %add
; CHECK-NEXT: ret float %add2
define float @fmul_used_multiple_times(float %a, float %b, float %c) {
entry:
  %mul = fmul float %a, %b
  %add = fadd float %mul, %c
  %add2 = fadd float %mul, %add
  ret float %add2
}

; Test 11: already fused with llvm.fmuladd
; CHECK-LABEL: @already_fused_fmuladd
; CHECK-NEXT: entry:
; CHECK-NEXT: %fma = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT: ret float %fma
define float @already_fused_fmuladd(float %a, float %b, float %c) {
entry:
  %fma = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
  ret float %fma
}
