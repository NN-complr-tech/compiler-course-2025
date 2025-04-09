; RUN: split-file %s %t

; RUN: opt -load-pass-plugin %llvmshlibdir/PassAdd_Komshina_Daria_FIIT1_LLVM_IR%pluginext -passes="PassAdd" -S %t/a.ll | FileCheck %t/a.ll
; RUN: opt -load-pass-plugin %llvmshlibdir/PassAdd_Komshina_Daria_FIIT1_LLVM_IR%pluginext -passes="PassAdd" -S %t/b.ll | FileCheck %t/b.ll

;--- a.ll
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

;--- b.ll
; CHECK-LABEL: define i32 @bar(i32 %m, i32 %n)
; CHECK-NEXT: %sum = add i32 %m, %n
; CHECK-NEXT: ret i32 %sum
; CHECK-NOT: call i32 @add

define i32 @bar(i32 %m, i32 %n) {
  %sum = add i32 %m, %n
  ret i32 %sum
}

; CHECK-LABEL: define i64 @goo(i64 %x, i64 %y)
; CHECK-NEXT: %sum = add i64 %x, %y
; CHECK-NEXT: ret i64 %sum

define i64 @goo(i64 %x, i64 %y) {
  %sum = add i64 %x, %y
  ret i64 %sum
}

; CHECK-NOT: define i64 @add(
; CHECK-LABEL: define i64 @foo_alt(i64 %x)

define i64 @foo_alt(i64 %x) {
  ret i64 %x
}
