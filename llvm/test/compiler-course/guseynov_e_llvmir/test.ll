; RUN: opt -load-pass-plugin %llvmshlibdir/addReplacePass_Guseynov_Emil_FIIT2_LLVM_IR%pluginext\
; RUN: -passes=addReplacePass -S %s | FileCheck %s

; Test case for AddReplacer pass

; === сама функция add ===
; CHECK-LABEL: define i32 @add(
; CHECK-NEXT:  entry:
; CHECK-NEXT:  add i32 %a, %b
; CHECK-NEXT:  ret i32 %result
define i32 @add(i32 %a, i32 %b) {
entry:
  %result = add i32 %a, %b
  ret i32 %result
}

; === функция foo: add должен быть заменен на call ===
; CHECK-LABEL: define i32 @foo(
; CHECK-NEXT:  entry:
; CHECK-NEXT:  call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT:  ret i32 %sum
define i32 @foo(i32 %x, i32 %y) {
entry:
  %sum = add i32 %x, %y
  ret i32 %sum
}

; === функция с разными типами: add НЕ должен быть заменен ===
; CHECK-LABEL: define i64 @bar(
; CHECK-NEXT:  entry:
; CHECK-NEXT:  add i64 %x, %y
; CHECK-NEXT:  ret i64 %sum
define i64 @bar(i64 %x, i64 %y) {
entry:
  %sum = add i64 %x, %y
  ret i64 %sum
}

; === функция с константами: add НЕ должен быть заменен ===
; CHECK-LABEL: define i32 @baz(
; CHECK-NEXT:  entry:
; CHECK-NEXT:  add i32 %x, 5
; CHECK-NEXT:  add i32 10, %y
; CHECK-NEXT:  call i32 @add(i32 %sum1, i32 %sum2)
; CHECK-NEXT:  ret i32 %total
define i32 @baz(i32 %x, i32 %y) {
entry:
  %sum1 = add i32 %x, 5
  %sum2 = add i32 10, %y
  %total = add i32 %sum1, %sum2
  ret i32 %total
}

; === функция с плавающей точкой: add НЕ должен быть заменен ===
; CHECK-LABEL: define float @float_add(
; CHECK-NEXT:  entry:
; CHECK-NEXT:  fadd float %x, %y
; CHECK-NEXT:  ret float %sum
define float @float_add(float %x, float %y) {
entry:
  %sum = fadd float %x, %y
  ret float %sum
}

; === используется именно @add, а не его сигнатура ===
; CHECK-LABEL: define i32 @wrong_sig_user(
; CHECK-NEXT:  entry:
; CHECK-NEXT:  call i32 @add(i32 %x, i32 %y)
; CHECK-NOT:   call i64 @add_wrong_sig
define i64 @add_wrong_sig(i64 %a, i64 %b) {
entry:
  %r = add i64 %a, %b
  ret i64 %r
}

define i32 @wrong_sig_user(i32 %x, i32 %y) {
entry:
  %sum = add i32 %x, %y
  ret i32 %sum
}