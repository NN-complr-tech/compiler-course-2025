; RUN: opt -load-pass-plugin %llvmshlibdir/PassAdd_Komshina_Daria_FIIT1_LLVM_IR%pluginext \
; RUN: -passes="PassAdd" -S %s | FileCheck %s --check-prefix=NOADD

; CHECK-LABEL: define i32 @no_add_func(i32 %x, i32 %y)
; CHECK-NOT: call i32 @add
; CHECK: %sum = add i32 %x, %y
; CHECK: ret i32 %sum
define i32 @no_add_func(i32 %x, i32 %y) {
  %sum = add i32 %x, %y
  ret i32 %sum
}

; RUN: opt -load-pass-plugin %llvmshlibdir/PassAdd_Komshina_Daria_FIIT1_LLVM_IR%pluginext \
; RUN: -passes="PassAdd" -S %s | FileCheck %s --check-prefix=WITHADD

; CHECK-LABEL: define i32 @add(i32 %a, i32 %b)
; CHECK: %result = add i32 %a, %b
; CHECK: ret i32 %result

; CHECK-LABEL: define i32 @foo(i32 %x, i32 %y)
; CHECK-NEXT: call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT: ret i32 %sum

define i32 @add(i32 %a, i32 %b) {
  %result = add i32 %a, %b
  ret i32 %result
}

define i32 @foo(i32 %x, i32 %y) {
  %sum = add i32 %x, %y
  ret i32 %sum
}

; RUN: opt -load-pass-plugin %llvmshlibdir/PassAdd_Komshina_Daria_FIIT1_LLVM_IR%pluginext \
; RUN: -passes="PassAdd" -S %s | FileCheck %s --check-prefix=BAR

; CHECK: define i64 @bar(i64 %m, i64 %n) {
; CHECK-NOT: call i64 @add
; CHECK: %sum = add i64 %m, %n
; CHECK: ret i64 %sum

define i64 @bar(i64 %m, i64 %n) {
  %sum = add i64 %m, %n
  ret i64 %sum
}
