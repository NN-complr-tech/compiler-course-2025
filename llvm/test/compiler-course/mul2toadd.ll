; RUN: opt -load-pass-plugin %shlib(CompilerCourseMul2ToAdd) -passes=mul2toadd -S %s | FileCheck %s

define i32 @foo(i32 %x) {
entry:
  %m = mul i32 %x, 2
  ret i32 %m
}

; CHECK-LABEL: @foo
; CHECK: %m = add i32 %x, %x
; CHECK-NOT: mul i32
