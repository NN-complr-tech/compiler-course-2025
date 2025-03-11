// RUN: %clang_cc1 -load %llvmshlibdir/CastCounter_DrozhdinovD_FIIT1_ClangAST%pluginext -plugin CastCounter %s -fsyntax-only 2>&1 | FileCheck %s

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
