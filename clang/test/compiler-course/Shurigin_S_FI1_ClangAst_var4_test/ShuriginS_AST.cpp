// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_1_ShuriginS_FIIT1_ClangAST%pluginext -plugin ClangAST_1_ShuriginS_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s

// CHECK-NEXT: int global_var1 = 0;
// CHECK-NEXT: int foo(int param_a, int param_b) {
// CHECK-NEXT:   static int static_var2 = 0;
// CHECK-NEXT:   int local_var3 = 123;
// CHECK-NEXT:   ++static_var2;
// CHECK-NEXT:   return param_a + param_b + global_var1 + static_var2 + local_var3;
// CHECK-NEXT: }

int var1 = 0;

int foo(int a, int b) {
  static int var2 = 0;
  int var3 = 123;
  ++var2;
  return a + b + var1 + var2 + var3;
}

