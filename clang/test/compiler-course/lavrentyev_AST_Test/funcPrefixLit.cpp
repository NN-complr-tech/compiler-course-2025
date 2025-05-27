// RUN: %clang_cc1 -load %llvmshlibdir/FuncPrefixPlugin_Lavrentyev_Alexey_FIIT3_ClangAST%pluginext -plugin prefix-renamer -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s

// CHECK: int global_var1 = 0;
// CHECK-NEXT: int foo(int param_a, int param_b) {
// CHECK-NEXT:   static int static_var2 = 0;
// CHECK-NEXT:   int local_var3 = 123;
// CHECK-NEXT:   ++static_var2;
// CHECK-NEXT:   return param_a + param_b + global_var1 + static_var2 + local_var3;
// CHECK-NEXT: }
// CHECK-NEXT: int static global_var4 = 3;
// CHECK-NEXT: int static global_var5 = foo(1, global_var4);
// CHECK-NEXT: unsigned global_global_var = 0;

int var1 = 0;
int foo(int a, int b) {
    static int var2 = 0;
    int var3 = 123;
    ++var2;
    return a + b + var1 + var2 + var3;
}
int static var4 = 3;
int static var5 = foo(1, var4);
unsigned global_var = 0;

// RUN: %clang_cc1 -load %llvmshlibdir/FuncPrefixPlugin_Lavrentyev_Alexey_FIIT3_ClangAST%pluginext -plugin prefix-renamer -plugin-arg-prefix-renamer --static=st_ -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s --check-prefix=CHECK-STATIC

// CHECK-STATIC: int foo(int param_a, int param_b) {
// CHECK-STATIC-NEXT: static int st_var2 = 0;
// CHECK-STATIC-NEXT: int local_var3 = 123;
// CHECK-STATIC-NEXT: ++st_var2;
// CHECK-STATIC-NEXT: return param_a + param_b + global_var1 + st_var2 + local_var3;
// CHECK-STATIC-NEXT: }

int goo(int a, int b) {
    static int var2 = 0;
    int var3 = 123;
    ++var2;
    return a + b + var1 + var2 + var3;
}

// RUN: %clang_cc1 -load %llvmshlibdir/FuncPrefixPlugin_Lavrentyev_Alexey_FIIT3_ClangAST%pluginext -plugin prefix-renamer -plugin-arg-prefix-renamer --global=gl_ -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s --check-prefix=CHECK-GLOBAL

// CHECK-GLOBAL: int static gl_variable = 3;

int static variable = 3;

// RUN: %clang_cc1 -load %llvmshlibdir/FuncPrefixPlugin_Lavrentyev_Alexey_FIIT3_ClangAST%pluginext -plugin prefix-renamer -plugin-arg-prefix-renamer --local=lc_ -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s --check-prefix=CHECK-LOCAL

// CHECK-LOCAL: int moo(int param_a, int param_b) {
// CHECK-LOCAL-NEXT: int lc_var3 = 123;
// CHECK-LOCAL-NEXT: return param_a + param_b + lc_var3;
// CHECK-LOCAL-NEXT: }

int moo(int a, int b) {
    int var3 = 123;
    return a + b + var3;
}

// RUN: %clang_cc1 -load %llvmshlibdir/FuncPrefixPlugin_Lavrentyev_Alexey_FIIT3_ClangAST%pluginext -plugin prefix-renamer -plugin-arg-prefix-renamer --param=p_ -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s --check-prefix=CHECK-PARAM

// CHECK-PARAM: int params(int p_a, int p_b) {
// CHECK-PARAM-NEXT: return p_a + p_b;
// CHECK-PARAM-NEXT: }

int params(int a, int b) {
    return a + b;
}