// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/Lab_4_mlir_Solovev_a_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(Lab_4_mlir_Solovev_a_FIIT1_MLIR)" %s | FileCheck %s

module {
  // Test 1. Simple loop with static trip count
  // CHECK-LABEL: func.func @test_simple_for
  // CHECK-NEXT: %c0 = arith.constant 0 : index
  // CHECK-NEXT: %c10 = arith.constant 10 : index
  // CHECK-NEXT: %c1 = arith.constant 1 : index
  // CHECK-NEXT: scf.for %{{.*}} = %c0 to %c10 step %c1 {
  // CHECK-NEXT:   %{{.*}} = arith.muli %{{.*}}, %c1 : index
  // CHECK-NEXT:   %{{.*}} = arith.addi %{{.*}}, %c0 : index
  // CHECK-NEXT: } {trip_count = 10 : index}
  // CHECK-NEXT: return
  func.func @test_simple_for() {
    %c0 = arith.constant 0 : index
    %c10 = arith.constant 10 : index
    %c1 = arith.constant 1 : index

    scf.for %i = %c0 to %c10 step %c1 {
      %mul = arith.muli %i, %c1 : index
      %add = arith.addi %mul, %c0 : index
    }

    return
  }
  
  // Test 2. Nested loops with static trip counts
  // CHECK-LABEL: func.func @test_nested_loops
  // CHECK-NEXT: %c0 = arith.constant 0 : index
  // CHECK-NEXT: %c4 = arith.constant 4 : index
  // CHECK-NEXT: %c2 = arith.constant 2 : index
  // CHECK-NEXT: scf.for %{{.*}} = %c0 to %c4 step %c2 {
  // CHECK-NEXT:   scf.for %{{.*}} = %c0 to %c4 step %c2 {
  // CHECK-NEXT:   } {trip_count = 2 : index}
  // CHECK-NEXT: } {trip_count = 2 : index}
  // CHECK-NEXT: return
  func.func @test_nested_loops() {
    %c0 = arith.constant 0 : index
    %c4 = arith.constant 4 : index
    %c2 = arith.constant 2 : index

    scf.for %i = %c0 to %c4 step %c2 {
      scf.for %j = %c0 to %c4 step %c2 {
      }
    }

    return
  }
  
  // Test 3. Loop with dynamic trip count (no attribute expected)
  // CHECK-LABEL: func.func @test_dynamic_loop
  // CHECK-NEXT: scf.for %{{.*}} = %{{.*}} to %{{.*}} step %{{.*}} {
  // CHECK-NEXT:   %{{.*}} = arith.muli %{{.*}}, %{{.*}} : index
  // CHECK-NEXT: }{{$}}
  // CHECK-NEXT: return
  func.func @test_dynamic_loop(%from: index, %to: index, %step: index) {
    scf.for %i = %from to %to step %step {
      %v = arith.muli %i, %step : index
    }

    return
  }
  
  // Test 4. Loop with non-zero lower bound
  // CHECK-LABEL: func.func @test_nonzero_lower_bound
  // CHECK-NEXT: %c5 = arith.constant 5 : index
  // CHECK-NEXT: %c15 = arith.constant 15 : index
  // CHECK-NEXT: %c2 = arith.constant 2 : index
  // CHECK-NEXT: scf.for %{{.*}} = %c5 to %c15 step %c2 {
  // CHECK-NEXT: } {trip_count = 5 : index}
  // CHECK-NEXT: return
  func.func @test_nonzero_lower_bound() {
    %c5 = arith.constant 5 : index
    %c15 = arith.constant 15 : index
    %c2 = arith.constant 2 : index

    scf.for %i = %c5 to %c15 step %c2 {
    }

    return
  }
  
  // Test 5. Loop inside conditional block
  // CHECK-LABEL: func.func @test_loop_in_if
  // CHECK-NEXT: %c0 = arith.constant 0 : index
  // CHECK-NEXT: %c3 = arith.constant 3 : index
  // CHECK-NEXT: %c1 = arith.constant 1 : index
  // CHECK-NEXT: scf.if %{{.*}} {
  // CHECK-NEXT:   scf.for %{{.*}} = %c0 to %c3 step %c1 {
  // CHECK-NEXT:   } {trip_count = 3 : index}
  // CHECK-NEXT: }
  // CHECK-NEXT: return
  func.func @test_loop_in_if(%cond: i1) {
    %c0 = arith.constant 0 : index
    %c3 = arith.constant 3 : index
    %c1 = arith.constant 1 : index

    scf.if %cond {
      scf.for %i = %c0 to %c3 step %c1 {
      }
    }

    return
  }

  // Test 6. Descending loop
  // CHECK-LABEL: func.func @test_descending_loop
  // CHECK-NEXT: %c10 = arith.constant 10 : index
  // CHECK-NEXT: %c0 = arith.constant 0 : index
  // CHECK-NEXT: %[[NEG3:.*]] = arith.constant -3 : index
  // CHECK-NEXT: scf.for %{{.*}} = %c10 to %c0 step %[[NEG3]] {
  // CHECK-NEXT: } {trip_count = 4 : index}
  // CHECK-NEXT: return
  func.func @test_descending_loop() {
    %c10 = arith.constant 10 : index
    %c0 = arith.constant 0 : index
    %c-3 = arith.constant -3 : index
    scf.for %arg0 = %c10 to %c0 step %c-3 {
    }

    return
  }

  // Test 7. Negative step with invalid range (should not set trip_count)
  // CHECK-LABEL: func.func @test_negative_step_wrong_range
  // CHECK-NEXT: %c0 = arith.constant 0 : index
  // CHECK-NEXT: %c3 = arith.constant 3 : index
  // CHECK-NEXT: %[[NEG2:.*]] = arith.constant -2 : index
  // CHECK-NEXT: scf.for %{{.*}} = %c0 to %c3 step %[[NEG2]] {
  // CHECK-NOT: trip_count
  // CHECK-NEXT: }
  // CHECK-NEXT: return
  func.func @test_negative_step_wrong_range() {
    %c0 = arith.constant 0 : index
    %c3 = arith.constant 3 : index
    %c_neg2 = arith.constant -2 : index
    scf.for %i = %c0 to %c3 step %c_neg2 {
    }

    return
  }
}
