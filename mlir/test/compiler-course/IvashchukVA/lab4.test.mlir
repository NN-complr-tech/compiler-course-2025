// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/CallCounterPass_IvashchukVA_FIIT2_MLIR%shlibext --pass-pipeline="builtin.module(call-counter)" %s | FileCheck %s

// Тест 1: Базовый тест с несколькими вызовами
func.func @function1() {
  return
}

func.func @function2() {
  return
}

func.func @main() {
  call @function1() : () -> ()
  call @function1() : () -> ()
  call @function2() : () -> ()
  return
}

// CHECK: func.func @main() {
// CHECK: call @function1() {call_count = 2 : i64} : () -> ()
// CHECK: call @function1() {call_count = 2 : i64} : () -> ()
// CHECK: call @function2() {call_count = 1 : i64} : () -> ()

// Тест 2: Простая рекурсивная функция
func.func @simple_recursive(%arg0: i32) -> i32 {
  %c1 = arith.constant 1 : i32
  %cond = arith.cmpi eq, %arg0, %c1 : i32
  cf.cond_br %cond, ^bb1, ^bb2

^bb1:
  return %c1 : i32

^bb2:
  %minus = arith.subi %arg0, %c1 : i32
  %result = call @simple_recursive(%minus) : (i32) -> i32
  return %result : i32
}

// CHECK: func.func @simple_recursive(%{{.*}} : i32) -> i32 {
// CHECK: call @simple_recursive(%{{.*}}) {call_count = 1 : i64} : (i32) -> i32

// Тест 3: Вложенные вызовы
func.func @inner() {
  return
}

func.func @middle() {
  call @inner() : () -> ()
  return
}

func.func @outer() {
  call @middle() : () -> ()
  call @inner() : () -> ()
  return
}

// CHECK: func.func @outer() {
// CHECK: call @middle() {call_count = 1 : i64} : () -> ()
// CHECK: call @inner() {call_count = 2 : i64} : () -> ()

// Тест 4: Разное количество вызовов
func.func @called_once() {
  return
}

func.func @called_twice() {
  return
}

func.func @test_calls() {
  call @called_once() : () -> ()
  call @called_twice() : () -> ()
  call @called_twice() : () -> ()
  return
}

// CHECK: func.func @test_calls() {
// CHECK: call @called_once() {call_count = 1 : i64} : () -> ()
// CHECK: call @called_twice() {call_count = 2 : i64} : () -> ()
// CHECK: call @called_twice() {call_count = 2 : i64} : () -> ()