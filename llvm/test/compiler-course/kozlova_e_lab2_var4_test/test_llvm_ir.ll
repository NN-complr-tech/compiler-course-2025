; RUN: opt -load-pass-plugin %llvmshlibdir/ReplaceDivPass_KozlovaEkaterina_FIIT3_LLVM_IR%pluginext\
; RUN: -passes=replace-div -S %s | FileCheck %s


define i32 @f1(i32 %value) {
; CHECK-LABEL: @f1
; CHECK-NEXT: ashr i32 %value, 3
  %div = sdiv i32 %value, 8
  ret i32 %div
}

define i32 @f2(i32 %value) {
; CHECK-LABEL: @f2
; CHECK-NEXT: ashr i32 %value, 4
  %div = sdiv i32 %value, 16
  ret i32 %div
}

define i32 @f3(i32 %value) {
; CHECK-LABEL: @f3
; CHECK-NEXT: ashr i32 %value, 3
; CHECK-NEXT: sub i32 0
  %div = sdiv i32 %value, -8
  ret i32 %div
}

define i32 @f4(i32 %value) {
; CHECK-LABEL: @f4
; CHECK-NEXT: ashr i32 %value, 1
  %div = sdiv i32 %value, 2
  ret i32 %div
}

define i32 @f5(i32 %value) {
; CHECK-LABEL: @f5
; CHECK-NEXT: ret i32 %value
  %div = sdiv i32 %value, 1
  ret i32 %div
}

define i32 @f6(i32 %value) {
; CHECK-LABEL: @f6
; CHECK-NEXT: sdiv i32 %value, 15
  %div = sdiv i32 %value, 15
  ret i32 %div
}

