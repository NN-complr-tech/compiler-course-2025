// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_1_ShuriginS_FIIT1_ClangAST%pluginext -plugin ClangAST_1_ShuriginS_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s

// CHECK: int static global_Var1 = 0;
// CHECK: extern int global_Var3;
// CHECK: int rA(int param_A) {
// CHECK:   return -param_A;
// CHECK: }
// CHECK: int static foo(int param_A, int param_B) {
// CHECK:   static int static_Var2 = 0;
// CHECK:   int local_Var3 = 123;
// CHECK:   int &local_rA = param_A;
// CHECK:   ++static_Var2;
// CHECK:   return local_rA + param_B + global_Var1 + static_Var2 + local_Var3;
// CHECK: }
// CHECK: int static global_X = foo(1, rA(global_Var3));

int static Var1 = 0;

extern int Var3;

int rA(int A) {
  return -A;
}

int static foo(int A, int B) {
  static int Var2 = 0;
  int Var3 = 123;
  int &rA = A;
  ++Var2;
  return rA + B + Var1 + Var2 + Var3;
}

int static X = foo(1, rA(Var3));


