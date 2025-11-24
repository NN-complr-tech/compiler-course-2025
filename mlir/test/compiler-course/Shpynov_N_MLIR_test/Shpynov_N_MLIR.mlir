// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/AnnotatorPass_Shpynov_Nikita_FIIT1_MLIR%shlibext --pass-pipeline="builtin.module(AnnotatorPass_Shpynov_Nikita_FIIT1_MLIR)" %s | FileCheck %s

func.func @positive_step() {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %c10 = arith.constant 10 : index
  // CHECK-LABEL: scf.for
  // CHECK: {trip_count = 10 : i64}
  scf.for %i = %c0 to %c10 step %c1 {
    %tmp = arith.addi %i, %c1 : index
  }
  return
}

func.func @double_step() {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %c2 = arith.constant 2 : index
  %c10 = arith.constant 10 : index
  // CHECK-LABEL: scf.for
  // CHECK: {trip_count = 5 : i64}
  scf.for %i = %c0 to %c10 step %c2 {
    %tmp = arith.addi %i, %c1 : index
  }
  return
}

func.func @negative_step() {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %c10 = arith.constant 10 : index
  %c_neg1 = arith.constant -1 : index
  // CHECK-LABEL: scf.for
  // CHECK: {trip_count = 10 : i64}
  scf.for %i = %c10 to %c0 step %c_neg1 {
    %tmp = arith.addi %i, %c1 : index
  }
  return
}

func.func @unknown_bounds(%n: index) {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  // CHECK-LABEL: scf.for
  // CHECK-NOT: trip_count
  scf.for %i = %c0 to %n step %c1 {
    %tmp = arith.addi %i, %c1 : index
  }
  return
}

func.func @zero_iterations() {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %c5 = arith.constant 5 : index
  // CHECK-LABEL: scf.for
  // CHECK-NOT: trip_count
  scf.for %i = %c5 to %c0 step %c1 {
    %tmp = arith.muli %i, %c1 : index
  }
  return
}

func.func @zero_step() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  // CHECK-LABEL: scf.for
  // CHECK-NOT: trip_count
  scf.for %i = %c0 to %c10 step %c0 {
    %tmp = arith.addi %i, %c0 : index
  }
  return
}