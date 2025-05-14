// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/rem_pass_Mironov_Arseniy_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(rem_pass_Mironov_Arseniy_FIIT1_MLIR)" %s | FileCheck %s -dump-input=always

// test.cpp
// unsigned test1(unsigned a, unsigned b) {
//   unsigned res = a % b;
//   return res;
// }
//
// signed test2(signed a, signed b) {
//   signed res = a % b;
//   return res;
// }
//
// long long test3(long long a, long long b){
//   long long res = a % b;
//   return res;
// }

// unsigned long long test4(unsigned long long a, unsigned long long b){
//   unsigned long long res = a % b;
//   return res;
// }

module {
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


  // CHECK-LABEL: func.func @test3
  // CHECK: %0 = arith.divsi %arg0, %arg1 : i64
  // CHECK-NEXT: %1 = arith.muli %0, %arg1 : i64
  // CHECK-NEXT: %2 = arith.subi %arg0, %1 : i64
  // CHECK-NEXT: return %2 : i64

  func.func @test3(%arg0: i64, %arg1: i64) -> i64 attributes {llvm.noundef} {
      %0 = arith.remsi %arg0, %arg1 : i64
      return %0 : i64
  }

  // CHECK-LABEL: func.func @test4
  // CHECK: %0 = arith.divui %arg0, %arg1 : i64
  // CHECK-NEXT: %1 = arith.muli %0, %arg1 : i64
  // CHECK-NEXT: %2 = arith.subi %arg0, %1 : i64
  // CHECK-NEXT: return %2 : i64

  func.func @test4(%arg0: i64, %arg1: i64) -> i64 attributes {llvm.noundef} {
      %0 = arith.remui %arg0, %arg1 : i64
      return %0 : i64
  }
}
