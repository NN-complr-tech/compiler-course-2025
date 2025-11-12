// RUN: mlir-opt --pass-pipeline="builtin.module(call-counter)" %s | FileCheck %s

func.func @foo() {
  return
}

func.func @bar() {
  call @foo() : () -> ()
  call @foo() : () -> ()
  return
}
// CHECK: func.func @bar() {{.*}} {call_count = 2}

func.func @baz() {
  call @bar() : () -> ()
  return
}
// CHECK: func.func @baz() {{.*}} {call_count = 1}