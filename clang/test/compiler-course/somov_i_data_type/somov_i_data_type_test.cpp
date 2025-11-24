// RUN: %clang_cc1 -load %llvmshlibdir/DataType_SomovIvan_FIIT1_ClangAST%pluginext -plugin data_type_plugin -fsyntax-only %s 2>&1 | FileCheck %s

//CHECK: Human
//CHECK-NEXT: |_Fields
//CHECK-NEXT: | |_ age (unsigned int|public)
//CHECK-NEXT: | |_ height (unsigned int|public)
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ sleep (void()|public|virtual|pure)
//CHECK-NEXT: | |_ eat (void()|public|virtual|pure)

struct Human {
  unsigned age;
  unsigned height;
  virtual void sleep() = 0;
  virtual void eat() = 0;
};

//CHECK: Engineer -> Human
//CHECK-NEXT: |_Fields
//CHECK-NEXT: | |_ salary (unsigned int|public)
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ sleep (void()|public|override)
//CHECK-NEXT: | |_ eat (void()|public|override)
//CHECK-NEXT: | |_ work (void()|public)

struct Engineer : Human {
  unsigned salary;
  void sleep() override { /* реализация */ }
  void eat() override { /* реализация */ }
  void work() { /* реализация */ }
};

// CHECK: EmptyStruct
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (no fields)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (no methods)

struct EmptyStruct {};

// CHECK: Data
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ publicField (int|public)
// CHECK-NEXT: | |_ protectedField (double|protected)
// CHECK-NEXT: | |_ privateField (char|private)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (no methods)

struct Data {
  int publicField;
protected:
  double protectedField;
private:
  char privateField;
};

// CHECK: MethodsExample
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (no fields)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ foo (void()|public)
// CHECK-NEXT: | |_ bar (int(int, double)|public)
// CHECK-NEXT: | |_ baz (void()|public|const)

struct MethodsExample {
  void foo() {}
  int bar(int x, double y) { return x; }
  void baz() const {}
};

// CHECK: Base
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (no fields)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ virtualFunc (void()|public|virtual|pure)

struct Base {
  virtual void virtualFunc() = 0;
};

// CHECK: Derived -> Base
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (no fields)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ virtualFunc (void()|public|override)

struct Derived : Base {
  void virtualFunc() override {}
};

// CHECK: A

struct A {};

// CHECK: B -> A
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ value (A|public)
// CHECK-NEXT: |
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (no methods)
// CHECK-NEXT: |
// CHECK-NEXT: |_Nested Types
// CHECK-NEXT: | |_ A

struct B : public A {
  struct A {};
  A value;
};