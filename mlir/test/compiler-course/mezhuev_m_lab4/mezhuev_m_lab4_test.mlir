// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/ForLoopTripCountAnnotator_Mezhuev_Maksim_FIIT2_MLIR%shlibext --pass-pipeline="builtin.module(ForLoopTripCountAnnotator_Mezhuev_Maksim_FIIT2_MLIR)" %s | FileCheck %s

func.func private @get_upper_bound() -> index

func.func @foo() {
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index
  %c1 = arith.constant 1 : index
  // CHECK: scf.for
  // CHECK: } {trip_count = 5 : i64}
  scf.for %i = %c0 to %c5 step %c1 {
  }
  return
}

func.func @bar() {
  %c2 = arith.constant 2 : index
  %c12 = arith.constant 17 : index
  %c2_step = arith.constant 2 : index
  // CHECK: scf.for
  // CHECK: } {trip_count = 8 : i64}
  scf.for %i = %c2 to %c12 step %c2_step {
  }
  return
}

func.func @foobar(%arg0: index) {
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index  
  // CHECK: scf.for
  // CHECK-NOT: trip_count
  scf.for %i = %arg0 to %c10 step %c1 {
  }
  return
}

func.func @barfoo(%arg0: index) {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %upper = call @get_upper_bound() : () -> index
  // CHECK: scf.for
  // CHECK-NOT: trip_count
  scf.for %i = %c0 to %upper step %c1 {
  }
  return
}

func.func @qux() {
  %c0 = arith.constant 1 : index
  %c5 = arith.constant 5 : index
  %c1 = arith.constant 0 : index
  // CHECK: scf.for
  // CHECK-NOT: trip_count
  scf.for %i = %c0 to %c5 step %c1 {
  }
  return
}

func.func @quxbar() {
  %c0 = arith.constant 5 : index
  %c5 = arith.constant 1 : index
  %c1 = arith.constant 1 : index
  // CHECK: scf.for
  // CHECK-NOT: trip_count
  scf.for %i = %c0 to %c5 step %c1 {
  }
  return
}

func.func @baz() {
  %c0 = arith.constant 5 : index
  %c5 = arith.constant 1 : index
  %c1 = arith.constant -1 : index
  // CHECK: scf.for
  // CHECK: } {trip_count = 4 : i64}
  scf.for %i = %c0 to %c5 step %c1 {
  }
  return
}

func.func @negative_step_valid() {
  %c10 = arith.constant 10 : index
  %c0 = arith.constant 0 : index
  %c_neg1 = arith.constant -1 : index
  // CHECK: scf.for
  // CHECK: } {trip_count = 10 : i64}
  scf.for %i = %c10 to %c0 step %c_neg1 {
  }
  return
}

func.func @large_numbers() {
  %c0 = arith.constant 0 : index
  %c_large = arith.constant 1000000 : index
  %c10 = arith.constant 10 : index
  // CHECK: scf.for
  // CHECK: } {trip_count = 100000 : i64}
  scf.for %i = %c0 to %c_large step %c10 {
  }
  return
}

func.func @step_equals_range() {
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index
  %c5_step = arith.constant 5 : index
  // CHECK: scf.for
  // CHECK: } {trip_count = 1 : i64}
  scf.for %i = %c0 to %c5 step %c5_step {
  }
  return
}

func.func @step_larger_than_range() {
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index
  %c10 = arith.constant 10 : index
  // CHECK: scf.for
  // CHECK: } {trip_count = 1 : i64}
  scf.for %i = %c0 to %c5 step %c10 {
  }
  return
}

func.func @foobaz() {
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 4611686018427387904 : index
  %c1 = arith.constant -128 : index
  // CHECK: scf.for
  // CHECK-NOT: trip_count
  scf.for %i = %c0 to %c5 step %c1 {
  }
  return
}

func.func @edge_case_zero_step() {
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index
  %c0_step = arith.constant 0 : index
  // CHECK: scf.for
  // CHECK-NOT: trip_count
  scf.for %i = %c0 to %c5 step %c0_step {
  }
  return
}