// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/IterCounterPass_Ivanov_Mikhail_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(IterCounterPass_Ivanov_Mikhail_FIIT1_MLIR)" %s | FileCheck --match-full-lines %s

// case1
// void loop_inc() {
//   for (int i = 0; i < 10; ++i)
// }

// case2
// void loop_dec() {
//   for (int i = 9; i > -1; --i)
// }

// case3
// void invalid_inc() {
//   for (int i = 10; i < 0; ++i)
// }

// case4
// void zero_step() {
//   for (int i = 0; i < 10; i += 0)
// }

module {
  func.func @loop_inc() {
    // CHECK-LABEL:  func.func @loop_inc() {
    // CHECK:   %c0 = arith.constant 0  : index
    // CHECK-NEXT:   %c10 = arith.constant 10 : index
    // CHECK-NEXT:   %c1 = arith.constant 1  : index
    // CHECK-NEXT:   scf.for %arg0 = %c0 to %c10 step %c1 {
    // CHECK-NEXT:   } {trip_count = 10 : index}
    %c0 = arith.constant 0 : index
    %c10 = arith.constant 10 : index
    %c1 = arith.constant 1 : index
    scf.for %arg0 = %c0 to %c10 step %c1 {
      scf.yield
    }
    return
  }

  func.func @loop_dec() {
    // CHECK-LABEL:  func.func @loop_dec() {
    // CHECK:        %c9  = arith.constant 9  : index
    // CHECK-NEXT:   %c-1 = arith.constant -1 : index
    // CHECK-NEXT:   scf.for %arg0 = %c9 to %c-1 step %c-1 {
    // CHECK-NEXT:   } {trip_count = 10 : index}
    %c9  = arith.constant 9  : index
    %cneg1 = arith.constant -1 : index
    scf.for %arg0 = %c9 to %cneg1 step %cneg1 {
      scf.yield
    }
    return
  }

  func.func @invalid_inc() {
    // CHECK-LABEL: func.func @invalid_inc() {
    // CHECK:        %c10 = arith.constant 10 : index
    // CHECK-NEXT:   %c0  = arith.constant 0  : index
    // CHECK-NEXT:   %c1  = arith.constant 1  : index
    // CHECK-NEXT:   scf.for %arg0 = %c10 to %c0 step %c1 {
    // CHECK-NEXT:   } {trip_count = 0 : index}
    %c10 = arith.constant 10 : index
    %c0  = arith.constant 0  : index
    %c1  = arith.constant 1  : index
    scf.for %arg0 = %c10 to %c0 step %c1 {
      scf.yield
    }
    return
  }

  func.func @zero_step() {
    // CHECK-LABEL: func.func @zero_step() {
    // CHECK:        %c0  = arith.constant 0  : index
    // CHECK-NEXT:   %c10 = arith.constant 10 : index
    // CHECK-NEXT:   %c0_0 = arith.constant 0  : index
    // CHECK-NEXT:   scf.for %arg0 = %c0 to %c10 step %c0_0 {
    // CHECK-NEXT:   }
    %c0  = arith.constant 0  : index
    %c10 = arith.constant 10 : index
    %c0_0 = arith.constant 0  : index
    scf.for %arg0 = %c0 to %c10 step %c0_0 {
      scf.yield
    }
    return
  }
}