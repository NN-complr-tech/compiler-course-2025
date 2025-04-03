// RUN: %clang_cc1 -load %llvmshlibdir/Prefix_Plugin_Ivanov_Mikhail_FIIT1_ClangAST%pluginext -plugin Prefix_Plugin_Ivanov_Mikhail_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s -dump-input=always

// CHECK: int global_var1 = 1;
// CHECK-NEXT: int foo(int param_a, int param_b){
// CHECK-NEXT:    int local_var2 = 0;
// CHECK-NEXT:    static int static_var3 = 10;
// CHECK-NEXT:    ++local_var2;
// CHECK-NEXT:    return param_a + param_b;
// CHECK-NEXT: }
// CHECK-NEXT: static int global_var4 = foo(global_var1, 10);
// CHECK-NEXT: extern double global_dvar1 = 0;
// CHECK-NEXT: static double global_dvar2 = 1;

int var1 = 1;
int foo(int a, int b){
    int var2 = 0;
    static int var3 = 10;
    ++var2;
    return a + b;
}
static int var4 = foo(var1, 10);
extern double dvar1 = 0;
static double dvar2 = 1;
