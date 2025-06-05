; RUN: opt -load-pass-plugin %llvmshlibdir/ReplaceAddWithCall_Sedova_Olga_FIIT1_LLVM_IR%pluginext\
; RUN: -passes=replace-add -S %s | FileCheck %s

; CHECK: define i32 @add_i32(i32 %a, i32 %b)
; CHECK-NEXT: entry:
; CHECK-NEXT: %result = add i32 %a, %b
; CHECK-NEXT: ret i32 %result

; CHECK: define i32 @foo_i32(i32 %x, i32 %y)
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call i32 @add_i32(i32 %x, i32 %y)
; CHECK-NEXT: ret i32 %0

; CHECK: define i64 @add_i64(i64 %a, i64 %b)
; CHECK-NEXT: entry:
; CHECK-NEXT: %result = add i64 %a, %b
; CHECK-NEXT: ret i64 %result

; CHECK: define i64 @foo_i64(i64 %x, i64 %y)
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call i64 @add_i64(i64 %x, i64 %y)
; CHECK-NEXT: ret i64 %0

; CHECK: define float @add_float(float %a, float %b)
; CHECK-NEXT: entry:
; CHECK-NEXT: %result = fadd float %a, %b
; CHECK-NEXT: ret float %result

; CHECK: define float @foo_float(float %x, float %y)
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = call float @add_float(float %x, float %y)
; CHECK-NEXT: ret float %0

; CHECK: define i32 @no_add_decl(i32 %x, i32 %y)
; CHECK-NEXT: entry:
; CHECK-NEXT: %sum = add i32 %x, %y
; CHECK-NOT:  call i32 @add

define i32 @add_i32(i32 %a, i32 %b) {
entry:
  %result = add i32 %a, %b
  ret i32 %result
}

define i32 @foo_i32(i32 %x, i32 %y) {
entry:
  %sum = add i32 %x, %y
  ret i32 %sum
}

define i64 @add_i64(i64 %a, i64 %b) {
entry:
  %result = add i64 %a, %b
  ret i64 %result
}

define i64 @foo_i64(i64 %x, i64 %y) {
entry:
  %sum = add i64 %x, %y
  ret i64 %sum
}

define float @add_float(float %a, float %b) {
entry:
  %result = fadd float %a, %b
  ret float %result
}

define float @foo_float(float %x, float %y) {
entry:
  %sum = fadd float %x, %y
  ret float %sum
}

define i32 @no_add_decl(i32 %x, i32 %y) {
entry:
  %sum = add i32 %x, %y
  ret i32 %sum
}
