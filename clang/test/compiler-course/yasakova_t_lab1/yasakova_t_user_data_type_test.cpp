// RUN: %clang_cc1 -load %llvmshlibdir/PrintData_Yasakova_Tatiana_FIIT1_ClangAST%pluginext -plugin PrintDataPlugin %s -fsyntax-only 2>&1 | FileCheck %s

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
// CHECK-NEXT: | |_ first_int (int|public)
// CHECK-NEXT: | |_ first_float (float|public)
union C {
  int first_int;
  float first_float;
};

// CHECK: AccessModifiers(class)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_x (float|private)
// CHECK-NEXT: | |_ m_y (double|protected)
// CHECK-NEXT: | |_ m_z (long long|public)
class AccessModifiers {
private:
    float m_x;
protected:
    double m_y;
public:
    long long m_z;
};

// CHECK: Simple(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_x (int|public)
// CHECK-NEXT: | |_ m_y (double|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ method (int|public)
struct Simple {
    int m_x;
    double m_y;
    int method(int param_x, int param_y) { return 0; }
};

// CHECK: Static(class)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ staticMethod (void|private|static)
class Static {
    static void staticMethod() {}
};

// CHECK: Template(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_value (T|private)
// CHECK-NEXT: | |_ m_id (int|private)
template <typename T>
class Template {
    T m_value;
    int m_id;
};

// CHECK: MultiTemplate(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_first (T|private)
// CHECK-NEXT: | |_ m_second (U|private)
template <typename T, typename U>
class MultiTemplate {
    T m_first;
    U m_second;
};

// CHECK: Base(class)
class Base {
public:
    virtual ~Base() = default;
};

// CHECK: PublicDerived(class)
// CHECK-NEXT: PublicDerived -> public Base
class PublicDerived : public Base {};

// CHECK: ProtectedDerived(class)
// CHECK-NEXT: ProtectedDerived -> protected Base
class ProtectedDerived : protected Base {};

// CHECK: PrivateDerived(class)
// CHECK-NEXT: PrivateDerived -> private Base
class PrivateDerived : private Base {};

// CHECK: AClass(class)
class AClass {};

// CHECK: BStruct(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_a (AClass|public)
struct BStruct {
    AClass m_a;
};

// CHECK: CMultiple(class)
// CHECK-NEXT: CMultiple -> public AClass
// CHECK-NEXT: CMultiple -> public BStruct
class CMultiple : public AClass, public BStruct {};

// CHECK: Person(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ m_age (unsigned int|public)
// CHECK-NEXT: | |_ m_height (unsigned int|public)
// CHECK-NEXT: |_Methods
// CHECK-DAG: | |_ ~Person (void|public|virtual)
// CHECK-DAG: | |_ sleep (void|public|virtual|pure)
// CHECK-DAG: | |_ eat (void|public|virtual|pure)
struct Person {
    unsigned m_age;
    unsigned m_height;
    virtual ~Person() = default;
    virtual void sleep() = 0;
    virtual void eat() = 0;
};

// CHECK: Student(class)
// CHECK-NEXT: Student -> public Person
// CHECK-NEXT: |_Methods
// CHECK-DAG: | |_ ~Student (void|public|virtual)
// CHECK-DAG: | |_ sleep (void|public|virtual|override)
// CHECK-DAG: | |_ eat (void|public|virtual|pure)
class Student : public Person {
public:
    void sleep() override {}
    void eat() override = 0;
    ~Student() override = default;
};
