// RUN: %clang_cc1 -load %llvmshlibdir/UnusedVars_Grudzin_Konstantin_FIIT1_ClangAST%pluginext -plugin grudzin_k_UnVars_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: \[\[maybe_unused\]\] int ntd=-1;
int ntd=-1;
// CHECK: int baz(int p, int q, \[\[maybe_unused\]\] int r) {
// CHECK: int a=5;
// CHECK: int b=17;
// CHECK: \[\[maybe_unused\]\]int c = a+b;
// CHECK: \[\[maybe_unused\]\] int temp = 42;
// CHECK: return p + q;
int baz(int p, int q, int r) {
    int a=5;
    int b=17;
    int c = a+b;
    int temp = 42;
    return p + q;
  }
  
// CHECK: void qux(double a, double b, \[\[maybe_unused\]\] double c = 5.0) {
// CHECK:   \[\[maybe_unused\]\] double result = a * b;
void qux(double a, double b, double c = 5.0) {
    double result = a * b;
  }
