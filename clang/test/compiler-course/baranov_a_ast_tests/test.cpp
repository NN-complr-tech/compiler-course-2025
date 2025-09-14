// RUN: %clang_cc1 -load %llvmshlibdir/DataTypes_BARANOV_ALEKSEY_FIIT1_ClangAST%pluginext -plugin BaranovDataPlugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: myClass(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ x (T|private)
// CHECK-NEXT: | |_ inner (int|private)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)

template <class T> class myClass {
  T x;
  int inner;
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

// CHECK: myClassBase(class)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
class myClassBase {};

// CHECK: myClassPubl(class) -> public myClassBase
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
class myClassPubl : public myClassBase {};

// CHECK: myClassPriv(class) -> private myClassBase
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
class myClassPriv : private myClassBase {};

// CHECK: publPriv(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ i (long long|public)
// CHECK-NEXT: | |_ x (float|private)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
struct publPriv {
  long long i;

private:
  float x;
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

// CHECK: twoTemplates(class|template)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ first (T|private)
// CHECK-NEXT: | |_ second (U|private)
template <class T, typename U> class twoTemplates {
  T first;
  U second;
};

// reviewers request

struct vi {
  virtual void foo() = 0;
};
// CHECK: overrideCheck(class) -> public vi
//  CHECK: |_Methods
//  CHECK-NEXT: | |_ foo (void()|private|override)
class overrideCheck : public vi {
  void foo() override;
};

// CHECK: staticCheck(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ r (long long|static|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ bbbb (int(double)|static|public)
struct staticCheck {
  static long long int r;
  static int bbbb(double arg);
};

// CHECK: E(union)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ F (int|public)
// CHECK-NEXT: | |_ I (float|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ t (float(int, double *)|static|public)
union E {
  static float t(int i, double *d);
  int F;
  float I;
};
