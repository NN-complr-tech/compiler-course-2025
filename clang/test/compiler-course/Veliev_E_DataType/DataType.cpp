// RUN: %clang_cc1 -load %llvmshlibdir/DataTypeLab_VelievElvin_FIIT1_ClangAST%pluginext -plugin DataTypeLab_VelievElvin_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck %s


// TEST 1. Completely empty struct
// CHECK: Empty(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
struct Empty {};

// TEST 2. Struct with a public field and a simple method
// CHECK: Base(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ id (int|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ reset (void()|public)
struct Base {
    int id;
    void reset() {}
};

// TEST 3. Class with protected inheritance and one private field
// CHECK: Derived(class) -> protected Base
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ code (int|private)
// CHECK-NEXT: | |_ id (int|protected)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ reset (void()|protected)
class Derived : protected Base {
    int code;
};

// TEST 4. Class with multiple inheritance (different access specifiers)
// CHECK: DualInheritance(class) -> public Base, private Empty
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ id (int|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ reset (void()|public)
class DualInheritance : public Base, private Empty {};

// TEST 5. Struct with mixed access fields (public + protected)
// CHECK: Numbers(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ count (unsigned long|public)
// CHECK-NEXT: | |_ ratio (double|protected)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
struct Numbers {
    unsigned long count;

protected:
    double ratio;
};

// TEST 6. Abstract base class with a pure virtual method and a protected field
// CHECK: Animal(class)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ age (int|protected)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ speak (void()|public|virtual|pure)
class Animal {
protected:
    int age;

public:
    virtual void speak() = 0;
};

// TEST 7, Derived class overriding a virtual method and adding a new one
// CHECK: Dog(class) -> public Animal
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ id (int|private)
// CHECK-NEXT: | |_ age (int|protected)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ speak (void()|public|override)
// CHECK-NEXT: | |_ bark (void(int)|public)
class Dog : public Animal {
    int id;

public:
    void speak() override {}
    void bark(int times) {}
};

// TEST 10. Template class with a single private field
// CHECK: Box(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ value (T|private)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
template <typename T>
class Box {
    T value;
};

// TEST 11. Template with two fields and a const accessor method
// CHECK: Pair(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ first (T|private)
// CHECK-NEXT: | |_ second (U|private)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ getFirst (T()|public|const)
template <class T, class U>
class Pair {
    T first;
    U second;

public:
    T getFirst() const { return first; }
};

// TEST 12, Union with multiple fields and a static factory method
// CHECK: Variant(union)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ i (int|public)
// CHECK-NEXT: | |_ d (double|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ make (Variant(int)|static|public)
union Variant {
    int i;
    double d;
    static Variant make(int x);
};

// TEST 13: Override method
// CHECK: Base1(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ foo (void()|public|virtual)
struct Base1 {
    virtual void foo();
};

// CHECK: Derived1(struct) -> public Base1
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ foo (void()|public|override)
struct Derived1 : Base1 {
    void foo() override;
};

// TEST 14: Const method
// CHECK: ConstTest(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ getValue (int()|public|const)
struct ConstTest {
    int getValue() const;
};

// TEST 15: constexpr method
// CHECK: ConstExprTest(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ square (int(int)|static|public|constexpr)
struct ConstExprTest {
    static constexpr int square(int x) { return x * x; }
};
