// RUN: %clang_cc1 -load %llvmshlibdir/ImplicitConvPlugin_Frolova_Elizaveta_FIIT3_ClangAST%pluginext -plugin ImplicitConvPlugin -fsyntax-only %s 2>&1 | FileCheck %s
using ll = long long;
#define long long ll

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

// CHECK: Function: checkCondition
// CHECK-NEXT: int -> bool: 1
// CHECK-NEXT: int -> double: 1
double checkCondition() {
    int x = 1;
    if (x) { x++; }
    return x;
}

// CHECK: Function: convertAndReturn
// CHECK-NEXT: double -> float: 1
// CHECK-NEXT: float -> int: 1
int convertAndReturn() {
    double x = 1.2;
    float y = x;
    return y;
}

// CHECK: Function: convertLongLongToInt
// CHECK-NEXT: ll -> int: 1
int convertLongLongToInt() {
    ll x = 10000000000LL;
    int y = x;
    return y;
}

// CHECK: Function: charToIntAndDouble
// CHECK-NEXT: char -> int: 1
// CHECK-NEXT: int -> double: 1
double charToIntAndDouble() {
    char c = 'A';
    int i = c;
    double d = i;
    return d;
}

// CHECK: Function: callProcessPointer
// CHECK-NEXT: int * -> const int *: 1
void processPointer(const int*);
void callProcessPointer() {
    int x = 42;
    processPointer(&x);
}

// CHECK: Function: createAndReturnObject
// CHECK-NEXT: int -> MyClass: 1
class MyClass {
    int x;
 public:
    MyClass(int);
};

MyClass createAndReturnObject() {
    return 10;
}

// CHECK:Function: foo
// CHECK-NEXT:int[3] -> int *: 1
int foo(int a) {
    static int aa[] = {1, 2, 3};
    return aa[a];
}