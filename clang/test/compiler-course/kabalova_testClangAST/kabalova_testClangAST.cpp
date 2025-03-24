
// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_Kabalova_Valeria_FIIT1_ClangAST%pluginext -plugin ClangAST_Kabalova_Valeria_FIIT1_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// CHECK:\[\[maybe_unused\]\] int f = 0;

// CHECK:int foo(int a, int b, \[\[maybe_unused\]\] int c) {
// CHECK-NEXT:\[\[maybe_unused\]\] double value = 0.0;
// CHECK-NEXT: double tmp = 1.0;
// CHECK-NEXT: return a + b + tmp;
// CHECK: \[\[maybe_unused\]\] int d = 0;

int f = 0;

int example(int a, int b, int c) {
    double value = 0.0;
    double tmp = 1.0;
    return a + b + tmp;
}

[[maybe_unused]] int d = 0;

// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_Kabalova_Valeria_FIIT1_ClangAST%pluginext -plugin ClangAST_Kabalova_Valeria_FIIT1_ClangAST -plugin-arg-ClangAST_Kabalova_Valeria_FIIT1_ClangAST --help %s -fsyntax-only 2>&1 | FileCheck %s 

// CHECK: This plugin searches for unused variables or unused parameters of functions and marks them with attribute \[\[maybe__unused\]\]
