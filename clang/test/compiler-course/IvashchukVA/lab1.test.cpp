// RUN: %clang_cc1 -load %llvmshlibdir/lab1_IvashchukVA_FIIT2_ClangAST%pluginext -plugin lab1_IvashchukVA_FIIT2_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// Тест 1: Базовая функциональность
int main() {
  int unused_var = 42;
  int used_var = 43;
  return 0;
}

// CHECK: int main() {
// CHECK: [[maybe_unused]] int unused_var = 42;
// CHECK: int used_var = 43;
// CHECK: return 0;

// Тест 2: Несколько unused переменных
void test_multiple() {
  int unused_first = 1;
  int normal_var = 2;
  int unused_second = 3;
}

// CHECK: void test_multiple() {
// CHECK: [[maybe_unused]] int unused_first = 1;
// CHECK: int normal_var = 2;
// CHECK: [[maybe_unused]] int unused_second = 3;

// Тест 3: Переменные без инициализации (не должны обрабатываться)
void test_no_init() {
  int unused_no_init;
  int normal_no_init;
}

// CHECK: void test_no_init() {
// CHECK: int unused_no_init;
// CHECK: int normal_no_init;

// Тест 4: Разные типы переменных
void test_different_types() {
  double unused_double = 3.14;
  char unused_char = 'a';
  float normal_float = 2.71f;
}

// CHECK: void test_different_types() {
// CHECK: [[maybe_unused]] double unused_double = 3.14;
// CHECK: [[maybe_unused]] char unused_char = 'a';
// CHECK: float normal_float = 2.71f;

// Тест 5: Static переменные
void test_static() {
  static int unused_static = 100;
  static int normal_static = 200;
}

// CHECK: void test_static() {
// CHECK: [[maybe_unused]] static int unused_static = 100;
// CHECK: static int normal_static = 200;

// Тест 6: Const переменные
void test_const() {
  const int unused_const = 300;
  const int normal_const = 400;
}

// CHECK: void test_const() {
// CHECK: [[maybe_unused]] const int unused_const = 300;
// CHECK: const int normal_const = 400;
