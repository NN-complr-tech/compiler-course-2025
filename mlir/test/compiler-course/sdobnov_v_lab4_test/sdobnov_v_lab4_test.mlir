// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/CallFrequencyAnalyzer_SdobnovV_FIIT2_MLIR%shlibext \
// RUN:   --pass-pipeline="builtin.module(call-frequency-analyzer)" %s \
// RUN: | FileCheck --implicit-check-not="call @unused_function() {invocation_count =" %s

module {
  func.func @alpha() -> () {
    return
  }

  func.func @beta() -> () {
    return
  }

  func.func @gamma() -> () {
    return
  }

  func.func @delta() -> () {
    return
  }

  func.func @unused_function() {
    return
  }

  // CHECK-LABEL: func.func @entry_point()
  func.func @entry_point() {
    // CHECK: call @alpha() {invocation_count = 4 : i64} : () -> ()
    call @alpha() : () -> ()
    // CHECK: call @beta() {invocation_count = 2 : i64} : () -> ()
    call @beta() : () -> ()
    // CHECK: call @alpha() {invocation_count = 4 : i64} : () -> ()
    call @alpha() : () -> ()
    // CHECK: call @gamma() {invocation_count = 1 : i64} : () -> ()
    call @gamma() : () -> ()
    return
  }

  // CHECK-LABEL: func.func @first_helper()
  func.func @first_helper() {
    // CHECK: call @alpha() {invocation_count = 4 : i64} : () -> ()
    call @alpha() : () -> ()
    // CHECK: call @beta() {invocation_count = 2 : i64} : () -> ()
    call @beta() : () -> ()
    return
  }

  // CHECK-LABEL: func.func @second_helper()
  func.func @second_helper() {
    // CHECK: call @alpha() {invocation_count = 4 : i64} : () -> ()
    call @alpha() : () -> ()
    // CHECK: call @delta() {invocation_count = 1 : i64} : () -> ()
    call @delta() : () -> ()
    return
  }

  // CHECK-LABEL: func.func @nested_caller()
  func.func @nested_caller() {
    // CHECK: call @first_helper() {invocation_count = 1 : i64} : () -> ()
    call @first_helper() : () -> ()
    // CHECK: call @second_helper() {invocation_count = 1 : i64} : () -> ()
    call @second_helper() : () -> ()
    return
  }
}