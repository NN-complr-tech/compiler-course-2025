// RUN: %clang_cc1 -load %llvmshlibdir/PrefixesPlugin_KholinKirill_FIIT3_ClangAST%pluginext -plugin PrefixesPlugin_KholinKirill_FIIT3_ClangAST %s -fsyntax-only 2>&1 | FileCheck -v %s

// CHECK: Found variable: var1 -> global_var1 at 18:5
// CHECK-NEXT: Found variable: var3 -> global_var3 at 19:14
// CHECK-NEXT: Found variable: ll -> global_ll at 20:21
// CHECK-NEXT: Found variable: a -> param_a at 21:13
// CHECK-NEXT: Found variable: b -> param_b at 21:20
// CHECK-NEXT: Found variable: var2 -> static_var2 at 22:14
// CHECK-NEXT: Found variable: var3 -> local_var3 at 23:7
// CHECK-NEXT: Found variable: var2 -> static_var2 at 24:5
// CHECK-NEXT: Found variable: a -> param_a at 25:10
// CHECK-NEXT: Found variable: b -> param_b at 25:14
// CHECK-NEXT: Found variable: var1 -> global_var1 at 25:18
// CHECK-NEXT: Found variable: var2 -> static_var2 at 25:25
// CHECK-NEXT: Found variable: var3 -> local_var3 at 25:32
// CHECK-NEXT: Found variable: ll -> global_ll at 25:39

int var1 = 0;
extern float var3;
constexpr long long ll = 1L;
int foo(int a, int b) {
  static int var2 = 0;
  int var3 = 123;
  ++var2;
  return a + b + var1 + var2 + var3 + ll;
}
