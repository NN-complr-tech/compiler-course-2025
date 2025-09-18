// RUN: %clang_cc1 -load %llvmshlibdir/UserDataTypePlugin_Opolin_Dmitry_FIIT2_ClangAST%pluginext -plugin PrintUserTypeInfo %s -fsyntax-only 2>&1 | FileCheck %s

// CHECK: EmptyStruct
// CHECK-NEXT: |_Fields
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
struct EmptyStruct {};

// CHECK: SimpleStruct
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ x (int|public)
// CHECK-NEXT: | |_ y (double|public)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
struct SimpleStruct {
    int x;
    double y;
};

// CHECK: SimpleClass
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ data (char *|private)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
class SimpleClass {
    char* data;
};

// CHECK: AccessTest
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ pubField (int|public)
// CHECK-NEXT: | |_ protField (float|protected)
// CHECK-NEXT: | |_ privField (_Bool|private)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ publicMethod (void()|public)
// CHECK-NEXT: | |_ protectedMethod (int()|protected)
// CHECK-NEXT: | |_ privateMethod (double()|private)
class AccessTest {
public:
    int pubField;
    void publicMethod() {}
protected:
    float protField;
    int protectedMethod() { return 1;}
private:
    bool privField;
    double privateMethod() { return 0.0; }
};

// CHECK: Base
// CHECK-NEXT: |_Fields
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
struct Base {};

// CHECK: Derived -> Base
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ derivedData (int|public)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
struct Derived : Base {
    int derivedData;
};

// CHECK: Base1
// CHECK-NEXT: |_Fields
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
struct Base1 {};

// CHECK: Base2
// CHECK-NEXT: |_Fields
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
struct Base2 {};

// CHECK: MultipleDerived -> Base1, Base2
// CHECK-NEXT: |_Fields
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
class MultipleDerived : public Base1, public Base2 {};

// CHECK: MethodTester
// CHECK-NEXT: |_Fields
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ simpleMethod (void()|public)
// CHECK-NEXT: | |_ methodWithParams (int(int, double)|public)
// CHECK-NEXT: | |_ constMethod (void()|public|const)
// CHECK-NEXT: | |_ anotherPrivate (_Bool(char)|private)
struct MethodTester {
    void simpleMethod() {}
    int methodWithParams(int a, double b) { return a; }
    void constMethod() const {}
private:
    bool anotherPrivate(char c) { return true; }
};

// CHECK: AbstractBase
// CHECK-NEXT: |_Fields
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ virtualFunc (void()|public|virtual)
// CHECK-NEXT: | |_ pureVirtualFunc (int()|public|virtual|pure)
class AbstractBase {
public:
    virtual void virtualFunc() {}
    virtual int pureVirtualFunc() = 0;
};

// CHECK: ConcreteDerived -> AbstractBase
// CHECK-NEXT: |_Fields
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ virtualFunc (void()|public|override)
// CHECK-NEXT: | |_ pureVirtualFunc (int()|public|override)
class ConcreteDerived : public AbstractBase {
public:
    void virtualFunc() override {}
    int pureVirtualFunc() override { return 42; }
};

// CHECK: Human
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ age (unsigned int|public)
// CHECK-NEXT: | |_ height (unsigned int|public)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ sleep (void()|public|virtual|pure)
// CHECK-NEXT: | |_ eat (void()|public|virtual|pure)
struct Human {
    unsigned age;
    unsigned height;
    virtual void sleep() = 0;
    virtual void eat() = 0;
};

// CHECK: Engineer -> Human
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ salary (unsigned int|public)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ sleep (void()|public|override)
// CHECK-NEXT: | |_ eat (void()|public|override)
// CHECK-NEXT: | |_ work (void()|public)
struct Engineer : Human {
    unsigned salary;
    void sleep() override { }
    void eat() override { }
    void work() { }
};

// CHECK-NOT: ForwardDeclaredStruct
class ForwardDeclaredStruct;

// CHECK: UseForward
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ ptr (ForwardDeclaredStruct *|private)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
class UseForward {
    ForwardDeclaredStruct *ptr;
};

// CHECK: OuterClass
// CHECK-NEXT: |_Fields
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ createInner (void()|public)
// CHECK-NOT: InnerStruct
class OuterClass {
public:
    void createInner() {
        struct InnerStruct {
            int inner_data;
        };
        InnerStruct is;
    }
};

// CHECK: MyTemplate
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ value (T|private)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
template<typename T>
class MyTemplate {
    T value;
};
