// RUN: mlir-opt -debug -load-pass-plugin=%mlir_lib_dir/RemDecomposePass_Frolova_Elizaveta_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(RemDecomposePass_Frolova_Elizaveta_FIIT3_MLIR)" %s 2>&1 | FileCheck %s

module {
  func.func @test_div_by_zero_constant(%a: i32) -> i32 {
    %zero = arith.constant 0 : i32
    %2 = arith.remsi %a, %zero : i32
    return %2 : i32
  }
}

// CHECK: RemSIOp: division by zero (constant)
// CHECK: arith.remsi
// CHECK-NOT: arith.divsi
// CHECK-NOT: arith.mul
// CHECK-NOT: arith.sub

