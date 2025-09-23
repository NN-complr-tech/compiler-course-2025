// RUN: %clang_cc1 -load
// %llvmshlibdir/DataTypesPlugin_Agafeev_Sergey_FIIT3_ClangAST%pluginext -plugin
// AgafeevPlugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: testClass(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ a (Type|private)
// CHECK-NEXT: | |_ value (int|private)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
template <class Type>
class testClass {
  Type a;
  int value;
};

// CHECK: testTemplatesClass(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ abc (TypeClass|private)
// CHECK-NEXT: | |_ def (Name|private)
template <class TypeClass, typename Name>
class testTemplatesClass {
  TypeClass abc;
  Name def;
};

// CHECK: A(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
struct A {};

// CHECK: B(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
struct B {};

// CHECK: C(class) -> public A, public B
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
class C : public A, public B {};

// CHECK: testBaseClass(class)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
class testBaseClass {};

// CHECK: testPublicClass(class) -> public testBaseClass
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
class testPublicClass : public testBaseClass {};

// CHECK: testPrivateClass(class) -> private testBaseClass
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
class testPrivateClass : private testBaseClass {};

// CHECK: testPublicPrivate(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ ll (long long|public)
// CHECK-NEXT: | |_ af (float|private)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
struct testPublicPrivate {
  long long ll;

 private:
  float af;
};

// CHECK: Person(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ age (unsigned int|public)
// CHECK-NEXT: | |_ height (unsigned int|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ sleep (void()|public|virtual|pure)
// CHECK-NEXT: | |_ eat (void()|public|virtual|pure)
struct Person {
  unsigned age;
  unsigned height;

  virtual void sleep() = 0;
  virtual void eat() = 0;
};

// CHECK: testStatic(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ ll (long long|static|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ func (int(double)|static|public)
struct testStatic {
  static long long ll;
  static int func(double arg);
};

// CHECK: testUnion(union)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ a (int|public)
// CHECK-NEXT: | |_ b (float|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ flfunc (float(double *, int)|static|public)
union testUnion {
  static float flfunc(double *dval, int ival);
  int a;
  float b;
};
