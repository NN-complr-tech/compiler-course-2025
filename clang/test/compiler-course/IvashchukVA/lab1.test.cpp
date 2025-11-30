// RUN: %clang_cc1 -load %llvmshlibdir/lab1_IvashchukVA_FIIT2_ClangAST%pluginext -plugin lab1_IvashchukVA_FIIT2_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// Тест 1: Основной тест
int main() {
  int unused_var = 42; // NOLINT
  int used_var = 43; // NOLINT
  return used_var;
}

// CHECK: int main() {
// CHECK-NEXT: {{\[\[}}maybe_unused{{\]\]}} int unused_var = 42;
// CHECK-NEXT: int used_var = 43;
// CHECK-NEXT: return used_var;
// CHECK-NEXT: }

// Тест 2: Несколько unused переменных
static void multipleUnused() {
  int unused_first = 1; // NOLINT
  int normal_var = 2; // NOLINT
  int unused_second = 3; // NOLINT
  int sum = normal_var; // NOLINT
  (void)sum;
}

// CHECK: static void multipleUnused() {
// CHECK-NEXT: {{\[\[}}maybe_unused{{\]\]}} int unused_first = 1;
// CHECK-NEXT: int normal_var = 2;
// CHECK-NEXT: {{\[\[}}maybe_unused{{\]\]}} int unused_second = 3;
// CHECK-NEXT: int sum = normal_var;
// CHECK-NEXT: (void)sum;
// CHECK-NEXT: }

// Тест 3: Const переменные
static void testConst() {
  const int unused_const = 100; // NOLINT
  const int normal_const = 200; // NOLINT
  int total = normal_const; // NOLINT
  (void)total;
}

// CHECK: static void testConst() {
// CHECK-NEXT: {{\[\[}}maybe_unused{{\]\]}} const int unused_const = 100;
// CHECK-NEXT: const int normal_const = 200;
// CHECK-NEXT: int total = normal_const;
// CHECK-NEXT: (void)total;
// CHECK-NEXT: }

// Тест 4: Разные типы данных
static void differentTypes() {
  double unused_double = 3.14; // NOLINT
  char unused_char = 'a'; // NOLINT
  float normal_float = 2.71f; // NOLINT
  int result = (int)normal_float; // NOLINT
  (void)result;
}

// CHECK: static void differentTypes() {
// CHECK-NEXT: {{\[\[}}maybe_unused{{\]\]}} double unused_double = 3.14;
// CHECK-NEXT: {{\[\[}}maybe_unused{{\]\]}} char unused_char = 'a';
// CHECK-NEXT: float normal_float = 2.71f;
// CHECK-NEXT: int result = (int)normal_float;
// CHECK-NEXT: (void)result;
// CHECK-NEXT: }