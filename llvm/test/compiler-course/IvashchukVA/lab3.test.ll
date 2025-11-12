; RUN: opt -load-pass-plugin=%llvmshlibdir/VectorCounterPass_IvashchukVA_FIIT2_BACKEND%shlibext -passes=vector-counter -S %s | FileCheck %s

define void @test_vector() {
  %vec = add <4 x i32> zeroinitializer, zeroinitializer
  ret void
}

; CHECK: @vector_instructions_counter = global i64 0
; CHECK: load i64, ptr @vector_instructions_counter
; CHECK: add i64 {{.*}}, 1
; CHECK: store i64 {{.*}}, ptr @vector_instructions_counter