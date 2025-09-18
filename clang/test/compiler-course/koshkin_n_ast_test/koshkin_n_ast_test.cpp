// RUN: %clang_cc1 -load %llvmshlibdir/MaybeUnused_Koshkin_Nikita_FIIT3_ClangAST%pluginext -plugin koshkin_n_MaybeUnused_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: int foo(int a, int b, \[\[maybe_unused\]\] int c) {
// CHECK-NEXT: \[\[maybe_unused\]\] double value = 0.0;
// CHECK-NEXT: return a + b;

int foo(int a, int b, int c) {
	double value = 0.0;
    return a + b;
}
// CHECK: \[\[maybe_unused\]\] int notusd=999;
// CHECK-NEXT:  \[\[maybe_unused\]\] double notusd2=2.42;

int notusd=999;
double notusd2=2.42;

// CHECK: int usdvar = 444; {
// CHECK-NEXT: foo1(\[\[maybe_unused\]\] int a) {
// CHECK-NEXT: usdvar = 0;
// CHECK-NEXT: \[\[maybe_unused\]\] int value;
// CHECK-NEXT: return usedvar;
int usdvar = 444;
int foo1(int a) {
	usdvar = 0;
	int value;
	return usdvar;
}

// CHECK: int globalvar = 999; 
// CHECK-NEXT: \[\[maybe_unused\]\] int gv2 = globalvar + 1;

int globalvar = 999;
int gv2 = globalvar + 1;

