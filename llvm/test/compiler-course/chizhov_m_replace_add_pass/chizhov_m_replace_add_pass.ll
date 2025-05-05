; RUN: opt -load-pass-plugin %llvmshlibdir/ReplaceAddPass_Chizhov_Maxim_FIIT3_LLVM_IR%pluginext\
; RUN: -passes=ReplaceAddPass -S %s | FileCheck %s

; CHECK-LABEL: define i32 @add(i32 %a, i32 %b)
; CHECK: entry:
; CHECK-NEXT: %result = add i32 %a, %b
; CHECK-NEXT: ret i32 %result

; CHECK-LABEL: define i32 @foo(i32 %x, i32 %y)
; CHECK: entry:
; CHECK-NEXT: call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT: ret i32 %sum


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
