// RUN: mlir-opt --pass-pipeline="builtin.module(func::CallCounterPass_IvashchukVA_FIIT2_MLIR)" %s | FileCheck %s

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
// CHECK: call @foo() {call_count = 2}
// CHECK: call @foo() {call_count = 2} 
// CHECK: call @bar() {call_count = 1}
