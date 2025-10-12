// RUN: %clang_cc1 -load %llvmshlibdir/ImplicitConversion_Polyakov_Alexey_FIIT2_ClangAST%pluginext -plugin ImplicitConversion_Polyakov_Alexey_FIIT2_ClangAST -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: Function: sum
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1
double sum(int a, float b) {
    return a + b; // 'a' (int) -> 'b' (float) -> 'double'
}

// CHECK: Function: mul
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1
int mul(float a, float b) {
    return a + sum(a, b);
}
