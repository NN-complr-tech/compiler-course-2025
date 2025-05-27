// RUN: %clang_cc1 -load %llvmshlibdir/ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST%pluginext -plugin ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// CHECK: Function `sum`
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1
// CHECK: Function `mul`
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1

// CHECK: Function `constructors`
// CHECK-NEXT: char -> int: 1
// CHECK-NEXT: int -> double: 1

// CHECK: Function `mixed_types`
// CHECK-NEXT: long -> double: 1
// CHECK-NEXT: short -> int: 1

// CHECK: Function `return_types`
// CHECK-NEXT: double -> float: 1
// CHECK: Function `return_int`
// CHECK-NEXT: int -> double: 1

// CHECK: Function `setValue`
// CHECK-NEXT: float -> double: 1
// CHECK: Function `getValue`
// CHECK-NEXT: double -> int: 1

// CHECK: Function `void_ptr_conversion`
// CHECK-NEXT: int * -> void *: 1

// CHECK: Function `multiple_conversions`
// CHECK-NEXT: float -> int: 1
// CHECK-NEXT: int -> double: 2

// RUN: %clang_cc1 -load %llvmshlibdir/ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST%pluginext -plugin ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST %s -plugin-arg-ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST --help 2>&1 | FileCheck %s --check-prefix=HELP

// HELP: ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST: Counts implicit conversions and constructor calls.
// HELP: Usage: clang -cc1 -load <plugin_path> -plugin ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST <source_file>

// HELP: This plugin counts the number of implicit conversions and constructor calls for each function in the code.

// HELP: Example:
// HELP:   clang++ -cc1 -load ./ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST.so -plugin ConversionsPlugin_Markin_Ivan_FIIT2_ClangAST test.cpp

double sum(int a, float b) {
	return a + b;
  }
  
  int mul(float a, float b) {
	return a + sum(a, b);
  }

void constructors() {
  double d = int(5); // Конструктор int -> double
  int i = 'a';        // Конструктор char -> int
}

void mixed_types(long l, short s) {
  double d = l;  // long -> double
  int i = s;     // short -> int
}

float return_types(double d) {
  return d; // double -> float
}

double return_int(int i) {
    return i; // int -> double
}

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

void void_ptr_conversion(int *p) {
  void *vp = p; // int * -> void *
}

void multiple_conversions(int a) {
    double d = a + a;  // int -> double
    double h = 3; // int -> double
    int i = 1.0f;  // float -> int
}
