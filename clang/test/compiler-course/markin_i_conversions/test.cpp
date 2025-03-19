// RUN: %clang_cc1 -load %llvmshlibdir/ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST%pluginext -plugin ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// CHECK: Function `sum`
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1
// CHECK: Function `mul`
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1

double sum(int a, float b) {
	return a + b;
  }
  
  int mul(float a, float b) {
	return a + sum(a, b);
  }

// CHECK: Function `constructors`
// CHECK-NEXT: char -> int: 1
// CHECK-NEXT: int -> double: 1

void constructors() {
  double d = int(5); // Конструктор int -> double
  int i = 'a';        // Конструктор char -> int
}

// CHECK: Function `mixed_types`
// CHECK-NEXT: long -> double: 1
// CHECK-NEXT: short -> int: 1

void mixed_types(long l, short s) {
  double d = l;  // long -> double
  int i = s;     // short -> int
}

// CHECK: Function `return_types`
// CHECK-NEXT: double -> float: 1
// CHECK: Function `return_int`
// CHECK-NEXT: int -> double: 1

float return_types(double d) {
  return d; // double -> float
}

double return_int(int i) {
    return i; // int -> double
}

// CHECK: Function `setValue`
// CHECK-NEXT: float -> double: 1
// CHECK: Function `getValue`
// CHECK-NEXT: double -> int: 1

class MyClass {
public:
  void setValue(float f) {
    value = f; // float -> double
  }
  int getValue() {
    return value; // double -> int
  }
private:
  double value;
};