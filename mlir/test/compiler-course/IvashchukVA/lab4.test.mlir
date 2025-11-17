// RUN: mlir-opt --pass-pipeline="builtin.module(func.func(call-counter))" %s | FileCheck %s

func.func @foo() {
  return
}

func.func @bar() {
  return
}

func.func @main() {
  func.call @foo()
  func.call @foo()
  func.call @bar()
  return
}

// CHECK: func.func @main() attributes {call_count = 3}