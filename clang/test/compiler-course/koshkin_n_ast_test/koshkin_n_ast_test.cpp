// RUN: %clang_cc1 -load %llvmshlibdir/MaybeUnused_Koshkin_Nikita_FIIT3_ClangAST%pluginext -plugin koshkin_n_MaybeUnused_plugin 2>&1 | FileCheck %s --help
// CHECK:This plugin marks unused variables with \[\[maybe_unused\]\] attribute

// RUN: %clang_cc1 -load %llvmshlibdir/MaybeUnused_Koshkin_Nikita_FIIT3_ClangAST%pluginext -plugin koshkin_n_MaybeUnused_plugin -fsyntax-only %s 2>&1 | FileCheck %s 

// CHECK: int foo(int a, int b, \[\[maybe_unused\]\] int c) {
// CHECK: \[\[maybe_unused\]\] double value = 0.0;
// CHECK:   return a + b;
int foo(int a, int b, int c) {
	double value = 0.0;
    return a + b;
}

// CHECK: \[\[maybe_unused\]\] int notusd=999;
int notusd=999;

// CHECK: \[\[maybe_unused\]\] double notusd2=2.42;
double notusd2=2.42;