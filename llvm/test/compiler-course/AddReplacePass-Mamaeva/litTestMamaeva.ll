; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacePass_Mamaeva_Olga_FIIT3_LLVM_IR%pluginext \
; RUN: -passes="add-replace" -S %s | FileCheck %s

; Тест 1: Нет функции @add - инструкции должны остаться без изменений
; CHECK-LABEL: define i32 @test_no_add_function
; CHECK-NEXT: %res = add i32 %x, %y
; CHECK-NEXT: ret i32 %res

; Тест 2: Есть функция @add - должна быть замена
; CHECK-LABEL: define i32 @add
; CHECK-NEXT: %result = add i32 %a, %b
; CHECK-NEXT: ret i32 %result

; CHECK-LABEL: define i32 @foo
; CHECK-NEXT: %[[RESULT:.*]] = call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT: ret i32 %[[RESULT]]

; Тест 3: Неподходящие типы (i64) - не должно быть замены
; CHECK-LABEL: define i64 @bar
; CHECK-NEXT: %sum = add i64 %a, %b
; CHECK-NEXT: ret i64 %sum

define i32 @test_no_add_function(i32 %x, i32 %y) {
    %res = add i32 %x, %y
    ret i32 %res
}

define i32 @add(i32 %a, i32 %b) {
    %result = add i32 %a, %b
    ret i32 %result
}

define i32 @foo(i32 %x, i32 %y) {
    %result = add i32 %x, %y
    ret i32 %result
}

define i64 @bar(i64 %a, i64 %b) {
    %sum = add i64 %a, %b
    ret i64 %sum
}

; Проверка, что нет лишних вызовов @add
; CHECK-NOT: call i32 @add(i64
; CHECK-NOT: call i32 @add({{.*}})
