// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/mlir_Shurigin_S_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(my-loop-depth-pass)" %s | FileCheck %s

// CHECK: module {

// CHECK-LABEL:  func.func @no_loops() {
// CHECK-NOT:      my_loop_depths
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @no_loops() {
  return
}

// CHECK-LABEL:  func.func @simple_scf_for() attributes {my_loop_depths = [1]} {
// CHECK-NEXT:     %c0 = arith.constant 0 : index
// CHECK-NEXT:     %c10 = arith.constant 10 : index
// CHECK-NEXT:     %c1 = arith.constant 1 : index
// CHECK-NEXT:     scf.for %arg0 = %c0 to %c10 step %c1 {
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @simple_scf_for() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c10 step %c1 {
  }
  return
}

// CHECK-LABEL:  func.func @simple_affine_for() attributes {my_loop_depths = [1]} {
// CHECK-NEXT:     affine.for %arg0 = 0 to 10 {
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @simple_affine_for() {
  affine.for %i = 0 to 10 {
  }
  return
}

// CHECK-LABEL:  func.func @scf_for_with_if() attributes {my_loop_depths = [2]} {
// CHECK-NEXT:     %c0 = arith.constant 0 : index
// CHECK-NEXT:     %c10 = arith.constant 10 : index
// CHECK-NEXT:     %c1 = arith.constant 1 : index
// CHECK-NEXT:     %true = arith.constant true
// CHECK-NEXT:     scf.for %arg0 = %c0 to %c10 step %c1 {
// CHECK-NEXT:       scf.if %true {
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @scf_for_with_if() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  %cond = arith.constant true
  scf.for %i = %c0 to %c10 step %c1 {
    scf.if %cond {
    }
  }
  return
}

// CHECK-LABEL:  func.func @nested_scf_for() attributes {my_loop_depths = [1, 2]} {
// CHECK-NEXT:     %c0 = arith.constant 0 : index
// CHECK-NEXT:     %c10 = arith.constant 10 : index
// CHECK-NEXT:     %c5 = arith.constant 5 : index
// CHECK-NEXT:     %c1 = arith.constant 1 : index
// CHECK-NEXT:     scf.for %arg0 = %c0 to %c10 step %c1 {
// CHECK-NEXT:       scf.for %arg1 = %c0 to %c5 step %c1 {
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @nested_scf_for() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c5 = arith.constant 5 : index
  %c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c10 step %c1 {
    scf.for %j = %c0 to %c5 step %c1 {
    }
  }
  return
}

// CHECK-LABEL:  func.func @deeply_nested_if_in_for() attributes {my_loop_depths = [4]} {
// CHECK-NEXT:     %c0 = arith.constant 0 : index
// CHECK-NEXT:     %c10 = arith.constant 10 : index
// CHECK-NEXT:     %c1 = arith.constant 1 : index
// CHECK-NEXT:     %true = arith.constant true
// CHECK-NEXT:     scf.for %arg0 = %c0 to %c10 step %c1 {
// CHECK-NEXT:       scf.if %true {
// CHECK-NEXT:         scf.if %true {
// CHECK-NEXT:           scf.if %true {
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @deeply_nested_if_in_for() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  %cond = arith.constant true
  scf.for %i = %c0 to %c10 step %c1 {
    scf.if %cond {
      scf.if %cond {
        scf.if %cond {
        }
      }
    }
  }
  return
}

// CHECK-LABEL:  func.func @two_independent_loops() attributes {my_loop_depths = [2, 1]} {
// CHECK-NEXT:     %c0 = arith.constant 0 : index
// CHECK-NEXT:     %c10 = arith.constant 10 : index
// CHECK-NEXT:     %c1 = arith.constant 1 : index
// CHECK-NEXT:     %true = arith.constant true
// CHECK-NEXT:     scf.for %arg0 = %c0 to %c10 step %c1 {
// CHECK-NEXT:       scf.if %true {
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:     affine.for %arg0 = 0 to 5 {
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @two_independent_loops() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  %cond = arith.constant true
  scf.for %i = %c0 to %c10 step %c1 {
    scf.if %cond {
    }
  }
  affine.for %j = 0 to 5 {
  }
  return
}

// CHECK-LABEL:  func.func @loop_if_loop() attributes {my_loop_depths = [1, 3]} {
// CHECK-NEXT:     %c0 = arith.constant 0 : index
// CHECK-NEXT:     %c10 = arith.constant 10 : index
// CHECK-NEXT:     %c1 = arith.constant 1 : index
// CHECK-NEXT:     %true = arith.constant true
// CHECK-NEXT:     scf.for %arg0 = %c0 to %c10 step %c1 {
// CHECK-NEXT:       scf.if %true {
// CHECK-NEXT:         affine.for %arg1 = 0 to 5 {
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @loop_if_loop() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  %cond = arith.constant true
  scf.for %i = %c0 to %c10 step %c1 {
    scf.if %cond {
      affine.for %j = 0 to 5 {
      }
    }
  }
  return
}

// CHECK-LABEL:  func.func @another_func_no_loops() {
// CHECK-NOT:      my_loop_depths
// CHECK-NEXT:     return
// CHECK-NEXT:   }

// CHECK-LABEL: func.func @another_func_with_loop() attributes {my_loop_depths = [1]} {
// CHECK-NOT:      my_loop_depths 
// CHECK-NEXT:     affine.for %arg0 = 0 to 1 {
// CHECK-NEXT:     }
// CHECK-NEXT:     return
// CHECK-NEXT:   }
// CHECK-NEXT: }
module {
  func.func @another_func_no_loops() {
    return
  }
  func.func @another_func_with_loop() {
    affine.for %i = 0 to 1 {
    }
    return
  }
}