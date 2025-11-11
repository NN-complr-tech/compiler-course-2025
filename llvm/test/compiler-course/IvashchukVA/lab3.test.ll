; RUN: opt -passes=vector-counter -S %s | FileCheck %s

define void @test_vector_ops(<4 x float> %a, <4 x float> %b) {
entry:
  %add = fadd <4 x float> %a, %b
  %mul = fmul <4 x float> %add, %b
  ret void
}

; CHECK: @vector_instructions_counter = global i64 0
; CHECK: load i64, i64* @vector_instructions_counter
; CHECK: add i64
; CHECK: store i64