// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/ExamplePass_Kozlova_Ekaterina_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(ExamplePass_Kozlova_Ekaterina_FIIT3_MLIR)" %s | FileCheck %s

// CHECK: module {

// CHECK-LABEL:  func.func @no_iterations() {
// CHECK-NOT:      my_loop_depths
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @no_iterations() {
  return
}

// CHECK-LABEL:  func.func @test1() attributes {my_loop_depths = [1]} {
// CHECK-NEXT:     %c0 = arith.constant 0 : index
// CHECK-NEXT:     %c7 = arith.constant 7 : index
// CHECK-NEXT:     %c1 = arith.constant 1 : index
// CHECK-NEXT:     scf.for %arg0 = %c0 to %c7 step %c1 {
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @test1() {
  %c0 = arith.constant 0 : index
  %c7 = arith.constant 7 : index
  %c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c7 step %c1 {
  }
  return
}

// CHECK-LABEL:  func.func @test2() attributes {my_loop_depths = [2]} {
// CHECK-NEXT:     %c0 = arith.constant 0 : index
// CHECK-NEXT:     %c7 = arith.constant 7 : index
// CHECK-NEXT:     %c1 = arith.constant 1 : index
// CHECK-NEXT:     %true = arith.constant true
// CHECK-NEXT:     scf.for %arg0 = %c0 to %c7 step %c1 {
// CHECK-NEXT:       scf.if %true {
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @test2() {
  %c0 = arith.constant 0 : index
  %c7 = arith.constant 7 : index
  %c1 = arith.constant 1 : index
  %cond = arith.constant true
  scf.for %i = %c0 to %c7 step %c1 {
    scf.if %cond {
    }
  }
  return
}

// CHECK-LABEL:  func.func @test3() attributes {my_loop_depths = [3]} {
// CHECK-NEXT:     %c0 = arith.constant 0 : index
// CHECK-NEXT:     %c7 = arith.constant 7 : index
// CHECK-NEXT:     %c3 = arith.constant 3 : index
// CHECK-NEXT:     %true = arith.constant true
// CHECK-NEXT:     scf.for %arg0 = %c0 to %c7 step %c3 {
// CHECK-NEXT:       scf.if %true {
// CHECK-NEXT:         scf.if %true {
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @test3() {
  %c0 = arith.constant 0 : index
  %c7 = arith.constant 7 : index
  %c3 = arith.constant 3 : index
  %cond = arith.constant true
  scf.for %i = %c0 to %c7 step %c3 {
    scf.if %cond {
      scf.if %cond {
      }
    }
  }
  return
}

// CHECK-LABEL:  func.func @test4() attributes {my_loop_depths = [2]} {
// CHECK-NEXT:     %c0 = arith.constant 0 : index
// CHECK-NEXT:     %c20 = arith.constant 20 : index
// CHECK-NEXT:     %c7 = arith.constant 7 : index
// CHECK-NEXT:     %c1 = arith.constant 1 : index
// CHECK-NEXT:     scf.for %arg0 = %c0 to %c20 step %c1 {
// CHECK-NEXT:       scf.for %arg1 = %c0 to %c7 step %c1 {
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @test4() {
  %c0 = arith.constant 0 : index
  %c20 = arith.constant 20 : index
  %c7 = arith.constant 7 : index
  %c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c20 step %c1 {
    scf.for %j = %c0 to %c7 step %c1 {
    }
  }
  return
}

// CHECK-LABEL:  func.func @test5() attributes {my_loop_depths = [3]} {
// CHECK-NEXT:     %c0 = arith.constant 0 : index
// CHECK-NEXT:     %c7 = arith.constant 7 : index
// CHECK-NEXT:     %c1 = arith.constant 1 : index
// CHECK-NEXT:     %true = arith.constant true
// CHECK-NEXT:     scf.if %true {
// CHECK-NEXT:       scf.for %arg0 = %c0 to %c7 step %c1 {
// CHECK-NEXT:         scf.if %true {
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }

func.func @test5() {
  %c0 = arith.constant 0 : index
  %c7 = arith.constant 7 : index
  %c1 = arith.constant 1 : index
  %cond = arith.constant true
  scf.if %cond {
    scf.for %i = %c0 to %c7 step %c1 {
      scf.if %cond {
      }
    }
  }
  return
}


// CHECK-LABEL:  func.func @test6() attributes {my_loop_depths = [1]} {
// CHECK-NEXT:     affine.for %arg0 = 0 to 10 {
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @test6() {
  affine.for %i = 0 to 10 {
  }
  return
}