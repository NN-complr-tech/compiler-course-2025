// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/SearchMaxDepthPass_Lysov_Ivan_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(SearchMaxDepthPass_Lysov_Ivan_FIIT3_MLIR)" %s | FileCheck %s

module {
  // CHECK-LABEL: func.func @multiple_loops() attributes {loop_max_depth = 3 : i32}
  // CHECK-NEXT:   %{{.*}} = arith.constant 0 : index
  // CHECK-NEXT:   %{{.*}} = arith.constant 1 : index
  // CHECK-NEXT:   %{{.*}} = arith.constant 5 : index
  // CHECK-NEXT:   %{{.*}} = arith.constant 10 : index
  // CHECK-NEXT:   %{{.*}} = arith.constant true
  // CHECK-NEXT:   scf.for
  // CHECK:         scf.for
  // CHECK:           scf.if
  // CHECK:             scf.for
  func.func @multiple_loops() {
    %c0 = arith.constant 0 : index
    %c1 = arith.constant 1 : index
    %c5 = arith.constant 5 : index
    %c10 = arith.constant 10 : index
    %true = arith.constant true

    scf.for %i = %c0 to %c10 step %c1 {
    }

    scf.for %i = %c0 to %c10 step %c1 {
      scf.if %true {
        scf.for %j = %c0 to %c5 step %c1 {
        }
      }
    }

    return
  }

  // CHECK-LABEL: func.func @only_shallow_loops() attributes {loop_max_depth = 1 : i32}
  // CHECK-NEXT:   %{{.*}} = arith.constant 0 : index
  // CHECK-NEXT:   %{{.*}} = arith.constant 1 : index
  // CHECK:         scf.for
  // CHECK:         affine.for
  func.func @only_shallow_loops() {
    %c0 = arith.constant 0 : index
    %c1 = arith.constant 1 : index

    scf.for %i = %c0 to %c1 step %c1 {
    }
    affine.for %j = 0 to 4 {
    }

    return
  }

  // CHECK-LABEL: func.func @no_loops() attributes {loop_max_depth = 0 : i32}
  // CHECK-NEXT:   %{{.*}} = arith.constant 42 : i32
  func.func @no_loops() {
    %x = arith.constant 42 : i32
    return
  }

  // CHECK-LABEL: func.func @affine_if_inside_loop() attributes {loop_max_depth = 2 : i32}
  // CHECK-NEXT:   %{{.*}} = arith.constant 0 : index
  // CHECK-NEXT:   %{{.*}} = arith.constant 1 : index
  // CHECK-NEXT:   %{{.*}} = arith.constant 10 : index
  // CHECK:         affine.for
  // CHECK:           affine.if
  func.func @affine_if_inside_loop() {
    %c0 = arith.constant 0 : index
    %c1 = arith.constant 1 : index
    %ten = arith.constant 10 : index

    affine.for %i = 0 to 4 {
      affine.if affine_set<(d0)[s0] : (d0 - s0 >= 0)>(%i)[%ten] {
      }
    }

    return
  }

  // CHECK-LABEL: func.func @equal_nested_loops() attributes {loop_max_depth = 2 : i32}
  // CHECK-NEXT:   %{{.*}} = arith.constant 0 : index
  // CHECK-NEXT:   %{{.*}} = arith.constant 1 : index
  // CHECK-NEXT:   %{{.*}} = arith.constant true
  // CHECK:         scf.for
  // CHECK:           scf.if
  // CHECK:         affine.for
  // CHECK:           scf.if
  func.func @equal_nested_loops() {
    %c0 = arith.constant 0 : index
    %c1 = arith.constant 1 : index
    %cond = arith.constant true

    scf.for %i = %c0 to %c1 step %c1 {
      scf.if %cond {
      }
    }

    affine.for %j = 0 to 5 {
      scf.if %cond {
      }
    }

    return
  }

  // CHECK-LABEL: func.func @while_with_nesting() attributes {loop_max_depth = 3 : i32}
  // CHECK-NEXT:   %{{.*}} = arith.constant true
  // CHECK:         scf.while
  // CHECK:           scf.if
  // CHECK:             scf.for
  func.func @while_with_nesting() {
    %cond = arith.constant true

    %res = scf.while (%arg0 = %cond) : (i1) -> (i1) {
      scf.condition(%arg0) %arg0 : i1
    } do {
      ^bb0(%arg1: i1):
        scf.if %arg1 {
          %c0 = arith.constant 0 : index
          %c10 = arith.constant 10 : index
          %c1 = arith.constant 1 : index
          scf.for %i = %c0 to %c10 step %c1 {
          }
        }
        scf.yield %arg1 : i1
    }

    return
  }
}
