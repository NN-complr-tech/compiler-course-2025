// RUN: %clang_cc1 -load %llvmshlibdir/Implicit_Conv_Chizhov_Maxim_FIIT3_ClangAST%pluginext -plugin Implicit_Conv_Chizhov_Maxim_FIIT3_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s

// CHECK: Function sum
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1
double sum(int a, float b) {
    return a + b;
}

// CHECK: Function mul
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1

int mul(float a, float b) {
    return a + sum(a, b);
}

// CHECK: Function Create
// CHECK-NEXT: int -> Class: 1
class Class {
    int x;
 public:
    Class(int);
};

Class Create() {
    return 1;
}

// CHECK: Function LongToFloat
// CHECK-NEXT: long long -> float: 1

float LongToFloat()
{
    long long x = 9223372036854775807LL;
    float y = x;
    return y;
}

// CHECK: Function CharToInt
// CHECK-NEXT: char -> int: 1

int CharToInt()
{
    char x = 'c';
    int y = x;
    return y;
}