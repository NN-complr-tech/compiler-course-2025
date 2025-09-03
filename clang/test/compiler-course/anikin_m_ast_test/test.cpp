// RUN: %clang_cc1 -load %llvmshlibdir/DataType_Anikin_Maksim_FIIT2_ClangAST%pluginext -plugin data_type -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: Human(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ age (unsigned int|public)
// CHECK-NEXT: | |_ height (unsigned int|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ sleep (void()|public|virtual|pure)
// CHECK-NEXT: | |_ eat (void()|public|virtual|pure)
// CHECK-NEXT: |_Nested Types
// CHECK-NEXT: | |_ (no nested types)
struct Human {
  unsigned age;
  unsigned height;
  virtual void sleep() = 0;
  virtual void eat() = 0;
};

// CHECK: Engineer(struct) -> public Human
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ salary (unsigned int|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ sleep (void()|public|override)
// CHECK-NEXT: | |_ eat (void()|public|override)
// CHECK-NEXT: | |_ work (double()|public)
// CHECK-NEXT: | |_ type_float (float()|public)
// CHECK-NEXT: | |_ type_char (char()|public)
// CHECK-NEXT: |_Nested Types
// CHECK-NEXT: | |_ (no nested types)
struct Engineer : Human {
  unsigned salary;
  void sleep() override { /* something */ }
  void eat() override { /* something */ }
  double work() { /* something */ }
  float type_float() { }
  char type_char() { }
};

// CHECK: BaseClass(class)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
// CHECK-NEXT: |_Nested Types
// CHECK-NEXT: | |_ (no nested types)
class BaseClass {};

// CHECK: NaslClass(class) -> public BaseClass
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
// CHECK-NEXT: |_Nested Types
// CHECK-NEXT: | |_ (no nested types)
class NaslClass : public BaseClass {};

// CHECK: TempClass(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ type_t (T|private)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
// CHECK-NEXT: |_Nested Types
// CHECK-NEXT: | |_ (no nested types)

template <class T> class TempClass {
  T type_t;
};

// CHECK: UnionCheck(union)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ foo (int|public)
// CHECK-NEXT: | |_ boo (char|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
// CHECK-NEXT: |_Nested Types
// CHECK-NEXT: | |_ (no nested types)
union UnionCheck {
  int foo;
  char boo;
};

// CHECK: A(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
// CHECK-NEXT: |_Nested Types
// CHECK-NEXT: | |_ (no nested types)
struct A {};

// CHECK: B(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
// CHECK-NEXT: |_Nested Types
// CHECK-NEXT: | |_ A
struct B {
  struct A {};
};