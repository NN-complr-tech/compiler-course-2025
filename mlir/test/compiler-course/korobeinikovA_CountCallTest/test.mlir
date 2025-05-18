// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/FunctionCallTracker_Korobeinikov_Arseny_FIIT1_MLIR%shlibext \
// RUN:   --pass-pipeline="builtin.module(FunctionCallTracker_Korobeinikov_Arseny_FIIT1_MLIR)" %s \
// RUN: | FileCheck %s

module {
  func.func @helper() {
    func.return
  }

  func.func @worker() {
    func.return
  }

  func.func @main() {
    // CHECK: call @helper() {invoke_total = 2 : i64}
    call @helper() : () -> ()
    
    // CHECK: call @worker() {invoke_total = 1 : i64}
    call @worker() : () -> ()
    
    // CHECK: call @helper() {invoke_total = 2 : i64}
    call @helper() : () -> ()
    
    func.return
  }

  // CHECK-LABEL: func.func @unused()
  // CHECK-NOT: invoke_total
  func.func @unused() {
    func.return
  }
}