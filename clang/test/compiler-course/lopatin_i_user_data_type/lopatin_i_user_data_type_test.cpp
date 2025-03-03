// RUN: %clang_cc1 -load %llvmshlibdir/UserDataTypePlugin_LopatinIlya_FIIT3_ClangAST%pluginext -plugin UserDataTypePlugin_LopatinIlya_FIIT3_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// CHECK: Base1
// CHECK: |_Fields
// CHECK: |_Methods
// CHECK: | |_ virtualMethod (void()|public|virtual|pure)
// CHECK: Base2
// CHECK: |_Fields
// CHECK: |_Methods
// CHECK: | |_ baseMethod (void()|public|virtual)
// CHECK: Derived -> Base1, Base2
// CHECK: |_Fields
// CHECK: | |_ publicField (int|public)
// CHECK: | |_ protectedField (float|protected)
// CHECK: | |_ privateField (char|private)
// CHECK: |_Methods
// CHECK: | |_ virtualMethod (void()|public|override)
// CHECK: | |_ baseMethod (void()|public|override)
// CHECK: | |_ constMethod (int()|private|const)
// CHECK: | |_ anotherMethod (void(int)|protected)

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