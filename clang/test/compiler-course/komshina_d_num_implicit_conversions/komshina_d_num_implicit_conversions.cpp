// RUN: %clang_cc1 -load %llvmshlibdir/Implicit_Conversions_Komshina_Daria_FIIT1_ClangAST%pluginext -plugin Implicit_Conversions_Komshina_Daria_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck %s
// CHECK: Function sum
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1

double sum(int a, float b) {
    return a + b;
}

// CHECK: Function mul
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1

int mul(float a, float b) {
    return a + sum(a, b);
}

// CHECK: Function createX
// CHECK-NEXT: int -> class X: 1

class X {
    int x;
public:
    X(int val) : x(val) {}
};

X createX() {
    return 10;
}

// CHECK: Function foo
// CHECK-NEXT: int -> float: 1
// CHECK-NEXT: float -> int: 1

using Abrakadabra = float;
using Boom = int;

void foo() {
    Abrakadabra x = Boom();
    Boom y = x;
}

// CHECK: Function boo
// CHECK-NEXT: int -> _Bool: 1

void boo() {
    int x = 3;
    bool b = x;
}

// CHECK: Function goo
// CHECK-NEXT: int * -> void *: 1

void loo(void*);

void goo() {
    int x = 20;
    loo(&x);
}

// CHECK: Total implicit conversions: 10
