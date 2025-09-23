; RUN: opt -load-pass-plugin %llvmshlibdir/DivPass_vasenkov_a_FIIT1_LLVM_IR%pluginext\
; RUN: -passes=div-optimize -S %s | FileCheck %s

; Тест оптимизации деления на степени двойки
define i32 @test_positive_power2(i32 %x) {
; CHECK-LABEL: @test_positive_power2
; CHECK-NEXT: ashr i32 %x, 4
; CHECK-NEXT: ret i32
  %result = sdiv i32 %x, 16
  ret i32 %result
}

define i32 @test_negative_power2(i32 %x) {
; CHECK-LABEL: @test_negative_power2
; CHECK-NEXT: ashr i32 %x, 2
; CHECK-NEXT: sub i32 0
; CHECK-NEXT: ret i32
  %result = sdiv i32 %x, -4
  ret i32 %result
}

; Тест деления на 1 и -1
define i32 @test_divide_by_one(i32 %x) {
; CHECK-LABEL: @test_divide_by_one
; CHECK-NEXT: ret i32 %x
  %result = sdiv i32 %x, 1
  ret i32 %result
}

define i32 @test_divide_by_minus_one(i32 %x) {
; CHECK-LABEL: @test_divide_by_minus_one
; CHECK-NEXT: sub i32 0, %x
; CHECK-NEXT: ret i32
  %result = sdiv i32 %x, -1
  ret i32 %result
}

; Тест неделимых чисел (должны остаться как есть)
define i32 @test_non_power2(i32 %x) {
; CHECK-LABEL: @test_non_power2
; CHECK-NEXT: sdiv i32 %x, 7
; CHECK-NEXT: ret i32
  %result = sdiv i32 %x, 7
  ret i32 %result
}

define i32 @test_another_non_power2(i32 %x) {
; CHECK-LABEL: @test_another_non_power2
; CHECK-NEXT: sdiv i32 %x, 13
; CHECK-NEXT: ret i32
  %result = sdiv i32 %x, 13
  ret i32 %result
}

; Тест беззнакового деления
define i32 @test_unsigned_division(i32 %x) {
; CHECK-LABEL: @test_unsigned_division
; CHECK-NEXT: lshr i32 %x, 3
; CHECK-NEXT: ret i32
  %result = udiv i32 %x, 8
  ret i32 %result
}

; Дополнительные тесты для граничных случаев
define i32 @test_divide_by_8(i32 %x) {
; CHECK-LABEL: @test_divide_by_8
; CHECK-NEXT: ashr i32 %x, 3
; CHECK-NEXT: ret i32
  %result = sdiv i32 %x, 8
  ret i32 %result
}

define i32 @test_divide_by_32(i32 %x) {
; CHECK-LABEL: @test_divide_by_32
; CHECK-NEXT: ashr i32 %x, 5
; CHECK-NEXT: ret i32
  %result = sdiv i32 %x, 32
  ret i32 %result
}