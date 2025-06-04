// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/FunctionCallAnalyzer_Savchenko_Maxim_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(FunctionCallAnalyzer_Savchenko_Maxim_FIIT1_MLIR)" %s \
// RUN: | FileCheck %s

module {
  func.func @taskA() -> () {
    func.return
  }

  func.func @taskB() -> () {
    func.return
  }

  func.func @taskC() -> () {
    func.return
  }

  // CHECK-LABEL: func.func @entry()
  func.func @entry() {
    
    // CHECK: call @taskA() {call_count = 3 : i64}
    call @taskA() : () -> ()
    
    // CHECK: call @taskA() {call_count = 3 : i64}
    call @taskA() : () -> ()
    
    // CHECK: call @dispatcher() {call_count = 1 : i64}
    call @dispatcher() : () -> ()
    
    func.return
  }

  // CHECK-LABEL: func.func @dispatcher()
  func.func @dispatcher() {
    
    // CHECK: call @taskA() {call_count = 3 : i64}
    call @taskA() : () -> ()
    
    // CHECK: call @taskB() {call_count = 1 : i64}
    call @taskB() : () -> ()
    
    func.return
  }

  // CHECK-LABEL: func.func @notUsed()
  // CHECK-NOT: call_count
  func.func @notUsed() {
    func.return
  }

}