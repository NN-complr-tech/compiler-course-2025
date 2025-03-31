// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_1_Mamaeva_Olga_FIIT3_ClangAST%pluginext -plugin ClangAST_1 -fsyntax-only %s 2>&1 | FileCheck %s -dump-input=always

enum class Color { Red, Green, Blue };
enum OldStyleEnum { ONE, TWO, THREE };

class NumberWrapper {
public:
    operator int() const { return 42; }
};

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

// CHECK: Function `useNumberWrapper`
// CHECK-NEXT: NumberWrapper -> int: 1
void useNumberWrapper() {
    NumberWrapper n;
    int value = n;
}

// CHECK: Function `useEnums`
// CHECK-NEXT: Color -> int: 1
// CHECK-NEXT: OldStyleEnum -> int: 1
void useEnums() {
    Color c = Color::Red;
    int colorCode = static_cast<int>(c);
    
    OldStyleEnum e = TWO;
    int enumValue = e;
}

// CHECK: Total implicit conversions: 12
