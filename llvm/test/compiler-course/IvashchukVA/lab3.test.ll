; RUN: opt -load-pass-plugin=%llvmshlibdir/VectorCounterPass_IvashchukVA_FIIT2_BACKEND%shlibext -passes=vector-counter -S %s | FileCheck %s

define void @test_vector() {
; CHECK: @vector_instructions_counter = global i64 0
  %vec1 = add <4 x i32> zeroinitializer, zeroinitializer
  %vec2 = mul <2 x double> zeroinitializer, zeroinitializer
  ret void
}

; CHECK-LABEL: @test_vector
; CHECK: load i64, ptr @vector_instructions_counter
; CHECK: add i64 {{.*}}, 1
; CHECK: store i64 {{.*}}, ptr @vector_instructions_counter
; CHECK: load i64, ptr @vector_instructions_counter
; CHECK: add i64 {{.*}}, 1
; CHECK: store i64 {{.*}}, ptr @vector_instructions_counter