// RUN: %clang_cc1 -load %llvmshlibdir/PrintData_Yasakova_Tatiana_FIIT1_ClangAST%pluginext -plugin PrintDataPlugin %s -fsyntax-only 2>&1 | FileCheck %s

// Проверка базовых структур
// CHECK: A(struct)
struct A {};

// CHECK: B(struct)
// CHECK-NOT: |_Fields
// CHECK: A(struct)
struct B {
  struct A {};
};

// CHECK: C(union)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ a (int|public)
// CHECK-NEXT: | |_ b (float|public)
union C {
  int a;
  float b;
};

// Проверка классов с разными спецификаторами доступа
// CHECK: AccessModifiers(class)
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

// Проверка простой структуры с методами
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

// Проверка статического метода
// CHECK: Static(class)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ method (void|private|static)
class Static {
    static void method() {}
};

// Проверка шаблонных классов
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

// Проверка наследования
// CHECK: Base(class)
class Base {};

// CHECK: PublicDerived(class)
// CHECK-NEXT: PublicDerived -> public Base
class PublicDerived : public Base {};

// CHECK: ProtectedDerived(class)
// CHECK-NEXT: ProtectedDerived -> protected Base
class ProtectedDerived : protected Base {};

// CHECK: PrivateDerived(class)
// CHECK-NEXT: PrivateDerived -> private Base
class PrivateDerived : private Base {};

// Проверка множественного наследования
// CHECK: A_Class(class)
class A_Class {};

// CHECK: B_Struct(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ a (A_Class|public)
struct B_Struct {
    A_Class a;
};

// CHECK: C_Multiple(class)
// CHECK-NEXT: C_Multiple -> public A_Class
// CHECK-NEXT: C_Multiple -> public B_Struct
class C_Multiple : public A_Class, public B_Struct {};

// Проверка абстрактных классов и виртуальных методов
// CHECK: Person(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ age (unsigned int|public)
// CHECK-NEXT: | |_ height (unsigned int|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ sleep (void|public|virtual|pure)
// CHECK-NEXT: | |_ eat (void|public|virtual|pure)
struct Person {
    unsigned age;
    unsigned height;
    virtual void sleep() = 0;
    virtual void eat() = 0;
};

// CHECK: Student(class)
// CHECK-NEXT: Student -> public Person
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ sleep (void|public|virtual|override)
// CHECK-NEXT: | |_ eat (void|public|virtual|pure)
class Student : public Person {
public:
    void sleep() override {}
    void eat() override = 0;
};
