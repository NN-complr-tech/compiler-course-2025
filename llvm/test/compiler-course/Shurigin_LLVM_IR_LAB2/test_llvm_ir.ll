; RUN: opt -load-pass-plugin %llvmshlibdir/LLVM_IR_LAB2_Shurigin_FIIT1_LLVM_IR%pluginext -passes=div2shift -S %s | FileCheck %s

define i32 @f1(i32 %value) {
; CHECK: @f1
; CHECK-NEXT: ashr i32 %value, 3
  %div = sdiv i32 %value, 8
  ret i32 %div
}

define i32 @f2(i32 %value) {
; CHECK: @f2
; CHECK-NEXT: ashr i32 %value, 1
  %div = sdiv i32 %value, 2
  ret i32 %div
}

define i32 @f3(i32 %value) {
; CHECK: @f3
; CHECK-NEXT: sdiv i32 %value, 3
  %div = sdiv i32 %value, 3
  ret i32 %div
}

define i32 @f4(i32 %value) {
; CHECK: @f4
; CHECK-NEXT: ashr i32 %value, 10
  %div = sdiv i32 %value, 1024
  ret i32 %div
}

define i32 @f5(i32 %value) {
; CHECK: @f5
; CHECK-NEXT: ashr i32 %value, 2
; CHECK-NEXT: sub i32 0, %
  %div = sdiv i32 %value, -4
  ret i32 %div
}

define i32 @f6(i32 %value) {
; CHECK: @f6
; CHECK-NEXT: add i32 %value, 0
  %div = sdiv i32 %value, 1
  ret i32 %div
}

define i32 @f14(i32 %value) {
; CHECK: @f14
; CHECK-NEXT: ashr i32 %value, 30
  %div = sdiv i32 %value, 1073741824  ; 2^30
  ret i32 %div
}

define i32 @f15(i32 %value) {
; CHECK: @f15
; CHECK-NEXT: sdiv i32 %value, 33
  %div = sdiv i32 %value, 33  
  ret i32 %div
}
