// RUN: %clang_cc1 -load %llvmshlibdir/myClangPlugin_Mamaeva_Olga_FIIT3_ClangAST%pluginext -plugin myClangPlugin_Mamaeva_Olga_FIIT3_ClangAST -fsyntax-only %s 2>&1 | FileCheck %s -dump-input=always

// CHECK: Function `sum`
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1

double sum(int a, float b) {
    return a + b;
}

// CHECK: Function `mul`
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1

int mul(float a, float b) {
    return a + sum(a, b);
}

// CHECK: Function `convertTypes`
// CHECK-NEXT: float -> int: 1
// CHECK-NEXT: int -> float: 1

using FloatType = float;
using IntType = int;

void convertTypes() {
    FloatType floatVar = IntType();
    IntType intVar = floatVar;
}

// CHECK: Function `handlePointers`
// CHECK-NEXT: int* -> void*: 1

void handlePointers() {
    int value;
    void* voidPtr = &value;
}

// CHECK: Function `checkPointer`
// CHECK-NEXT: int* -> bool: 1

void checkPointer(int* ptr) {
    if (ptr) {}
}

// CHECK: Function `convertToBool`
// CHECK-NEXT: int -> bool: 1

void convertToBool() {
    int number = 7;
    bool booleanValue = number;
}

// CHECK: Function `classConversions`
// CHECK-NEXT: MyClass -> int: 1
// CHECK-NEXT: int -> AnotherClass: 1

class MyClass {
public:
    operator int() const { return 42; } // Преобразование MyClass -> int
};

class AnotherClass {
public:
    AnotherClass(int x) {} // Преобразование int -> AnotherClass
};

void classConversions() {
    MyClass obj1;
    int x = obj1; // MyClass -> int
    AnotherClass obj2 = x; // int -> AnotherClass
}

// CHECK: Function `enumConversions`
// CHECK-NEXT: Color -> int: 1
// CHECK-NEXT: int -> Status: 1

enum Color { Red, Green, Blue };
enum Status { Ok = 0, Error = 1 };

void enumConversions() {
    Color color = Green;
    int colorCode = color; // Color -> int
    Status status = static_cast<Status>(colorCode); // int -> Status
}

// CHECK: Total implicit conversions: 14
