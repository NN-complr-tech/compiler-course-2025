// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_1_MamaevaO_FIIT3_ClangAST%pluginext -plugin ClangAST_1_MamaevaO_FIIT3_ClangAST -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s

// CHECK: Function `sum`
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1

static double sum(int a, float b) {
    return a + b;
}

// CHECK: Function `mul`
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1

static int mul(float a, float b) {
    return a + sum(a, b);
}

// CHECK: Function `convertTypes`
// CHECK-NEXT: float -> int: 1
// CHECK-NEXT: int -> float: 1

using FloatType = float;
using IntType = int;

static void convertTypes() {
    FloatType float_var = IntType();
    [[maybe_unused]] IntType int_var = float_var;  // Переменная не используется, но оставлена для будущего использования.
}

// CHECK: Function `handlePointers`
// CHECK-NEXT: int* -> void*: 1

static void handlePointers() {
    int value;
    [[maybe_unused]] void *void_ptr = &value;  // Переменная не используется, но оставлена для будущего использования.
}

// CHECK: Function `checkPointer`
// CHECK-NEXT: int* -> bool: 1

static void checkPointer(int* ptr) {
    if (ptr) {}
}

// CHECK: Function `convertToBool`
// CHECK-NEXT: int -> bool: 1

static void convertToBool() {
    int number = 7;
    [[maybe_unused]] bool boolean_value = number;  // Переменная не используется, но оставлена для будущего использования.
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

static void classConversions() {
    MyClass obj1;
    int x = obj1;           // MyClass -> int
    AnotherClass obj2 = x;  // int -> AnotherClass
}

// CHECK: Function `enumConversions`
// CHECK-NEXT: Color -> int: 1
// CHECK-NEXT: int -> Status: 1

enum Color { Red, Green, Blue };
enum Status { Ok = 0, Error = 1 };

static void enumConversions() {
    Color color = Green;
    int color_code = color;                       // Color -> int
    [[maybe_unused]] Status status = static_cast<Status>(color_code); // int -> Status
}

// CHECK: Total implicit conversions: 14
