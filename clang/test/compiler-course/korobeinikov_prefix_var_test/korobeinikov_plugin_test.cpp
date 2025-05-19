// RUN: %clang_cc1 -load %llvmshlibdir/PrefixVarPlugin_Korobeinikov_Arseny_FIIT1_ClangAST%pluginext -plugin PrefixVarPlugin_Korobeinikov_Arseny_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s

// CHECK: int global_var1 = 0;
// CHECK-NEXT: int foo(int param_a, int param_b) {
// CHECK-NEXT:   static int static_var2 = 0;
// CHECK-NEXT:   int local_var3 = 123;
// CHECK-NEXT:   ++static_var2;
// CHECK-NEXT:   return param_a + param_b + global_var1 + static_var2 + local_var3;
// CHECK-NEXT: }
// CHECK-NEXT: int static global_var4 = 0;

int var1 = 0;
int foo(int a, int b) {
    static int var2 = 0;
    int var3 = 123;
    ++var2;
    return a + b + var1 + var2 + var3;
}
int static var4 = 0;

// RUN: %clang_cc1 -load %llvmshlibdir/PrefixVarPlugin_Korobeinikov_Arseny_FIIT1_ClangAST%pluginext -plugin PrefixVarPlugin_Korobeinikov_Arseny_FIIT1_ClangAST -plugin-arg-PrefixVarPlugin_Korobeinikov_Arseny_FIIT1_ClangAST --help -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s --check-prefix=CHECK-HELP
// CHECK-HELP: PrefixVarPlugin_by_Korobeinikov_Arseny:
// CHECK-HELP-NEXT: This plugin changes names by adding prefixes to variables and parameters.