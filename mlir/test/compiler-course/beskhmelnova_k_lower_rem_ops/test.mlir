// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/LowerRemOpsPass_Beskhmelnova_Kseniya_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(LowerRemOpsPass_Beskhmelnova_Kseniya_FIIT1_MLIR)" %s | FileCheck %s

// CHECK: module attributes {{.*}} {
module attributes {dlti.dl_spec = #dlti.dl_spec<#dlti.dl_entry<f64, dense<64> : vector<2xi64>>, #dlti.dl_entry<f16, dense<16> : vector<2xi64>>, #dlti.dl_entry<f128, dense<128> : vector<2xi64>>, #dlti.dl_entry<!llvm.ptr<270>, dense<32> : vector<4xi64>>, #dlti.dl_entry<!llvm.ptr<271>, dense<32> : vector<4xi64>>, #dlti.dl_entry<!llvm.ptr<272>, dense<64> : vector<4xi64>>, #dlti.dl_entry<i64, dense<64> : vector<2xi64>>, #dlti.dl_entry<i128, dense<128> : vector<2xi64>>, #dlti.dl_entry<f80, dense<128> : vector<2xi64>>, #dlti.dl_entry<!llvm.ptr, dense<64> : vector<4xi64>>, #dlti.dl_entry<i1, dense<8> : vector<2xi64>>, #dlti.dl_entry<i8, dense<8> : vector<2xi64>>, #dlti.dl_entry<i16, dense<16> : vector<2xi64>>, #dlti.dl_entry<i32, dense<32> : vector<2xi64>>, #dlti.dl_entry<"dlti.endianness", "little">, #dlti.dl_entry<"dlti.stack_alignment", 128 : i64>>} {

  // Test 1: Signed remainder
  // CHECK-LABEL: func.func @test_signed
  // CHECK-NEXT:     %[[DIV:.*]] = arith.divsi %arg0, %arg1 : i32
  // CHECK-NEXT:     %[[MUL:.*]] = arith.muli %[[DIV]], %arg1 : i32
  // CHECK-NEXT:     %[[RES:.*]] = arith.subi %arg0, %[[MUL]] : i32
  // CHECK-NEXT:     return %[[RES]] : i32
  func.func @test_signed(%a: i32, %b: i32) -> i32 {
    %0 = arith.remsi %a, %b : i32
    return %0 : i32
  }

  // Test 2: Unsigned remainder
  // CHECK-LABEL: func.func @test_unsigned
  // CHECK-NEXT:     %[[DIV:.*]] = arith.divui %arg0, %arg1 : i32
  // CHECK-NEXT:     %[[MUL:.*]] = arith.muli %[[DIV]], %arg1 : i32
  // CHECK-NEXT:     %[[RES:.*]] = arith.subi %arg0, %[[MUL]] : i32
  // CHECK-NEXT:     return %[[RES]] : i32
  func.func @test_unsigned(%x: i32, %y: i32) -> i32 {
    %1 = arith.remui %x, %y : i32
    return %1 : i32
  }

  // Test 3: Two remainders in one function
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
}
