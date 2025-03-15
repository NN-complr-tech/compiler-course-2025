// RUN: %clang_cc1 -load %llvmshlibdir/myClangPlugin_mamaeva_olga_FIIT3_ClangAST%pluginext -plugin myClangPlugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: Function sum
// CHECK-NEXT: int -> float: 1
// CHECK-NEXT: float -> double: 1
double sum(int a, float b) {
    return a + b;
}

// CHECK: Function mul
// CHECK-NEXT: float -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: double -> int: 1
int mul(float a, float b) {
    return a + sum(a, b);
}
