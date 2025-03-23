; RUN: opt -load-pass-plugin %llvmshlibdir/ReplaceAddPass_Beskhmelnova_Kseniya_FIIT1_LLVM_IR%pluginext \
; RUN: -passes="ReplaceAddPass" -S %s | FileCheck %s

; CHECK: define i32 @add(i32 %a, i32 %b)
; CHECK: %result = add i32 %a, %b
; CHECK: ret i32 %result

; CHECK: define i32 @foo(i32 %x, i32 %y)
; CHECK: call i32 @add(i32 %x, i32 %y)
; CHECK: ret i32 %sum

; CHECK-NOT: add i32

define i32 @add(i32 %a, i32 %b) {
entry:
  %result = add i32 %a, %b
  ret i32 %result
}

define i32 @foo(i32 %x, i32 %y) {
entry:
  %sum = add i32 %x, %y
  ret i32 %sum
}