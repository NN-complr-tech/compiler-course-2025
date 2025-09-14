// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/FunctionCallTracker_Korobeinikov_Arseny_FIIT1_MLIR%shlibext \
// RUN:   --pass-pipeline="builtin.module(FunctionCallTracker_Korobeinikov_Arseny_FIIT1_MLIR)" %s \
// RUN: | FileCheck %s

module {
  "llvm.func"() ({
    "llvm.return"() : () -> ()
  }) {
    sym_name = "helper",
    function_type = !llvm.func<void ()>,
    linkage = #llvm.linkage<external>
  } : () -> ()

  func.func @worker() {
    func.return
  }

  func.func @main() {
    // CHECK: llvm.call @helper() {invoke_total = 2 : i64}
    "llvm.call"() {callee = @helper} : () -> ()

    // CHECK: call @worker() {invoke_total = 1 : i64}
    call @worker() : () -> ()

    // CHECK: llvm.call @helper() {invoke_total = 2 : i64}
    "llvm.call"() {callee = @helper} : () -> ()

    func.return
  }

  // CHECK-LABEL: func.func @unused()
  // CHECK-NOT: invoke_total
  func.func @unused() {
    func.return
  }
}