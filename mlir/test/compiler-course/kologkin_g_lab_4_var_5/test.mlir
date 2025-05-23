// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/ReplaceCeilWithFloorNegPlugin_KolodkinGrigorii_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(ReplaceCeilWithFloorNegPlugin_KolodkinGrigorii_FIIT3_MLIR)" %s | FileCheck %s

//CHECK: func.func @test_simple(%arg0: f32) -> f32 {
//CHECK-NEXT: %0 = arith.negf %arg0 : f32
//CHECK-NEXT: %1 = math.floor %0 : f32
//CHECK-NEXT: %2 = arith.negf %1 : f32
//CHECK-NEXT: return %2 : f32
//CHECK-NEXT: }

func.func @test_simple(%arg0: f32) -> f32 {
  %0 = math.ceil %arg0 : f32
  return %0 : f32
}

//CHECK: func.func @test_single_arg(%arg0: f32) -> f32 {
//CHECK-NEXT: %0 = arith.negf %arg0 : f32
//CHECK-NEXT: %1 = math.floor %0 : f32
//CHECK-NEXT: %2 = arith.negf %1 : f32
//CHECK-NEXT: %3 = arith.negf %2 : f32
//CHECK-NEXT: %4 = math.floor %3 : f32
//CHECK-NEXT: %5 = arith.negf %4 : f32
//CHECK-NEXT: return %5 : f32
//CHECK-NEXT: }

func.func @test_single_arg(%arg: f32) -> f32 {
  %c1 = math.ceil %arg : f32
  %c2 = math.ceil %c1 : f32
  return %c2 : f32
}

//CHECK: func.func @test_vector_after_ceil(%arg0: vector<2xf32>) -> vector<2xf32> {
//CHECK-NEXT: %0 = arith.negf %arg0 : vector<2xf32>
//CHECK-NEXT: %1 = math.floor %0 : vector<2xf32>
//CHECK-NEXT: %2 = arith.negf %1 : vector<2xf32>
//CHECK-NEXT: %3 = arith.mulf %2, %arg0 : vector<2xf32>
//CHECK-NEXT: return %3 : vector<2xf32>
//CHECK-NEXT: }

func.func @test_vector_after_ceil(%vec: vector<2xf32>) -> vector<2xf32> {
  %ceil_vec = math.ceil %vec : vector<2xf32>
  %result = arith.mulf %ceil_vec, %vec : vector<2xf32>
  return %result : vector<2xf32>
}

//CHECK: func.func @test_two_args(%arg0: f64, %arg1: f64) -> f64 {
//CHECK-NEXT: %0 = arith.negf %arg0 : f64
//CHECK-NEXT: %1 = math.floor %0 : f64
//CHECK-NEXT: %2 = arith.negf %1 : f64
//CHECK-NEXT: %3 = arith.negf %arg1 : f64
//CHECK-NEXT: %4 = math.floor %3 : f64
//CHECK-NEXT: %5 = arith.negf %4 : f64
//CHECK-NEXT: %6 = arith.mulf %2, %5 : f64
//CHECK-NEXT: return %6 : f64
//CHECK-NEXT: }

func.func @test_two_args(%x: f64, %y: f64) -> f64 {
  %ceil_x = math.ceil %x : f64
  %ceil_y = math.ceil %y : f64
  %sum = arith.mulf %ceil_x, %ceil_y : f64
  return %sum : f64
}
