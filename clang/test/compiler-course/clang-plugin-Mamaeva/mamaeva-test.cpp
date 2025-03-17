// RUN: %clang_cc1 -load %llvmshlibdir/myClangPlugin_Mamaeva_Olga_FIIT3_ClangAST%pluginext -plugin myClangPlugin -fsyntax-only %s 2>&1 | FileCheck %s -dump-input=always

// CHECK: In testing
// CHECK-NEXT: int -> float: 1
float testing = 7;

// CHECK: In function: sum
// CHECK-NEXT: int -> float: 1
// CHECK-NEXT: float -> double: 1

double sum(int a, float b) {
    return a + b;
}

// CHECK: In function: mul
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: double -> int: 1

int mul(float a, float b) {
    return a + sum(a, b);
}

// CHECK: Total implicit conversions: 6
