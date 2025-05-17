// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/InsertLoopTracePass_Stroganov_Mikhail_FIIT2_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(InsertLoopTracePass_Stroganov_Mikhail_FIIT2_MLIR)" %s | FileCheck %s

// CHECK-LABEL: func.func @affine_sum()
// CHECK: affine.for
// CHECK: func.call @trace_loop_iter_begin() : () -> ()
// CHECK: arith.muli
// CHECK: arith.addi
// CHECK: func.call @trace_loop_iter_end() : () -> ()
// CHECK: affine.yield
func.func @affine_sum() -> i32 {
  %init = arith.constant 0 : i32
  %res = affine.for %i = 0 to 8 iter_args(%acc = %init) -> i32 {
    %i32 = arith.index_cast %i : index to i32
    %square = arith.muli %i32, %i32 : i32
    %new_acc = arith.addi %acc, %square : i32
    affine.yield %new_acc : i32
  }
  return %res : i32
}

// CHECK-LABEL: func.func @simple_loop()
// CHECK: scf.for
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: %0 = arith.addi
// CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
func.func @simple_loop() {
  %start = arith.constant 1 : index
  %end = arith.constant 5 : index
  %step = arith.constant 1 : index
  scf.for %i = %start to %end step %step {
    %x = arith.addi %i, %step : index
  }
  return
}

// CHECK-LABEL: func.func @parallel_loop()
// CHECK: scf.parallel
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: arith.addi
// CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
func.func @parallel_loop() {
  %start = arith.constant 0 : index
  %end = arith.constant 10 : index
  %step = arith.constant 1 : index
  %offset = arith.constant 5 : index
  scf.parallel (%loop_var) = (%start) to (%end) step (%step) {
    %tmp = arith.addi %loop_var, %offset : index
  }
  return
}

// CHECK-LABEL: func.func @while_loop(
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
func.func @while_loop(%arg0: index) {
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

// CHECK-LABEL: func.func @nested_loops()
// CHECK: scf.for
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: scf.for
// CHECK-NEXT: func.call @trace_loop_iter_begin() : () -> ()
// CHECK-NEXT: %{{.*}} = arith.addi
// CHECK-NEXT: func.call @trace_loop_iter_end() : () -> ()
// CHECK func.call @trace_loop_iter_end() : () -> ()

func.func @nested_loops() {
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index
  %c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c5 step %c1 {
    scf.for %j = %c0 to %c5 step %c1 {
      %sum = arith.addi %i, %j : index
    }
  }
  return
}

