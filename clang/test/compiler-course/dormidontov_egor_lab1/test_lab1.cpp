// RUN: %clang_cc1 -load %llvmshlibdir/PrefixCallbackPlugin_Dormidontov_Egor_FIIT2_ClangAST%pluginext -plugin prefix_var_callback_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: int global_var1 = 0;
// CHECK: int foo(int param_a, int param_b) {
// CHECK:   static int static_var2 = 0;
// CHECK:   int local_var3 = 123;
// CHECK:   ++static_var2;
// CHECK:   return param_a + param_b + global_var1 + static_var2 + local_var3;
// CHECK: }
// CHECK: int bar(int param_x) {
// CHECK:   int local_y = 0;
// CHECK:   long long local_var1 = 9LL;
// CHECK:   for (int local_i = 0; local_i < param_x; ++local_i) {
// CHECK:     local_y += global_var1 + param_x + local_i;
// CHECK:   }
// CHECK:   return local_y;
// CHECK: }

int var1 = 0;

int foo(int a, int b) {
  static int var2 = 0;
  int var3 = 123;
  ++var2;
  return a + b + var1 + var2 + var3;
}

int bar(int x) {
  int y = 0;
  long long var1 = 9LL;
  for (int i = 0; i < x; ++i) {
    y += var1 + x + i;
  }
  return y;
}