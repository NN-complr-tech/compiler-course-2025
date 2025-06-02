// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/ExpandRemsiPass_Ionova_Ekaterina_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(ExpandRemsiPass_Ionova_Ekaterina_FIIT1_MLIR)" %s | FileCheck %s

// CHECK-LABEL: func.func @test_double_remsi
// CHECK-NEXT: %[[DIV1:.*]] = arith.divsi %arg1, %arg0 : i32
// CHECK-NEXT: %[[MUL1:.*]] = arith.muli %[[DIV1]], %arg0 : i32
// CHECK-NEXT: %[[SUB1:.*]] = arith.subi %arg1, %[[MUL1]] : i32
// CHECK-NEXT: %[[DIV2:.*]] = arith.divsi %[[SUB1]], %arg0 : i32
// CHECK-NEXT: %[[MUL2:.*]] = arith.muli %[[DIV2]], %arg0 : i32
// CHECK-NEXT: %[[SUB2:.*]] = arith.subi %[[SUB1]], %[[MUL2]] : i32
// CHECK-NEXT: return %[[SUB2]] : i32
func.func @test_double_remsi(%arg0: i32, %arg1: i32) -> i32 {
  %remainder1 = arith.remsi %arg1, %arg0 : i32
  %remainder2 = arith.remsi %remainder1, %arg0 : i32
  return %remainder2 : i32
}

// CHECK-LABEL: func.func @test_remainder_condition
// CHECK: %[[DIV:.*]] = arith.divsi %arg1, %arg0 : i32
// CHECK-NEXT: %[[MUL:.*]] = arith.muli %[[DIV]], %arg0 : i32
// CHECK-NEXT: %[[SUB:.*]] = arith.subi %arg1, %[[MUL]] : i32
// CHECK-NEXT: %[[CMP:.*]] = arith.cmpi eq, %[[SUB]], %c0{{.*}} : i32
// CHECK-NEXT: %[[RESULT:.*]] = arith.select %[[CMP]], %c1{{.*}}, %c0{{.*}} : i32
// CHECK-NEXT: return %[[RESULT]] : i32
func.func @test_remainder_condition(%arg0: i32, %arg1: i32) -> i32 {
  %c0 = arith.constant 0 : i32
  %c1 = arith.constant 1 : i32
  %remainder = arith.remsi %arg1, %arg0 : i32
  %is_divisible = arith.cmpi eq, %remainder, %c0 : i32
  %result = arith.select %is_divisible, %c1, %c0 : i32
  return %result : i32
}

// CHECK-LABEL: func.func @test_cyclic_remainder
// CHECK-NEXT: %[[SUM:.*]] = arith.addi %arg0, %arg1 : i32
// CHECK-NEXT: %[[DIV:.*]] = arith.divsi %[[SUM]], %arg2 : i32
// CHECK-NEXT: %[[MUL:.*]] = arith.muli %[[DIV]], %arg2 : i32
// CHECK-NEXT: %[[SUB:.*]] = arith.subi %[[SUM]], %[[MUL]] : i32
// CHECK-NEXT: return %[[SUB]] : i32
func.func @test_cyclic_remainder(%arg0: i32, %arg1: i32, %arg2: i32) -> i32 {
  %sum = arith.addi %arg0, %arg1 : i32
  %remainder = arith.remsi %sum, %arg2 : i32
  return %remainder : i32
}

// CHECK-LABEL: func.func @test_combined_operations
// CHECK-NEXT: %[[PRODUCT:.*]] = arith.muli %arg0, %arg1 : i32
// CHECK-NEXT: %[[DIV:.*]] = arith.divsi %[[PRODUCT]], %arg2 : i32
// CHECK-NEXT: %[[MUL:.*]] = arith.muli %[[DIV]], %arg2 : i32
// CHECK-NEXT: %[[SUB:.*]] = arith.subi %[[PRODUCT]], %[[MUL]] : i32
// CHECK-NEXT: return %[[SUB]] : i32
func.func @test_combined_operations(%arg0: i32, %arg1: i32, %arg2: i32) -> i32 {
  %product = arith.muli %arg0, %arg1 : i32
  %remainder = arith.remsi %product, %arg2 : i32
  return %remainder : i32
}

// CHECK-LABEL: func.func @test_remui
// CHECK-NEXT: %[[DIV:.*]] = arith.divui %arg0, %arg1 : i32
// CHECK-NEXT: %[[MUL:.*]] = arith.muli %[[DIV]], %arg1 : i32
// CHECK-NEXT: %[[SUB:.*]] = arith.subi %arg0, %[[MUL]] : i32
// CHECK-NEXT: return %[[SUB]] : i32
func.func @test_remui(%arg0: i32, %arg1: i32) -> i32 {
  %0 = arith.remui %arg0, %arg1 : i32
  return %0 : i32
}

// CHECK-LABEL: func.func @test_remui_with_operations
// CHECK-NEXT: %[[DIV:.*]] = arith.divui %arg0, %arg1 : i32
// CHECK-NEXT: %[[MUL:.*]] = arith.muli %[[DIV]], %arg1 : i32
// CHECK-NEXT: %[[SUB:.*]] = arith.subi %arg0, %[[MUL]] : i32
// CHECK-NEXT: %[[ADD:.*]] = arith.addi %[[SUB]], %arg2 : i32
// CHECK-NEXT: return %[[ADD]] : i32
func.func @test_remui_with_operations(%arg0: i32, %arg1: i32, %arg2: i32) -> i32 {
  %remainder = arith.remui %arg0, %arg1 : i32
  %result = arith.addi %remainder, %arg2 : i32
  return %result : i32
}

// CHECK-LABEL: func.func @test_cyclic_remui
// CHECK-NEXT: %[[SUM:.*]] = arith.addi %arg0, %arg1 : i32
// CHECK-NEXT: %[[DIV:.*]] = arith.divui %[[SUM]], %arg2 : i32
// CHECK-NEXT: %[[MUL:.*]] = arith.muli %[[DIV]], %arg2 : i32
// CHECK-NEXT: %[[SUB:.*]] = arith.subi %[[SUM]], %[[MUL]] : i32
// CHECK-NEXT: return %[[SUB]] : i32
func.func @test_cyclic_remui(%arg0: i32, %arg1: i32, %arg2: i32) -> i32 {
  %sum = arith.addi %arg0, %arg1 : i32
  %remainder = arith.remui %sum, %arg2 : i32
  return %remainder : i32
}

// CHECK-LABEL: func.func @test_double_remui
// CHECK-NEXT: %[[DIV1:.*]] = arith.divui %arg0, %arg1 : i32
// CHECK-NEXT: %[[MUL1:.*]] = arith.muli %[[DIV1]], %arg1 : i32
// CHECK-NEXT: %[[SUB1:.*]] = arith.subi %arg0, %[[MUL1]] : i32
// CHECK-NEXT: %[[DIV2:.*]] = arith.divui %[[SUB1]], %arg1 : i32
// CHECK-NEXT: %[[MUL2:.*]] = arith.muli %[[DIV2]], %arg1 : i32
// CHECK-NEXT: %[[SUB2:.*]] = arith.subi %[[SUB1]], %[[MUL2]] : i32
// CHECK-NEXT: return %[[SUB2]] : i32
func.func @test_double_remui(%arg0: i32, %arg1: i32) -> i32 {
  %remainder1 = arith.remui %arg0, %arg1 : i32
  %remainder2 = arith.remui %remainder1, %arg1 : i32
  return %remainder2 : i32
}
