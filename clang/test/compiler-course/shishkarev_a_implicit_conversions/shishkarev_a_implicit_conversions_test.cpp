// RUN: %clang_cc1 -load %llvmshlibdir/Implicit_Conv_Kudryashova_Irina_FIIT3_ClangAST%pluginext -plugin ImplicitConversionCounterPlugin %s -fsyntax-only 2>&1 | FileCheck %s

// CHECK: Function `sum`
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1

double sum(int a, float b) {
	return a + b;
}

// CHECK-NEXT: Function `mul`
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1

int mul(float a, float b) {
	return a + sum(a, b);
}

// CHECK: Function `one`
// CHECK-NEXT: int* -> void*: 1
void one() {
    int x;
    void* v = &x;
}

// CHECK: Function `two`
// CHECK-NEXT: int* -> bool: 1
void two(int* x) {
    if (x) {}
}

// CHECK: Function `three`
// CHECK-NEXT: int -> bool: 1
void three() {
    int x = 7;
    bool y = x;
}

// CHECK: Total implicit conversions: 10