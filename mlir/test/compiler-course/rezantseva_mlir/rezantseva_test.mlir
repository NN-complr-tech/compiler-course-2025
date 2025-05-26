// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/MaxDepthPass_RezantsevaAnastasia_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(func.func(MaxDepthPass_RezantsevaAnastasia_FIIT1_MLIR))" %s | FileCheck %s
#set = affine_set<(d0): (d0 >= 0)>

// CHECK: func.func @only_affine_if() attributes {my_loop_depths = []}
// CHECK-NEXT: %c0 = arith.constant 0 : index
// CHECK-NEXT: affine.if #set(%c0) {
// CHECK-NEXT: }
// CHECK-NEXT: return
// CHECK-NEXT: }
func.func @only_affine_if() {
  %c0 = arith.constant 0 : index
  affine.if #set(%c0) {
  }
  return
}

// CHECK: func.func @single_scf_for() attributes {my_loop_depths = [1]}
// CHECK-NEXT: %c0 = arith.constant 0 : index
// CHECK-NEXT: %c10 = arith.constant 10 : index
// CHECK-NEXT: %c1 = arith.constant 1 : index
// CHECK-NEXT: scf.for %arg0 = %c0 to %c10 step %c1 {
// CHECK-NEXT: }
// CHECK-NEXT: return
// CHECK-NEXT: }
func.func @single_scf_for() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c10 step %c1 {
  }
  return
}

// CHECK: func.func @affine_for_in_affine_if() attributes {my_loop_depths = [1]}
// CHECK-NEXT: %c0 = arith.constant 0 : index
// CHECK-NEXT: %c10 = arith.constant 10 : index
// CHECK-NEXT: affine.if #set(%c0) {
// CHECK-NEXT: affine.for %arg0 = 0 to 10
// CHECK-NEXT:   }
// CHECK-NEXT: }
// CHECK-NEXT: return
// CHECK-NEXT: }
func.func @affine_for_in_affine_if() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  affine.if #set(%c0) {
    affine.for %arg0 = 0 to 10 {
    }
  }
  return
}

// CHECK: func.func @single_scf_while() attributes {my_loop_depths = [1]} {
// CHECK-NEXT:   %c0 = arith.constant 0 : index
// CHECK-NEXT:   %c10 = arith.constant 10 : index
// CHECK-NEXT:   %c1 = arith.constant 1 : index
// CHECK-NEXT:   %0 = scf.while (%arg0 = %c0) : (index) -> index {
// CHECK-NEXT:     %1 = arith.cmpi slt, %arg0, %c10 : index
// CHECK-NEXT:     scf.condition(%1) %arg0 : index
// CHECK-NEXT:   } do {
// CHECK-NEXT:   ^bb0(%arg0: index):
// CHECK-NEXT:     %1 = arith.addi %arg0, %c1 : index
// CHECK-NEXT:     scf.yield %1 : index
// CHECK-NEXT:   }
// CHECK-NEXT:   return
// CHECK-NEXT: }

func.func @single_scf_while() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  %res = scf.while (%i = %c0) : (index) -> index {
    %cond = arith.cmpi slt, %i, %c10 : index
    scf.condition(%cond) %i : index
  } do {
  ^bb0(%i: index):
    %next = arith.addi %i, %c1 : index
    scf.yield %next : index
  }
  return
}



// CHECK: func.func @scf_for_with_if() attributes {my_loop_depths = [2]} {
// CHECK-NEXT:   %c0 = arith.constant 0 : index
// CHECK-NEXT:   %c10 = arith.constant 10 : index
// CHECK-NEXT:   %c1 = arith.constant 1 : index
// CHECK-NEXT:   %true = arith.constant true
// CHECK-NEXT:   scf.for %{{.*}} = %c0 to %c10 step %c1 {
// CHECK-NEXT:     scf.if %{{.*}} {
// CHECK-NEXT:     }
// CHECK-NEXT:     scf.if %{{.*}} {
// CHECK-NEXT:     }
// CHECK-NEXT:   }
// CHECK-NEXT:   return
// CHECK-NEXT: }

func.func @scf_for_with_if() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c1 = arith.constant 1 : index
  %cond = arith.constant true
  scf.for %i = %c0 to %c10 step %c1 {
    scf.if %cond {
    }
    scf.if %cond {
    }
  }
  return
}


// CHECK: func.func @deeply_nested_if_in_for() attributes {my_loop_depths = [4]} {
// CHECK-NEXT:   %c0 = arith.constant 0 : index
// CHECK-NEXT:   %c10 = arith.constant 10 : index
// CHECK-NEXT:   %c1 = arith.constant 1 : index
// CHECK-NEXT:   %{{.*}} = arith.constant true
// CHECK-NEXT:   scf.for %{{.*}} = %c0 to %c10 step %c1 {
// CHECK-NEXT:     scf.if %{{.*}} {
// CHECK-NEXT:       scf.if %{{.*}} {
// CHECK-NEXT:         scf.if %{{.*}} {
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:   }
// CHECK-NEXT:   return
// CHECK-NEXT: }
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

// CHECK: func.func @nested_loops() attributes {my_loop_depths = [4, 2]} {
// CHECK-NEXT:   %c0 = arith.constant 0 : index
// CHECK-NEXT:   %c20 = arith.constant 20 : index
// CHECK-NEXT:   %c2 = arith.constant 2 : index
// CHECK-NEXT:   scf.for %arg0 = %c0 to %c20 step %c2 {
// CHECK-NEXT:     scf.for %arg1 = %c0 to %c20 step %c2 {
// CHECK-NEXT:       scf.for %arg2 = %c0 to %c20 step %c2 {
// CHECK-NEXT:         scf.for %arg3 = %c0 to %c20 step %c2 {
// CHECK-NEXT:           %0 = arith.addi %arg3, %c2 : index
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:   }
// CHECK-NEXT:   scf.for %arg0 = %c0 to %c20 step %c2 {
// CHECK-NEXT:     scf.for %arg1 = %c0 to %c20 step %c2 {
// CHECK-NEXT:     }
// CHECK-NEXT:   }
// CHECK-NEXT:   return
// CHECK-NEXT: }

func.func @nested_loops() {
  %c0 = arith.constant 0 : index
  %c20 = arith.constant 20 : index
  %c2 = arith.constant 2 : index
  scf.for %outer = %c0 to %c20 step %c2 {
    scf.for %inner1 = %c0 to %c20 step %c2 {
      scf.for %inner2 = %c0 to %c20 step %c2 {
        scf.for %inner3 = %c0 to %c20 step %c2 {
          %sum = arith.addi %inner3, %c2 : index
        }
      }
    }
  }
  scf.for %outer2 = %c0 to %c20 step %c2 {
    scf.for %inner4 = %c0 to %c20 step %c2 {
    }
  }
  return
}