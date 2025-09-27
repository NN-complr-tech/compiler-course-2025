// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_var2_Shpynov_Nikita_FIIT1_ClangAST%pluginext -plugin ClangAST_var2_Shpynov_Nikita_FIIT1_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// CHECK:\[\[maybe_unused\]\] int d = 0;

// CHECK:int foo(int a, int b, \[\[maybe_unused\]\] int c) {
// CHECK-NEXT:\[\[maybe_unused\]\] double value = 0.0;
// CHECK-NEXT: return a + b;

// CHECK: \[\[maybe_unused\]\] int e = 0;
int d = 0;

int foo(int a, int b, int c) {
    double value = 0.0;
    return a + b;
}

int e = 0;
