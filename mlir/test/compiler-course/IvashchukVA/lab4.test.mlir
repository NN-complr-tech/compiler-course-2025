// RUN: mlir-opt --pass-pipeline="builtin.module(call-counter)" %s | FileCheck %s

func.func @foo() {
  return
}

func.func @bar() {
  // CHECK: func.func @bar()
  // CHECK-SAME: attributes {call_count = 2}
  call @foo() : () -> ()
  call @foo() : () -> ()
  return
}

func.func @baz() {
  // CHECK: func.func @baz()
  // CHECK-SAME: attributes {call_count = 1}
  call @bar() : () -> ()
  return
}