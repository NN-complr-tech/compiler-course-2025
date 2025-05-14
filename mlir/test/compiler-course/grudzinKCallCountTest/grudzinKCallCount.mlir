// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/CallCountPass_Grudzin_Konstantin_FIIT1_MLIR%shlibext \
// RUN:   --pass-pipeline="builtin.module(CallCountPass_Grudzin_Konstantin_FIIT1_MLIR)" %s \
// RUN: | FileCheck --implicit-check-not="call @unused() {call_count =" %s

module {
  func.func @callee1() -> () {
    return
  }

  func.func @callee2() -> () {
    return
  }

  func.func @callee3() -> () {
    return
  }

  // Not called
  // CHECK-LABEL: func.func @unused()
  // CHECK-NOT: call_count
  func.func @unused() {
    return
  }

  // CHECK-LABEL: func.func @main()
  func.func @main() {
    // CHECK:     call @callee1() {call_count = 3 : i64} : () -> ()
    call @callee1() : () -> ()
    // CHECK-NEXT: call @callee2() {call_count = 2 : i64} : () -> ()
    call @callee2() : () -> ()
    // CHECK-NEXT: call @callee1() {call_count = 3 : i64} : () -> ()
    call @callee1() : () -> ()
    // CHECK-NEXT: call @driver()  {call_count = 1 : i64} : () -> ()
    call @driver() : () -> ()
    return
  }

  // CHECK-LABEL: func.func @driver()
  func.func @driver() {
    // CHECK:     call @callee1() {call_count = 3 : i64} : () -> ()
    call @callee1() : () -> ()
    // CHECK-NEXT: call @callee2() {call_count = 2 : i64} : () -> ()
    call @callee2() : () -> ()
    // CHECK-NEXT: call @callee3() {call_count = 1 : i64} : () -> ()
    call @callee3() : () -> ()
    return
  }
}
