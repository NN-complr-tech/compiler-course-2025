// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/rem_pass_Mironov_Arseniy_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(rem_pass_Mironov_Arseniy_FIIT1_MLIR)" %s | FileCheck %s -dump-input=always

// test.cpp
// unsigned test1(unsigned a, unsigned b) {
//   unsigned res = a % b;
//   return res;
// }

// signed test2(signed a, signed b) {
//   signed res = a % b;
//   return res;
// }


module attributes {dlti.dl_spec = #dlti.dl_spec<#dlti.dl_entry<f128, dense<128> : vector<2xi64>>, #dlti.dl_entry<f80, dense<128> : vector<2xi64>>, #dlti.dl_entry<i64, dense<64> : vector<2xi64>>, #dlti.dl_entry<!llvm.ptr, dense<64> : vector<4xi64>>, #dlti.dl_entry<i1, dense<8> : vector<2xi64>>, #dlti.dl_entry<i32, dense<32> : vector<2xi64>>, #dlti.dl_entry<i16, dense<16> : vector<2xi64>>, #dlti.dl_entry<i8, dense<8> : vector<2xi64>>, #dlti.dl_entry<!llvm.ptr<271>, dense<32> : vector<4xi64>>, #dlti.dl_entry<!llvm.ptr<272>, dense<64> : vector<4xi64>>, #dlti.dl_entry<!llvm.ptr<270>, dense<32> : vector<4xi64>>, #dlti.dl_entry<f64, dense<64> : vector<2xi64>>, #dlti.dl_entry<f16, dense<16> : vector<2xi64>>, #dlti.dl_entry<"dlti.stack_alignment", 128 : i64>, #dlti.dl_entry<"dlti.endianness", "little">>} {
  
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
}