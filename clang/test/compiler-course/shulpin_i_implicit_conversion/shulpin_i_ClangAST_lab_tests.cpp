// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_Lab_ShulpinIlya_FIIT1_ClangAST%pluginext -plugin ClangAST_Lab_ShulpinIlya_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s

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

// CHECK: Function structConversion
// CHECK-NEXT: const Base -> Base: 1
// CHECK-NEXT: const struct Derived -> const struct Base: 1

struct Base {};
struct Derived : Base {};

void structConversion() {
    Derived d;
    Base b = d;
}

// CHECK: Function testPointers
// CHECK-NEXT: char * -> void *: 1
// CHECK-NEXT: int * -> void *: 1

void testPointers() {
    char* cptr;
    int* iptr;
    void* v1 = cptr;
    void* v2 = iptr;
}

// CHECK: Function foo
// CHECK-NEXT: int -> float: 1
// CHECK-NEXT: float -> int: 1
// CHECK-NEXT: int * -> void *: 1

using Abra = float;
using Kadabra = int;

void Alakazam(void*);

void foo() {
    Abra x = Kadabra();
    Kadabra y = x;
    Alakazam(&y);
}

// CHECK: Function createX
// CHECK-NEXT: double -> X: 1

class X {
    double x;
public:
    X(double val) : x(val) {}
};

X createX() {
    return 10.0;
}

// CHECK: Total implicit conversions: 13