// RUN: %clang_cc1 -load %llvmshlibdir/lab1_IvashchukVA_FIIT2_ClangAST%pluginext -plugin lab1_IvashchukVA_FIIT2_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// Тест 1: Основной тест
int main() {
  int unused_var = 42;
  int used_var = 43;
  return used_var;
}

// CHECK: int main() {
// CHECK-NEXT: {{\[\[}}maybe_unused{{\]\]}} int unused_var = 42;
// CHECK-NEXT: int used_var = 43;
// CHECK-NEXT: return used_var;
// CHECK-NEXT: }

// Тест 2: Несколько unused переменных
static void multiple_unused() {
  int unused_first = 1;
  int normal_var = 2;
  int unused_second = 3;
  int sum = normal_var;
}

// CHECK: static void multiple_unused() {
// CHECK-NEXT: {{\[\[}}maybe_unused{{\]\]}} int unused_first = 1;
// CHECK-NEXT: int normal_var = 2;
// CHECK-NEXT: {{\[\[}}maybe_unused{{\]\]}} int unused_second = 3;
// CHECK-NEXT: int sum = normal_var;
// CHECK-NEXT: }

// Тест 3: Const переменные
static void test_const() {
  const int unused_const = 100;
  const int normal_const = 200;
  int total = normal_const;
}

// CHECK: static void test_const() {
// CHECK-NEXT: {{\[\[}}maybe_unused{{\]\]}} const int unused_const = 100;
// CHECK-NEXT: const int normal_const = 200;
// CHECK-NEXT: int total = normal_const;
// CHECK-NEXT: }

// Тест 4: Разные типы данных
static void different_types() {
  double unused_double = 3.14;
  char unused_char = 'a';
  float normal_float = 2.71f;
  int result = (int)normal_float;
}

// CHECK: static void different_types() {
// CHECK-NEXT: {{\[\[}}maybe_unused{{\]\]}} double unused_double = 3.14;
// CHECK-NEXT: {{\[\[}}maybe_unused{{\]\]}} char unused_char = 'a';
// CHECK-NEXT: float normal_float = 2.71f;
// CHECK-NEXT: int result = (int)normal_float;
// CHECK-NEXT: }