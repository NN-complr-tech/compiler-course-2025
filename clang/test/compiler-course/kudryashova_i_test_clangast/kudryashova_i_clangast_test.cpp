// RUN: %clang_cc1 -load %llvmshlibdir/Implicit_Conv_Kudryashova_Irina_FIIT3_ClangAST%pluginext -plugin ImplicitConvPlugin %s -fsyntax-only 2>&1 | FileCheck %s

// CHECK: Function `sum`
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1

// CHECK-NEXT: Function `mul`
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1

// CHECK Total implicit conversions: 5

double sum(int a, float b) {
	return a + b;
}

int mul(float a, float b) {
	return a + sum(a, b);
}
