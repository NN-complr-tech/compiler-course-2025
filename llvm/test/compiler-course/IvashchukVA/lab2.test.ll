; RUN: opt -load-pass-plugin=%llvmshlibdir/PureFunctionsPass_IvashchukVA_FIIT2_LLVM_IR%shlibext -passes=mark-pure-functions -S %s | FileCheck %s

; Тест 1: Простая чистая функция
define void @pure_simple() {
; CHECK: define void @pure_simple() #0
  ret void
}

; Тест 2: Функция с доступом к памяти (нечистая)
define i32 @impure_memory_read(ptr %ptr) {
; CHECK: define i32 @impure_memory_read(ptr %ptr) {
  %val = load i32, ptr %ptr
  ret i32 %val
}

; Тест 3: Рекурсивная чистая функция
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

; Тест 4: Функция с intrinsic вызовом (чистая)
define double @pure_intrinsic(double %x) {
; CHECK: define double @pure_intrinsic(double %x) #0
  %result = call double @llvm.sqrt.f64(double %x)
  ret double %result
}

; Вспомогательные объявления
declare void @external_function()
declare double @llvm.sqrt.f64(double)

; Функция main чтобы избежать предупреждений о неиспользуемых функциях
define i32 @main() {
  call void @pure_simple()
  
  %ptr = alloca i32
  store i32 42, ptr %ptr
  
  %val1 = call i32 @impure_memory_read(ptr %ptr)
  %sqrt_val = call double @pure_intrinsic(double 4.0)
  %fact = call i32 @pure_recursive_factorial(i32 5)
  
  ret i32 0
}

; CHECK: attributes #0 = { memory(none) }