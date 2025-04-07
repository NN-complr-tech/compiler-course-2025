; RUN: opt -load-pass-plugin %llvmshlibdir/PassAdd_Komshina_Daria_FIIT1_LLVM_IR%pluginext \
; RUN: -passes="PassAdd" -S %s | FileCheck %s --check-prefix=CHECK-ADD
; RUN: opt -load-pass-plugin %llvmshlibdir/PassAdd_Komshina_Daria_FIIT1_LLVM_IR%pluginext \
; RUN: -passes="PassAdd" -S %s --disable-output /dev/null 2>&1 | FileCheck %s --check-prefix=CHECK-NOADD

; CHECK-ADD-LABEL: define i32 @add(i32 %a, i32 %b)
; CHECK-ADD: %result = add i32 %a, %b
; CHECK-ADD: ret i32 %result

; CHECK-ADD-LABEL: define i32 @foo(i32 %x, i32 %y)
; CHECK-ADD: call i32 @add(i32 %x, i32 %y)
; CHECK-ADD-NOT: add i32

define i32 @add(i32 %a, i32 %b) {
  %result = add i32 %a, %b
  ret i32 %result
}

define i32 @foo(i32 %x, i32 %y) {
  %sum = add i32 %x, %y
  ret i32 %sum
}

; CHECK-ADD-LABEL: define i64 @bar(i64 %m, i64 %n)
; CHECK-ADD-NOT: call i64 @add
; CHECK-ADD: %sum = add i64 %m, %n

define i64 @bar(i64 %m, i64 %n) {
  %sum = add i64 %m, %n
  ret i64 %sum
}

; CHECK-NOADD-LABEL: define i32 @test_no_add_function(i32 %a, i32 %b)
; CHECK-NOADD: %sum = add i32 %a, %b
; CHECK-NOADD-NOT: call i32 @add

; Модуль без функции add
; MODULE: test_no_add_function.ll
define i32 @test_no_add_function(i32 %a, i32 %b) {
  %sum = add i32 %a, %b
  ret i32 %sum
}