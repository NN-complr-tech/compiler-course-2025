; RUN: opt -load-pass-plugin %llvmshlibdir/BinShiftPass_Shkurinskaya_Elena_FIIT2_LLVM_IR%pluginext -passes=bin-shift -S %s | FileCheck %s

define i32 @test_sdiv_power_of_two(i32 %x) {
; CHECK-LABEL: @test_sdiv_power_of_two
; CHECK-NEXT: ashr i32 %x, 3
  %div = sdiv i32 %x, 8
  ret i32 %div
}

define i32 @test_udiv_power_of_two(i32 %x) {
; CHECK-LABEL: @test_udiv_power_of_two
; CHECK-NEXT: lshr i32 %x, 4
  %div = udiv i32 %x, 16
  ret i32 %div
}

define i32 @test_sdiv_sub_one(i32 %x) {
; CHECK-LABEL: @test_sdiv_sub_one
; CHECK-NEXT: sub i32 0, %x
  %div = sdiv i32 %x, -1
  ret i32 %div
}

define i32 @test_sdiv_one(i32 %x) {
; CHECK-LABEL: @test_sdiv_one
; CHECK-NEXT: ret i32 %x
  %div = sdiv i32 %x, 1
  ret i32 %div
}

define i32 @test_udiv_one(i32 %x) {
; CHECK-LABEL: @test_udiv_one
; CHECK-NEXT: ret i32 %x
  %div = udiv i32 %x, 1
  ret i32 %div
}

define i32 @test_no_optimization(i32 %x) {
; CHECK-LABEL: @test_no_optimization
; CHECK-NEXT: sdiv i32 %x, 5
  %div = sdiv i32 %x, 5
  ret i32 %div
}

define i32 @test_mixed(i32 %x) {
; CHECK-LABEL: @test_mixed
; CHECK-NEXT: ashr i32 %x, 2
; CHECK-NEXT: sdiv i32 %x, 3
; CHECK-NEXT: lshr i32 %x, 5
  %div1 = sdiv i32 %x, 4
  %div2 = sdiv i32 %x, 3
  %div3 = udiv i32 %x, 32
  %result = add i32 %div1, %div2
  %result2 = add i32 %result, %div3
  ret i32 %result2
}
