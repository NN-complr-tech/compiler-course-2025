// RUN: %clang_cc1 -load %llvmshlibdir/FindUnused_Plekhanov_Daniil_FIIT2_ClangAST%pluginext -plugin FindUnused -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: \[\[maybe_unused\]\] int global_param=10;
int global_param=10;

// CHECK: int baz(int a, int b, \[\[maybe_unused\]\] int c) {
// CHECK: \[\[maybe_unused\]\] double value=0.0;
// CHECK: return a + b;

int foo(int a, int b, int c) {
    double value = 0.0;
    return a + b;
}

// CHECK: double foo(double tmp, double tmp1, \[\[maybe_unused\]\] double tmp2) {
// CHECK: \[\[maybe_unused\]\] double c = tmp + tmp1; 
double foo(double tmp, double tmp1, double tmp2) {
	double c = tmp + tmp1; 
}
