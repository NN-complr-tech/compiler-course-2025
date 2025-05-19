// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/RemDecomposePass_Frolova_Elizaveta_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(RemDecomposePass_Frolova_Elizaveta_FIIT3_MLIR)" %s | FileCheck %s


module {
// Test 1: Signed remainder, renamed
  // CHECK-LABEL: func.func @test_signed_basic
  // CHECK-NEXT:%0 = arith.divsi %arg0, %arg1 : i32
  // CHECK-NEXT:%1 = arith.muli %0, %arg1 : i32
  // CHECK-NEXT:%2 = arith.subi %arg0, %1 : i32
  // CHECK-NEXT:return %2 : i32
  
  func.func @test_signed_basic(%a: i32, %b: i32) -> i32 {
    %0 = arith.remsi %a, %b : i32
    return %0 : i32
  }

  // Test 2: Unsigned remainder, renamed
  // CHECK-LABEL: func.func @test_unsigned_basic
  // CHECK-NEXT:%0 = arith.divui %arg0, %arg1 : i32
  // CHECK-NEXT:%1 = arith.muli %0, %arg1 : i32
  // CHECK-NEXT:%2 = arith.subi %arg0, %1 : i32
  // CHECK-NEXT:return %2 : i32

  func.func @test_unsigned_basic(%x: i32, %y: i32) -> i32 {
    %1 = arith.remui %x, %y : i32
    return %1 : i32
  }

  // Test 3: Signed remainder with constant zero divisor — should not be rewritten
  // CHECK-LABEL: func.func @test_div_by_zero_constant
  // CHECK-NEXT:%c0_i32 = arith.constant 0 : i32
  // CHECK-NEXT:%0 = arith.remsi %arg0, %c0_i32 : i32
  // CHECK-NEXT:return %0 : i32
  
  func.func @test_div_by_zero_constant(%a: i32) -> i32 {
    %zero = arith.constant 0 : i32
    %2 = arith.remsi %a, %zero : i32
    return %2 : i32
  }

  // Test 4: Signed remainder on i64 type
  // CHECK-LABEL: func.func @test_signed_i64
  // CHECK-NEXT:%0 = arith.divsi %arg0, %arg1 : i64
  // CHECK-NEXT:%1 = arith.muli %0, %arg1 : i64
  // CHECK-NEXT:%2 = arith.subi %arg0, %1 : i64
  // CHECK-NEXT:return %2 : i64
  
  func.func @test_signed_i64(%a: i64, %b: i64) -> i64 {
    %0 = arith.remsi %a, %b : i64
    return %0 : i64
  }

  // Test 5: Mixed signed and unsigned remainders
  // CHECK-LABEL: func.func @test_signed_and_unsigned_mixed
  // CHECK-NEXT:%0 = arith.divsi %arg0, %arg1 : i32
  // CHECK-NEXT:%1 = arith.muli %0, %arg1 : i32
  // CHECK-NEXT:%2 = arith.subi %arg0, %1 : i32
  // CHECK-NEXT:%3 = arith.divui %arg2, %arg3 : i32
  // CHECK-NEXT:%4 = arith.muli %3, %arg3 : i32
  // CHECK-NEXT:%5 = arith.subi %arg2, %4 : i32
  // CHECK-NEXT:%6 = arith.addi %2, %5 : i32
  // CHECK-NEXT:return %6 : i32

  func.func @test_signed_and_unsigned_mixed(%a: i32, %b: i32, %x: i32, %y: i32) -> i32 {
    %0 = arith.remsi %a, %b : i32
    %1 = arith.remui %x, %y : i32
    %2 = arith.addi %0, %1 : i32
    return %2 : i32
  }
}