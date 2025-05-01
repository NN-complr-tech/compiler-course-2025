// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/LoopTracePass_Chistov_Alexey_FIIT1_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(LoopTracePass_Chistov_Alexey_FIIT1_MLIR)" %s | FileCheck %s

// CHECK: Count operations: 7
// CHECK-NEXT: module attributes {{.*}} {
// CHECK-NEXT:   llvm.func local_unnamed_addr @_Z6isEveni(%arg0: i32 {llvm.noundef}) -> (i1 {llvm.noundef, llvm.zeroext}) attributes {{.*}} {
// CHECK-NEXT:     %0 = llvm.mlir.constant(1 : i32) : i32
// CHECK-NEXT:     %1 = llvm.mlir.constant(0 : i32) : i32
// CHECK-NEXT:     %2 = llvm.and %arg0, %0  : i32
// CHECK-NEXT:     %3 = llvm.icmp "eq" %2, %1 : i32
// CHECK-NEXT:     llvm.return %3 : i1
// CHECK-NEXT:   }
// CHECK-NEXT: }

module attributes {dlti.dl_spec = #dlti.dl_spec<#dlti.dl_entry<f64, dense<64> : vector<2xi64>>, #dlti.dl_entry<f16, dense<16> : vector<2xi64>>, #dlti.dl_entry<f128, dense<128> : vector<2xi64>>, #dlti.dl_entry<!llvm.ptr<270>, dense<32> : vector<4xi64>>, #dlti.dl_entry<!llvm.ptr<271>, dense<32> : vector<4xi64>>, #dlti.dl_entry<!llvm.ptr<272>, dense<64> : vector<4xi64>>, #dlti.dl_entry<i64, dense<64> : vector<2xi64>>, #dlti.dl_entry<i128, dense<128> : vector<2xi64>>, #dlti.dl_entry<f80, dense<128> : vector<2xi64>>, #dlti.dl_entry<!llvm.ptr, dense<64> : vector<4xi64>>, #dlti.dl_entry<i1, dense<8> : vector<2xi64>>, #dlti.dl_entry<i8, dense<8> : vector<2xi64>>, #dlti.dl_entry<i16, dense<16> : vector<2xi64>>, #dlti.dl_entry<i32, dense<32> : vector<2xi64>>, #dlti.dl_entry<"dlti.endianness", "little">, #dlti.dl_entry<"dlti.stack_alignment", 128 : i64>>} {
  llvm.func local_unnamed_addr @_Z6isEveni(%arg0: i32 {llvm.noundef}) -> (i1 {llvm.noundef, llvm.zeroext}) attributes {memory = #llvm.memory_effects<other = none, argMem = none, inaccessibleMem = none>, no_unwind, passthrough = ["mustprogress", "nofree", "norecurse", "nosync", ["uwtable", "2"], ["min-legal-vector-width", "0"], ["no-trapping-math", "true"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cmov", "+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, tune_cpu = "generic", will_return} {
    %0 = llvm.mlir.constant(1 : i32) : i32
    %1 = llvm.mlir.constant(0 : i32) : i32
    %2 = llvm.and %arg0, %0  : i32
    %3 = llvm.icmp "eq" %2, %1 : i32
    llvm.return %3 : i1
  }
}
