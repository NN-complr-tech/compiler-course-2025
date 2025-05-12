// RUN: mlir-opt -split-input-file -verify-diagnostics %s \
// RUN: -pass-pipeline="builtin.module(func.func(rem_pass_Mamaeva_Olga_FIIT3_MLIR))" | FileCheck %s

// -----

// CHECK-LABEL: func.func @test_simple_remsi
// CHECK-NEXT: %[[DIV:.*]] = arith.divsi %arg0, %arg1 : i32
// CHECK-NEXT: %[[MUL:.*]] = arith.muli %[[DIV]], %arg1 : i32
// CHECK-NEXT: %[[RES:.*]] = arith.subi %arg0, %[[MUL]] : i32
// CHECK-NEXT: return %[[RES]] : i32
func.func @test_simple_remsi(%arg0: i32, %arg1: i32) -> i32 {
  %0 = arith.remsi %arg0, %arg1 : i32
  return %0 : i32
}

// -----

// CHECK-LABEL: func.func @test_simple_remui
// CHECK-NEXT: %[[DIV:.*]] = arith.divui %arg0, %arg1 : i32
// CHECK-NEXT: %[[MUL:.*]] = arith.muli %[[DIV]], %arg1 : i32
// CHECK-NEXT: %[[RES:.*]] = arith.subi %arg0, %[[MUL]] : i32
// CHECK-NEXT: return %[[RES]] : i32
func.func @test_simple_remui(%arg0: i32, %arg1: i32) -> i32 {
  %0 = arith.remui %arg0, %arg1 : i32
  return %0 : i32
}
