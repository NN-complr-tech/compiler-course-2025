// RUN: %clang_cc1 -load %llvmshlibdir/ImplicitConversionCounter_Shishkarev_Andrey_FIIT2_ClangAST%pluginext -plugin ImplicitConvPlugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: Function `sum`
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1

// CHECK: Function `mul`
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1

// CHECK: Function `testMixedTypes`
// CHECK-NEXT: int -> float: 1
// CHECK-NEXT: int -> double: 1
// CHECK-NEXT: float -> int: 2


double sum(int a, float b) {
    return a + b;
}

int mul(float a, float b) {
    return a + sum(a, (int)b);
}

int testMixedTypes(int x, float y, double z) {
    float result = x + y;
    int intResult = x + z;
    return intResult + y;
}
