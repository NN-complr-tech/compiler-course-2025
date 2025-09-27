// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/TripCount_Budazhapova_Ekaterina_FIIT3_BACKEND%shlibext --pass-pipeline="builtin.module(loop-iteration-marker)" %s | FileCheck %s

module {
func.func @simple_loop() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index  
  %c1 = arith.constant 1 : index
  scf.for %i = %c0 to %c10 step %c1 {
    scf.yield
  }
  // CHECK: {trip_count = 10 : i64}
  return
}

func.func @loop_with_large_step() {
  %c0 = arith.constant 0 : index
  %c20 = arith.constant 20 : index
  %c3 = arith.constant 3 : index
  scf.for %i = %c0 to %c20 step %c3 {
    scf.yield
  }
  // CHECK: {trip_count = 7 : i64}
  return
}

func.func @reverse_loop() {
  %c10 = arith.constant 10 : index
  %c0 = arith.constant 0 : index
  %c_minus2 = arith.constant -2 : index 
  scf.for %i = %c10 to %c0 step %c_minus2 {
    scf.yield
  }
  // CHECK: {trip_count = 5 : i64}
  return
}

func.func @dynamic_loop(%arg0: index, %arg1: index) {
  %c1 = arith.constant 1 : index
  scf.for %i = %arg0 to %arg1 step %c1 {
    scf.yield
  }
  // CHECK-NOT: trip_count
  return
}

func.func @zero_step_loop() {
  %c0 = arith.constant 0 : index
  %c10 = arith.constant 10 : index
  %c0_step = arith.constant 0 : index
  scf.for %i = %c0 to %c10 step %c0_step {
    scf.yield
  }
  // CHECK-NOT: trip_count
  return
}

func.func @empty_loop() {
  %c5 = arith.constant 5 : index
  %c2 = arith.constant 2 : index
  %c1 = arith.constant 1 : index
  scf.for %i = %c5 to %c2 step %c1 {
    scf.yield
  }
  // CHECK: {trip_count = 0 : i64}
  return
}

func.func @reverse_empty_loop() {
  %c2 = arith.constant 2 : index
  %c5 = arith.constant 5 : index
  %c_minus1 = arith.constant -1 : index
  scf.for %i = %c2 to %c5 step %c_minus1 {
    scf.yield
  }
  // CHECK: {trip_count = 0 : i64}
  return
}
} // module
