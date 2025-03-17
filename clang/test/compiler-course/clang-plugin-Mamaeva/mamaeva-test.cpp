// RUN: %clang_cc1 -load %llvmshlibdir/myClangPlugin_mamaeva_olga_FIIT3_ClangAST%pluginext -plugin myClangPlugin -fsyntax-only %s 2>&1 | FileCheck %s -dump-input=always

// CHECK: In testing
// CHECK-NEXT: int -> float: 1
float testing = 7;


// CHECK: In function: mul
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1

// CHECK: In function: sum
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1

double sum(int a, float b) {
	return a + b;
}

int mul(float a, float b) {
	return a + sum(a, b);
}
