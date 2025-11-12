; RUN: opt -load-pass-plugin=%llvmshlibdir/VectorCounterPass_IvashchukVA_FIIT2_BACKEND%shlibext -passes=vector-counter -S %s | FileCheck %s

define void @test_vector() {
; CHECK-LABEL: @test_vector
  %vec1 = add <4 x i32> zeroinitializer, zeroinitializer
; CHECK: load i64, ptr @vector_instructions_counter
; CHECK: add i64 {{.*}}, 1
; CHECK: store i64 {{.*}}, ptr @vector_instructions_counter
  %vec2 = fadd <2 x double> zeroinitializer, zeroinitializer
; CHECK: load i64, ptr @vector_instructions_counter
; CHECK: add i64 {{.*}}, 1
; CHECK: store i64 {{.*}}, ptr @vector_instructions_counter
  ret void
}

; CHECK-DAG: @vector_instructions_counter = global i64 0