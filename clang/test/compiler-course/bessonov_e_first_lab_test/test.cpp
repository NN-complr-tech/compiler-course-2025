// RUN: %clang_cc1 -load %llvmshlibdir/first_lab_Bessonov_Egor_FIIT2_ClangAST%pluginext -plugin bessonov_e_unused_var_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: \[\[maybe_unused\]\] int unused_global = -1;
int unused_global = -1;

// CHECK: int foo(int a, int b, \[\[maybe_unused\]\] int c) {
// CHECK:   \[\[maybe_unused\]\] double temp = 0.0;
// CHECK:   return a + b;
int foo(int a, int b, int c) {
    double temp = 0.0;
    return a + b;
}

// CHECK: double multiply(double x, double y, \[\[maybe_unused\]\] double z = 5.0) {
// CHECK:   \[\[maybe_unused\]\] double result = x * y;
// CHECK:   return x + y;
double multiply(double x, double y, double z = 5.0) {
    double result = x * y;
    return x + y;
}

// CHECK: void process(int p, \[\[maybe_unused\]\] int q, int r) {
// CHECK:   \[\[maybe_unused\]\] int local = p + r;
// CHECK:   int used = p * r;
void process(int p, int q, int r) {
    int local = p + r;
    int used = p * r;
}