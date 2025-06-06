// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/MaxLoopRegionDepthPass_Dormidontov_Egor_FIIT2_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(MaxLoopRegionDepthPass_Dormidontov_Egor_FIIT2_MLIR)" %s | FileCheck %s

// ###Простая функция без циклов/if
// CHECK-LABEL: func.func @no_loops
// CHECK: attributes {max_loop_region_depth = 0 : i32}
func.func @no_loops() -> () {
  %c0 = arith.constant 0 : index
  return
}

// ###Одна петля, глубина = 1
// CHECK-LABEL: func.func @simple_loop
// CHECK: attributes {max_loop_region_depth = 1 : i32}
func.func @simple_loop() -> () {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %c10 = arith.constant 10 : index
  scf.for %i = %c0 to %c10 step %c1 {
    // level 1
  }
  return
}

// ###Вложенный цикл, глубина = 2
// CHECK-LABEL: func.func @nested_loops
// CHECK: attributes {max_loop_region_depth = 2 : i32}
func.func @nested_loops() -> () {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %c10 = arith.constant 10 : index
  scf.for %i = %c0 to %c10 step %c1 {
    scf.for %j = %c0 to %c10 step %c1 {
      // level 2
    }
  }
  return
}

// ###Цикл с if внутри и if во вложении, глубина = 3
// CHECK-LABEL: func.func @deep_nest
// CHECK: attributes {max_loop_region_depth = 3 : i32}
func.func @deep_nest(%arg0: i1) -> () {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %c10 = arith.constant 10 : index
  scf.for %i = %c0 to %c10 step %c1 {
    scf.if %arg0 {
      scf.if %arg0 {
      }
    }
  }
  return
}
