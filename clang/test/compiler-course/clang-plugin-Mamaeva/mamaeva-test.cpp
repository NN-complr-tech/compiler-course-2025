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

// User-defined classes and enums
enum class Color { Red, Green, Blue };
enum OldStyleEnum { ONE, TWO, THREE };

class Base {
public:
    operator int() const { return 42; }
};

class Derived : public Base {
public:
    operator float() const { return 3.14f; }
};

// CHECK: Function `useClasses`
// CHECK-NEXT: Derived -> Base: 1
// CHECK-NEXT: Derived -> float: 1
// CHECK-NEXT: Base -> int: 1
// CHECK-NEXT: Color -> int: 1
// CHECK-NEXT: OldStyleEnum -> int: 1
void useClasses() {
    Derived d;
    Base b = d;         // Derived to Base
    float f = d;        // Derived to float
    int i = b;          // Base to int
    
    Color col = Color::Red;
    int colorInt = static_cast<int>(col); // Color to int
    
    OldStyleEnum old = TWO;
    int oldInt = old;   // OldStyleEnum to int
}

// CHECK: Function `useEnumConversions`
// CHECK-NEXT: int -> Color: 1
// CHECK-NEXT: int -> OldStyleEnum: 1
void useEnumConversions() {
    Color c = static_cast<Color>(1);
    OldStyleEnum e = static_cast<OldStyleEnum>(2);
}

// CHECK: Total implicit conversions: 16
