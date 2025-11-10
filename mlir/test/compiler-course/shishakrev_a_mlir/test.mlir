// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/TraceLoopPass_Shishkarev_Andrey_FIIT2_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(TraceLoopPass_KudryashovaIrina_FIIT3_MLIR)" %s | FileCheck %s

module {
  // CHECK: func.func private @trace_loop_iter_end()
  // CHECK-NEXT: func.func private @trace_loop_iter_begin()
  // CHECK-LABEL: func.func @kern()
  func.func @kern() {
    // CHECK: affine.for %arg0 = 0 to 4 {
    // CHECK-NEXT:   func.call @trace_loop_iter_begin() : () -> ()
    // CHECK-DAG:   %cst = arith.constant 0.000000e+00 : f32
    // CHECK-DAG:   %cst_0 = arith.constant 1.000000e+00 : f32
    // CHECK-NEXT:   %0 = arith.addf %cst, %cst_0 : f32
    // CHECK-NEXT:   func.call @trace_loop_iter_end() : () -> ()
    affine.for %i = 0 to 4 {
      %c0 = arith.constant 0.0 : f32
      %c1 = arith.constant 1.0 : f32
      %add = arith.addf %c0, %c1 : f32
      affine.yield
    }
    return
  }
  // CHECK-LABEL: func.func @work()
  func.func @work() {
    // CHECK-DAG: %c0_i32 = arith.constant 0 : i32
    // CHECK-DAG: %c0 = arith.constant 0 : index
    // CHECK-NEXT: scf.for %arg0 = %c0 to %c0 step %c0 {
    // CHECK-NEXT:   func.call @trace_loop_iter_begin() : () -> ()
    // CHECK-DAG:   %c1_i32 = arith.constant 1 : i32
    // CHECK-NEXT:   func.call @trace_loop_iter_end() : () -> ()
    %0 = arith.constant 0 : i32
    %c0 = arith.constant 0 : index
    scf.for %i = %c0 to %c0 step %c0 {
      %one = arith.constant 1 : i32
      scf.yield
    }
    return
  }
  // CHECK-LABEL: func.func @while_example(%arg0: index)
  func.func @while_example(%arg0: index) {
    // CHECK-DAG: %c1 = arith.constant 1 : index
    // CHECK-NEXT: %0 = scf.while (%arg1 = %arg0) : (index) -> index {
    // CHECK-DAG:   %c4 = arith.constant 4 : index
    // CHECK-NEXT:   %1 = arith.cmpi slt, %arg1, %c4 : index
    // CHECK-NEXT:   scf.condition(%1) %arg1 : index
    // CHECK-NEXT: } do {
    // CHECK-NEXT: ^bb0(%arg1: index):
    // CHECK-NEXT:   func.call @trace_loop_iter_begin() : () -> ()
    // CHECK-DAG:   %c7_i32 = arith.constant 7 : i32
    // CHECK-NEXT:   %1 = arith.addi %arg1, %c1 : index
    // CHECK-NEXT:   func.call @trace_loop_iter_end() : () -> ()
    // CHECK-NEXT:   scf.yield %1 : index
    // CHECK-NEXT: }
    %one   = arith.constant 1 : index
    %res = scf.while (%i = %arg0) : (index) -> (index) {
      %limit = arith.constant 4 : index
      %cond  = arith.cmpi slt, %i, %limit : index
      scf.condition(%cond) %i : index
    } do {
      ^bb0(%i_curr: index):
        %v   = arith.constant 7 : i32 
        %inc = arith.addi %i_curr, %one : index
        scf.yield %inc : index 
    }
    return
  }
}