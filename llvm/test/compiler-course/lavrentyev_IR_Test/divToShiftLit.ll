; RUN: opt -load-pass-plugin %llvmshlibdir/DivToShiftPass_Lavrentyev_Alexey_FIIT3_LLVM_IR%pluginext -passes=div-to-shift-combined -S %s | FileCheck %s

; Test replacing signed division by positive power-of-two divisor
; CHECK-LABEL: @test_signed_pow2
; CHECK:    ashr i32 %x, 3
define i32 @test_signed_pow2(i32 %x) {
entry:
  %div = sdiv i32 %x, 8
  ret i32 %div
}

; Test replacing unsigned division by power-of-two divisor
; CHECK-LABEL: @test_unsigned_pow2
; CHECK:    lshr i32 %y, 4
define i32 @test_unsigned_pow2(i32 %y) {
entry:
  %udiv = udiv i32 %y, 16
  ret i32 %udiv
}

; Test identity for signed divisor=1
; CHECK-LABEL: @test_identity
; CHECK-NOT: sdiv
; CHECK:    ret i32 %z
define i32 @test_identity(i32 %z) {
entry:
  %div1 = sdiv i32 %z, 1
  ret i32 %div1
}

; Test negating for signed divisor = -1
; CHECK-LABEL: @test_neg_one
; CHECK:    sub i32 0, %w
define i32 @test_neg_one(i32 %w) {
entry:
  %divn = sdiv i32 %w, -1
  ret i32 %divn
}

; Test negative power-of-two divisor for signed
; CHECK-LABEL: @test_signed_negative_pow2
; CHECK:    ashr i32 %u, 2
; CHECK:    sub i32 0, %ashr
define i32 @test_signed_negative_pow2(i32 %u) {
entry:
  %div2 = sdiv i32 %u, -4
  ret i32 %div2
}

; Test non-power-of-two divisor: should not be replaced
; CHECK-LABEL: @test_non_pow2
; CHECK:    sdiv i32 %n, 3
define i32 @test_non_pow2(i32 %n) {
entry:
  %div = sdiv i32 %n, 3
  ret i32 %div
}

; Test non-constant divisor: dynamic divisor should remain
; CHECK-LABEL: @test_var_div
; CHECK:    sdiv i32 %n, %d
define i32 @test_var_div(i32 %n, i32 %d) {
entry:
  %div = sdiv i32 %n, %d
  ret i32 %div
}

; Test unsigned identity division
; CHECK-LABEL: @test_uidentity
; CHECK-NOT: udiv
; CHECK:    ret i32 %u
define i32 @test_uidentity(i32 %u) {
entry:
  %div = udiv i32 %u, 1
  ret i32 %div
}

; Test 64-bit signed power-of-two division
; CHECK-LABEL: @test_i64_pow2
; CHECK:    ashr i64 %p, 5
define i64 @test_i64_pow2(i64 %p) {
entry:
  %div = sdiv i64 %p, 32
  ret i64 %div
}

; Test 64-bit unsigned power-of-two division
; CHECK-LABEL: @test_i64_upow2
; CHECK:    lshr i64 %q, 6
define i64 @test_i64_upow2(i64 %q) {
entry:
  %div = udiv i64 %q, 64
  ret i64 %div
}

; Test 32-bit constants
; CHECK-LABEL: @test_i32_constants
; CHECK:    ret i32 15
define i32 @test_i32_constants() {
  %div = sdiv i32 123, 8
  ret i32 %div
}
