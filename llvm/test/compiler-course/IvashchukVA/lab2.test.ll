; RUN: opt -passes=mark-pure-functions -S %s | FileCheck %s

define i32 @pure_function(i32 %a, i32 %b) {
entry:
  %add = add i32 %a, %b
  %mul = mul i32 %add, 2
  ret i32 %mul
}
; CHECK: define i32 @pure_function(i32 %a, i32 %b) #0
; CHECK: attributes #0 = { memory(none) }

declare void @external_function()

define i32 @impure_function_call(i32 %a) {
entry:
  call void @external_function()
  %result = add i32 %a, 1
  ret i32 %result
}
; CHECK: define i32 @impure_function_call(i32 %a)