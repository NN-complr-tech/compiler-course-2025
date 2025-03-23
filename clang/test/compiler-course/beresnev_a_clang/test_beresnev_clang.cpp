// RUN: %clang_cc1 -load %llvmshlibdir/UserDataTypePlugin_beresnev_a_FIIT1_ClangAST%pluginext -plugin UserDataTypePlugin_beresnev_a_clang -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: int foo(int a, int b, /[/[maybe_unused/]/] int c) {
// CHECK: /[/[maybe_unused/]/] double value=0.0;
// CHECK: return a + b;

int foo(int a, int b, int c) {
    double value = 0.0;
    return a + b;
}