; RUN: opt -load-pass-plugin=%llvmshlibdir/PureFuncPass_Koshkin_Nikita_FIIT3_LLVM_IR%pluginext -passes="mark_pure_func_pass" -S %s 2>&1 | FileCheck %s

; ------------------------------------------------------------------------
; 1) Тесты из примера 
; ------------------------------------------------------------------------
; CHECK: define dso_local i32 @add(i32 %a, i32 %b) #0 {
define dso_local i32 @add(i32 %a, i32 %b) {
entry:
  %0 = add nsw i32 %a, %b
  ret i32 %0
}

@.g_value = external global float

; CHECK-NOT: define dso_local float @foo(float %a) #0
define dso_local float @foo(float %a) {
entry:
  %0 = load float, float* @.g_value, align 4
  %1 = fmul float %a, %0
  ret float %1
}

; ------------------------------------------------------------------------
; 2) Простая чистая функция
; ------------------------------------------------------------------------
; CHECK: define dso_local i32 @compute(i32 %a, i32 %b, i32 %c) #0 {
define dso_local i32 @compute(i32 %a, i32 %b, i32 %c) {
entry:
  %sum  = add nsw i32 %b, %c
  %prod = mul nsw i32 %a, %sum
  ret i32 %prod
}

; ------------------------------------------------------------------------
; 3) Внешние переменные и функции, нечистые функции, примеры по википедии
; ------------------------------------------------------------------------
@global_state = global i32 0

declare i32 @rand()
declare i64 @time(i64*)

; CHECK-NOT: define dso_local float @use_global() #0
define dso_local float @use_global() {
entry:
  %v = load i32, i32* @global_state, align 4
  %f = sitofp i32 %v to float
  ret float %f
}

; CHECK-NOT: define dso_local i32 @get_random() #0
define dso_local i32 @get_random() {
entry:
  %r = call i32 @rand()
  ret i32 %r
}

; CHECK-NOT: define dso_local i64 @get_time() #0
define dso_local i64 @get_time() {
entry:
  %t = call i64 @time(i64* null)
  ret i64 %t
}

; CHECK-NOT: define dso_local void @set_global(i32 %x) #0
define dso_local void @set_global(i32 %x) {
entry:
  store i32 %x, i32* @global_state, align 4
  ret void
}

; CHECK: attributes #0 = { "pure" }
attributes #0 = { "pure" }