// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/librem_pass_Mamaeva_Olga_FIIT3_MLIR.so \
// RUN: --pass-pipeline="builtin.module(rem_pass_Mamaeva_Olga_FIIT3_MLIR)" %s | FileCheck %s

// CHECK-LABEL: func.func @test_remsi
func.func @test_remsi(%arg0: i32, %arg1: i32) -> i32 {
  // CHECK: %[[DIV:.*]] = arith.divsi %arg1, %arg0 : i32
  // CHECK: %[[MUL:.*]] = arith.muli %[[DIV]], %arg0 : i32
  // CHECK: %[[RES:.*]] = arith.subi %arg1, %[[MUL]] : i32
  %rem = arith.remsi %arg1, %arg0 : i32
  // CHECK: return %[[RES]]
  return %rem : i32
}
