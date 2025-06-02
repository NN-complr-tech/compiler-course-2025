; RUN: opt -load-pass-plugin %llvmshlibdir/ReplaceAddWithCall_Sedova_Olga_FIIT1_LLVM_IR%pluginext\
; RUN: -passes=replace-add -S %s | FileCheck %s

; CHECK-LABEL: define i32 @add(
; CHECK:         add i32 %
; CHECK:         ret i32

; CHECK-LABEL: define i32 @foo(
; CHECK:         call i32 @add(i32 %x, i32 %y)
; CHECK:         ret i32

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