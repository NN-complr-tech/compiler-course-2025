// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/CallCountPass_Grudzin_Konstantin_FIIT1_MLIR%shlibext \
// RUN:   --pass-pipeline="builtin.module(CallCountPass_Grudzin_Konstantin_FIIT1_MLIR)" %s \
// RUN: | FileCheck --implicit-check-not="call @unused() {call_count =" %s

// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/FunctionCallCounterPass_Suvorov_Dmitrii_FIIT1_MLIR%shlibext \
// RUN:   --pass-pipeline="builtin.module(FunctionCallCounterPass_Suvorov_Dmitrii_FIIT1_MLIR)" %s \
// RUN: | FileCheck --implicit-check-not="call @not_called() {call_count =" %s

module {
  func.func @a() -> () {
    return
  }

  func.func @b() -> () {
    return
  }

  func.func @c() -> () {
    return
  }

  func.func @d() -> () {
    return
  }

  func.func @not_called() {
    return
  }

  // CHECK-LABEL: func.func @main()
  func.func @main() {
    // CHECK: call @a() {call_count = 4 : i64} : () -> ()
    call @a() : () -> ()
    // CHECK: call @b() {call_count = 2 : i64} : () -> ()
    call @b() : () -> ()
    // CHECK: call @a() {call_count = 4 : i64} : () -> ()
    call @a() : () -> ()
    // CHECK: call @c() {call_count = 1 : i64} : () -> ()
    call @c() : () -> ()
    return
  }

  // CHECK-LABEL: func.func @helper1()
  func.func @helper1() {
    // CHECK: call @a() {call_count = 4 : i64} : () -> ()
    call @a() : () -> ()
    // CHECK: call @b() {call_count = 2 : i64} : () -> ()
    call @b() : () -> ()
    return
  }

  // CHECK-LABEL: func.func @helper2()
  func.func @helper2() {
    // CHECK: call @a() {call_count = 4 : i64} : () -> ()
    call @a() : () -> ()
    // CHECK: call @d() {call_count = 1 : i64} : () -> ()
    call @d() : () -> ()
    return
  }
}
