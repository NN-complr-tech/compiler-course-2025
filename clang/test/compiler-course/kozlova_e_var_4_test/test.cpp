// RUN: %clang_cc1 -load %llvmshlibdir/FirstPlugin_KozlovaEkaterina_FIIT3_ClangAST%pluginext -plugin FirstPlugin_KozlovaEkaterina_FIIT3_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s


// CHECK: int global_var1 = 0;
// CHECK-NEXT: int foo(int param_a, int param_b) {
// CHECK-NEXT:   static int static_var2 = 0;
// CHECK-NEXT:   int local_var3 = 123;
// CHECK-NEXT:   ++static_var2;
// CHECK-NEXT:   return param_a + param_b + global_var1 + static_var2 + local_var3;
// CHECK-NEXT: }
// CHECK-NEXT: int static static_global_var4 = 1;
// CHECK-NEXT: int static static_global_var5 = foo(1, static_global_var4);
// CHECK-NEXT: int foo2(int param_a, int param_b_default_10) {
// CHECK-NEXT: return param_a * param_b_default_10;
// CHECK-NEXT: }
// CHECK-NEXT: int global_var6 = foo2(1);
// CHECK-NEXT: int global_var7 = foo2(1, 2);


int var1 = 0;
int foo(int a, int b) {
    static int var2 = 0;
    int var3 = 123;
    ++var2;
    return a + b + var1 + var2 + var3;
}
int static var4 = 1;
int static var5 = foo(1, var4);
int foo2(int a, int b = 10) {
    return a * b;
}
int var6 = foo2(1);
int var7 = foo2(1, 2);
