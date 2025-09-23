; RUN: opt -load-pass-plugin %llvmshlibdir/FMAOptimizer_Chastov_Vyacheslav_FIIT2_LLVM_IR%pluginext

; CHECK-LABEL: @test_basic_fusion(
; CHECK-NEXT:  %res = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT:  ret float %res
define float @test_basic_fusion(float %a, float %b, float %c) {
  %mul = fmul float %a, %b
  %add = fadd float %mul, %c
  ret float %add
}

; CHECK-LABEL: @test_reversed_operands(
; CHECK-NEXT:  %res = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT:  ret float %res
define float @test_reversed_operands(float %a, float %b, float %c) {
  %mul = fmul float %a, %b
  %add = fadd float %c, %mul
  ret float %add
}

; CHECK-LABEL: @test_no_fusion_add_first(
; CHECK:       %sum = fadd float %a, %b
; CHECK-NEXT:  %res = fmul float %sum, %c
define float @test_no_fusion_add_first(float %a, float %b, float %c) {
  %sum = fadd float %a, %b
  %res = fmul float %sum, %c
  ret float %res
}

; CHECK-LABEL: @test_constants(
; CHECK-NEXT:  %res = call double @llvm.fmuladd.f64(double 2.500000e+00, double 4.000000e+00, double 1.000000e+00)
; CHECK-NEXT:  ret double %res
define double @test_constants() {
  %mul = fmul double 2.5, 4.0
  %add = fadd double %mul, 1.0
  ret double %add
}

; CHECK-LABEL: @test_vector_type(
; CHECK-NEXT:  %res = call <2 x float> @llvm.fmuladd.v2f32(<2 x float> %a, <2 x float> %b, <2 x float> %c)
; CHECK-NEXT:  ret <2 x float> %res
define <2 x float> @test_vector_type(<2 x float> %a, <2 x float> %b, <2 x float> %c) {
  %mul = fmul <2 x float> %a, %b
  %add = fadd <2 x float> %mul, %c
  ret <2 x float> %add
}

; CHECK-LABEL: @test_no_fusion_due_to_multiple_uses(
; CHECK:       %mul = fmul float %a, %b
; CHECK-NEXT:  %add1 = fadd float %mul, %c
; CHECK-NEXT:  %add2 = fadd float %mul, %d
define float @test_no_fusion_due_to_multiple_uses(float %a, float %b, float %c, float %d) {
  %mul = fmul float %a, %b
  %add1 = fadd float %mul, %c
  %add2 = fadd float %mul, %d
  %res = fadd float %add1, %add2
  ret float %res
}

; CHECK-LABEL: @test_nested_operations(
; CHECK:       %fma = call float @llvm.fmuladd.f32(float %a, float %b, float %c)
; CHECK-NEXT:  %res = fadd float %fma, %d
define float @test_nested_operations(float %a, float %b, float %c, float %d) {
  %mul = fmul float %a, %b
  %add1 = fadd float %mul, %c
  %res = fadd float %add1, %d
  ret float %res
}

; CHECK-LABEL: @test_half_type(
; CHECK-NEXT:  %res = call half @llvm.fmuladd.f16(half %a, half %b, half %c)
; CHECK-NEXT:  ret half %res
define half @test_half_type(half %a, half %b, half %c) {
  %mul = fmul half %a, %b
  %add = fadd half %mul, %c
  ret half %add
}

; CHECK-LABEL: @test_negative_values(
; CHECK-NEXT:  %res = call float @llvm.fmuladd.f32(float %a, float -3.000000e+00, float -5.000000e+00)
; CHECK-NEXT:  ret float %res
define float @test_negative_values(float %a) {
  %mul = fmul float %a, -3.0
  %add = fadd float %mul, -5.0
  ret float %add
}

; CHECK-LABEL: @test_mixed_types(
; CHECK:       %mul = fmul float %a, %b
; CHECK-NEXT:  %add = fadd double %conv, %c
define double @test_mixed_types(float %a, float %b, double %c) {
  %mul = fmul float %a, %b
  %conv = fpext float %mul to double
  %add = fadd double %conv, %c
  ret double %add
}