// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_1_Solovev_a_FIIT1_ClangAST%pluginext -plugin ClangAST_1_Solovev_a_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s

// CHECK: int global_a = 0;
// CHECK-NEXT: int global_b = 3;
// CHECK-NEXT: int sum(int param_a, int param_b) {
// CHECK-NEXT:   static int static_d = param_a + param_b;
// CHECK-NEXT:   return static_d;
// CHECK-NEXT: }
// CHECK-NEXT: int minus(int param_c) {
// CHECK-NEXT:   int local_d = -param_c;
// CHECK-NEXT:   return local_d;
// CHECK-NEXT: }

int a = 0;
int b = 3;
int sum(int a, int b) {
  static int d = a + b;
  return d;
}
int minus(int c) {
  int d = -c;
  return d;
}
