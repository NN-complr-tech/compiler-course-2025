// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/TraceLoopPass_Kurakin_Matvey_FIIT1_MLIR%shlibext --pass-pipeline="builtin.module(TraceLoopPass_Kurakin_Matvey_FIIT1_MLIR)" %s | FileCheck %s

module {

// CHECK: func.func @affine_for() -> i32 {
// CHECK-NEXT: %c0_i32 = arith.constant 0 : i32
// CHECK-NEXT: %0 = affine.for %arg0 = 0 to 10 iter_args(%arg1 = %c0_i32) -> (i32) {
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: %1 = arith.index_cast %arg0 : index to i32
// CHECK-NEXT: %2 = arith.addi %arg1, %1 : i32
// CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
// CHECK-NEXT: affine.yield %2 : i32
// CHECK-NEXT: }
// CHECK-NEXT: return %0 : i32
// CHECK-NEXT: }

  func.func @affine_for() -> i32 {
    %sum_init = arith.constant 0 : i32
    %res = affine.for %i = 0 to 10 iter_args(%arg = %sum_init) -> i32 {
      %i_32 = arith.index_cast %i : index to i32
      %sum = arith.addi %arg, %i_32 : i32
      affine.yield %sum : i32
    }
    return %res : i32
  }

// CHECK: func.func @scf_for() -> i32 {
// CHECK-NEXT: %c0_i32 = arith.constant 0 : i32
// CHECK-NEXT: %c0 = arith.constant 0 : index
// CHECK-NEXT: %c10 = arith.constant 10 : index
// CHECK-NEXT: %c1 = arith.constant 1 : index
// CHECK-NEXT: %0 = scf.for %arg0 = %c0 to %c10 step %c1 iter_args(%arg1 = %c0_i32) -> (i32) {
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: %1 = arith.index_cast %arg0 : index to i32
// CHECK-NEXT: %2 = arith.addi %arg1, %1 : i32
// CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
// CHECK-NEXT: scf.yield %2 : i32
// CHECK-NEXT: }
// CHECK-NEXT: return %0 : i32
// CHECK-NEXT: }

  func.func @scf_for() -> i32 {
    %sum_init = arith.constant 0 : i32
    %begin = arith.constant 0 : index
    %end = arith.constant 10 : index
    %step = arith.constant 1 : index
    
    %result = scf.for %i = %begin to %end step %step iter_args(%arg = %sum_init) -> i32 {
      %i_32 = arith.index_cast %i : index to i32
      %sum = arith.addi %arg, %i_32 : i32
      scf.yield %sum : i32
    }
    return %result : i32
  }

// CHECK: func.func @scf_while() -> i32 {
// CHECK-NEXT: %c0_i32 = arith.constant 0 : i32
// CHECK-NEXT: %c0_i32_0 = arith.constant 0 : i32
// CHECK-NEXT: %c10_i32 = arith.constant 10 : i32
// CHECK-NEXT: %0:2 = scf.while (%arg0 = %c0_i32, %arg1 = %c0_i32_0) : (i32, i32) -> (i32, i32) {
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: %1 = arith.cmpi slt, %arg1, %c10_i32 : i32
// CHECK-NEXT: scf.condition(%1) %arg0, %arg1 : i32, i32
// CHECK-NEXT: } do {
// CHECK-NEXT: ^bb0(%arg0: i32, %arg1: i32):
// CHECK-NEXT: %1 = arith.addi %arg0, %arg1 : i32
// CHECK-NEXT: %c1_i32 = arith.constant 1 : i32
// CHECK-NEXT: %2 = arith.addi %arg1, %c1_i32 : i32
// CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
// CHECK-NEXT: scf.yield %1, %2 : i32, i32
// CHECK-NEXT: }
// CHECK-NEXT: return %0#0 : i32
// CHECK-NEXT: }

  func.func @scf_while() -> i32 {
    %sum_init = arith.constant 0 : i32
    %i_init = arith.constant 0 : i32
    %sum_limit = arith.constant 10 : i32

    %result:2 = scf.while (%sum = %sum_init, %i = %i_init) : (i32, i32) -> (i32, i32) {
      %cmp = arith.cmpi slt, %i, %sum_limit : i32
      scf.condition(%cmp) %sum, %i : i32, i32
    } do {
    ^bb0(%sum_arg: i32, %i_arg: i32):
      %sum = arith.addi %sum_arg, %i_arg : i32
      %step = arith.constant 1 : i32
      %new_i = arith.addi %i_arg, %step : i32
      scf.yield %sum, %new_i : i32, i32
    }
    return %result#0 : i32
  }

// CHECK: func.func @affine_for_and_scf_for() -> i32 {
// CHECK-NEXT: %c0_i32 = arith.constant 0 : i32
// CHECK-NEXT: %c0 = arith.constant 0 : index
// CHECK-NEXT: %c5 = arith.constant 5 : index
// CHECK-NEXT: %c1 = arith.constant 1 : index
// CHECK-NEXT: %0 = affine.for %arg0 = 0 to 10 iter_args(%arg1 = %c0_i32) -> (i32) {
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: %1 = arith.index_cast %arg0 : index to i32
// CHECK-NEXT: %2 = scf.for %arg2 = %c0 to %c5 step %c1 iter_args(%arg3 = %arg1) -> (i32) {
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: %3 = arith.index_cast %arg2 : index to i32
// CHECK-NEXT: %4 = arith.addi %arg3, %3 : i32
// CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
// CHECK-NEXT: scf.yield %4 : i32
// CHECK-NEXT: }
// CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
// CHECK-NEXT: affine.yield %2 : i32
// CHECK-NEXT: }
// CHECK-NEXT: return %0 : i32
// CHECK-NEXT: }
  
  func.func @affine_for_and_scf_for() -> i32 {
    %sum_init = arith.constant 0 : i32
    %begin = arith.constant 0 : index
    %end = arith.constant 5 : index
    %step = arith.constant 1 : index

    %outer_result = affine.for %i = 0 to 10 iter_args(%outer_sum = %sum_init) -> i32 {
      %i_i32 = arith.index_cast %i : index to i32
      
      %inner_result = scf.for %j = %begin to %end step %step iter_args(%inner_sum = %outer_sum) -> i32 {
        %j_i32 = arith.index_cast %j : index to i32
        %sum = arith.addi %inner_sum, %j_i32 : i32
        scf.yield %sum : i32
      }
      affine.yield %inner_result : i32
    }
    return %outer_result : i32
  }

// CHECK: func.func @affine_for_and_scf_while() -> i32 {
// CHECK-NEXT: %c0_i32 = arith.constant 0 : i32
// CHECK-NEXT: %c0_i32_0 = arith.constant 0 : i32
// CHECK-NEXT: %c5_i32 = arith.constant 5 : i32
// CHECK-NEXT: %0 = affine.for %arg0 = 0 to 10 iter_args(%arg1 = %c0_i32) -> (i32) {
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: %1 = arith.index_cast %arg0 : index to i32
// CHECK-NEXT: %2:2 = scf.while (%arg2 = %arg1, %arg3 = %c0_i32_0) : (i32, i32) -> (i32, i32) {
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: %3 = arith.cmpi slt, %arg2, %c5_i32 : i32
// CHECK-NEXT: scf.condition(%3) %arg2, %arg3 : i32, i32
// CHECK-NEXT: } do {
// CHECK-NEXT: ^bb0(%arg2: i32, %arg3: i32):
// CHECK-NEXT: %3 = arith.addi %arg2, %arg3 : i32
// CHECK-NEXT: %c1_i32 = arith.constant 1 : i32
// CHECK-NEXT: %4 = arith.addi %arg3, %c1_i32 : i32
// CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
// CHECK-NEXT: scf.yield %3, %4 : i32, i32
// CHECK-NEXT: }
// CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
// CHECK-NEXT: affine.yield %2#0 : i32
// CHECK-NEXT: }
// CHECK-NEXT: return %0 : i32
// CHECK-NEXT: }

  func.func @affine_for_and_scf_while() -> i32 {
    %sum_init = arith.constant 0 : i32
    %j_init = arith.constant 0 : i32
    %sum_limit = arith.constant 5 : i32
    
    %outer_result = affine.for %i = 0 to 10 iter_args(%outer_sum = %sum_init) -> i32 {
      %i_i32 = arith.index_cast %i : index to i32
      %inner_result:2 = scf.while (%inner_sum = %outer_sum, %j = %j_init) : (i32, i32) -> (i32, i32) {
        %cmp = arith.cmpi slt, %inner_sum, %sum_limit : i32
        scf.condition(%cmp) %inner_sum, %j : i32, i32
      } do {
      ^bb0(%sum_arg: i32, %j_arg: i32):
        %sum = arith.addi %sum_arg, %j_arg : i32
        %step = arith.constant 1 : i32
        %new_j = arith.addi %j_arg, %step : i32
        scf.yield %sum, %new_j : i32, i32
      }
      affine.yield %inner_result#0 : i32
    }
    return %outer_result : i32
  }

// CHECK: func.func @scf_for_and_scf_while() -> i32 {
// CHECK-NEXT: %c0_i32 = arith.constant 0 : i32
// CHECK-NEXT: %c0 = arith.constant 0 : index
// CHECK-NEXT: %c5 = arith.constant 5 : index
// CHECK-NEXT: %c1 = arith.constant 1 : index
// CHECK-NEXT: %c0_i32_0 = arith.constant 0 : i32
// CHECK-NEXT: %c5_i32 = arith.constant 5 : i32
// CHECK-NEXT: %0 = scf.for %arg0 = %c0 to %c5 step %c1 iter_args(%arg1 = %c0_i32) -> (i32) {
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: %c0_i32_1 = arith.constant 0 : i32
// CHECK-NEXT: %1:2 = scf.while (%arg2 = %arg1, %arg3 = %c0_i32_0) : (i32, i32) -> (i32, i32) {
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: %2 = arith.cmpi slt, %arg2, %c5_i32 : i32
// CHECK-NEXT: scf.condition(%2) %arg2, %arg3 : i32, i32
// CHECK-NEXT: } do {
// CHECK-NEXT: ^bb0(%arg2: i32, %arg3: i32):
// CHECK-NEXT: %2 = arith.addi %arg2, %arg3 : i32
// CHECK-NEXT: %c1_i32 = arith.constant 1 : i32
// CHECK-NEXT: %3 = arith.addi %arg3, %c1_i32 : i32
// CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
// CHECK-NEXT: scf.yield %2, %3 : i32, i32
// CHECK-NEXT: }
// CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
// CHECK-NEXT: scf.yield %1#0 : i32
// CHECK-NEXT: }
// CHECK-NEXT: return %0 : i32
// CHECK-NEXT: }

  func.func @scf_for_and_scf_while() -> i32 {
    %sum_init = arith.constant 0 : i32
    %begin = arith.constant 0 : index
    %end = arith.constant 5 : index
    %step = arith.constant 1 : index    
    %j_init = arith.constant 0 : i32
    %sum_limit = arith.constant 5 : i32
    
    %outer_result = scf.for %i = %begin to %end step %step iter_args(%outer_sum = %sum_init) -> i32 {
      %counter = arith.constant 0 : i32
      %inner_result:2 = scf.while (%inner_sum = %outer_sum, %j = %j_init) : (i32, i32) -> (i32, i32) {
        %cmp = arith.cmpi slt, %inner_sum, %sum_limit : i32
        scf.condition(%cmp) %inner_sum, %j : i32, i32
      } do {
      ^bb0(%sum_arg: i32, %j_arg: i32):
        %sum = arith.addi %sum_arg, %j_arg : i32
        %step_j = arith.constant 1 : i32
        %new_j = arith.addi %j_arg, %step_j : i32
        scf.yield %sum, %new_j : i32, i32
      }
      scf.yield %inner_result#0 : i32
    }
    return %outer_result : i32
  }

// CHECK: func.func private @trace_loop_iter_begin()
// CHECK-NEXT: func.func private @trace_loop_iter_end()

}
