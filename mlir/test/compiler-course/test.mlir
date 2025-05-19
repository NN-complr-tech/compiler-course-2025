// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/rem_pass_Mamaeva_Olga_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(rem_pass_Mamaeva_Olga_FIIT3_MLIR)" %s | FileCheck %s

// ==================================================
// Тест для знакового остатка (remsi)
// ==================================================
// CHECK-LABEL: func.func @test_remsi
func.func @test_remsi(%arg0: i32, %arg1: i32) -> i32 {
  // Проверяем что остаток заменяется на: a - (a / b) * b
  // CHECK: %[[DIV:.*]] = arith.divsi %arg1, %arg0 : i32
  // CHECK: %[[MUL:.*]] = arith.muli %[[DIV]], %arg0 : i32
  // CHECK: %[[RES:.*]] = arith.subi %arg1, %[[MUL]] : i32
  %rem = arith.remsi %arg1, %arg0 : i32
  
  // CHECK: return %[[RES]]
  return %rem : i32
}

// ==================================================
// Тест для беззнакового остатка (remui)
// ==================================================
// CHECK-LABEL: func.func @test_remui
func.func @test_remui(%arg0: i32, %arg1: i32) -> i32 {
  // Для беззнакового используем divui
  // CHECK: %[[DIV:.*]] = arith.divui %arg1, %arg0 : i32
  // CHECK: %[[MUL:.*]] = arith.muli %[[DIV]], %arg0 : i32
  // CHECK: %[[RES:.*]] = arith.subi %arg1, %[[MUL]] : i32
  %rem = arith.remui %arg1, %arg0 : i32
  
  // CHECK: return %[[RES]]
  return %rem : i32
}

// ==================================================
// Тест для 64-битных целых
// ==================================================
// CHECK-LABEL: func.func @test_i64
func.func @test_i64(%arg0: i64, %arg1: i64) -> i64 {
  // Проверяем работу с другим типом данных
  // CHECK: arith.divsi
  // CHECK: arith.muli
  // CHECK: arith.subi
  %rem = arith.remsi %arg1, %arg0 : i64
  
  // CHECK: return
  return %rem : i64
}

// ==================================================
// Тест с несколькими операциями остатка
// ==================================================
// CHECK-LABEL: func.func @test_multiple
func.func @test_multiple(%arg0: i32, %arg1: i32) -> (i32, i32) {
  // Проверяем замену нескольких операций в одной функции
  // CHECK: arith.divsi
  // CHECK: arith.muli
  // CHECK: arith.subi
  %rem1 = arith.remsi %arg1, %arg0 : i32
  
  // CHECK: arith.divsi
  // CHECK: arith.muli
  // CHECK: arith.subi
  %rem2 = arith.remsi %arg0, %arg1 : i32
  
  // CHECK: return
  return %rem1, %rem2 : i32, i32
}

// ==================================================
// Тест с константными значениями
// ==================================================
// CHECK-LABEL: func.func @test_constants
func.func @test_constants() -> i32 {
  %c42 = arith.constant 42 : i32
  %c5 = arith.constant 5 : i32
  
  // Проверяем работу с константами
  // CHECK: arith.divsi
  // CHECK: arith.muli
  // CHECK: arith.subi
  %rem = arith.remsi %c42, %c5 : i32
  
  // CHECK: return
  return %rem : i32
}
