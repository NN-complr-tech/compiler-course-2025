; RUN: opt -load-pass-plugin %llvmshlibdir/PassAdd_Komshina_Daria_FIIT1_LLVM_IR%pluginext \
; RUN: -passes="PassAdd" -S %s | FileCheck %s

; CHECK: define i32 @add(i32 %a, i32 %b)
; CHECK: %result = add i32 %a, %b
; CHECK: ret i32 %result

; CHECK-NOT: add i32

define i32 @add(i32 %a, i32 %b) {
  %result = add i32 %a, %b
  ret i32 %result
}

; CHECK-LABEL: define i32 @foo(i32 %x, i32 %y)
; CHECK-NEXT: call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT: ret i32 %sum

; CHECK-NOT: add i32

define i32 @foo(i32 %x, i32 %y) {
  %sum = add i32 %x, %y
  ret i32 %sum
}

; CHECK: define i64 @bar(i64 %m, i64 %n) {
; CHECK-NOT: call i64 @add
; CHECK: %sum = add i64 %m, %n
; CHECK: ret i64 %sum

define i64 @bar(i64 %m, i64 %n) {
  %sum = add i64 %m, %n
  ret i64 %sum
}

; RUN: opt -load-pass-plugin %llvmshlibdir/PassAdd_Komshina_Daria_FIIT1_LLVM_IR%pluginext \
; RUN: -passes="PassAdd" -S %s | FileCheck %s

; CHECK-LABEL: define i64 @goo(i64 %x, i64 %y)
; CHECK: %sum = add i64 %x, %y
; CHECK: ret i64 %sum

define i64 @goo(i64 %x, i64 %y) {
  %sum = add i64 %x, %y
  ret i64 %sum
}

; CHECK-NOT: define i64 @add(
; CHECK: define i64 @foo_alt(i64 %x)

define i64 @foo_alt(i64 %x) {
  ret i64 %x
}
