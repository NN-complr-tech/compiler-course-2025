// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/RemsiuiPass_Khovansky_Dmitry_FIIT2_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(RemsiuiPass_Khovansky_Dmitry_FIIT2_MLIR)" %s | FileCheck %s

func.func @test_remsi(%a: i32, %b: i32) -> i32 {
  %rem = arith.remsi %a, %b : i32
  return %rem : i32
}

// CHECK-LABEL: func.func @test_remsi
// CHECK-NOT: arith.remsi
// CHECK-NEXT: %[[DIV:.*]] = arith.divsi %{{.*}}, %{{.*}} : i32
// CHECK-NEXT: %[[MUL:.*]] = arith.muli %[[DIV]], %{{.*}} : i32
// CHECK-NEXT: %[[RES:.*]] = arith.subi %{{.*}}, %[[MUL]] : i32
// CHECK-NEXT: return %[[RES]] : i32


func.func @test_remui(%x: i32, %y: i32) -> i32 {
  %rem = arith.remui %x, %y : i32
  return %rem : i32
}

// CHECK-LABEL: func.func @test_remui
// CHECK-NOT: arith.remui
// CHECK-NEXT: %[[DIV:.*]] = arith.divui %{{.*}}, %{{.*}} : i32
// CHECK-NEXT: %[[MUL:.*]] = arith.muli %[[DIV]], %{{.*}} : i32
// CHECK-NEXT: %[[RES:.*]] = arith.subi %{{.*}}, %[[MUL]] : i32
// CHECK-NEXT: return %[[RES]] : i32

func.func @test_multiple(%a: i32, %b: i32) -> (i32, i32) {
  %r1 = arith.remsi %a, %b : i32
  %r2 = arith.remui %a, %b : i32
  return %r1, %r2 : i32, i32
}

// CHECK-LABEL: func.func @test_multiple
// CHECK-NOT: arith.remsi
// CHECK-NEXT: %[[DIV1:.*]] = arith.divsi %{{.*}}, %{{.*}} : i32
// CHECK-NEXT: %[[MUL1:.*]] = arith.muli %[[DIV1]], %{{.*}} : i32
// CHECK-NEXT: %[[RES1:.*]] = arith.subi %{{.*}}, %[[MUL1]] : i32
// CHECK-NOT: arith.remui
// CHECK-NEXT: %[[DIV2:.*]] = arith.divui %{{.*}}, %{{.*}} : i32
// CHECK-NEXT: %[[MUL2:.*]] = arith.muli %[[DIV2]], %{{.*}} : i32
// CHECK-NEXT: %[[RES2:.*]] = arith.subi %{{.*}}, %[[MUL2]] : i32
// CHECK-NEXT: return %[[RES1]], %[[RES2]] : i32, i32


func.func @test_types(%a: i64, %b: i64) -> i64 {
  %r = arith.remsi %a, %b : i64
  return %r : i64
}

// CHECK-LABEL: func.func @test_types
// CHECK-NOT: arith.remsi
// CHECK-NEXT: %[[DIV:.*]] = arith.divsi %{{.*}}, %{{.*}} : i64
// CHECK-NEXT: %[[MUL:.*]] = arith.muli %[[DIV]], %{{.*}} : i64
// CHECK-NEXT: %[[RES:.*]] = arith.subi %{{.*}}, %[[MUL]] : i64
// CHECK-NEXT: return %[[RES]] : i64


func.func @test_ignore(%a: i32, %b: i32) -> i32 {
  %r = arith.addi %a, %b : i32
  return %r : i32
}

// CHECK-LABEL: func.func @test_ignore
// CHECK-NEXT: arith.addi
