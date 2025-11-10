; RUN: opt -passes=mark-pure-functions -S %s | FileCheck %s

; Чистая функция - только операции с регистрами
define i32 @pure_function(i32 %a, i32 %b) {
entry:
  %add = add i32 %a, %b
  %mul = mul i32 %add, 2
  ret i32 %mul
}
; CHECK: define i32 @pure_function(i32 %a, i32 %b) #0
; CHECK: attributes #0 = { memory(none) }

; Нечистая функция - есть вызов другой функции
declare void @external_function()

define i32 @impure_function_call(i32 %a) {
entry:
  call void @external_function()
  %result = add i32 %a, 1
  ret i32 %result
}
; CHECK: define i32 @impure_function_call(i32 %a)

; Нечистая функция - работа с памятью
define i32 @impure_function_memory(i32* %ptr) {
entry:
  %val = load i32, i32* %ptr
  %result = add i32 %val, 1
  ret i32 %result
}
; CHECK: define i32 @impure_function_memory(i32* %ptr)

; Чистая функция с константной глобальной переменной
@global_constant = constant i32 42

define i32 @pure_function_constant() {
entry:
  %val = load i32, i32* @global_constant
  ret i32 %val
}
; CHECK: define i32 @pure_function_constant() #0
; CHECK: attributes #0 = { memory(none) }