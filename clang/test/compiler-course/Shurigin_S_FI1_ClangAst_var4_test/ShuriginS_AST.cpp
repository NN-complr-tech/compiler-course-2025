// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_1_ShuriginS_FIIT1_ClangAST%pluginext -plugin ClangAST_1_ShuriginS_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s

// CHECK: int global_Var1 = 0;
// CHECK-NEXT: int foo(int param_a, int param_b) {
// CHECK-NEXT:   static int static_Var2 = 0;
// CHECK-NEXT:   int local_Var3 = 123;
// CHECK-NEXT:   ++static_Var2;
// CHECK-NEXT:   return param_a + param_b + global_Var1 + static_Var2 + local_Var3;
// CHECK-NEXT: }

int Var1 = 0;

int foo(int a, int b) {
  static int Var2 = 0;
  int Var3 = 123;
  ++Var2;
  return a + b + Var1 + Var2 + Var3;
}

