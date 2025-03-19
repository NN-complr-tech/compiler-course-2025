// RUN: %clang_cc1 -load %llvmshlibdir/DataType_Anikin_Maksim_FIIT2_ClangAST%pluginext -plugin data_type -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: Human(struct)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ age (unsigned int|public)
// CHECK-NEXT: | |_ height (unsigned int|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ sleep (void()|public|virtual|pure)
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
// CHECK-NEXT: | |_ work (void()|public)
struct Engineer : Human {
  unsigned salary;
  void sleep() override { /* something */ }
  void eat() override { /* something */ }
  void work() { /* something */ }
};

// CHECK: BaseClass(class)
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
class BaseClass {};

// CHECK: NaslClass(class) -> public BaseClass
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ (has no fields)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ (has no methods)
class NaslClass : public BaseClass {};