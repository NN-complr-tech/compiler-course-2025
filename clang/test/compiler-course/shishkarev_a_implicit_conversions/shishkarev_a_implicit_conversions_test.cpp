// RUN: %clang_cc1 -load %llvmshlibdir/ImplicitConversionCounter_Shishkarev_Andrey_FIIT2_ClangAST%pluginext -plugin implicit_conversion_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: Function `sum`
// CHECK-NEXT: int -> float: 1
// CHECK-NEXT: float -> double: 1

// CHECK: Function `mul`
// CHECK-NEXT: float -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: double -> int: 1

double sum(int a, float b) {
    return a + b;
}

int mul(float a, float b) {
    return a + sum(a, b);
}