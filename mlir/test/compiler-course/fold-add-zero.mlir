// RUN: mlir-opt %s -cc-fold-add-zero | FileCheck %s
module {
  func.func @foo(%x: i32) -> i32 {
    %c0 = arith.constant 0 : i32
    %r  = arith.addi %x, %c0 : i32
    return %r : i32
  }
}

// CHECK: func.func @foo(%arg0: i32) -> i32
// CHECK-NOT: arith.addi
// CHECK: return %arg0 : i32
