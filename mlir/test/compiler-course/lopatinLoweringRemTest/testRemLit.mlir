// RUN: mlir-opt -load-pass-plugin=%mlir_lib_dir/LowerRemPass_IlyaLopatin_FIIT3_MLIR%shlibext \
// RUN: --pass-pipeline="builtin.module(LowerRemPass_IlyaLopatin_FIIT3_MLIR)" %s | FileCheck %s

module attributes {dlti.dl_spec = #dlti.dl_spec<#dlti.dl_entry<f80, dense<128> : vector<2xi64>>, #dlti.dl_entry<i1, dense<8> : vector<2xi64>>, #dlti.dl_entry<!llvm.ptr, dense<64> : vector<4xi64>>, #dlti.dl_entry<i64, dense<64> : vector<2xi64>>, #dlti.dl_entry<i128, dense<128> : vector<2xi64>>, #dlti.dl_entry<!llvm.ptr<270>, dense<32> : vector<4xi64>>, #dlti.dl_entry<f128, dense<128> : vector<2xi64>>, #dlti.dl_entry<f64, dense<64> : vector<2xi64>>, #dlti.dl_entry<f16, dense<16> : vector<2xi64>>, #dlti.dl_entry<!llvm.ptr<272>, dense<64> : vector<4xi64>>, #dlti.dl_entry<!llvm.ptr<271>, dense<32> : vector<4xi64>>, #dlti.dl_entry<i16, dense<16> : vector<2xi64>>, #dlti.dl_entry<i8, dense<8> : vector<2xi64>>, #dlti.dl_entry<i32, dense<32> : vector<2xi64>>, #dlti.dl_entry<"dlti.stack_alignment", 128 : i64>, #dlti.dl_entry<"dlti.endianness", "little">>} {
  // CHECK-LABEL: @test_signed_rem
  // CHECK-NEXT:     %0 = arith.divsi %arg0, %arg1 : i32
  // CHECK-NEXT:     %1 = arith.muli %0, %arg1 : i32
  // CHECK-NEXT:     %2 = arith.subi %arg0, %1 : i32
  // CHECK-NEXT:     llvm.return %2 : i32
  // CHECK-NEXT:   }
  llvm.func local_unnamed_addr @test_signed_rem(%arg0: i32 {llvm.noundef}, %arg1: i32 {llvm.noundef}) -> (i32 {llvm.noundef}) attributes {memory = #llvm.memory_effects<other = none, argMem = none, inaccessibleMem = none>, no_unwind, passthrough = ["mustprogress", "nofree", "norecurse", "nosync", ["uwtable", "2"], ["min-legal-vector-width", "0"], ["no-trapping-math", "true"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cmov", "+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, tune_cpu = "generic", will_return} {
    %0 = arith.remsi %arg0, %arg1  : i32
    llvm.return %0 : i32
  }

  // CHECK-LABEL: @test_unsigned_rem
  // CHECK-NEXT:     %0 = arith.divui %arg0, %arg1 : i32
  // CHECK-NEXT:     %1 = arith.muli %0, %arg1 : i32
  // CHECK-NEXT:     %2 = arith.subi %arg0, %1 : i32
  // CHECK-NEXT:     llvm.return %2 : i32
  // CHECK-NEXT:   }
  // CHECK-NEXT: }
  llvm.func local_unnamed_addr @test_unsigned_rem(%arg0: i32 {llvm.noundef}, %arg1: i32 {llvm.noundef}) -> (i32 {llvm.noundef}) attributes {memory = #llvm.memory_effects<other = none, argMem = none, inaccessibleMem = none>, no_unwind, passthrough = ["mustprogress", "nofree", "norecurse", "nosync", ["uwtable", "2"], ["min-legal-vector-width", "0"], ["no-trapping-math", "true"], ["stack-protector-buffer-size", "8"], ["target-cpu", "x86-64"]], target_cpu = "x86-64", target_features = #llvm.target_features<["+cmov", "+cx8", "+fxsr", "+mmx", "+sse", "+sse2", "+x87"]>, tune_cpu = "generic", will_return} {
    %0 = arith.remui %arg0, %arg1  : i32
    llvm.return %0 : i32
  }
}
