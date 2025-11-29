; RUN: opt -load-pass-plugin=%llvmshlibdir/PureFunctionsPass_IvashchukVA_FIIT2_LLVM_IR%shlibext -passes=mark-pure-functions -S %s | FileCheck %s

; Тест 1: Простая чистая функция
define void @pure_simple() {
; CHECK: define void @pure_simple() #0
  ret void
}

; Тест 2: Функция с внешним вызовом (нечистая)
define void @impure_external_call() {
; CHECK: define void @impure_external_call() {
  call void @external_function()
  ret void
}

; Тест 3: Функция с доступом к памяти (нечистая)
define i32 @impure_memory_read(i32* %ptr) {
; CHECK: define i32 @impure_memory_read(i32* %ptr) {
  %val = load i32, i32* %ptr
  ret i32 %val
}

; Тест 4: Функция с записью в память (нечистая)
define void @impure_memory_write(i32* %ptr, i32 %val) {
; CHECK: define void @impure_memory_write(i32* %ptr, i32 %val) {
  store i32 %val, i32* %ptr
  ret void
}

; Тест 5: Функция с intrinsic вызовом (чистая)
define double @pure_intrinsic(double %x) {
; CHECK: define double @pure_intrinsic(double %x) #0
  %result = call double @llvm.sqrt.f64(double %x)
  ret double %result
}

; Тест 6: Функция только с чтением памяти (нечистая)
define i32 @impure_only_read(i32* %ptr1, i32* %ptr2) {
; CHECK: define i32 @impure_only_read(i32* %ptr1, i32* %ptr2) {
  %a = load i32, i32* %ptr1
  %b = load i32, i32* %ptr2
  %sum = add i32 %a, %b
  ret i32 %sum
}

; Тест 7: Рекурсивная чистая функция (с базовым случаем)
define i32 @pure_recursive_factorial(i32 %n) {
; CHECK: define i32 @pure_recursive_factorial(i32 %n) #0
  %cmp = icmp sle i32 %n, 1
  br i1 %cmp, label %base, label %recursive

base:
  ret i32 1

recursive:
  %n_minus_one = sub i32 %n, 1
  %recursive_result = call i32 @pure_recursive_factorial(i32 %n_minus_one)
  %result = mul i32 %n, %recursive_result
  ret i32 %result
}

; Тест 8: Функция с ветвлением (чистая)
define i32 @pure_branching_max(i32 %a, i32 %b) {
; CHECK: define i32 @pure_branching_max(i32 %a, i32 %b) #0
  %cmp = icmp sgt i32 %a, %b
  br i1 %cmp, label %then, label %else

then:
  ret i32 %a

else:
  ret i32 %b
}

; Тест 9: Функция с вызовом чистой функции (чистая)
define i32 @pure_calls_pure(i32 %x, i32 %y) {
; CHECK: define i32 @pure_calls_pure(i32 %x, i32 %y) #0
  %sum = call i32 @pure_adder(i32 %x, i32 %y)
  ret i32 %sum
}

; Тест 10: Функция с вызовом нечистой функции (нечистая)
define void @impure_calls_impure() {
; CHECK: define void @impure_calls_impure() {
  call void @impure_external_call()
  ret void
}

; Вспомогательные объявления
declare void @external_function()
declare double @llvm.sqrt.f64(double)

; Вспомогательная чистая функция
define i32 @pure_adder(i32 %a, i32 %b) {
; CHECK: define i32 @pure_adder(i32 %a, i32 %b) #0
  %result = add i32 %a, %b
  ret i32 %result
}

; Функция main чтобы избежать предупреждений о неиспользуемых функциях
define i32 @main() {
  call void @pure_simple()
  call void @impure_external_call()
  
  %ptr = alloca i32
  store i32 42, i32* %ptr
  
  %val1 = call i32 @impure_memory_read(i32* %ptr)
  call void @impure_memory_write(i32* %ptr, i32 %val1)
  
  %sqrt_val = call double @pure_intrinsic(double 4.0)
  %val2 = call i32 @impure_only_read(i32* %ptr, i32* %ptr)
  
  %fact = call i32 @pure_recursive_factorial(i32 5)
  %max_val = call i32 @pure_branching_max(i32 10, i32 20)
  %sum_val = call i32 @pure_calls_pure(i32 1, i32 2)
  
  call void @impure_calls_impure()
  
  ret i32 0
}

; CHECK: attributes #0 = { memory(none) }