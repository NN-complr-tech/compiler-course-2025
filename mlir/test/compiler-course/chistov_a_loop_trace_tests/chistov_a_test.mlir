// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/LoopTracePass_Chistov_Alexey_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(LoopTracePass)" %s | FileCheck %s

module {
  // CHECK: func.func private @trace_loop_iter_end()
  // CHECK-NEXT: func.func private @trace_loop_iter_begin()

  // CHECK-LABEL: func.func @affine_for()
  // CHECK: affine.for
  // CHECK-DAG: func.call @trace_loop_iter_begin() : () -> ()
  // CHECK-NEXT: %1 = arith.index_cast %arg0 : index to i32
  // CHECK-NEXT: %2 = arith.addi %arg1, %1 : i32
  // CHECK-DAG: func.call @trace_loop_iter_end() : () -> ()
  func.func @affine_for() -> i32 {
    %sum_init = arith.constant 0 : i32
    %result = affine.for %i = 0 to 10 iter_args(%sum_iter = %sum_init) -> i32 {
      %i_i32 = arith.index_cast %i : index to i32
      %new_sum = arith.addi %sum_iter, %i_i32 : i32
      affine.yield %new_sum : i32
    }
    return %result : i32
  }

  // CHECK-LABEL: func.func @scf_for()
  // CHECK: scf.for
  // CHECK-DAG: func.call @trace_loop_iter_begin() : () -> ()
  // CHECK-NEXT: %1 = arith.index_cast %arg0 : index to i32
  // CHECK-NEXT: %2 = arith.addi %arg1, %1 : i32
  // CHECK-DAG: func.call @trace_loop_iter_end() : () -> ()
  func.func @scf_for() -> i32 {
    %sum_init = arith.constant 0 : i32
    %lower = arith.constant 0 : index
    %upper = arith.constant 10 : index
    %step = arith.constant 1 : index

    %result = scf.for %i = %lower to %upper step %step iter_args(%sum_iter = %sum_init) -> i32 {
      %i_i32 = arith.index_cast %i : index to i32
      %new_sum = arith.addi %sum_iter, %i_i32 : i32
      scf.yield %new_sum : i32
    }

    return %result : i32
  }

  // CHECK-LABEL: func.func @scf_while()
  // CHECK: scf.while
  // CHECK: do {
  // CHECK-DAG: func.call @trace_loop_iter_begin() : () -> ()
  // CHECK-NEXT: %1 = arith.addi %arg0, %arg1 : i32
  // CHECK-NEXT: %c1_i32 = arith.constant 1 : i32
  // CHECK-NEXT: arith.addi %arg1, %c1_i32 : i32
  // CHECK-DAG: func.call @trace_loop_iter_end() : () -> ()
  func.func @scf_while() -> i32 {
    %sum_init = arith.constant 0 : i32
    %i_init = arith.constant 0 : i32
    %limit = arith.constant 10 : i32

    %result:2 = scf.while (%sum = %sum_init, %i = %i_init) : (i32, i32) -> (i32, i32) {
      %cmp = arith.cmpi slt, %i, %limit : i32
      scf.condition(%cmp) %sum, %i : i32, i32
    } do {
    ^bb0(%sum_arg: i32, %i_arg: i32):
      %new_sum = arith.addi %sum_arg, %i_arg : i32
      %one = arith.constant 1 : i32
      %new_i = arith.addi %i_arg, %one : i32
      scf.yield %new_sum, %new_i : i32, i32
    }

    return %result#0 : i32
  }

  // CHECK-LABEL: func.func @scf_parallel()
  // CHECK: scf.parallel
  // CHECK-DAG: func.call @trace_loop_iter_begin() : () -> ()
  // CHECK-NEXT: %1 = arith.index_cast %arg0 : index to i32
  // CHECK-NEXT: %2 = arith.addi %c0_i32, %1 : i32
  // CHECK-DAG: func.call @trace_loop_iter_end() : () -> ()
  func.func @scf_parallel() -> i32 {
    %sum_init = arith.constant 0 : i32
    %lower = arith.constant 0 : index
    %upper = arith.constant 10 : index
    %step = arith.constant 1 : index

    %result = scf.parallel (%i) = (%lower) to (%upper) step (%step) 
              init (%sum_init) -> i32 {
      %i_i32 = arith.index_cast %i : index to i32
      %partial_sum = arith.addi %sum_init, %i_i32 : i32
      "scf.reduce"(%partial_sum) ({
        ^bb0(%lhs: i32, %rhs: i32):
          %sum = arith.addi %lhs, %rhs : i32
          "scf.reduce.return"(%sum) : (i32) -> ()
      }) : (i32) -> ()
    }

    return %result : i32
  }
}
