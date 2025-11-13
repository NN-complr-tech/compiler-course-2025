// RUN: mlir-opt --call-counter %s | FileCheck %s

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
// CHECK-SAME: {call_count = 3}