// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/ExamplePass_Sedova_Olga_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(ExamplePass_Sedova_Olga_FIIT1_MLIR)" %s | FileCheck %s


func.func @trace_loop_iter_begin() {
  return
}

func.func @trace_loop_iter_end() {
  return
}

func.func @test_affine(%arg0: memref<10xf32>) {
  affine.for %i = 0 to 10 {
    // CHECK: func.call @trace_loop_iter_begin()
    %val = affine.load %arg0[%i] : memref<10xf32>
    // CHECK: func.call @trace_loop_iter_end()
  }
  return
}

func.func @test_scf_for(%arg0: memref<10xf32>) {
%c0 = arith.constant 0 : index
%c10 = arith.constant 10 : index
%c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c10 step %c1  {
    // CHECK: func.call @trace_loop_iter_begin()
    %val = memref.load %arg0[%i] : memref<10xf32>
    // CHECK: func.call @trace_loop_iter_end()
  }
  return
}

func.func @test_nested_loops(%arg0: memref<10xf32>) {
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index

  affine.for %i = 0 to 5 {
    // CHECK: func.call @trace_loop_iter_begin()
    affine.for %j = 0 to 10 {
      // CHECK: func.call @trace_loop_iter_begin()
      %val = memref.load %arg0[%j] : memref<10xf32>
      // CHECK: func.call @trace_loop_iter_end()
    }
    // CHECK: func.call @trace_loop_iter_end()
  }
  return
}


