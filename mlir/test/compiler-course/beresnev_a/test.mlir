// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/UserPass_Beresnev_Anton_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(UserPass_Beresnev_Anton_FIIT1_MLIR)" %s | FileCheck %s

module {
  func.func @test() {
	%a = arith.constant 0 : index
	%b = arith.constant 10 : index
	%c = arith.constant 1 : index
	scf.for %i = %a to %b step %c {  }
	// CHECK: {trip_count = 10 : i64}
	func.return
  }

  func.func @test1() {
	%a = arith.constant 0 : index
	%b = arith.constant 120 : index
	%c = arith.constant 4 : index
	scf.for %i = %a to %b step %c {  }
	// CHECK: {trip_count = 30 : i64}
	func.return
  }

  func.func @test2() {
	%a = arith.constant 20 : index
	%b = arith.constant -20 : index
	%c = arith.constant -3 : index
	scf.for %i = %a to %b step %c {  }
	// CHECK: {trip_count = 14 : i64}
	func.return
  }

  func.func @test3() {
	%a = arith.constant 20 : index
	%b = arith.constant 20 : index
	%c = arith.constant 0 : index
	scf.for %i = %a to %b step %c {  }
	// CHECK: {trip_count = 0 : i64}
	func.return
  }

  // CHECK-LABEL: func.func @test4
  func.func @test4(%a : index) {
	%b = arith.constant -20 : index
	%c = arith.constant -3 : index
	scf.for %i = %a to %b step %c {  }
	// CHECK-NOT: trip_count
	func.return
  }

  // CHECK-LABEL: func.func @test5
  func.func @test5() {
	%a = arith.constant 0 : index
	%b = arith.constant 20 : index
	%c = arith.constant 0 : index
	scf.for %i = %a to %b step %c {  }
	// CHECK-NOT: trip_count
	func.return
  }
}
