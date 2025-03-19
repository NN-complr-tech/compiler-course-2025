// RUN: %clang_cc1 -load %llvmshlibdir/Data_types_Dudchenko_Olesya_FIIT2_ClangAST%pluginext -plugin type_info_plugin -fsyntax-only %s 2>&1 | FileCheck %s

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

struct Engineer : Human {
  unsigned salary;
  void sleep() override { /* something */ }
  void eat() override { /* something */ }
  void work() { /* something */ }
};
