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

// CHECK: Function createX
// CHECK-NEXT: int -> class X: 1

class X {
    int x;
public:
    X(int val) : x(val) {}
};

X createX() {
    return 50;
}

// CHECK: Function: make_bool
// CHECK-NEXT: int -> bool: 1

bool make_bool() {
  return 55;
}

// CHECK: Function test_bool_cond
// CHECK-NEXT: int -> bool: 1

void test_bool_cond() {
    int x = 5;
    if (x) {}  
}

// CHECK: Function test_char_int
// CHECK-NEXT: char -> int: 1

int test_char_int() {
    char c = 'A';
    return c;
}
