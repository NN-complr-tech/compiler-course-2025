// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_1_BeskhmelnovaK_FIIT1_ClangAST%pluginext -plugin ClangAST_1_BeskhmelnovaK_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s

// Проверка порядка выводимых строк:
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
// CHECK-NEXT: int -> X: 1

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
// CHECK-NEXT: int * -> void *: 1

using Abrakadabra = float;
using Boom = int;

void boo(void*);

void foo() {
  Abrakadabra x = Boom();
  Boom y = x;
  boo(&y);
}

// CHECK: Total implicit conversions: 9
