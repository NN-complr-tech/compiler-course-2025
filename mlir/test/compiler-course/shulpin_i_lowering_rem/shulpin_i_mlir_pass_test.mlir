// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/LowerRemPass_ShulpinIlya_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(LowerRemPass_ShulpinIlya_FIIT1_MLIR)" %s | FileCheck %s

// -----------------------------------------------------------------------------
// 1) Cyclic remainder: compute rem(addi(arg0, arg1), arg2)
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// 2) Combined signed and unsigned remainder addition
// -----------------------------------------------------------------------------
// CHECK-LABEL: func.func @test_both
// CHECK-NEXT:     %[[DIV1:.*]] = arith.divsi %arg0, %arg1 : i32
// CHECK-NEXT:     %[[MUL1:.*]] = arith.muli %[[DIV1]], %arg1 : i32
// CHECK-NEXT:     %[[RES1:.*]] = arith.subi %arg0, %[[MUL1]] : i32
// CHECK-NEXT:     %[[DIV2:.*]] = arith.divui %arg0, %arg1 : i32
// CHECK-NEXT:     %[[MUL2:.*]] = arith.muli %[[DIV2]], %arg1 : i32
// CHECK-NEXT:     %[[RES2:.*]] = arith.subi %arg0, %[[MUL2]] : i32
// CHECK-NEXT:     %[[SUM:.*]] = arith.addi %[[RES1]], %[[RES2]] : i32
// CHECK-NEXT:     return %[[SUM]] : i32
func.func @test_both(%m: i32, %n: i32) -> i32 {
  %2 = arith.remsi %m, %n : i32
  %3 = arith.remui %m, %n : i32
  %4 = arith.addi %2, %3 : i32
  return %4 : i32
}

// -----------------------------------------------------------------------------
// 3) Divisibility check: select 1 if signed remainder is zero, else 0
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// 4) Nested rem inside further multiplication chain
// -----------------------------------------------------------------------------
 // CHECK-LABEL: func.func @test_mul_remsi_mul
// CHECK-NEXT: %[[PROD1:.*]] = arith.muli %arg0, %arg1 : i32
// CHECK-NEXT: %[[DIV:.*]]   = arith.divsi %[[PROD1]], %arg2 : i32
// CHECK-NEXT: %[[MUL:.*]]   = arith.muli %[[DIV]], %arg2 : i32
// CHECK-NEXT: %[[SUB:.*]]   = arith.subi %[[PROD1]], %[[MUL]] : i32
// CHECK-NEXT: %[[PROD2:.*]] = arith.muli %[[SUB]], %arg3 : i32
// CHECK-NEXT: return %[[PROD2]] : i32
func.func @test_mul_remsi_mul(%arg0: i32, %arg1: i32, %arg2: i32, %arg3: i32) -> i32 {
  %prod = arith.muli %arg0, %arg1 : i32
  %rem  = arith.remsi %prod, %arg2   : i32
  %res  = arith.muli %rem, %arg3     : i32
  return %res : i32
}

// -----------------------------------------------------------------------------
// 5) Expand signed remainder operation
// -----------------------------------------------------------------------------
// CHECK-LABEL: func.func @test_signed
// CHECK-NEXT:     %[[DIV:.*]] = arith.divsi %arg0, %arg1 : i32
// CHECK-NEXT:     %[[MUL:.*]] = arith.muli %[[DIV]], %arg1 : i32
// CHECK-NEXT:     %[[RES:.*]] = arith.subi %arg0, %[[MUL]] : i32
// CHECK-NEXT:     return %[[RES]] : i32
func.func @test_signed(%a: i32, %b: i32) -> i32 {
  %0 = arith.remsi %a, %b : i32
  return %0 : i32
}

// -----------------------------------------------------------------------------
// 6) Expand unsigned remainder operation
// -----------------------------------------------------------------------------
// CHECK-LABEL: func.func @test_unsigned
// CHECK-NEXT:     %[[DIV:.*]] = arith.divui %arg0, %arg1 : i32
// CHECK-NEXT:     %[[MUL:.*]] = arith.muli %[[DIV]], %arg1 : i32
// CHECK-NEXT:     %[[RES:.*]] = arith.subi %arg0, %[[MUL]] : i32
// CHECK-NEXT:     return %[[RES]] : i32
func.func @test_unsigned(%x: i32, %y: i32) -> i32 {
  %1 = arith.remui %x, %y : i32
  return %1 : i32
}
