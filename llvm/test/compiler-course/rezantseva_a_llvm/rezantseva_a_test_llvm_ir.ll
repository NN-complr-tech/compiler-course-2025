; RUN: opt -load-pass-plugin %llvmshlibdir/DivToShiftPass_RezantsevaAnastasia_FIIT1_LLVM_IR%pluginext \
; RUN: -passes=div-to-shift -S %s | FileCheck %s


define i32 @test_sdiv_pow2_8(i32 %x) {
; CHECK-LABEL: @test_sdiv_pow2_8
; CHECK-NEXT: %1 = ashr i32 %x, 3
; CHECK-NEXT: ret i32 %1
  %div = sdiv i32 %x, 8
  ret i32 %div
}

define i32 @test_sdiv_pow2_16(i32 %x) {
; CHECK-LABEL: @test_sdiv_pow2_16
; CHECK-NEXT: %1 = ashr i32 %x, 4
; CHECK-NEXT: ret i32 %1
  %div = sdiv i32 %x, 16
  ret i32 %div
}

define i32 @test_sdiv_negative_pow2(i32 %x) {
; CHECK-LABEL: @test_sdiv_negative_pow2
; CHECK-NEXT: %1 = ashr i32 %x, 3
; CHECK-NEXT: %2 = sub i32 0, %1
; CHECK-NEXT: ret i32 %2
  %div = sdiv i32 %x, -8
  ret i32 %div
}

define i32 @test_sdiv_non_pow2(i32 %x) {
; CHECK-LABEL: @test_sdiv_non_pow2
; CHECK-NEXT: %div = sdiv i32 %x, 7
; CHECK-NOT: ashr
  %div = sdiv i32 %x, 7
  ret i32 %div
}

define i32 @test_sdiv_by_1(i32 %x) {
; CHECK-LABEL: @test_sdiv_by_1
; CHECK-NEXT: ret i32 %x
  %div = sdiv i32 %x, 1
  ret i32 %div
}

define i32 @test_sdiv_by_neg1(i32 %x) {
; CHECK-LABEL: @test_sdiv_by_neg1
; CHECK-NEXT: %1 = sub i32 0, %x
; CHECK-NEXT: ret i32 %1
  %div = sdiv i32 %x, -1
  ret i32 %div
}

define i32 @test_udiv_pow2_16(i32 %x) {
; CHECK-LABEL: @test_udiv_pow2_16
; CHECK-NEXT: %1 = lshr i32 %x, 4
; CHECK-NEXT: ret i32 %1
  %div = udiv i32 %x, 16
  ret i32 %div
}

define i32 @test_udiv_non_pow2(i32 %x) {
; CHECK-LABEL: @test_udiv_non_pow2
; CHECK-NEXT: %div = udiv i32 %x, 3
; CHECK-NOT: lshr
  %div = udiv i32 %x, 3
  ret i32 %div
}

define i32 @test_udiv_by_1(i32 %x) {
; CHECK-LABEL: @test_udiv_by_1
; CHECK-NEXT: ret i32 %x
  %div = udiv i32 %x, 1
  ret i32 %div
}

define i32 @test_mixed_divisions(i32 %x, i32 %y) {
; CHECK-LABEL: @test_mixed_divisions
; CHECK-NEXT: %1 = ashr i32 %x, 2
; CHECK-NEXT: %div2 = udiv i32 %y, 3
; CHECK-NEXT: %sum = add i32 %1, %div2
; CHECK-NEXT: ret i32 %sum
  %div1 = sdiv i32 %x, 4
  %div2 = udiv i32 %y, 3
  %sum = add i32 %div1, %div2
  ret i32 %sum
}

define i32 @test_sdiv_zero(i32 %x) {
; CHECK-LABEL: @test_sdiv_zero
; CHECK-NEXT: %div = sdiv i32 %x, 0
; CHECK-NEXT: ret i32 %div
  %div = sdiv i32 %x, 0
  ret i32 %div
}

define i32 @test_udiv_zero(i32 %x) {
; CHECK-LABEL: @test_udiv_zero
; CHECK-NEXT: %div = udiv i32 %x, 0
; CHECK-NEXT: ret i32 %div
  %div = udiv i32 %x, 0
  ret i32 %div
}
 