// RUN: %clang_cc1 -load %llvmshlibdir/UserDataTypePlugin_LopatinIlya_FIIT3_ClangAST%pluginext -plugin UserDataTypePlugin_LopatinIlya_FIIT3_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// CHECK: Human
// CHECK: |_Fields
// CHECK: | |_ age (unsigned int|public)
// CHECK: | |_ height (unsigned int|public)
// CHECK: |_Methods
// CHECK: | |_ sleep (void()|public|virtual|pure)
// CHECK: | |_ eat (void()|public|virtual|pure)

// CHECK: Engineer -> Human
// CHECK: |_Fields
// CHECK: | |_ salary (unsigned int|public)
// CHECK: |_Methods
// CHECK: | |_ sleep (void()|public|override)
// CHECK: | |_ eat (void()|public|override)
// CHECK: | |_ work (void()|public)

struct Human {
  unsigned age;
  unsigned height;
  virtual void sleep() = 0;
  virtual void eat() = 0;
};

struct Engineer : Human {
  unsigned salary;
  void sleep() override {}
  void eat() override {}
  void work() {}
};
