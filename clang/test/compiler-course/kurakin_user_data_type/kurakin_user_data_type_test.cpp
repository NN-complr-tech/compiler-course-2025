// RUN: %clang_cc1 -load %llvmshlibdir/UserDataTypePlugin_Kurakin_Matvey_FIIT1_ClangAST%pluginext -plugin UserDataTypePlugin_Kurakin_Matvey_FIIT1_ClangAST -fsyntax-only %s 2>&1 | FileCheck %s

//CHECK: Human
//CHECK-NEXT: |_Fields
//CHECK-NEXT: | |_ age (unsigned int|public)
//CHECK-NEXT: | |_ height (unsigned int|public)
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ sleep (void()|public|virtual|pure)
//CHECK-NEXT: | |_ eat (void()|public|virtual|pure)

//CHECK: Engineer -> Human
//CHECK-NEXT: |_Fields
//CHECK-NEXT: | |_ salary (unsigned int|public)
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ sleep (void()|public|override)
//CHECK-NEXT: | |_ eat (void()|public|override)
//CHECK-NEXT: | |_ work (void()|public)

//CHECK: A
//CHECK-NEXT: |_Fields
//CHECK-NEXT: | |_ a1 (int|private)
//CHECK-NEXT: | |_ a2 (float|private)
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ somefunc (int(float)|private)

//CHECK: B
//CHECK-NEXT: |_Fields
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ b1 (void(int,int)|private|virtual)

//CHECK: C -> A,B
//CHECK-NEXT: |_Fields
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ b1 (void(int,int)|private|override)
//CHECK-NEXT: | |_ somefunc (int(float)|private)

//CHECK: D
//CHECK-NEXT: |_Fields
//CHECK-NEXT: | |_ d1 (int[5]|private)
//CHECK-NEXT: | |_ d2 (T|protected)
//CHECK-NEXT: |
//CHECK-NEXT: |_Methods
//CHECK-NEXT: | |_ d3 (T(T,int)|public|virtual|pure)

struct Human {
  unsigned age;
  unsigned height;
  virtual void sleep() = 0;
  virtual void eat() = 0;
};

struct Engineer : Human {
  unsigned salary;
  void sleep() override { /* something */ }
  void eat() override { /* something */ }
  void work() { /* something */ }
};

class A {
  int a1;
  float a2;
  int somefunc(float);
};

class B {
  virtual void b1(int b2, int b3);
};

class C : A,B{
  void b1(int b2, int b3) override;
  int somefunc(float);
};

template <typename T>
struct D{
private:
  int d1[5];
protected:
  T d2;
public:
  virtual T d3(T d4, int d5) = 0;
};