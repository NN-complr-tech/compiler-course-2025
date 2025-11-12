// RUN: mlir-opt --pass-pipeline="builtin.module(func.func(call-counter-IvashchukVA-FIIT2))" %s | FileCheck %s

func.func @foo() {
  return
}

func.func @bar() {
  // CHECK: func.func @bar() attributes {call_count = 2}
  call @foo() : () -> ()
  call @foo() : () -> ()
  return
}

func.func @baz() {
  // CHECK: func.func @baz() attributes {call_count = 1}
  call @bar() : () -> ()
  return
}