// RUN: %clang_cc1 -load %llvmshlibdir/PrintData_Chistov_Alexey_FIIT1_ClangAST%pluginext -plugin PrintDataPlugin %s -fsyntax-only 2>&1 | FileCheck %s

class A {};
// CHECK: A(class)

struct B {};
// CHECK: B(struct)

struct Simple {
    int x;
    double y;

    int method(int x, int y) {}
};

// CHECK: Simple(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ x (int|public)
// CHECK-NEXT: | |_ y (double|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ method (int|public)

template <typename T>
class Template {
    T x;
    int z;
};

// CHECK: Template(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ x (T|private)
// CHECK-NEXT: | |_ z (int|private)

template <typename T, typename U>
class MultiTemplate {
    T first;
    U second;
};

// CHECK: MultiTemplate(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ first (T|private)
// CHECK-NEXT: | |_ second (U|private)

class C : public A, public B {};

// CHECK: C -> public A
// CHECK: C -> public B

class Base {};
class PublicDerived : public Base {};
class ProtectedDerived : protected Base {};
class PrivateDerived : private Base {};

// CHECK: PublicDerived -> public Base
// CHECK: ProtectedDerived -> protected Base
// CHECK: PrivateDerived -> private Base

class AccessModifiers {
private:
    float x;
protected:
    double y;
public:
    long long z;
};

// CHECK: AccessModifiers
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ x (float|private)
// CHECK-NEXT: | |_ y (double|protected)
// CHECK-NEXT: | |_ z (long long|public)

struct Person {
    unsigned age;
    unsigned height;

    virtual void sleep() = 0;
    virtual void eat() = 0;
};

// CHECK: Person
// CHECK: |_Fields
// CHECK: | |_ age (unsigned int|public)
// CHECK: | |_ height (unsigned int|public)
// CHECK: |_Methods
// CHECK: | |_ sleep (void|public|virtual|pure)
// CHECK: | |_ eat (void|public|virtual|pure)
