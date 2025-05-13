// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/MlirPassLoopIterBeginEnd_Baranov_Aleksey_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(MlirPassLoopIterBeginEnd_Baranov_Aleksey_FIIT1_MLIR)" %s | FileCheck %s

//===----------------------------------------------------------------------===//
// affine.for
//===----------------------------------------------------------------------===//
module {
  func.func private @trace_loop_iter_begin_1(index) -> ()
  func.func private @trace_loop_iter_end_1(index) -> ()

  // CHECK-LABEL: func.func @affine_loop
  func.func @affine_loop() {
    %c0 = arith.constant 0 : index
    %c10 = arith.constant 10 : index

    affine.for %i = %c0 to %c10 {
      // CHECK: call @trace_loop_iter_begin_1
      // CHECK: call @trace_loop_iter_end_1
      "test.op"() : () -> ()
    }
    return
  }
}

//===----------------------------------------------------------------------===//
// scf.for
//===----------------------------------------------------------------------===//
module {
  func.func private @trace_loop_iter_begin_1(index) -> ()
  func.func private @trace_loop_iter_end_1(index) -> ()

  // CHECK-LABEL: func.func @scf_loop
  func.func @scf_loop() {
    %c0 = arith.constant 0 : index
    %c10 = arith.constant 10 : index
    %c1 = arith.constant 1 : index

    scf.for %i = %c0 to %c10 step %c1 {
      // CHECK: call @trace_loop_iter_begin_1
      // CHECK: call @trace_loop_iter_end_1
      "test.op"() : () -> ()
    }
    return
  }
}

//===----------------------------------------------------------------------===//
// scf.while
//===----------------------------------------------------------------------===//
module {
  func.func private @trace_loop_iter_begin_1(i32) -> ()
  func.func private @trace_loop_iter_end_1(i32) -> ()

  // CHECK-LABEL: func.func @scf_while
  func.func @scf_while() -> i32 {
    %sum_init = arith.constant 0 : i32
    %i_init = arith.constant 0 : i32
    %limit = arith.constant 10 : i32

    %result:2 = scf.while (%sum = %sum_init, %i = %i_init) : (i32, i32) -> (i32, i32) {
      %cmp = arith.cmpi slt, %i, %limit : i32
      scf.condition(%cmp) %sum, %i : i32, i32
    } do {
    ^bb0(%sum_arg: i32, %i_arg: i32):
      // CHECK: call @trace_loop_iter_begin_1
      // CHECK: call @trace_loop_iter_end_1
      %new_sum = arith.addi %sum_arg, %i_arg : i32
      %one = arith.constant 1 : i32
      %new_i = arith.addi %i_arg, %one : i32
      scf.yield %new_sum, %new_i : i32, i32
    }

    return %result#0 : i32
  }
}


module {
  func.func private @trace_loop_iter_begin_2(index, index) -> ()
  func.func private @trace_loop_iter_end_2(index, index) -> ()

  // CHECK-LABEL: func.func @parallel_loop
  func.func @parallel_loop() {
    affine.parallel (%i, %j) = (0, 0) to (10, 10) step (2, 2) {
      // CHECK: func.call @trace_loop_iter_begin_2
      "test.op"() : () -> ()
      // CHECK: func.call @trace_loop_iter_end_2
    }
    return
  }
}
