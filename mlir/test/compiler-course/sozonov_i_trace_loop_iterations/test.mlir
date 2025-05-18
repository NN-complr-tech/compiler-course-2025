// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/TraceLoopIterationsPass_Sozonov_Ilya_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(TraceLoopIterationsPass_Sozonov_Ilya_FIIT3_MLIR)" %s | FileCheck %s

// CHECK-LABEL: func @test_scf_for
func.func @test_scf_for() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  %c2 = arith.constant 2 : index

  // CHECK: scf.for
  // CHECK-NEXT: func.call @trace_loop_iter_begin
  // CHECK-NEXT: %[[VAL:.*]] = arith.addi
  // CHECK-NEXT: func.call @trace_loop_iter_end

  scf.for %i = %c0 to %c10 step %c1 {
    %0 = arith.addi %c1, %c2 : index
  }
  return
}

// CHECK-LABEL: func @test_scf_while
func.func @test_scf_while() {
  %c10 = arith.constant 10 : i32
  %c1 = arith.constant 1 : i32
  %init = arith.constant 0 : i32

  // CHECK: scf.while
  // CHECK-NEXT: func.call @trace_loop_iter_begin()
  // CHECK-NEXT: %[[COND:.*]] = arith.cmpi
  // CHECK-NEXT: func.call @trace_loop_iter_end()
  // CHECK-NEXT: scf.condition(%[[COND]]) %{{.*}} : i32
  // CHECK-NEXT: } do {
  // CHECK-NEXT: ^bb0
  // CHECK-NEXT: %[[INC:.*]] = arith.addi
  // CHECK-NEXT: scf.yield %[[INC]] : i32

  scf.while (%x = %init) : (i32) -> (i32) {
    %cond = arith.cmpi slt, %x, %c10 : i32
    scf.condition(%cond) %x : i32
  } do {
  ^bb0(%x: i32):
    %inc = arith.addi %x, %c1 : i32
    scf.yield %inc : i32
  }
  return
}

// CHECK-LABEL: func @test_affine_for
func.func @test_affine_for() {
  %c1 = arith.constant 1 : i32
  %c2 = arith.constant 2 : i32

  // CHECK: affine.for
  // CHECK-NEXT: func.call @trace_loop_iter_begin
  // CHECK-NEXT: %[[V:.*]] = arith.addi
  // CHECK-NEXT: func.call @trace_loop_iter_end

  affine.for %i = 0 to 10 {
    %v = arith.addi %c1, %c2 : i32
  }
  return
}

// CHECK-LABEL: func @test_nested_loops
func.func @test_nested_loops() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  %c2 = arith.constant 2 : index
  %ci1 = arith.constant 1 : i32
  %ci2 = arith.constant 2 : i32

  // CHECK: scf.for
  // CHECK-NEXT: func.call @trace_loop_iter_begin
  // CHECK-NEXT: affine.for
  // CHECK-NEXT: func.call @trace_loop_iter_begin
  // CHECK-NEXT: %[[V:.*]] = arith.addi
  // CHECK-NEXT: func.call @trace_loop_iter_end
  // CHECK-NEXT: }
  // CHECK-NEXT: func.call @trace_loop_iter_end

  scf.for %i = %c0 to %c10 step %c1 {
    affine.for %j = 0 to 5 {
      %v = arith.addi %ci1, %ci2 : i32
    }
  }
  return
}

// CHECK-LABEL: func @test_no_loops
func.func @test_no_loops() {

  // CHECK-NOT: func.call @trace_loop_iter_begin
  // CHECK-NOT: func.call @trace_loop_iter_end

  %c1 = arith.constant 1 : i32
  %c2 = arith.constant 2 : i32
  %v = arith.addi %c1, %c2 : i32
  return
}
