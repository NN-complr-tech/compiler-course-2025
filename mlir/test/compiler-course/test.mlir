// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/rem_pass_Mamaeva_Olga_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(rem_pass_Mamaeva_Olga_FIIT3_MLIR)" %s | FileCheck %s -dump-input=always

module attributes {dlti.dl_spec = #dlti.dl_spec<...>} {

  // CHECK-LABEL: func.func @test1
  // CHECK: %0 = arith.divui %arg0, %arg1 : i32
  // CHECK-NEXT: %1 = arith.muli %0, %arg1 : i32
  // CHECK-NEXT: %2 = arith.subi %arg0, %1 : i32
  // CHECK-NEXT: return %2 : i32

  func.func @test1(%arg0: i32, %arg1: i32) -> i32 attributes {llvm.noundef} {
    %0 = arith.remui %arg0, %arg1 : i32
    return %0 : i32
  }

  // CHECK-LABEL: func.func @test2
  // CHECK: %0 = arith.divsi %arg0, %arg1 : i32
  // CHECK-NEXT: %1 = arith.muli %0, %arg1 : i32
  // CHECK-NEXT: %2 = arith.subi %arg0, %1 : i32
  // CHECK-NEXT: return %2 : i32

  func.func @test2(%arg0: i32, %arg1: i32) -> i32 attributes {llvm.noundef} {
    %0 = arith.remsi %arg0, %arg1 : i32
    return %0 : i32
  }
