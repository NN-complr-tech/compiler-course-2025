// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/LoopNestsPass_KabalovaValeria_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(func.func(LoopNestsPass_KabalovaValeria_FIIT1_MLIR))" %s | FileCheck %s

#set = affine_set<(d0): (d0 >= 0)>


//CHECK: func.func @no_loops() attributes {"Max_loop_depths:" = []} {
//CHECK-NEXT: return
//CHECK-NEXT: }


func.func @no_loops() {
  func.return
}


//CHECK: func.func @affine_2depth() attributes {"Max_loop_depths:" = [2]} { 
//CHECK-NEXT:  affine.for %arg0 = 0 to 2 {
//CHECK-NEXT:      affine.if #set(%arg0) {
//CHECK-NEXT:      }
//CHECK-NEXT:  }
//CHECK-NEXT:  return
//CHECK-NEXT:}


func.func @affine_2depth() {
  affine.for %arg0 = 0 to 2 {
      affine.if #set(%arg0) {
      }
  }
  func.return
} 


//CHECK: func.func @scf_more_loops() attributes {"Max_loop_depths:" = [3, 2]} { 
//CHECK-NEXT:  %c0 = arith.constant 0 : index
//CHECK-NEXT:  %c10 = arith.constant 10 : index
//CHECK-NEXT:  %c1 = arith.constant 1 : index
//CHECK-NEXT:  scf.for %arg0 = %c0 to %c10 step %c1 {
//CHECK-NEXT:    scf.for %arg1 = %c0 to %c10 step %c1 {
//CHECK-NEXT:      scf.for %arg2 = %c0 to %c10 step %c1 {
//CHECK-NEXT:      }
//CHECK-NEXT:    }
//CHECK-NEXT:  }
//CHECK-NEXT:  scf.for %arg0 = %c0 to %c10 step %c1 {
//CHECK-NEXT:    scf.for %arg1 = %c0 to %c10 step %c1 {
//CHECK-NEXT:    }
//CHECK-NEXT:  }
//CHECK-NEXT:  return 
//CHECK-NEXT:}


func.func @scf_more_loops() {
  %i1 = arith.constant 0 : index
  %i2 = arith.constant 10 : index
  %i3 = arith.constant 1 : index
  scf.for %i = %i1 to %i2 step %i3 {
    scf.for %j = %i1 to %i2 step %i3 {
      scf.for %k = %i1 to %i2 step %i3 {
      }
    }
  }
  scf.for %l = %i1 to %i2 step %i3 {
    scf.for %f = %i1 to %i2 step %i3 {
    }
  }
  func.return 
}

//CHECK: func.func @scf_if() -> index attributes {"Max_loop_depths:" = [2]} {
//CHECK-NEXT:  %c0 = arith.constant 0 : index
//CHECK-NEXT:  %c10 = arith.constant 10 : index
//CHECK-NEXT:  %c1 = arith.constant 1 : index
//CHECK-NEXT:  %0 = scf.for %arg0 = %c0 to %c10 step %c1 iter_args(%arg1 = %c0) -> (index) {
//CHECK-NEXT:    %true = arith.constant true 
//CHECK-NEXT:    %1 = scf.if %true -> (index) {
//CHECK-NEXT:      %c1_0 = arith.constant 1 : index
//CHECK-NEXT:      scf.yield %c1_0 : index
//CHECK-NEXT:    } else {
//CHECK-NEXT:      %c2 = arith.constant 2 : index
//CHECK-NEXT:      scf.yield %c2 : index 
//CHECK-NEXT:    }
//CHECK-NEXT:    scf.yield %1 : index
//CHECK-NEXT:  }
//CHECK-NEXT:  return %0 : index
//CHECK-NEXT:}


func.func @scf_if () -> index{
  %i1 = arith.constant 0 : index
  %i2 = arith.constant 10 : index
  %i3 = arith.constant 1 : index
  %res = scf.for %i = %i1 to %i2 step %i3 iter_args(%sum = %i1) -> index {
    %true = arith.constant true
    %res1 = scf.if %true -> (index){
      %c1 = arith.constant 1 : index
      scf.yield %c1 : index
    } else {
      %c2 = arith.constant 2 : index
      scf.yield %c2 : index
    }
    scf.yield %res1 : index
  }
  func.return %res : index
}

//CHECK: func.func @scf_while() -> index attributes {"Max_loop_depths:" = [1]} {
//CHECK-NEXT:  %c0 = arith.constant 0 : index
//CHECK-NEXT:  %c10 = arith.constant 10 : index
//CHECK-NEXT:  %c1 = arith.constant 1 : index
//CHECK-NEXT:  %0 = scf.while (%arg0 = %c0) : (index) -> index {
//CHECK-NEXT:    %1 = arith.cmpi eq, %arg0, %c10 : index
//CHECK-NEXT:    scf.condition(%1) %arg0 : index
//CHECK-NEXT:  } do {
//CHECK-NEXT:    ^bb0(%arg0: index):
//CHECK-NEXT:    %1 = arith.addi %arg0, %c1 : index
//CHECK-NEXT:    scf.yield %1 : index
//CHECK-NEXT:  }
//CHECK-NEXT:  return %0 : index 
//CHECK-NEXT:}

func.func @scf_while() -> index{
  %i1 = arith.constant 0 : index
  %i2 = arith.constant 10 : index
  %i3 = arith.constant 1 : index
  %res = scf.while(%i = %i1): (index) -> index {
    %continue = arith.cmpi eq, %i, %i2 : index
    scf.condition(%continue) %i : index
  } do {
    ^bb0(%i: index):
    %new = arith.addi %i, %i3 : index
    scf.yield %new : index
  }
  func.return %res : index
}

//CHECK: func.func @scf_forall() attributes {"Max_loop_depths:" = [1]} {
//CHECK-NEXT:  %c0 = arith.constant 0 : index
//CHECK-NEXT:    scf.forall (%arg0) in (%c0) {
//CHECK-NEXT:  }
//CHECK-NEXT:  return
//CHECK-NEXT:}

func.func @scf_forall() {
  %i1 = arith.constant 0 : index
  scf.forall (%i) in (%i1) {
  }
  func.return
}

//CHECK: func.func @spirv() attributes {"Max_loop_depths:" = [2]} {
//CHECK-NEXT:  %c0_i32 = arith.constant 0 : i32
//CHECK-NEXT:  %c10_i32 = arith.constant 10 : i32 
//CHECK-NEXT:  %c1_i32 = arith.constant 1 : i32 
//CHECK-NEXT:  spirv.mlir.loop {
//CHECK-NEXT:    spirv.Branch ^bb1(%c0_i32 : i32)
//CHECK-NEXT:    ^bb1(%0: i32): // 2 preds: ^bb0, ^bb3
//CHECK-NEXT:      %1 = arith.cmpi eq, %0, %c10_i32 : i32
//CHECK-NEXT:      spirv.BranchConditional %1, ^bb2, ^bb4
//CHECK-NEXT:    ^bb2: // pred: ^bb1
//CHECK-NEXT:    spirv.mlir.selection {
//CHECK-NEXT:        %true = arith.constant true
//CHECK-NEXT:        spirv.BranchConditional %true, ^bb1, ^bb2 
//CHECK-NEXT:        ^bb1: // pred: ^bb0
//CHECK-NEXT:          spirv.Branch ^bb3(%c0_i32 : i32)
//CHECK-NEXT:        ^bb2: // pred: ^bb0
//CHECK-NEXT:          spirv.Branch ^bb3(%c10_i32 : i32)
//CHECK-NEXT:        ^bb3(%3: i32): // 2 preds: ^bb1, ^bb2
//CHECK-NEXT:          spirv.Return
//CHECK-NEXT:        ^bb4: // no predecessors 
//CHECK-NEXT:          spirv.mlir.merge
//CHECK-NEXT:      }
//CHECK-NEXT:      spirv.Branch ^bb3 
//CHECK-NEXT:    ^bb3: // pred: ^bb2
//CHECK-NEXT:      %2 = arith.addi %0, %c1_i32 : i32
//CHECK-NEXT:      spirv.Branch ^bb1(%2 : i32)
//CHECK-NEXT:    ^bb4: // pred: ^bb1 
//CHECK-NEXT:    spirv.mlir.merge
//CHECK-NEXT:  }
//CHECK-NEXT:  return
//CHECK-NEXT:}




func.func @spirv() {
  %i1 = arith.constant 0 : i32
  %i2 = arith.constant 10 : i32
  %i3 = arith.constant 1 : i32
  spirv.mlir.loop {
    spirv.Branch ^header (%i1 : i32)
    ^header(%i : i32):
      %cond = arith.cmpi eq, %i, %i2 : i32
      spirv.BranchConditional %cond, ^body, ^merge
    ^body:
    spirv.mlir.selection {
        %true = arith.constant true
        spirv.BranchConditional %true, ^true, ^false
        ^true:
          spirv.Branch ^phi(%i1: i32)
        ^false:
          spirv.Branch ^phi(%i2: i32)
        ^phi (%arg: i32):
          spirv.Return
        ^merge_block:
          spirv.mlir.merge
      }
      spirv.Branch ^continue

    ^continue:
      %new = arith.addi %i, %i3 : i32
      spirv.Branch ^header (%new : i32)

    ^merge:
    spirv.mlir.merge
  }
  func.return
}