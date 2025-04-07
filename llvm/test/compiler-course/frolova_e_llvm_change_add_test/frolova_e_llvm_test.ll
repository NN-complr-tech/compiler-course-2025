; RUN: opt -load-pass-plugin %llvmshlibdir/ChangeADD_Frolova_Elizaveta_FIIT3_LLVM_IR%pluginext\
; RUN: -passes=ChangeADD -S %s | FileCheck %s

; CHECK: define i32 @add(i32 %a, i32 %b)
; CHECK-NEXT: %result = add i32 %a, %b
; CHECK-NEXT: ret i32 %result

define i32 @add(i32 %a, i32 %b) {
  %result = add i32 %a, %b
  ret i32 %result
}

; CHECK: define i32 @foo(i32 %x, i32 %y)
; CHECK-NEXT: %result1 = call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT: ret i32 %result1
; CHECK-NOT: add i32

define i32 @foo(i32 %x, i32 %y) {
  %result = add i32 %x, %y
  ret i32 %result
}

; CHECK: define float @fadd(float %a, float %b)
; CHECK-NEXT: %result = fadd float %a, %b
; CHECK-NEXT: ret float %result

define float @fadd(float %a, float %b) {
  %result = fadd float %a, %b
  ret float %result
}

; CHECK: define float @bar(float %x, float %y)
; CHECK-NEXT: %result1 = call float @fadd(float %x, float %y)
; CHECK-NEXT: ret float %result1
; CHECK-NOT: fadd i32

define float @bar(float %x, float %y) {
  %result = fadd float %x, %y
  ret float %result
}

; CHECK: define i32 @subtest(i32 %a, i32 %b)
; CHECK-NEXT: %result = sub i32 %a, %b
; CHECK-NEXT: ret i32 %result

define i32 @subtest(i32 %a, i32 %b) {
  %result = sub i32 %a, %b
  ret i32 %result
}

; CHECK: define i32 @foo4(i32 %x, i32 %y, i32 %z)
; CHECK-NEXT: %sum1 = call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT: %res2 = call i32 @add(i32 %sum1, i32 %z)
; CHECK-NEXT: ret i32 %res2
; CHECK-NOT: add i32

define i32 @foo4(i32 %x, i32 %y, i32 %z) {
  %sum = add i32 %x, %y
  %res = add i32 %sum, %z
  ret i32 %res
}

; CHECK: define i64 @foo3(i64 %x, i64 %y)
; CHECK-NEXT: %res64 = add i64 %x, %y
; CHECK-NEXT: ret i64 %res64
; CHECK-NOT: call i32 @add

define i64 @foo3(i64 %x, i64 %y) {
  %res64 = add i64 %x, %y
  ret i64 %res64
}

