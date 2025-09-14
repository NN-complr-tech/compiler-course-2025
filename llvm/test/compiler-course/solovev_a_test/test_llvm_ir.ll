; RUN: opt -load-pass-plugin %llvmshlibdir/Lab_2_llvm_ir_Solovev_a_FIIT1_LLVM_IR%pluginext -passes=div-to-shift -S %s | FileCheck %s

define i32 @test1(i32 %value) {
; CHECK-LABEL: @test1
; CHECK-NEXT: ashr i32 %value, 3
; CHECK-NEXT: ashr i32 %value, 5
  %div1 = sdiv i32 %value, 8
  %div2 = sdiv i32 %value, 32
  %result = add i32 %div1, %div2
  ret i32 %result
}

define i32 @test2(i32 %value) {
; CHECK-LABEL: @test2
; CHECK-NEXT: ashr i32 %value, 2
; CHECK-NEXT: ashr i32 %value, 4
; CHECK-NEXT: sub i32 0, 
  %div1 = sdiv i32 %value, 4
  %div2 = sdiv i32 %value, -16 
  %neg_div2 = sub i32 0, %div2
  %result = sub i32 %div1, %neg_div2
  ret i32 %result
}

define i32 @test3(i32 %value) {
; CHECK-LABEL: @test3
; CHECK-NEXT: ret i32 %value
  %div = sdiv i32 %value, 1
  ret i32 %div
}

define i32 @test4(i32 %value) {
; CHECK-LABEL: @test4
; CHECK-NEXT: ashr i32 %value, 6
; CHECK-NEXT: ashr i32 %value, 1
  %div1 = sdiv i32 %value, 64
  %div2 = sdiv i32 %value, 2
  %result = sub i32 %div2, %div1
  ret i32 %result
}
define i32 @test_udiv1(i32 %value) {
; CHECK-LABEL: @test_udiv1
; CHECK-NEXT: lshr i32 %value, 3
; CHECK-NEXT: lshr i32 %value, 5
  %div1 = udiv i32 %value, 8
  %div2 = udiv i32 %value, 32
  %result = add i32 %div1, %div2
  ret i32 %result
}
