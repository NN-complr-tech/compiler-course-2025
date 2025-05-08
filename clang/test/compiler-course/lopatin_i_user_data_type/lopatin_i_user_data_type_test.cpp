// RUN: %clang_cc1 -load %llvmshlibdir/UserDataTypePlugin_LopatinIlya_FIIT3_ClangAST%pluginext -plugin UserDataTypePlugin_LopatinIlya_FIIT3_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// CHECK: Base1
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ virtualMethod (void()|public|virtual|pure)

// CHECK: Base2
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ baseMethod (void()|public|virtual)

// CHECK: Derived -> Base1, Base2
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ publicField (int|public)
// CHECK-NEXT: | |_ protectedField (float|protected)
// CHECK-NEXT: | |_ privateField (char|private)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ virtualMethod (void()|public|override)
// CHECK-NEXT: | |_ baseMethod (void()|public|override)
// CHECK-NEXT: | |_ constMethod (int()|private|const)
// CHECK-NEXT: | |_ anotherMethod (void(int)|protected)

// CHECK: Array
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ ptr (T[SZ]|public)


struct Base1 {
  virtual void virtualMethod() = 0;
};

struct Base2 {
  virtual void baseMethod() {}
};

struct Derived : Base1, Base2 {
public:
  int publicField;
protected:
  float protectedField;
private:
  char privateField;

public:
  void virtualMethod() override {}
  void baseMethod() override {}
  
private:
  int constMethod() const { return 0; }
  
protected:
  void anotherMethod(int) {}
};

template <typename T, unsigned SZ>
struct Array {
  T ptr[SZ]{};
};

