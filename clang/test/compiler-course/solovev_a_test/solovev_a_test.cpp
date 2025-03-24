// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_1_Solovev_a_FIIT1_ClangAST%pluginext -plugin ClangAST_1_Solovev_a_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s

// CHECK: int global_a = 0;
// CHECK-NEXT: static int static_global_b = 3;
// CHECK-NEXT: int sum(int param_a, int param_b) {
// CHECK-NEXT:   static int static_d = param_a + param_b;
// CHECK-NEXT:   return static_d;
// CHECK-NEXT: }
// CHECK-NEXT: int minus(int param_c) {
// CHECK-NEXT:   int local_d = -param_c;
// CHECK-NEXT:   return local_d;
// CHECK-NEXT: }
// CHECK-NEXT: void func_1(int param_val_1, int param_val_2, int param_val_3) {
// CHECK-NEXT:   int local_mult = param_val_1 * param_val_2;
// CHECK-NEXT:   for (int local_i = 0; local_i < local_mult; local_i++) {
// CHECK-NEXT:     param_val_3 += local_i;
// CHECK-NEXT:   }
// CHECK-NEXT: }
// CHECK-NEXT: void func_2(int param_val_4, int param_val_5) {
// CHECK-NEXT:   static int static_x = param_val_4 / param_val_5;
// CHECK-NEXT:   static int static_y = param_val_5 / param_val_4;
// CHECK-NEXT:   int local_res = static_x + static_y;
// CHECK-NEXT: }

int a = 0;
static int b = 3;
int sum(int a, int b) {
  static int d = a + b;
  return d;
}
int minus(int c) {
  int d = -c;
  return d;
}
void func_1(int val_1, int val_2, int val_3) {
  int mult = val_1 * val_2;
  for (int i = 0; i < mult; i++) {
    val_3 += i;
  }
}
void func_2(int val_4, int val_5) {
  static int x = val_4 / val_5;
  static int y = val_5 / val_4;
  int res = x + y;
}
