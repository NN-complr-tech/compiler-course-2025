// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/GenericCallCounter_Koshkin_Nikita_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(GenericCallCounter_Koshkin_Nikita_FIIT3_MLIR)" %s | FileCheck --implicit-check-not="call @unused() {func.call_count =" %s

module {
  func.func @foo() -> () {
    func.return
  }

  func.func @bar() -> () {
    func.return
  }

  func.func @baz() -> () {
    func.return
  }

  func.func @util() -> () {
    func.return
  }

  // CHECK-LABEL: func.func @main()
  // CHECK-NEXT: call @foo() {func.call_count = 3 : index} : () -> ()
  // CHECK-NEXT: call @foo() {func.call_count = 3 : index} : () -> ()
  // CHECK-NEXT: call @bar() {func.call_count = 2 : index} : () -> ()
  // CHECK-NEXT: call @baz() {func.call_count = 1 : index} : () -> ()
  func.func @main() -> () {
    call @foo() : () -> ()
    call @foo() : () -> ()
    call @bar() : () -> ()
    call @baz() : () -> ()
    func.return
  }

  // CHECK-LABEL: func.func @withNested()
  // CHECK-DAG: call @util() {func.call_count = 2 : index} : () -> ()
  // CHECK-DAG: call @util() {func.call_count = 2 : index} : () -> ()
  // CHECK-DAG: call @bar() {func.call_count = 2 : index} : () -> ()
  func.func @withNested() -> () {
    call @util() : () -> ()
    call @bar() : () -> ()
    call @util() : () -> ()
    func.return
  }

  // CHECK-LABEL: func.func @noCalls()
  // CHECK-NOT: call_count
  func.func @noCalls() -> () {
    func.return
  }

  // CHECK-LABEL: func.func @mixed()
  // CHECK: call @foo() {func.call_count = 3 : index} : () -> ()
  func.func @mixed() -> () {
    call @foo() : () -> ()
    func.return
  }
}
