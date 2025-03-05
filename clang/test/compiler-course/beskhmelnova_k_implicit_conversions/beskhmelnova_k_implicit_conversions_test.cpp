// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_1_BeskhmelnovaK_FIIT1_ClangAST%pluginext -plugin ClangAST_1_BeskhmelnovaK_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s

// CHECK Total implicit conversions: 5

// Проверка порядка выводимых строк:
// CHECK: Function sum
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1
// CHECK: Function mul
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1

double sum(int a, float b) {
    return a + b;
}

int mul(float a, float b) {
    return a + sum(a, b);
}