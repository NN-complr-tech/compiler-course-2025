; RUN: opt -load-pass-plugin %llvmshlibdir/CallFunctionPass_DrozhdinovD_FIIT1_LLVM_IR%pluginext\
; RUN: -passes=callfunc -S %s | FileCheck %s --check-prefixes CHECK,CHECK-INT32,CHECK-DOUBLE,CHECK-INT64

; /* TEST INTEGER ADD REPLACEMENT */
; CHECK-INT-LABEL: define i32 @add(i32 %a, i32 %b)
; CHECK: entry:
; CHECK-INT-NEXT: %result = add i32 %a, %b
; CHECK-INT-NEXT: ret i32 %result

; CHECK-INT32-LABEL: define i32 @foo(i32 %x, i32 %y)
; CHECK: entry:
; CHECK-INT32-NEXT: call i32 @add(i32 %x, i32 %y)
; CHECK-INT32-NEXT: ret i32 %sum

; CHECK-NOT: add i32

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

; /* TEST DOUBLE ARGUMENTS */
; CHECK-DOUBLE-LABEL: define double @foo1(double %a, double %b)
; CHECK: entry:
; CHECK-DOUBLE-NEXT: %result = fadd double %a, %b
; CHECK-DOUBLE-NEXT: ret double %result

define double @foo1(double %a, double %b) {
entry:
  %result = fadd double %a, %b
  ret double %result
}

; CHECK-INT32-LABEL: define i32 @foo2(i32 %x, i32 %y)
; CHECK: entry:
; CHECK-INT32-NEXT: call i32 @add(i32 %x, i32 %y)
; CHECK-INT32-NEXT: call i32 @add(i32 %y, i32 %x)
; CHECK-INT32-NEXT: ret i32 %sum

define i32 @foo2(i32 %x, i32 %y) {
entry:
  %sum = add i32 %x, %y
  %res = add i32 %y, %x
  ret i32 %sum
}

; CHECK-INT64-LABEL: define i64 @foo3(i64 %x, i64 %y)
; CHECK: entry:
; CHECK-INT64-NEXT: %res64 = add i64 %x, %y
; CHECK-INT64-NEXT: ret i64 %res64

; CHECK-INT64-NOT: call i32 @add(i32 %x, i32 %y)

define i64 @foo3(i64 %x, i64 %y) {
entry:
  %res64 = add i64 %x, %y
  ret i64 %res64
}

; CHECK-INT32-LABEL: define i32 @foo4(i32 %x, i32 %y, i32 %z)
; CHECK: entry:
; CHECK-INT32-NEXT: call i32 @add(i32 %x, i32 %y)
; CHECK-INT32-NEXT: call i32 @add(i32 %sum1, i32 %z)
; CHECK-INT32-NEXT: ret i32 %res2

define i32 @foo4(i32 %x, i32 %y, i32 %z) {
entry:
  %sum = add i32 %x, %y
  %res = add i32 %sum, %z
  ret i32 %res
}

