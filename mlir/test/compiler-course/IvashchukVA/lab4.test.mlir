// RUN: mlir-opt --generic-call-counter-IvashchukVA %s | FileCheck %s

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

// CHECK: func.func @main()
// CHECK: call @foo() {func.call_count = 2}
// CHECK: call @foo() {func.call_count = 2}
// CHECK: call @bar() {func.call_count = 1}