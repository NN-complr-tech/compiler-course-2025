// RUN: %clang_cc1 -load %llvmshlibdir/Data_types_Dudchenko_Olesya_FIIT2_ClangAST%pluginext -plugin type_info_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// Сначала проверяем отладочный вывод
// CHECK-DAG: DEBUG: ParseArgs called
// CHECK-DAG: DEBUG: Creating AST consumer
// CHECK-DAG: DEBUG: TypeInfoConsumer created
// CHECK-DAG: DEBUG: TypeInfoVisitor created
// CHECK-DAG: DEBUG: Handling translation unit

// CHECK: Human
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ age (unsigned int|public)
// CHECK-NEXT: | |_ height (unsigned int|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ sleep (void()|public|virtual|pure)
// CHECK-NEXT: | |_ eat (void()|public|virtual|pure)
// CHECK-NEXT: |
struct Human {
  unsigned age;
  unsigned height;
  virtual void sleep() = 0;
  virtual void eat() = 0;
};

// CHECK: Engineer -> Human
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ salary (unsigned int|public)
// CHECK-NEXT: |_Methods
// CHECK-NEXT: | |_ sleep (void()|public|override)
// CHECK-NEXT: | |_ eat (void()|public|override)
// CHECK-NEXT: | |_ work (void()|public)
// CHECK-NEXT: |
struct Engineer : Human {
  unsigned salary;
  void sleep() override { /* something */ }
  void eat() override { /* something */ }
  void work() { /* something */ }
};

// ДОБАВЛЕННЫЕ ТЕСТЫ из ревью:

// CHECK: A
// CHECK-NEXT: |
struct A {};

// CHECK: B
// CHECK-NEXT: |
struct B {
  struct A {};
};

// CHECK: EmptyClass
// CHECK-NEXT: |
struct EmptyClass {};

// CHECK: WithNested
// CHECK-NEXT: |_Fields
// CHECK-NEXT: | |_ nested (B::A|public)
// CHECK-NEXT: |
struct WithNested {
  B::A nested;
};
