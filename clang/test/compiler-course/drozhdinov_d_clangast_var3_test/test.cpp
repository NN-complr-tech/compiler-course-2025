// RUN: %clang_cc1 -load %llvmshlibdir/CastCounter_DrozhdinovD_FIIT1_ClangAST%pluginext -plugin CastCounter_DrozhdinovD_FIIT1_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s
using Abrakadabra = int;
#define int Abrakadabra

// CHECK: Function sum
// CHECK-NEXT: Abrakadabra -> float: 1
// CHECK-NEXT: float -> double: 1

// CHECK: Function mul
// CHECK-NEXT: double -> Abrakadabra: 1
// CHECK-NEXT: float -> Abrakadabra: 1
// CHECK-NEXT: float -> double: 1

double sum(int a, float b) {
	return a + b;
}

int mul(float a, float b) {
	return a + sum(a, b);
}

// CHECK: Function func
// CHECK-NEXT: Abrakadabra -> double: 1
// CHECK-NEXT: _Bool -> Abrakadabra: 1
// CHECK-NEXT: double -> _Bool: 2

void func() {
	bool b = 3.14;
	bool bb = 0.00;
	int x = b;
	double d = x;
}

// CHECK: Function foo
// CHECK-NEXT: Abrakadabra * -> const Abrakadabra *: 1

void bar(const int*);
void foo() {
	int x = 42;
	bar(&x);
}

// CHECK: Function createX
// CHECK-NEXT; Abrakadabra -> X: 1

class X {
	int x;
public:
	X(int val) : x(val) {}
};


X createX() {
	return 10;
}
