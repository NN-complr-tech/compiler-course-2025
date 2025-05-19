// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/LoopPassBeginEnd_Chizhov_Maxim_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(LoopPassBeginEnd_Chizhov_Maxim_FIIT3_MLIR)" %s | FileCheck %s

module {
  func.func private @trace_loop_iter_begin() -> ()
  func.func private @trace_loop_iter_end() -> ()

  // CHECK-LABEL: func.func @affine_loop
  func.func @affine_loop() {
    %c0 = arith.constant 0 : index
    %c10 = arith.constant 10 : index

    affine.for %i = %c0 to %c10 {
      // CHECK: call @trace_loop_iter_begin
      "test.op"() : () -> ()
      // CHECK: call @trace_loop_iter_end
    }
    return
  }
}

module {
  func.func private @trace_loop_iter_begin() -> ()
  func.func private @trace_loop_iter_end() -> ()

  // CHECK-LABEL: func.func @scf_loop
  func.func @scf_loop() {
    %c0 = arith.constant 0 : index
    %c1 = arith.constant 1 : index
    %c10 = arith.constant 10 : index

    scf.for %i = %c0 to %c10 step %c1 {
      // CHECK: call @trace_loop_iter_begin
      "test.op"() : () -> ()
      // CHECK: call @trace_loop_iter_end
    }

    return
  }
}

module {
  func.func private @trace_loop_iter_begin() -> ()
  func.func private @trace_loop_iter_end() -> ()

  // CHECK-LABEL: func.func @scf_while
  func.func @scf_while() {
    %c0 = arith.constant 0 : index
    %c1 = arith.constant 1 : index
    %c10 = arith.constant 10 : index

    %init = scf.while (%i = %c0) : (index) -> (index) {
      %cond = arith.cmpi "slt", %i, %c10 : index
      scf.condition(%cond) %i : index
    } do {
      ^bb0(%i_in: index):
        // CHECK: call @trace_loop_iter_begin
        "test.op"() : () -> ()
        %inc = arith.addi %i_in, %c1 : index
        // CHECK: call @trace_loop_iter_end
        scf.yield %inc : index
    }

    return
  }
}