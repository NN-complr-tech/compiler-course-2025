; RUN: opt -load-pass-plugin %llvmshlibdir/DivOptimizePass_Suvorov_Dmitrii_FIIT1_LLVM_IR%pluginext -passes=div-optimize -S %s | FileCheck %s

; CHECK-LABEL: @f_pos
; CHECK-NEXT: ashr i32 %value, 3
define i32 @f_pos(i32 %value) {
  %div = sdiv i32 %value, 8
  ret i32 %div
}

; CHECK-LABEL: @f_neg
; CHECK-NEXT: ashr i32 %value, 2
; CHECK-NEXT: sub i32 0, 
define i32 @f_neg(i32 %value) {
  %div = sdiv i32 %value, -4
  ret i32 %div
}

; CHECK-LABEL: @f_zero
; CHECK-NEXT: sdiv i32 %value, 0
define i32 @f_zero(i32 %value) {
  %div = sdiv i32 %value, 0
  ret i32 %div
}

; CHECK-LABEL: @f_one
; CHECK-NOT: sdiv
define i32 @f_one(i32 %value) {
  %div = sdiv i32 %value, 1
  ret i32 %div
}

; CHECK-LABEL: @f_minus_one
; CHECK-NEXT: sub i32 0, %value
define i32 @f_minus_one(i32 %value) {
  %div = sdiv i32 %value, -1
  ret i32 %div
}

; CHECK-LABEL: @f_non_power
; CHECK-NEXT: sdiv i32 %value, 3
define i32 @f_non_power(i32 %value) {
  %div = sdiv i32 %value, 3
  ret i32 %div
}

; CHECK-LABEL: @f_seq
; CHECK-NEXT: ashr i32 %value, 2
; CHECK-NEXT: sdiv i32 %value, 5
; CHECK-NEXT: ashr i32 %value, 3
; CHECK-NEXT: sub i32 0,
define i32 @f_seq(i32 %value) {
  %a = sdiv i32 %value, 4
  %b = sdiv i32 %value, 5
  %c = sdiv i32 %value, -8
  ret i32 %c
}

; CHECK-LABEL: @u_pos
; CHECK-NEXT: lshr i32 %value, 3
define i32 @u_pos(i32 %value) {
  %div = udiv i32 %value, 8
  ret i32 %div
}

; CHECK-LABEL: @u_non_power
; CHECK-NEXT: udiv i32 %value, 3
define i32 @u_non_power(i32 %value) {
  %div = udiv i32 %value, 3
  ret i32 %div
}

; CHECK-LABEL: @u_one
; CHECK-NOT: udiv
define i32 @u_one(i32 %value) {
  %div = udiv i32 %value, 1
  ret i32 %div
}
