// RUN: %clang_cc1 -load %llvmshlibdir/CastCounter_DrozhdinovD_FIIT1_ClangAST%pluginext -plugin CastCounter_DrozhdinovD_FIIT1_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s
using Abrakadabra = int;
#define int Abrakadabra

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

// CHECK: Function func
// CHECK-NEXT: _Bool -> int: 1
// CHECK-NEXT: double -> _Bool: 2
// CHECK-NEXT: int -> double: 1

void func() {
	bool b = 3.14;
	bool bb = 0.00;
	int x = b;
	double d = x;
}

// CHECK: Function foo
// CHECK-NEXT: int * -> const int *: 1

void bar(const int*);
void foo() {
	int x = 42;
	bar(&x);
}

// CHECK: Function createX
// CHECK-NEXT: int -> X: 1

class X {
	int x;
public:
	X(int val) : x(val) {}
};


X createX() {
	return 10;
}
