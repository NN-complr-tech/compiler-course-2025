; RUN: opt -load-pass-plugin=%llvmshlibdir/VectorCounterPass_IvashchukVA_FIIT2_BACKEND%shlibext -passes=vector-counter -S %s | FileCheck %s

define void @test_vector() {
; CHECK-LABEL: @test_vector
  %vec1 = add <4 x i32> zeroinitializer, zeroinitializer
; CHECK: load i64, ptr @vector_instructions_counter
; CHECK: add i64
; CHECK: store i64
  %vec2 = fadd <2 x double> zeroinitializer, zeroinitializer
; CHECK: load i64, ptr @vector_instructions_counter
; CHECK: add i64
; CHECK: store i64
  ret void
}

; CHECK: @vector_instructions_counter = global i64 0