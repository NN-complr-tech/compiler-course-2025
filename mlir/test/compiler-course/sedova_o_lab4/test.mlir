// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/ExamplePass_Sedova_Olga_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(ExamplePass_Sedova_Olga_FIIT1_MLIR)" %s | FileCheck %s


func.func @trace_loop_iter_begin() {
  return
}

func.func @trace_loop_iter_end() {
  return
}

func.func @test_affine(%arg0: memref<10xf32>) {
  // CHECK: affine.for {{%[a-zA-Z0-9]+}} = 0 to 10
  // CHECK-NEXT: func.call @trace_loop_iter_begin()
  affine.for %arg1 = 0 to 10 {
    %0 = affine.load %arg0[%arg1] : memref<10xf32>
    // CHECK: func.call @trace_loop_iter_end()
    // CHECK-NEXT: }
  }
  return
}

func.func @test_scf_for(%arg0: memref<10xf32>) {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  // CHECK: scf.for {{%[a-zA-Z0-9]+}} = %c0 to %c10 step %c1
  // CHECK-NEXT: func.call @trace_loop_iter_begin()
  scf.for %i = %c0 to %c10 step %c1  {
    %0 = memref.load %arg0[%i] : memref<10xf32>
    // CHECK: func.call @trace_loop_iter_end()
    // CHECK-NEXT: }
  }
  return
}

func.func @test_nested_loops(%arg0: memref<10xf32>) {
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index

  // CHECK: affine.for {{%[a-zA-Z0-9]+}} = 0 to 5
  // CHECK-NEXT: func.call @trace_loop_iter_begin()
  affine.for %i = 0 to 5 {
    // CHECK: affine.for {{%[a-zA-Z0-9]+}} = 0 to 10
    // CHECK-NEXT: func.call @trace_loop_iter_begin()
    affine.for %j = 0 to 10 {
      %0 = memref.load %arg0[%j] : memref<10xf32>
      // CHECK: func.call @trace_loop_iter_end()
      // CHECK-NEXT: }
    }
    // CHECK: func.call @trace_loop_iter_end()
    // CHECK-NEXT: }
  }
  return
}

func.func @test_while_loop(%arg0: memref<10xf32>) {
  %c0 = arith.constant 0 : index
  %c0_0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index

  // CHECK: %{{.*}} = scf.while {{.*}} : (index, index) -> (index, index) {
  // CHECK-NEXT: %{{.*}} = arith.cmpi slt, %{{.*}}, %{{.*}} : index
  // CHECK-NEXT: scf.condition(%{{.*}}) %{{.*}}, %{{.*}} : index, index
  // CHECK-NEXT: } do {
  // CHECK-NEXT: ^bb0(%{{.*}}: index, %{{.*}}: index):
  // CHECK-NEXT: func.call @trace_loop_iter_begin()
  // CHECK-NEXT: %{{.*}} = arith.addi %{{.*}}, %{{.*}} : index
  // CHECK-NEXT: %{{.*}} = arith.constant 1 : index
  // CHECK-NEXT: %{{.*}} = arith.addi %{{.*}}, %{{.*}} : index
  // CHECK-NEXT: func.call @trace_loop_iter_end()
  scf.while (%iter1 = %c0, %iter2 = %c0_0) : (index, index) -> (index, index) {
    %cond = arith.cmpi slt, %iter2, %c10 : index
    scf.condition(%cond) %iter1, %iter2 : index, index
  } do {
  ^bb0(%arg1: index, %arg2: index):
      %arg = arith.addi %arg1, %arg2 : index
      %c1 = arith.constant 1 : index
      %new_val = arith.addi %arg2, %c1 : index
    scf.yield %arg, %new_val : index, index
  }
  return
}
