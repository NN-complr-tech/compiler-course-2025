// RUN: %clang_cc1 -load %llvmshlibdir/PrintData_Yasakova_Tatiana_FIIT1_ClangAST%pluginext -plugin PrintDataPlugin %s -fsyntax-only 2>&1 | FileCheck %s

// Проверка пустой структуры (уже есть в ваших тестах, оставляем для контекста)
// CHECK: A(struct)
struct A {};

// Проверка вложенной структуры с тем же именем
// CHECK: B(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ A (struct A|public)
struct B {
  struct A {};
};

// Проверка union с полями
// CHECK: C(union)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ a (int|public)
// CHECK-NEXT: | |_ b (float|public)
union C {
  int a;
  float b;
};

// CHECK: A(class)
class A {};

// CHECK: B(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ a (A|public)
struct B {
    A a;
};

// CHECK: Simple(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ x (int|public)
// CHECK-NEXT: | |_ y (double|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ method (int|public)
struct Simple {
    int x;
    double y;

    int method(int x, int y) {}
};

// CHECK: Static(class)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ method (void|private|static)
class Static {
    static void method() {}
};

// CHECK: Template(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ x (T|private)
// CHECK-NEXT: | |_ z (int|private)
template <typename T>
class Template {
    T x;
    int z;
};

// CHECK: MultiTemplate(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ first (T|private)
// CHECK-NEXT: | |_ second (U|private)
template <typename T, typename U>
class MultiTemplate {
    T first;
    U second;
};

// CHECK: C -> public A
// CHECK: C -> public B
class C : public A, public B {};

// CHECK: PublicDerived -> public Base
// CHECK: ProtectedDerived -> protected Base
// CHECK: PrivateDerived -> private Base
class Base {};
class PublicDerived : public Base {};
class ProtectedDerived : protected Base {};
class PrivateDerived : private Base {};

// CHECK: AccessModifiers
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ x (float|private)
// CHECK-NEXT: | |_ y (double|protected)
// CHECK-NEXT: | |_ z (long long|public)
class AccessModifiers {
private:
    float x;
protected:
    double y;
public:
    long long z;
};

// CHECK: Person
// CHECK: |_Fields
// CHECK: | |_ age (unsigned int|public)
// CHECK: | |_ height (unsigned int|public)
// CHECK: |_Methods
// CHECK: | |_ sleep (void|public|virtual|pure)
// CHECK: | |_ eat (void|public|virtual|pure)
struct Person {
    unsigned age;
    unsigned height;

    virtual void sleep() = 0;
    virtual void eat() = 0;
};

// CHECK: Student
// CHECK-NEXT: Student -> public Person
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ sleep (void|public|virtual|override)
// CHECK-NEXT: | |_ eat (void|public|virtual|pure)
class Student : public Person {
public:
    void sleep() override {}
    void eat() override = 0;
};
