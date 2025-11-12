// RUN: mlir-opt --pass-pipeline="builtin.module(CallCounterPass_IvashchukVA_FIIT2_MLIR)" %s | FileCheck %s

func.func @foo() {
  return
}

func.func @bar() {
  // CHECK: call_count = 2
  call @foo() : () -> ()
  call @foo() : () -> ()
  return
}