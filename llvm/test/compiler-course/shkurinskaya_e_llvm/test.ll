; RUN: opt -load-pass-plugin %llvmshlibdir/BinShiftPass_Shkurinskaya_Elena_FIIT2_LLVM_IR%pluginext -passes=bin-shift -S %s | FileCheck %s
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

define i8 @test_sdiv_i8(i8 %x) {
; CHECK-LABEL: @test_sdiv_i8
; CHECK-NEXT: ashr i8 %x, 1
  %div = sdiv i8 %x, 2
  ret i8 %div
}

define i8 @test_udiv_i8(i8 %x) {
; CHECK-LABEL: @test_udiv_i8
; CHECK-NEXT: lshr i8 %x, 1
  %div = udiv i8 %x, 2
  ret i8 %div
}

define i16 @test_sdiv_i16(i16 %x) {
; CHECK-LABEL: @test_sdiv_i16
; CHECK-NEXT: ashr i16 %x, 2
  %div = sdiv i16 %x, 4
  ret i16 %div
}

define i16 @test_udiv_i16(i16 %x) {
; CHECK-LABEL: @test_udiv_i16
; CHECK-NEXT: lshr i16 %x, 2
  %div = udiv i16 %x, 4
  ret i16 %div
}

define i64 @test_sdiv_i64(i64 %x) {
; CHECK-LABEL: @test_sdiv_i64
; CHECK-NEXT: ashr i64 %x, 3
  %div = sdiv i64 %x, 8
  ret i64 %div
}

define i64 @test_udiv_i64(i64 %x) {
; CHECK-LABEL: @test_udiv_i64
; CHECK-NEXT: lshr i64 %x, 3
  %div = udiv i64 %x, 8
  ret i64 %div
}

define i128 @test_sdiv_i128(i128 %x) {
; CHECK-LABEL: @test_sdiv_i128
; CHECK-NEXT: ashr i128 %x, 4
  %div = sdiv i128 %x, 16
  ret i128 %div
}

define i128 @test_udiv_i128(i128 %x) {
; CHECK-LABEL: @test_udiv_i128
; CHECK-NEXT: lshr i128 %x, 4
  %div = udiv i128 %x, 16
  ret i128 %div
}

define i32 @test_sdiv_negative_power_of_two(i32 %x) {
; CHECK-LABEL: @test_sdiv_negative_power_of_two
; CHECK-NEXT: sub i32 0, %x
; CHECK-NEXT: ashr i32 %negation_tmp, 4
  %div = sdiv i32 %x, -16
  ret i32 %div
}

define i32 @test_non_constant_divisor(i32 %x, i32 %y) {
; CHECK-LABEL: @test_non_constant_divisor
; CHECK-NEXT: sdiv i32 %x, %y
  %div = sdiv i32 %x, %y
  ret i32 %div
}

define i32 @test_var_power_of_two(i32 %x) {
; CHECK-LABEL: @test_var_power_of_two
; CHECK-DAG: %y = mul i32 1, 8
; CHECK-DAG: %unsigned_shift = lshr i32 %x, 3
  %y = mul i32 1, 8
  %div = udiv i32 %x, %y
  ret i32 %div
}

define i32 @test_var_negative_power_of_two(i32 %x) {
; CHECK-LABEL: @test_var_negative_power_of_two
; CHECK-DAG: %y = mul i32 -1, 4
; CHECK-DAG: %negation_tmp = sub i32 0, %x
; CHECK-DAG: %signed_shift = ashr i32 %negation_tmp, 2
  %y = mul i32 -1, 4
  %div = sdiv i32 %x, %y
  ret i32 %div
}
