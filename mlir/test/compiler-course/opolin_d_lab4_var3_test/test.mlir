// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/RemLowPass_Opolin_Dmitry_FIIT2_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(func.func(RemLowPass_Opolin_Dmitry_FIIT2_MLIR))" %s | FileCheck %s

// Test 1: base arith.remsi i32
// CHECK-LABEL: func.func @test_remsi_basic_i32
// CHECK-NOT: arith.remsi
// CHECK-NEXT:    %[[DIV:.*]] = arith.divsi %arg0, %arg1 : i32
// CHECK-NEXT:    %[[MUL:.*]] = arith.muli %[[DIV]], %arg1 : i32
// CHECK-NEXT:    %[[SUB:.*]] = arith.subi %arg0, %[[MUL]] : i32
// CHECK-NEXT:    return %[[SUB]] : i32
func.func @test_remsi_basic_i32(%a: i32, %b: i32) -> i32 {
  %0 = arith.remsi %a, %b : i32
  return %0 : i32
}

// Test 2: base arith.remui i32
// CHECK-LABEL: func.func @test_remui_basic_i32
// CHECK-NOT: arith.remui
// CHECK-NEXT:    %[[DIV:.*]] = arith.divui %arg0, %arg1 : i32
// CHECK-NEXT:    %[[MUL:.*]] = arith.muli %[[DIV]], %arg1 : i32
// CHECK-NEXT:    %[[SUB:.*]] = arith.subi %arg0, %[[MUL]] : i32
// CHECK-NEXT:    return %[[SUB]] : i32
func.func @test_remui_basic_i32(%x: i32, %y: i32) -> i32 {
  %0 = arith.remui %x, %y : i32
  return %0 : i32
}

// Test 3:  arith.remsi i64
// CHECK-LABEL: func.func @test_remsi_i64
// CHECK-NOT: arith.remsi
// CHECK-NEXT:    %[[DIV:.*]] = arith.divsi %arg0, %arg1 : i64
// CHECK-NEXT:    %[[MUL:.*]] = arith.muli %[[DIV]], %arg1 : i64
// CHECK-NEXT:    %[[SUB:.*]] = arith.subi %arg0, %[[MUL]] : i64
// CHECK-NEXT:    return %[[SUB]] : i64
func.func @test_remsi_i64(%val1: i64, %val2: i64) -> i64 {
  %0 = arith.remsi %val1, %val2 : i64
  return %0 : i64
}

// Test 4: arith.remui i16
// CHECK-LABEL: func.func @test_remui_i16
// CHECK-NOT: arith.remui
// CHECK-NEXT:    %[[DIV:.*]] = arith.divui %arg0, %arg1 : i16
// CHECK-NEXT:    %[[MUL:.*]] = arith.muli %[[DIV]], %arg1 : i16
// CHECK-NEXT:    %[[SUB:.*]] = arith.subi %arg0, %[[MUL]] : i16
// CHECK-NEXT:    return %[[SUB]] : i16
func.func @test_remui_i16(%input_a: i16, %input_b: i16) -> i16 {
  %0 = arith.remui %input_a, %input_b : i16
  return %0 : i16
}

// Test 5: using arith.remsi result
// CHECK-LABEL: func.func @test_remsi_used_result
// CHECK-NOT: arith.remsi
// CHECK-NEXT:    %[[DIV_REM:.*]] = arith.divsi %arg0, %arg1 : i32
// CHECK-NEXT:    %[[MUL_REM:.*]] = arith.muli %[[DIV_REM]], %arg1 : i32
// CHECK-NEXT:    %[[SUB_REM:.*]] = arith.subi %arg0, %[[MUL_REM]] : i32
// CHECK-NEXT:    %[[ADD_RES:.*]] = arith.addi %[[SUB_REM]], %arg2 : i32
// CHECK-NEXT:    return %[[ADD_RES]] : i32
func.func @test_remsi_used_result(%a: i32, %b: i32, %c: i32) -> i32 {
  %rem_val = arith.remsi %a, %b : i32
  %result = arith.addi %rem_val, %c : i32
  return %result : i32
}

// Test 6: using arith.remui result
// CHECK-LABEL: func.func @test_remui_used_result
// CHECK-NOT: arith.remui
// CHECK-NEXT:    %[[DIV_REM:.*]] = arith.divui %arg0, %arg1 : i32
// CHECK-NEXT:    %[[MUL_REM:.*]] = arith.muli %[[DIV_REM]], %arg1 : i32
// CHECK-NEXT:    %[[SUB_REM:.*]] = arith.subi %arg0, %[[MUL_REM]] : i32
// CHECK-NEXT:    %[[ADD_RES:.*]] = arith.addi %[[SUB_REM]], %arg2 : i32
// CHECK-NEXT:    return %[[ADD_RES]] : i32
func.func @test_remui_used_result(%x: i32, %y: i32, %z: i32) -> i32 {
  %rem_val = arith.remui %x, %y : i32
  %result = arith.addi %rem_val, %z : i32
  return %result : i32
}

// Test 7: no changes
// CHECK-LABEL: func.func @test_no_rem_ops
// CHECK-NEXT:    %[[ADD:.*]] = arith.addi %arg0, %arg1 : i32
// CHECK-NEXT:    return %[[ADD]] : i32
// CHECK-NOT: arith.divsi
// CHECK-NOT: arith.divui
// CHECK-NOT: arith.muli
// CHECK-NOT: arith.subi
func.func @test_no_rem_ops(%val_a: i32, %val_b: i32) -> i32 {
  %0 = arith.addi %val_a, %val_b : i32
  return %0 : i32
}
