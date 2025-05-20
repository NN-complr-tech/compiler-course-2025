// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/LoopPassBeginEnd_Chizhov_Maxim_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(LoopPassBeginEnd_Chizhov_Maxim_FIIT3_MLIR)" %s | FileCheck %s



module {

  // CHECK-LABEL: func.func @loop_without_decls
  // CHECK-NEXT:   %c0 = arith.constant 0 : index
  // CHECK-NEXT:   %c10 = arith.constant 10 : index
  // CHECK-NEXT:   affine.for %{{.*}} = %c0 to %c10 {
  // CHECK-NEXT:     call @trace_loop_iter_begin() : () -> ()
  // CHECK-NEXT:     "test.op"() : () -> ()
  // CHECK-NEXT:     call @trace_loop_iter_end() : () -> ()

  func.func @loop_without_decls() {
    %c0 = arith.constant 0 : index
    %c10 = arith.constant 10 : index

    affine.for %i = %c0 to %c10 {
      "test.op"() : () -> ()
    }

    return
  }
}
