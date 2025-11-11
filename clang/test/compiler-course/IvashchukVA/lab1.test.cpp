// RUN: %clang -fplugin=%llvmshlibdir/IvashchukVA_lab1%pluginext -Xclang -plugin -Xclang IvashchukVA_lab1_2var %s -S -emit-llvm -o - | FileCheck %s

void test_function(int unused, int used) {
  [[maybe_unused]] int unused_var = 42;
  int used_var = 43;
}

// CHECK: @_Z13test_functioni