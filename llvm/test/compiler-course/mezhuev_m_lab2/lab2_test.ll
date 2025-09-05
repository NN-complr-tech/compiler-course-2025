; RUN: opt -load-pass-plugin %llvmshlibdir/IntegerDivisionOptimizer_Mezhuev_Maksim_FIIT2_LLVM_IR%pluginext\
; RUN: -passes=int-div-optimize -S %s | FileCheck %s

define i32 @test_sdiv_positive(i32 %x) {
; CHECK-LABEL: @test_sdiv_positive(
; CHECK-NOT: sdiv
; CHECK: ashr i32 %x, 3
; CHECK: ret i32
  %result = sdiv i32 %x, 8
  ret i32 %result
}

define i32 @test_sdiv_negative(i32 %x) {
; CHECK-LABEL: @test_sdiv_negative(
; CHECK-NOT: sdiv
; CHECK: ashr i32 %x, 3
; CHECK: sub i32 0, 
; CHECK: ret i32
  %result = sdiv i32 %x, -8
  ret i32 %result
}

define i32 @test_udiv(i32 %x) {
; CHECK-LABEL: @test_udiv(
; CHECK-NOT: udiv
; CHECK: lshr i32 %x, 4
; CHECK: ret i32
  %result = udiv i32 %x, 16
  ret i32 %result
}

define i32 @test_non_power(i32 %x) {
; CHECK-LABEL: @test_non_power(
; CHECK: sdiv i32 %x, 3
; CHECK: ret i32
  %result = sdiv i32 %x, 3
  ret i32 %result
}

define i32 @test_variable_div(i32 %x, i32 %y) {
; CHECK-LABEL: @test_variable_div(
; CHECK: sdiv i32 %x, %y
; CHECK: ret i32
  %result = sdiv i32 %x, %y
  ret i32 %result
}

define i32 @test_div_by_one(i32 %x) {
; CHECK-LABEL: @test_div_by_one(
; CHECK: sdiv i32 %x, 1
; CHECK: ret i32
  %result = sdiv i32 %x, 1
  ret i32 %result
}