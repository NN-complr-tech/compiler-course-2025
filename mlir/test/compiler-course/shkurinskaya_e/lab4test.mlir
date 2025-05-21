// RUN: mlir-opt
// --load-pass-plugin=%mlir\_lib\_dir/TripCountPass\_Shkurinskaya\_Elena\_FIIT2\_MLIR%shlibext&#x20;
// RUN:
// --pass-pipeline="builtin.module(TripCountPass\_Shkurinskaya\_Elena\_FIIT2\_MLIR)"
// %s | FileCheck %s

module {
  // Test 1: positive step, exact division 0..8 step 2 => 4
  // CHECK-LABEL: func.func @testA
  // CHECK: scf.for %{{.\*}} = %c0 to %c8 step %c2 {  }
  // CHECK-SAME: {trip\_count = 4 : index}
  func.func @testA() {
%c0 = arith.constant 0 : index
%c8 = arith.constant 8 : index
%c2 = arith.constant 2 : index
scf.for %i = %c0 to %c8 step %c2 {  }
func.return
  }

  // Test 2: uneven division 1..9 step 2 => ceil(8/2)=4
  // CHECK-LABEL: func.func @testB
  // CHECK: scf.for %{{.\*}} = %c1 to %c9 step %c2 {  }
  // CHECK-SAME: {trip\_count = 4 : index}
  func.func @testB() {
%c1 = arith.constant 1 : index
%c9 = arith.constant 9 : index
%c2 = arith.constant 2 : index
scf.for %i = %c1 to %c9 step %c2 {  }
func.return
  }

  // Test 3: dynamic parameters => no trip\_count
  // CHECK-LABEL: func.func @testC
  // CHECK-NOT: trip\_count
  func.func @testC(% a : index, % b : index, % s : index) {
scf.for %i = %a to %b step %s {
scf.yield
}
func.return
  }

  // Test 4: zero step => no trip\_count
  // CHECK-LABEL: func.func @testD
  // CHECK-NOT: trip\_count
  func.func @testD() {
%c0 = arith.constant 0 : index
%c5 = arith.constant 5 : index
scf.for %i = %c0 to %c5 step %c0 {
scf.yield
}
func.return
  }

  // Test 5: descending valid 10..2 step -3 => ceil(8/3)=3
  // CHECK-LABEL: func.func @testE
  // CHECK: scf.for %{{.\*}} = %c10 to %c2 step %cneg3 {  }
  // CHECK-SAME: {trip\_count = 3 : index}
  func.func @testE() {
%c10 = arith.constant 10 : index
%c2  = arith.constant 2  : index
%cneg3 = arith.constant -3 : index
scf.for %i = %c10 to %c2 step %cneg3 {  }
func.return
  }

  // Test 6: positive step but lb > ub => no trip_count
  // CHECK-LABEL: func.func @test_no_iter
  // CHECK-NOT: trip_count
  func.func @test_no_iter() {
%c10 = arith.constant 10 : index
%c1  = arith.constant 1  : index
%c1s = arith.constant 1  : index
scf.for %i = %c10 to %c1 step %c1s {  }
func.return
  }
}
