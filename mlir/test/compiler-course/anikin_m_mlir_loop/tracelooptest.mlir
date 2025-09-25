// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/mlirtraceloop_Anikin_Maksim_FIIT2_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(mlirtraceloop_Anikin_Maksim_FIIT2_MLIR)" %s | FileCheck %s

// Affine For Loop Instrumentation Tests
module {
  func.func private @trace_loop_iter_begin_1(index) -> ()
  func.func private @trace_loop_iter_end_1(index) -> ()

  // CHECK-LABEL: func.func @affine_loop_tracing
  func.func @affine_loop_tracing() {
    %initial = arith.constant 0 : index
    %limit = arith.constant 10 : index

    affine.for %counter = %initial to %limit {
      // VERIFY: func.call @trace_loop_iter_begin_1(%arg0) : (index) -> ()
      // VERIFY-NEXT: "test.operation"
      // VERIFY-NEXT: func.call @trace_loop_iter_end_1(%arg0) : (index) -> ()
      "test.operation"() : () -> ()
    }
    func.return
  }
}

// SCF For Loop Verification
module {
  func.func private @trace_loop_iter_begin_1(index) -> ()
  func.func private @trace_loop_iter_end_1(index) -> ()

  // CHECK-LABEL: func.func @scf_for_loop_instrumentation
  func.func @scf_for_loop_instrumentation() {
    %start_val = arith.constant 0 : index
    %end_val = arith.constant 10 : index
    %increment = arith.constant 1 : index

    scf.for %index = %start_val to %end_val step %increment {
      // CHECK: func.call @trace_loop_iter_begin_1(%arg0) : (index) -> ()
      // CHECK-NEXT: "test.operation"
      // CHECK-NEXT: func.call @trace_loop_iter_end_1(%arg0) : (index) -> ()
      "test.operation"() : () -> ()
    }
    func.return
  }
}

// SCF While Loop Testing
module {
  func.func private @trace_loop_iter_begin_2(i32, i32) -> ()
  func.func private @trace_loop_iter_end_2(i32, i32) -> ()

  // CHECK-LABEL: func.func @while_loop_tracing
  func.func @while_loop_tracing() -> i32 {
    %initial_sum = arith.constant 0 : i32
    %initial_index = arith.constant 0 : i32
    %max_value = arith.constant 10 : i32

    %final_values:2 = scf.while (%accumulator = %initial_sum, %current = %initial_index) : (i32, i32) -> (i32, i32) {
      %condition = arith.cmpi slt, %current, %max_value : i32
      scf.condition(%condition) %accumulator, %current : i32, i32
    } do {
    ^bb0(%sum_arg: i32, %idx_arg: i32):
      // CHECK: func.call @trace_loop_iter_begin_2
      // CHECK-NEXT: arith.addi
      // CHECK: func.call @trace_loop_iter_end_2
      %updated_sum = arith.addi %sum_arg, %idx_arg : i32
      %unit = arith.constant 1 : i32
      %next_index = arith.addi %idx_arg, %unit : i32
      scf.yield %updated_sum, %next_index : i32, i32
    }

    func.return %final_values#0 : i32
  }
}

// Affine Parallel Loop Verification
module {
  func.func private @trace_loop_iter_begin_2(index, index) -> ()
  func.func private @trace_loop_iter_end_2(index, index) -> ()

  // CHECK-LABEL: func.func @parallel_loop_instrumentation
  func.func @parallel_loop_instrumentation() {
    affine.parallel (%dim1, %dim2) = (0, 0) to (10, 10) step (2, 2) {
      // CHECK: func.call @trace_loop_iter_begin_2(%arg0, %arg1) : (index, index) -> ()
      // CHECK-NEXT: "test.operation"() : () -> ()
      // CHECK-NEXT: func.call @trace_loop_iter_end_2(%arg0, %arg1) : (index, index) -> ()
      "test.operation"() : () -> ()
    }
    func.return
  }
}
