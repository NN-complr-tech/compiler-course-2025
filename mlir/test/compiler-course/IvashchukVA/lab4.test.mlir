// RUN: mlir-opt --call-counter-IvashchukVA-FIIT2 %s | FileCheck %s

func.func @foo() {
  return
}

func.func @bar() {
  // CHECK: call_count = 2
  call @foo() : () -> ()
  call @foo() : () -> ()
  return
}

func.func @baz() {
  // CHECK: call_count = 1
  call @bar() : () -> ()
  return
}