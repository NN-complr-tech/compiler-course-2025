// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/TripCountPass_Lavrentyev_Alexey_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(TripCountPass_Lavrentyev_Alexey_FIIT3_MLIR)" %s | FileCheck %s

// test.cpp
// volatile int sink; // prevents aggressive DCE
// 
// void const_bounds() {
//   int sum = 0;
//   for (int i = 0; i < 10; ++i) // 10 iterations, expect trip_count = 10
//     sum += i;
//   sink = sum;
// }
// 
// void runtime_bounds(int n) {
//   int sum = 0;
//   for (int i = 0; i < n; i += 2) // unknown trip count, no attribute
//     sum += i;
//   sink = sum;
// }
// 
// void negative_step() {
//   int sum = 0;
//   for (int i = 9; i >= 0; --i) // 10 iterations with step ‑1
//     sum += i;
//   sink = sum;
// }

module {
  // 1. const_bounds()   for (i = 0; i < 10; ++i)
  // CHECK-LABEL: func.func @const_bounds() -> i32 {
  func.func @const_bounds() -> i32 {
    // CHECK: %c0 = arith.constant 0 : index
    // CHECK-DAG: %c10 = arith.constant 10 : index
    // CHECK-DAG: %c1 = arith.constant 1 : index
    // CHECK-DAG: %c0_i32 = arith.constant 0 : i32
    // CHECK-NEXT: %0 = scf.for %arg0 = %c0 to %c10 step %c1 iter_args(%arg1 = %c0_i32) -> (i32) {
    // CHECK-NEXT:   %1 = arith.index_cast %arg0 : index to i32
    // CHECK-NEXT:   %2 = arith.addi %arg1, %1 : i32
    // CHECK-NEXT:   scf.yield %2 : i32
    // CHECK-NEXT: } {trip_count = 10 : i64}
    // CHECK-NEXT: return %0 : i32
    // CHECK-NEXT: }
    %c0      = arith.constant 0      : index
    %c10     = arith.constant 10     : index
    %c1      = arith.constant 1      : index
    %sum0    = arith.constant 0      : i32

    // iter_args initialises the running sum; result returned by scf.for
    %sum1 = scf.for %i = %c0 to %c10 step %c1
             iter_args(%s = %sum0) -> (i32) {
      %i_i32 = arith.index_cast %i : index to i32
      %s2    = arith.addi %s, %i_i32 : i32
      scf.yield %s2 : i32
    }
    return %sum1 : i32
  }

  // 2. runtime_bounds(int n)   for (i = 0; i < n; i += 2)
  // CHECK-LABEL: func.func @runtime_bounds(%arg0: i32) -> i32 {
  func.func @runtime_bounds(%n : i32) -> i32 {
    // CHECK-DAG: %c0 = arith.constant 0 : index
    // CHECK-DAG: %c2 = arith.constant 2 : index
    // CHECK-NEXT: %0 = arith.index_cast %arg0 : i32 to index
    // CHECK-NEXT: %c0_i32 = arith.constant 0 : i32
    // CHECK-NEXT: %1 = scf.for %arg1 = %c0 to %0 step %c2 iter_args(%arg2 = %c0_i32) -> (i32) {
    // CHECK-NEXT:   %2 = arith.index_cast %arg1 : index to i32
    // CHECK-NEXT:   %3 = arith.addi %arg2, %2 : i32
    // CHECK-NEXT:   scf.yield %3 : i32
    // CHECK-NEXT: }
    // CHECK-NEXT: return %1 : i32
    // CHECK-NEXT: }
    %c0      = arith.constant 0  : index
    %c2      = arith.constant 2  : index
    %n_idx   = arith.index_cast %n : i32 to index
    %sum0    = arith.constant 0  : i32

    %sum1 = scf.for %i = %c0 to %n_idx step %c2
             iter_args(%s = %sum0) -> (i32) {
      %i_i32 = arith.index_cast %i : index to i32
      %s2    = arith.addi %s, %i_i32 : i32
      scf.yield %s2 : i32
    }
    return %sum1 : i32
  }

  // 3. negative_step()   for (i = 9; i >= 0; --i)
  // CHECK-LABEL: func.func @negative_step() -> i32 {
  func.func @negative_step() -> i32 {
    // CHECK-DAG: %c9 = arith.constant 9 : index
    // CHECK-DAG: %c-1 = arith.constant -1 : index
    // CHECK-DAG: %c-1_0 = arith.constant -1 : index
    // CHECK-DAG: %c0_i32 = arith.constant 0 : i32
    // CHECK-NEXT: %0 = scf.for %arg0 = %c9 to %c-1 step %c-1_0 iter_args(%arg1 = %c0_i32) -> (i32) {
    // CHECK-NEXT:   %1 = arith.index_cast %arg0 : index to i32
    // CHECK-NEXT:   %2 = arith.addi %arg1, %1 : i32
    // CHECK-NEXT:   scf.yield %2 : i32
    // CHECK-NEXT: } {trip_count = 10 : i64}
    // CHECK-NEXT: return %0 : i32
    // CHECK-NEXT: }
    %c9      = arith.constant 9   : index       // lower bound
    %cMinus1 = arith.constant -1  : index       // upper bound (exclusive)
    %cStep   = arith.constant -1  : index
    %sum0    = arith.constant 0   : i32

    %sum1 = scf.for %i = %c9 to %cMinus1 step %cStep
             iter_args(%s = %sum0) -> (i32) {
      %i_i32 = arith.index_cast %i : index to i32
      %s2    = arith.addi %s, %i_i32 : i32
      scf.yield %s2 : i32
    }
    return %sum1 : i32
  }
}
