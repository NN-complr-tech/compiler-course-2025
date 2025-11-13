// RUN: mlir-opt --pass-pipeline="builtin.module(call-counter)" %s | FileCheck %s

func.func @foo() {
  return
}

func.func @bar() {
  return
}

func.func @main() {
  call @foo()
  call @foo()
  call @bar()
  return
}

// CHECK: func.func @main() attributes {call_count = 3}