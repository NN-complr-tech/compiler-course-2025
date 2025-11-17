// RUN: mlir-opt --pass-pipeline="builtin.module(func.func(call-counter))" %s | FileCheck %s

func.func @foo() {
  func.return
}

func.func @bar() {
  func.return
}

func.func @main() {
  func.call @foo() : () -> ()
  func.call @foo() : () -> ()
  func.call @bar() : () -> ()
  func.return
}

// CHECK: func.func @main() attributes {call_count = 3}