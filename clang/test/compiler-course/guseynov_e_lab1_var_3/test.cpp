// RUN: %clang_cc1 -load %llvmshlibdir/ImplicitCastPlugin_Guseynov_Emil_FIIT2_ClangAST%pluginext -plugin implicit_cast_counter -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: FUNCTION sum
// CHECK-NEXT: float -> double: 1 
// CHECK-NEXT: int -> float: 1
double sum(int a, float b) {
	return a + b;
}

// CHECK: FUNCTION mul
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1
int mul(float a, float b) {
	return a + sum(a, b);
}

// CHECK: FUNCTION test_basic_conversions
// CHECK-NEXT: char -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1
// CHECK-NEXT: int -> short: 1
// CHECK-NEXT: short -> long: 1
void test_basic_conversions() {
    char c = 'A';
    short s = 10;
    int i = 100;
    float f = 3.14f;
    double d = 2.71;
    
    i = c;       // char -> int
    d = f;       // float -> double  
    f = i;       // int -> float
    long l = s;  // short -> long
}

// CHECK: FUNCTION test_pointer_conversions
// CHECK-NOT: int* -> void*
// CHECK-NOT: float* -> double*
void test_pointer_conversions() {
    int* ip = nullptr;
    float* fp = nullptr;
    
    void* vp = ip;    // int* -> void* (should not record)
    double* dp = (double*)fp; // float* -> double* (should not record)
}

// CHECK: FUNCTION test_arithmetic
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1
double test_arithmetic(int a, float b, double c) {
    return a + b + c; 
}

// CHECK: FUNCTION test_return_conversions
// CHECK-NEXT: double -> float: 1
float test_return_conversions() {
    double d = 3.14;
    int i = 42;
    return d; // double -> float
}

// CHECK: FUNCTION test_mixed_expressions  
// CHECK-NEXT: char -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1
double test_mixed_expressions(char c, int i, float f) {
    return c + i + f; // char->int, then int->float?, then float->double
}

// CHECK: FUNCTION test_explicit_casts
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> int: 1
void test_explicit_casts() {
    double d = 3.14;
    float f = 2.71f;
    int i1 = (int)d; // explicit cast, but implicit double->int somewhere
    int i2 = static_cast<int>(f); // explicit cast, implicit float->int
}

// CHECK: FUNCTION test_no_conversions
// CHECK-NOT: ->
void test_no_conversions() {
    int a = 1;
    int b = 2;
    int c = a + b; // no conversions
}

// CHECK: FUNCTION test_sign_conversions
// CHECK-NEXT: int -> unsigned int: 2
// CHECK-NEXT: unsigned int -> int: 1
void test_sign_conversions() {
    int signed_val = -5;
    unsigned int unsigned_val = 10;
    
    unsigned_val = signed_val;     // int -> unsigned int
    signed_val = unsigned_val;     // unsigned int -> int
}

// CHECK: FUNCTION test_size_promotions
// CHECK-NEXT: char -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> short: 1
// CHECK-NEXT: short -> int: 1
void test_size_promotions() {
    char c = 'X';
    short s = 100;
    float f = 1.0f;
    
    int i1 = c;  // char -> int
    int i2 = s;  // short -> int
    double d = f; // float -> double
}