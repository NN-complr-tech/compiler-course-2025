; RUN: opt -load-pass-plugin %llvmshlibdir/FmulFaddPass_Sozonov_Ilya_FIIT3_LLVM_IR%pluginext -passes=FmulFaddPass -S %s | FileCheck %s

; Test 1: (a * b) + c - should be replaced with llvm.fmuladd(a, b, c)
; CHECK-LABEL: @mul_add_direct
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT: ret float %0

define float @mul_add_direct(float %a, float %b, float %c) {
entry:
  %mul = fmul float %a, %b
  %add = fadd float %mul, %c
  ret float %add
}

; Test 2: c + (a * b) - should also be replaced with llvm.fmuladd(a, b, c)
; CHECK-LABEL: @add_commuted_mul
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT: ret float %0

define float @add_commuted_mul(float %a, float %b, float %c) {
entry:
  %mul = fmul float %a, %b
  %add = fadd float %c, %mul
  ret float %add
}

; Test 3: non-pattern input - should NOT be replaced
; CHECK-LABEL: @no_pattern
; CHECK-NEXT: entry:
; CHECK-NEXT: %add = fadd float %c, %b
; CHECK-NEXT: %mul = fmul float %a, %add
; CHECK-NEXT: ret float %mul

define float @no_pattern(float %a, float %b, float %c) {
entry:
  %add = fadd float %c, %b
  %mul = fmul float %a, %add
  ret float %mul
}