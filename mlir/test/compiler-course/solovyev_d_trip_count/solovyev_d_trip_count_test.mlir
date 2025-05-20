// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/TripCountPass_Solovyev_Danila_FIIT3_MLIR%shlibext --pass-pipeline="builtin.module(TripCountPass_Solovyev_Danila_FIIT3_MLIR)" %s | FileCheck %s

func.func private @get_upper_bound() -> index

func.func @foo() {
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index
  %c1 = arith.constant 1 : index
  // CHECK: scf.for
  // CHECK-NEXT: {trip_count = 5 : i64}
  scf.for %i = %c0 to %c5 step %c1 {
  }
  return
}

func.func @bar() {
  %c2 = arith.constant 2 : index
  %c12 = arith.constant 17 : index
  %c2_step = arith.constant 2 : index
  // CHECK: scf.for
  // CHECK-NEXT: {trip_count = 7 : i64}
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
