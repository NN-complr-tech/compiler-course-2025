// RUN: %clang_cc1 -load %llvmshlibdir/Lab1_AST_Mironov_Arseniy_FIIT1_ClangAST%pluginext -plugin Lab1_AST_Mironov_Arseniy_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck %s -dump-input=always


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

// CHECK: In function: test_casts1
// CHECK-NEXT: double -> _Bool
// CHECK-NEXT: double -> float
// CHECK-NEXT: double -> int
// CHECK-NEXT: int -> _Bool: 1
// CHECK-NEXT: int -> double: 1

void test_casts1(){
	bool f1 = 1;
	bool f2 = 1.0;
	int x = 1.0;
	float y = 1.0;
	double c = 1;
}

// CHECK: In function: test_casts2
// CHECK-NEXT: _Bool -> char: 1
// CHECK-NEXT: _Bool -> double: 2
// CHECK-NEXT: _Bool -> float: 1
// CHECK-NEXT: _Bool -> int: 1

void test_casts2(){
	int c = true;
	double c1 = true;
	double c2 = false;
	float c3 = false;
	char c4= true;
}