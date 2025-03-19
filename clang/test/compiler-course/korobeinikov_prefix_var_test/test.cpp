// RUN: %clang_cc1 -load %llvmshlibdir/PrefixVarPlugin_Korobeinikov_FIIT1_ClangAST%pluginext -plugin PrefixVarPlugin_Korobeinikov_FIIT1_ClangAST %s -fsyntax-only 2>&1 | FileCheck --match-full-lines %s

// CHECK: int global_var1 = 0;
// CHECK-NEXT: int global_var2 = 2;
// CHECK-NEXT: int foo(int param_a, int param_b) {
// CHECK-NEXT:   static int static_var3 = 0;
// CHECK-NEXT:   int local_var4 = 123;
// CHECK-NEXT:   ++static_var3;
// CHECK-NEXT:   return param_a + param_b + global_var1 + static_var3 + local_var4;
// CHECK-NEXT: }

int var1 = 0;
int static var2 = 2;
int foo(int a, int b) {
    static int var3 = 0;
    int var4 = 123;
    ++var3;
    return a + b + var1 + var3 + var4;
}

// RUN: %clang_cc1 -load %llvmshlibdir/PrefixVarPlugin_Korobeinikov_FIIT1_ClangAST%pluginext -plugin PrefixVarPlugin_Korobeinikov_FIIT1_ClangAST -plugin-arg-PrefixVarPlugin_Korobeinikov_FIIT1_ClangAST --help -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s --check-prefix=CHECK-HELP-ARG
// CHECK-HELP: PrefixVarPlugin_by_Korobeinikov_Arseny:
// CHECK-HELP-NEXT: This plugin changes names by adding prefixes to variables and parameters.