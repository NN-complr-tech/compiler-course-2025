// RUN: mlir-opt --pass-pipeline="builtin.module(call-counter)" %s | FileCheck %s

func.func @test() {
  call @helper() : () -> ()
  call @helper() : () -> ()
  return
}
// CHECK: call_count = 2

func.func @helper() {
  return
}