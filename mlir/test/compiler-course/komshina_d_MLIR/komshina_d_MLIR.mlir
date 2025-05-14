// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/ReplaceCeil_Komshina_Daria_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(ReplaceCeil_Komshina_Daria_FIIT1_MLIR)" %s | FileCheck %s

module {
  // Test 1: Simple ceil
  // CHECK-LABEL: func.func @test_simple
  // CHECK-NEXT:   %[[NEG:.*]] = arith.negf %arg0 : f32
  // CHECK-NEXT:   %[[FLOOR:.*]] = math.floor %[[NEG]] : f32
  // CHECK-NEXT:   %[[RES:.*]] = arith.negf %[[FLOOR]] : f32
  // CHECK-NEXT:   return %[[RES]] : f32
  func.func @test_simple(%arg0: f32) -> f32 {
    %0 = math.ceil %arg0 : f32
    return %0 : f32
  }

  // Test 2: Two ceil operations
  // CHECK-LABEL: func.func @test_multiple
  // CHECK:       %[[NEG1:.*]] = arith.negf %arg0 : f32
  // CHECK-NEXT:  %[[FLOOR1:.*]] = math.floor %[[NEG1]] : f32
  // CHECK-NEXT:  %[[RES1:.*]] = arith.negf %[[FLOOR1]] : f32
  // CHECK-NEXT:  %[[NEG2:.*]] = arith.negf %arg1 : f32
  // CHECK-NEXT:  %[[FLOOR2:.*]] = math.floor %[[NEG2]] : f32
  // CHECK-NEXT:  %[[RES2:.*]] = arith.negf %[[FLOOR2]] : f32
  // CHECK-NEXT:  %[[SUM:.*]] = arith.addf %[[RES1]], %[[RES2]] : f32
  // CHECK-NEXT:  return %[[SUM]] : f32
  func.func @test_multiple(%arg0: f32, %arg1: f32) -> f32 {
    %a = math.ceil %arg0 : f32
    %b = math.ceil %arg1 : f32
    %c = arith.addf %a, %b : f32
    return %c : f32
  }

  // Test 3: ceil inside control flow
  // CHECK-LABEL: func.func @test_if
  // CHECK:       %[[NEG:.*]] = arith.negf %arg0 : f32
  // CHECK-NEXT:  %[[FLOOR:.*]] = math.floor %[[NEG]] : f32
  // CHECK-NEXT:  %[[RES:.*]] = arith.negf %[[FLOOR]] : f32
  func.func @test_if(%arg0: f32, %cond: i1) -> f32 {
    %res = scf.if %cond -> (f32) {
      %tmp = math.ceil %arg0 : f32
      scf.yield %tmp : f32
    } else {
      scf.yield %arg0 : f32
    }
    return %res : f32
  }
}

  // Test 4: ceil on vector of floats
  // CHECK-LABEL: func.func @test_vector
  // CHECK-NEXT:   %[[NEG:.*]] = arith.negf %arg0 : vector<4xf32>
  // CHECK-NEXT:   %[[FLOOR:.*]] = math.floor %[[NEG]] : vector<4xf32>
  // CHECK-NEXT:   %[[RES:.*]] = arith.negf %[[FLOOR]] : vector<4xf32>
  // CHECK-NEXT:   return %[[RES]] : vector<4xf32>
  func.func @test_vector(%arg0: vector<4xf32>) -> vector<4xf32> {
    %0 = math.ceil %arg0 : vector<4xf32>
    return %0 : vector<4xf32>
  }

  // Test 5: Ceil on f64 input
  // CHECK-LABEL: func.func @test_f64
  // CHECK-NEXT:   %[[NEG:.*]] = arith.negf %arg0 : f64
  // CHECK-NEXT:   %[[FLOOR:.*]] = math.floor %[[NEG]] : f64
  // CHECK-NEXT:   %[[RES:.*]] = arith.negf %[[FLOOR]] : f64
  // CHECK-NEXT:   return %[[RES]] : f64
  func.func @test_f64(%arg0: f64) -> f64 {
    %0 = math.ceil %arg0 : f64
    return %0 : f64
  }
