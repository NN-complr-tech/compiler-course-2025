; RUN: opt -load-pass-plugin=%llvmshlibdir/VectorCounterPass_IvashchukVA_FIIT2_BACKEND%shlibext -passes=vector-counter -S %s | FileCheck %s

; Тест 1: Одна векторная инструкция
define void @test_vector_add() {
  %vec = add <4 x i32> zeroinitializer, zeroinitializer
  ret void
}

; Тест 2: Несколько векторных инструкций разных типов
define void @test_multiple_vector_ops() {
  %vec1 = add <4 x i32> zeroinitializer, zeroinitializer
  %vec2 = mul <2 x i64> zeroinitializer, zeroinitializer
  %vec3 = fadd <8 x float> zeroinitializer, zeroinitializer
  ret void
}

; Тест 3: Смешанные векторные и скалярные инструкции
define void @test_mixed_instructions() {
  ; Скалярные операции
  %scalar1 = add i32 0, 0
  %scalar2 = mul i64 0, 0
  
  ; Векторные операции
  %vec1 = sub <4 x i32> zeroinitializer, zeroinitializer
  %vec2 = fdiv <2 x float> zeroinitializer, zeroinitializer
  
  ; Еще скалярные
  %scalar3 = and i1 true, false
  ret void
}

; Тест 4: Векторные инструкции в циклах
define void @test_vector_in_loop() {
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %i.next, %loop ]
  %vec = mul <4 x i32> zeroinitializer, zeroinitializer
  %i.next = add i32 %i, 1
  %cond = icmp slt i32 %i.next, 10
  br i1 %cond, label %loop, label %exit

exit:
  ret void
}

; CHECK: @vector_instructions_counter = global i64 0

; Проверяем что счетчик создается и используется
; CHECK: load i64, ptr @vector_instructions_counter
; CHECK: add i64 {{.*}}, 1
; CHECK: store i64 {{.*}}, ptr @vector_instructions_counter

; Должно быть несколько инкрементов счетчика
; CHECK: load i64, ptr @vector_instructions_counter
; CHECK: add i64 {{.*}}, 1
; CHECK: store i64 {{.*}}, ptr @vector_instructions_counter
; CHECK: load i64, ptr @vector_instructions_counter
; CHECK: add i64 {{.*}}, 1
; CHECK: store i64 {{.*}}, ptr @vector_instructions_counter