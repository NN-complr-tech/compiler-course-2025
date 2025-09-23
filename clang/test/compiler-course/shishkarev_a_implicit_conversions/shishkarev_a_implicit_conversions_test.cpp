// RUN: %clang_cc1 -load %llvmshlibdir/ImplicitConversionCounter_Shishkarev_Andrey_FIIT2_ClangAST%pluginext -plugin ImplicitConvPlugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: Function `sum`
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1

// CHECK: Function `mul`
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 2
// CHECK-NEXT: int -> float: 1


// CHECK: Function `one`
// CHECK-NEXT: int -> bool: 1

// CHECK: Function `two`
// CHECK-NEXT: int* -> bool: 1

double sum(int a, float b) {
    return a + b;
}

int mul(float a, float b) {
    return a + sum(a, (int)b);
}

void one() {
    int x = 7;
    bool y = x;
}

void two(int* x){
    if(x){}
}
