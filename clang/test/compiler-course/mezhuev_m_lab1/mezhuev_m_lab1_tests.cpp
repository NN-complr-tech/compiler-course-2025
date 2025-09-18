// RUN: %clang_cc1 -load %llvmshlibdir/clangAstPrefix_1_MezhuevM_FIIT2_ClangAST%pluginext -plugin clangAstPrefix_1 -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: int static global_Var1 = 0;
// CHECK-NEXT: extern int global_Var3;
// CHECK-NEXT: int rA(int param_A) {
// CHECK-NEXT:   return -param_A;
// CHECK-NEXT: }
// CHECK-NEXT: int static foo(int param_A, int param_B) {
// CHECK-NEXT:   static int static_Var2 = 0;
// CHECK-NEXT:   int local_Var3 = 123;
// CHECK-NEXT:   int &local_rA = param_A;
// CHECK-NEXT:   ++static_Var2;
// CHECK-NEXT:   return local_rA + param_B + global_Var1 + static_Var2 + local_Var3;
// CHECK-NEXT: }
// CHECK-NEXT: int static global_X = foo(1, rA(global_Var3));

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