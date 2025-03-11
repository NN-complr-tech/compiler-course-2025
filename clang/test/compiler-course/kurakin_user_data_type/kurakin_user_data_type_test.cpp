// RUN: %clang_cc1 -load %llvmshlibdir/UserDataTypePlugin_Kurakin_Matvey_FIIT1_ClangAST%pluginext -plugin UserDataTypePlugin_Kurakin_Matvey_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck %s

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
  void sleep() override { /* something */ }
  void eat() override { /* something */ }
  void work() { /* something */ }
};

//CHECK: A
//CHECK-NEXT: |_Fields
//CHECK-NEXT: | |_ a1 (int|private)
//CHECK-NEXT: | |_ a2 (float|private)
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ somefunc (int(float)|private)

class A {
  int a1;
  float a2;
  int somefunc(float);
};

//CHECK: B
//CHECK-NEXT: |_Fields
//CHECK-NEXT: | |_ (has no fields)
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ b1 (void(int,int)|private|virtual)

class B {
  virtual void b1(int b2, int b3);
};

//CHECK: C -> A,B
//CHECK-NEXT: |_Fields
//CHECK-NEXT: | |_ (has no fields)
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ b1 (void(int,int)|private|override)
//CHECK-NEXT: | |_ somefunc (int(float)|private)

class C : A,B{
  void b1(int b2, int b3) override;
  int somefunc(float);
};

//CHECK: D
//CHECK-NEXT: |_Fields
//CHECK-NEXT: | |_ d1 (int[5]|private)
//CHECK-NEXT: | |_ d2 (T|protected)
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ d3 (T(T,int)|public|virtual|pure)

template <typename T>
struct D{
private:
  int d1[5];
protected:
  T d2;
public:
  virtual T d3(T d4, int d5) = 0;
};


//CHECK: E
//CHECK-NEXT: |_Fields
//CHECK-NEXT: | |_ (has no fields)
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ e2 (char(char)|public|static)

struct E{
  static char e2(char);
};

//CHECK: F
//CHECK-NEXT: |_Fields
//CHECK-NEXT: | |_ f1 (int|private)
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ (has no methods)

class F{
  int f1;
};
