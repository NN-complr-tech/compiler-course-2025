; RUN: opt -load-pass-plugin %llvmshlibdir/addReplacePass_Guseynov_Emil_FIIT2_LLVM_IR%pluginext\
; RUN: -passes=addReplacePass -S %s | FileCheck %s

; CHECK-LABEL: define i32 @test_with_add(
; CHECK-NEXT:  entry:
; CHECK-NEXT:  add i32 %x, %y
; CHECK-NEXT:  ret i32
; CHECK-NOT:   call i32 @add(i32 %x, i32 %y)

define i32 @test_with_add(i32 %x, i32 %y) {
entry:
  %sum = add i32 %x, %y
  ret i32 %sum
}