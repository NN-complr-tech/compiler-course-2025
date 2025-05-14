; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacer_Khovansky_Dmitry_FIIT2_LLVM_IR%pluginext\
; RUN: -passes=AddReplacer -S %s | FileCheck %s

; === сама функция add ===
; CHECK-LABEL: define i32 @add(
; CHECK-NEXT:  entry:
; CHECK-NEXT:  %result = add i32 %a, %b
; CHECK-NEXT:  ret i32 %result
define i32 @add(i32 %a, i32 %b) {
entry:
  %result = add i32 %a, %b
  ret i32 %result
}

; === Test 1: Заменяется add на вызов @add ===
; CHECK-LABEL: define i32 @should_replace(
; CHECK-NEXT:  entry:
; CHECK-NEXT:  call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT:  ret i32
; CHECK-NOT:   add i32 %x, %y
define i32 @should_replace(i32 %x, i32 %y) {
entry:
  %sum = add i32 %x, %y
  ret i32 %sum
}

; === Test 2: используется именно @add, а не его сигнатура ===
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


; === Test 3: проверка константы ===
; CHECK-LABEL: define i32 @const_operand(
; CHECK-NEXT:  entry:
; CHECK-NEXT:  add i32 %x, 42
; CHECK-NEXT:  ret i32
; CHECK-NOT:   call i32 @add(i32 %x, 42)
define i32 @const_operand(i32 %x) {
entry:
  %sum = add i32 %x, 42
  ret i32 %sum
}

; === Test 4: проверка fadd ===
; CHECK-LABEL: define double @double_add(
; CHECK-NEXT:  entry:
; CHECK-NEXT:  fadd double %a, %b
; CHECK-NEXT:  ret double
; CHECK-NOT:   call i32 @add(i32 %a, i32 %b)
define double @double_add(double %a, double %b) {
entry:
  %sum = fadd double %a, %b
  ret double %sum
}
