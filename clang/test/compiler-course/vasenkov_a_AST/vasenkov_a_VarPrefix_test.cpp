
// RUN: %clang_cc1 -load %llvmshlibdir/VarPrefixPlugin_VasenkovAndrey_FIIT1_ClangAST%pluginext -plugin VarPrefixPlugin_VasenkovAndrey_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: int global_var1 = 10;
// CHECK-NEXT: int foo(int param_a, int param_b){
// CHECK-NEXT:    int local_var2 = 0;
// CHECK-NEXT:    static int static_var3 = 23;
// CHECK-NEXT:    ++local_var2;
// CHECK-NEXT:    return param_a + param_b + local_var2 + static_var3;
// CHECK-NEXT: }
// CHECK-NEXT: static int global_var4 = foo(global_var1, 1);
// CHECK-NEXT: extern double global_vard = 0;


int var1 = 10;
int foo(int a, int b){
    int var2 = 0;
    static int var3 = 23;
    ++var2;
    return a + b + var2 + var3;
}
static int var4 = foo(var1, 1);
extern double vard = 0;