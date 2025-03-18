// RUN: %clang_cc1 -load %llvmshlibdir/unUsedVarPlugin_LysovIvan_FIIT3_ClangAST%pluginext -plugin unUsedVarPlugin_LysovIvan_FIIT3_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// CHECK: Marking variable as maybe_unused: c
// CHECK-NEXT: Marking variable as maybe_unused: value
int foo(int a, int b, int c) {
    double value = 0.0;
    return a + b;
}

// CHECK: Marking variable as maybe_unused: unused
// CHECK-NEXT: Marking variable as maybe_unused: x
void test_params(int used, int unused) {
    int x = used;
}

// CHECK: Marking variable as maybe_unused: globalVar
int globalVar = 6;