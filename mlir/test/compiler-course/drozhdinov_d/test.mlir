// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/LoopTracePass_DrozhdinovD_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(LoopTracePass_DrozhdinovD_FIIT1_MLIR)" %s | FileCheck %s

module {
  // CHECK: func.func private @trace_loop_iter_end()
  // CHECK-NEXT: func.func private @trace_loop_iter_begin()

  // CHECK-LABEL: func.func @affine_for()
  // CHECK: affine.for
  // CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
  // CHECK-NEXT: %1 = arith.index_cast
  // CHECK-NEXT: %2 = arith.addi
  // CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
  func.func @affine_for() -> i32 {
    %sum_init = arith.constant 0 : i32
    %result = affine.for %i = 0 to 10 iter_args(%sum_iter = %sum_init) -> i32 {
      %i_i32 = arith.index_cast %i : index to i32
      %new_sum = arith.addi %sum_iter, %i_i32 : i32
      affine.yield %new_sum : i32
    }
    return %result : i32
  }

  // CHECK-LABEL: func.func @scf_while(
  // CHECK: func.call @trace_loop_iter_begin() : () -> ()
  // CHECK-NEXT: %c10 = arith.constant 10 : index
  // CHECK-NEXT: %1 = arith.cmpi slt, %arg1, %c10 : index
  // CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
  // CHECK-NEXT: scf.condition(%1) %arg1 : index
  // CHECK-NEXT: } do {
  // CHECK-NEXT: ^bb0(%arg1: index):
  // CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
  // CHECK-NEXT: %1 = arith.addi %arg1, %c1 : index
  // CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
  func.func @scf_while(%arg0: index) {
    %one = arith.constant 1 : index
    %res = scf.while (%i = %arg0) : (index) -> (index) {
      %limit = arith.constant 10 : index
      %cond = arith.cmpi slt, %i, %limit : index
      scf.condition(%cond) %i : index
    } do {
    ^bb0(%i_curr: index):
      %inc = arith.addi %i_curr, %one : index
      scf.yield %inc : index
    }
    return
  }

  // CHECK-LABEL: func.func @scf_for()
  // CHECK: scf.for
  // CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
  // CHECK-NEXT: %0 = arith.addi
  // CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
  func.func @scf_for() {
    %c0 = arith.constant 0 : index
    %c10 = arith.constant 10 : index
    %c1 = arith.constant 1 : index
    scf.for %i = %c0 to %c10 step %c1 {
      %x = arith.addi %i, %c1 : index
    }
    return
  }

  // CHECK-LABEL: func.func @scf_parallel()
  // CHECK: scf.parallel
  // CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
  // CHECK-NEXT: %0 = arith.addi
  // CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
  func.func @scf_parallel() {
    %c0 = arith.constant 0 : index
    %c10 = arith.constant 10 : index
    %c1 = arith.constant 1 : index
    scf.parallel (%i) = (%c0) to (%c10) step (%c1) {
      %x = arith.addi %i, %c1 : index
    }
    return
  }
}
