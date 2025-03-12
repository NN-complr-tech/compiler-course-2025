// RUN: %clang_cc1 -load %llvmshlibdir/Implicit_Conv_Kudryashova_Irina_FIIT3_ClangAST%pluginext -plugin ImplicitConvPlugin %s -fsyntax-only 2>&1 | FileCheck %s

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

// CHECK: Function `foo`
// CHECK-NEXT: float -> int: 1
// CHECK-NEXT: int -> float: 1

using Abrakadabra = float;
using Boom = int;

void foo() {
  Abrakadabra x = Boom();
  Boom y = x;
}

// CHECK: Function `moo`
// CHECK-NEXT: int* -> void*: 1
void moo() {
    int x;
    void* vp = &x;
}

// CHECK: Function `goo`
// CHECK-NEXT: int* -> bool: 1
void goo(int* p) {
    if (p) {}
}

// CHECK: Function `boo`
// CHECK-NEXT: int -> bool: 1
void boo() {
    int x = 7;
    bool b = x;
}

// CHECK: Total implicit conversions: 10