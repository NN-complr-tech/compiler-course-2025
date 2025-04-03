; RUN: opt -load-pass-plugin %llvmshlibdir/FmuladdPass_Chistov_Alexey_FIIT1_LLVM_IR%pluginext -passes=FmuladdPass -S %s | FileCheck %s

; Test case 1: Basic addition without multiplication
; Should not be optimized to fmuladd
; CHECK-LABEL: define dso_local noundef double @test_addition_only(double noundef %x, double noundef %y) {
; CHECK-NEXT: start:
; CHECK-NEXT: %result = fadd double %x, %y
; CHECK-NEXT: ret double %result
; CHECK-NEXT: }

define dso_local noundef double @test_addition_only(double noundef %x, double noundef %y) {
start:
  %result = fadd double %x, %y
  ret double %result
}

; Test case 2: Addition after multiplication - should stay unchanged
; CHECK-LABEL: define dso_local noundef double @test_mul_then_add(
; CHECK-SAME: double noundef %val1, double noundef %val2, double noundef %val3) {
; CHECK-NEXT: begin:
; CHECK-NEXT: %tmp_sum = fadd double %val1, %val2
; CHECK-NEXT: %final = fmul double %tmp_sum, %val3
; CHECK-NEXT: ret double %final
; CHECK-NEXT: }

define dso_local noundef double @test_mul_then_add(
    double noundef %val1, double noundef %val2, double noundef %val3) {
begin:
  %tmp_sum = fadd double %val1, %val2
  %final = fmul double %tmp_sum, %val3
  ret double %final
}

; Test case 3: Chained operations with one optimization opportunity
; CHECK-LABEL: define dso_local noundef double @test_combined_operations(
; CHECK-SAME: double noundef %param1, double noundef %param2, 
; CHECK-SAME: double noundef %param3, double noundef %param4) {
; CHECK-NEXT: entrypoint:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %param1, double %param2, double %param3)
; CHECK-NEXT: %final_result = fadd double %0, %param4
; CHECK-NEXT: ret double %final_result
; CHECK-NEXT: }

define dso_local noundef double @test_combined_operations(
    double noundef %param1, double noundef %param2,
    double noundef %param3, double noundef %param4) {
entrypoint:
  %product = fmul double %param1, %param2
  %intermediate = fadd double %product, %param3
  %final_result = fadd double %intermediate, %param4
  ret double %final_result
}

; Test case 4: Multiple optimization opportunities
; CHECK-LABEL: define dso_local noundef double @test_multiple_patterns(
; CHECK-SAME: double noundef %input1, double noundef %input2,
; CHECK-SAME: double noundef %input3, double noundef %input4,
; CHECK-SAME: double noundef %input5, double noundef %input6) {
; CHECK-NEXT: init:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %input1, double %input2, double %input3)
; CHECK-NEXT: %1 = call double @llvm.fmuladd.f64(double %input4, double %input5, double %input6)
; CHECK-NEXT: %total = fadd double %0, %1
; CHECK-NEXT: ret double %total
; CHECK-NEXT: }

define dso_local noundef double @test_multiple_patterns(
    double noundef %input1, double noundef %input2,
    double noundef %input3, double noundef %input4,
    double noundef %input5, double noundef %input6) {
init:
  %prod1 = fmul double %input1, %input2
  %sum1 = fadd double %prod1, %input3
  %prod2 = fmul double %input4, %input5
  %sum2 = fadd double %prod2, %input6
  %total = fadd double %sum1, %sum2
  ret double %total
}

; Additional test case: Nested operations
; CHECK-LABEL: define dso_local noundef double @test_nested_operations(
; CHECK-SAME: double noundef %v1, double noundef %v2, 
; CHECK-SAME: double noundef %v3, double noundef %v4) {
; CHECK-NEXT: begin:
; CHECK-NEXT: %0 = call double @llvm.fmuladd.f64(double %v1, double %v2, double %v3)
; CHECK-NEXT: ret double %0
; CHECK-NEXT: }

define dso_local noundef double @test_nested_operations(
    double noundef %v1, double noundef %v2,
    double noundef %v3, double noundef %v4) {
begin:
  %tmp = fmul double %v1, %v2
  %res = fadd double %tmp, %v3
  ret double %res
}