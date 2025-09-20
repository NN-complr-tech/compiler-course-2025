; RUN: opt -load-pass-plugin %llvmshlibdir/Ivanov_M_Division_Pass_Ivanov_Mikhail_FIIT1_LLVM_IR%pluginext\
; RUN: -passes=bit_shift_pass -S %s | FileCheck %s

define i32 @test_divider_pos_one(i32 %value) {
; CHECK-LABEL: @test_divider_pos_one
; CHECK-NEXT: ret i32 %value
  %div1 = sdiv i32 %value, 1
  ret i32 %div1
}

define i32 @test_divider_neg_one(i32 %value) {
; CHECK-LABEL: @test_divider_neg_one
; CHECK-NEXT: sub i32 0, %value
  %div1 = sdiv i32 %value, -1
  ret i32 %div1
}

define i32 @test_divider_zero(i32 %value) {
; CHECK-LABEL: @test_divider_zero
; CHECK-NEXT: sdiv i32 %value, 0
  %div = sdiv i32 %value, 0
  ret i32 %div
}

define i32 @test_neg_divider(i32 %value) {
; CHECK-LABEL: @test_neg_divider
; CHECK-NEXT: ashr i32 %value, 2
; CHECK-NEXT: ashr i32 %value, 5
; CHECK-NEXT: sub i32 0,
  %div1 = sdiv i32 %value, 4
  %div2 = sdiv i32 %value, -32
  %result = sub i32 %div1, %div2
  ret i32 %result
}

define i32 @test_unsigned_division(i32 %value) {
; CHECK-LABEL: @test_unsigned_division
; CHECK-NEXT: lshr i32 %value, 2
; CHECK-NEXT: lshr i32 %value, 7
  %div1 = udiv i32 %value, 4
  %div2 = udiv i32 %value, 128
  %result = add i32 %div1, %div2
  ret i32 %result
}

define i32 @test_signed_division(i32 %value) {
; CHECK-LABEL: @test_signed_division
; CHECK-NEXT: ashr i32 %value, 2
; CHECK-NEXT: ashr i32 %value, 7
  %div1 = sdiv i32 %value, 4
  %div2 = sdiv i32 %value, 128
  %result = add i32 %div1, %div2
  ret i32 %result
}

define i32 @test_mixed_division_pos(i32 %value) {
; CHECK-LABEL: @test_mixed_division_pos
; CHECK-NEXT: lshr i32 %value, 2
; CHECK-NEXT: ashr i32 %value, 7
  %div1 = udiv i32 %value, 4
  %div2 = sdiv i32 %value, 128
  %result = add i32 %div1, %div2
  ret i32 %result
}

define i32 @test_mixed_division_neg(i32 %value) {
; CHECK-LABEL: @test_mixed_division_neg
; CHECK-NEXT: lshr i32 %value, 2
; CHECK-NEXT: ashr i32 %value, 7
; CHECK-NEXT: sub i32 0, %ashr
  %div1 = udiv i32 %value, 4
  %div2 = sdiv i32 %value, -128
  %result = sub i32 %div1, %div2
  ret i32 %result
}
