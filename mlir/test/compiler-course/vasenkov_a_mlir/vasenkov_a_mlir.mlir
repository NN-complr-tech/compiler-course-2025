// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/ExamplePass_VasenkovAndrey_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(ExamplePass_VasenkovAndrey_FIIT1_MLIR)" %s | FileCheck %s

// CHECK: module {

// CHECK-LABEL:  func.func @empty_function() {
// CHECK-NOT:      my_loop_depths
// CHECK-NEXT:     return
// CHECK-NEXT:   }
func.func @empty_function() {
  return
}

// CHECK-LABEL:  func.func @single_affine_if() attributes {my_loop_depths = [1]} {
// CHECK:         %true = arith.constant true
// CHECK:         affine.if
// CHECK:         return
// CHECK:       }
func.func @single_affine_if() {
  %cond = arith.constant true
  affine.if affine_set<() : (0 == 0)>() {
  }
  return
}

// CHECK-LABEL:  func.func @nested_scf_conditions() attributes {my_loop_depths = [3]} {
// CHECK:         %c0 = arith.constant 0 : index
// CHECK:         %c10 = arith.constant 10 : index
// CHECK:         %true = arith.constant true
// CHECK:         scf.for
// CHECK:         return
// CHECK:       }
func.func @nested_scf_conditions() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %cond = arith.constant true
  scf.for %i = %c0 to %c10 step %c10 {
    scf.if %cond {
      scf.if %cond {
      }
    }
  }
  return
}

// CHECK-LABEL:  func.func @mixed_affine_scf() attributes {my_loop_depths = [4]} {
// CHECK:         %c0 = arith.constant 0 : index
// CHECK:         %c5 = arith.constant 5 : index
// CHECK:         %true = arith.constant true
// CHECK:         affine.for
// CHECK:         return
// CHECK:       }
func.func @mixed_affine_scf() {
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index
  %cond = arith.constant true
  affine.for %i = 0 to 10 {
    scf.for %j = %c0 to %c5 step %c5 {
      affine.if affine_set<() : (0 == 0)>() {
        scf.if %cond {
        }
      }
    }
  }
  return
}

// CHECK-LABEL:  func.func @complex_nesting() attributes {my_loop_depths = [5]} {
// CHECK:         %c0 = arith.constant 0 : index
// CHECK:         %c8 = arith.constant 8 : index
// CHECK:         %c1 = arith.constant 1 : index
// CHECK:         %true = arith.constant true
// CHECK:         scf.if
// CHECK:         return
// CHECK:       }
func.func @complex_nesting() {
  %c0 = arith.constant 0 : index
  %c8 = arith.constant 8 : index
  %c1 = arith.constant 1 : index
  %cond = arith.constant true
  scf.if %cond {
    scf.for %i = %c0 to %c8 step %c1 {
      affine.for %j = 0 to 5 {
        scf.if %cond {
          scf.for %k = %c0 to %c8 step %c1 {
          }
        }
      }
    }
  }
  return
}

// CHECK-LABEL:  func.func @parallel_loops_same_depth() attributes {my_loop_depths = [2, 2]} {
// CHECK:         %c0 = arith.constant 0 : index
// CHECK:         %c10 = arith.constant 10 : index
// CHECK:         %c1 = arith.constant 1 : index
// CHECK:         %true = arith.constant true
// CHECK:         scf.for
// CHECK:         affine.for
// CHECK:         return
// CHECK:       }
func.func @parallel_loops_same_depth() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  %cond = arith.constant true
  scf.for %i = %c0 to %c10 step %c1 {
    scf.if %cond {
    }
  }
  affine.for %j = 0 to 7 {
    affine.if affine_set<() : (0 == 0)>() {
    }
  }
  return
}

// CHECK-LABEL:  func.func @multiple_ifs_no_loops() attributes {my_loop_depths = [1, 1]} {
// CHECK:         %true = arith.constant true
// CHECK:         scf.if
// CHECK:         affine.if
// CHECK:         return
// CHECK:       }
func.func @multiple_ifs_no_loops() {
  %cond = arith.constant true
  scf.if %cond {
  }
  affine.if affine_set<() : (0 == 0)>() {
  }
  return
}

// CHECK-LABEL:  func.func @deeply_nested_affine() attributes {my_loop_depths = [4]} {
// CHECK:         %true = arith.constant true
// CHECK:         affine.for
// CHECK:         return
// CHECK:       }
func.func @deeply_nested_affine() {
  %cond = arith.constant true
  affine.for %i = 0 to 3 {
    affine.for %j = 0 to 3 {
      affine.if affine_set<() : (0 == 0)>() {
        affine.for %k = 0 to 3 {
        }
      }
    }
  }
  return
}

// CHECK-LABEL:  func.func @triple_nested_scf() attributes {my_loop_depths = [3]} {
// CHECK:         %c0 = arith.constant 0 : index
// CHECK:         %c5 = arith.constant 5 : index
// CHECK:         %c1 = arith.constant 1 : index
// CHECK:         scf.for
// CHECK:         return
// CHECK:       }
func.func @triple_nested_scf() {
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index
  %c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c5 step %c1 {
    scf.for %j = %c0 to %c5 step %c1 {
      scf.for %k = %c0 to %c5 step %c1 {
      }
    }
  }
  return
}

// CHECK-LABEL:  func.func @simple_scf_if_only() attributes {my_loop_depths = [1]} {
// CHECK:         %true = arith.constant true
// CHECK:         scf.if
// CHECK:         return
// CHECK:       }
func.func @simple_scf_if_only() {
  %cond = arith.constant true
  scf.if %cond {
  }
  return
}

// CHECK-LABEL:  func.func @affine_inside_scf() attributes {my_loop_depths = [3]} {
// CHECK:         %c0 = arith.constant 0 : index
// CHECK:         %c10 = arith.constant 10 : index
// CHECK:         %c1 = arith.constant 1 : index
// CHECK:         scf.for
// CHECK:         return
// CHECK:       }
func.func @affine_inside_scf() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c10 step %c1 {
    affine.for %j = 0 to 5 {
      affine.for %k = 0 to 3 {
      }
    }
  }
  return
}

// CHECK-LABEL:  func.func @scf_inside_affine() attributes {my_loop_depths = [3]} {
// CHECK:         %c0 = arith.constant 0 : index
// CHECK:         %c5 = arith.constant 5 : index
// CHECK:         %c1 = arith.constant 1 : index
// CHECK:         affine.for
// CHECK:         return
// CHECK:       }
func.func @scf_inside_affine() {
  %c0 = arith.constant 0 : index
  %c5 = arith.constant 5 : index
  %c1 = arith.constant 1 : index
  affine.for %i = 0 to 10 {
    scf.for %j = %c0 to %c5 step %c1 {
      scf.for %k = %c0 to %c5 step %c1 {
      }
    }
  }
  return
}