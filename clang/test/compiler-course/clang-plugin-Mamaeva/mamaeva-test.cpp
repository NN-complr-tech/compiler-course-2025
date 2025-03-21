// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_1_Mamaeva_Olga_FIIT3_ClangAST%pluginext -plugin ClangAST_1 -fsyntax-only %s 2>&1 | FileCheck %s -dump-input=always

// CHECK: Function `sum`
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1

double sum(int a, float b) {
    return a + b;
}

// CHECK-NEXT: Function `mul`
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

// CHECK: Function `userDefinedClasses`
// CHECK-NEXT: MyClass -> int: 1
// CHECK-NEXT: int -> MyClass: 1

class MyClass {
public:
    MyClass(int value) : m_value(value) {}
    operator int() const { return m_value; }
private:
    int m_value;
};

void userDefinedClasses() {
    MyClass obj = 42; // int -> MyClass
    int value = obj;  // MyClass -> int
}

// CHECK: Function `userDefinedEnums`
// CHECK-NEXT: MyEnum -> int: 1
// CHECK-NEXT: int -> MyEnum: 1

enum MyEnum { A, B, C };

void userDefinedEnums() {
    MyEnum e = static_cast<MyEnum>(1); // int -> MyEnum
    int value = e;                     // MyEnum -> int
}

// CHECK: Total implicit conversions: 14
