; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacer_Khovansky_Dmitry_FIIT2_LLVM_IR%pluginext\
; RUN: -passes=AddReplacer -S %s | FileCheck %s

; === Test 5: user's add function with 3 arguments ===
; CHECK-LABEL: define i32 @test_three_args(
; CHECK-NEXT:  entry:
; CHECK-NEXT:  add i32 %x, %y
; CHECK-NEXT:  ret i32
; CHECK-NOT:   call i32 @add(i32 %x, i32 %y)

define i32 @add(i32 %a, i32 %b, i32 %c) {
entry:
  %sum = add i32 %a, %b
  %sum2 = add i32 %sum, %c
  ret i32 %sum2
}

define i32 @test_three_args(i32 %x, i32 %y) {
entry:
  %sum = add i32 %x, %y
  ret i32 %sum
}
