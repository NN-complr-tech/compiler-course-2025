; RUN: opt -passes=vector-counter -S %s | FileCheck %s

define void @test_vector_ops(<4 x float> %a, <4 x float> %b) {
entry:
  ; Vector addition
  %add = fadd <4 x float> %a, %b
  ; CHECK: fadd <4 x float>
  ; CHECK: load i64, i64* @vector_instructions_counter
  
  ; Vector multiplication
  %mul = fmul <4 x float> %add, %b
  ; CHECK: fmul <4 x float>
  ; CHECK: load i64, i64* @vector_instructions_counter
  
  ret void
}

define void @test_scalar_ops(float %a, float %b) {
entry:
  ; Scalar operations - should not be counted
  %add = fadd float %a, %b
  %mul = fmul float %add, %b
  ret void
}

; CHECK: @vector_instructions_counter = global i64 0
; CHECK: define void @print_vector_counter()