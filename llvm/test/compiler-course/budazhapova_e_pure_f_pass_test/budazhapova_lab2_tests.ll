; RUN: opt -load-pass-plugin %llvmshlibdir/PureFunctionPass_Budazhapova_Ekaterina_FIIT3_LLVM_IR%pluginext\
; RUN: -passes=mark-pure -S %s | FileCheck %s

define i32 @pure_add(i32 %a, i32 %b) {
; CHECK: define i32 @pure_add(i32 %a, i32 %b) #0
  %result = add i32 %a, %b
  ret i32 %result
}

define i32 @impure_function(i32 %a) {
; CHECK: define i32 @impure_function(i32 %a)
  %ptr = alloca i32
  store i32 %a, ptr %ptr
  %val = load i32, ptr %ptr
  ret i32 %val
}

declare void @external_func()
define i32 @function_with_call(i32 %a) {
; CHECK: define i32 @function_with_call(i32 %a)
  call void @external_func()
  %result = add i32 %a, 1
  ret i32 %result
}

define float @pure_multiply(float %x, float %y) {
; CHECK: define float @pure_multiply(float %x, float %y) #0
  %result = fmul float %x, %y
  ret float %result
}

; CHECK: attributes #0 = { "pure" }
