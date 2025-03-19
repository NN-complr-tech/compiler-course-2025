// RUN: %clang_cc1 -load %llvmshlibdir/UnusedVariables_Savchenko_Maxim_FIIT1_ClangAST%pluginext -plugin savchenko_m_UnusedVars_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: double used = 3.0;
// CHECK: \[\[maybe_unused\]\] double unused = 2.0;
double used = 3.0;
double unused = 2.0;

// CHECK: int foo1(int a, int b, \[\[maybe_unused\]\] int c) {
// CHECK: \[\[maybe_unused\]\] int tmp = 47;
// CHECK: int x = 24;
// CHECK: int y = 54;
// CHECK: \[\[maybe_unused\]\] int xy = x + y;
// CHECK: return a + b;
int foo1(int a, int b, int c) {
    int tmp = 47;
    int x = 24;
    int y = 54;
    int xy = x + y;
    return a + b;
}

// CHECK: double mult(double a, double b, \[\[maybe_unused\]\] double c) {
// CHECK: \[\[maybe_unused\]\] double result = a * b;
// CHECK: return a * b * used;
double mult(double a, double b, double c) {
    double result = a * b;
    return a * b * used;
}