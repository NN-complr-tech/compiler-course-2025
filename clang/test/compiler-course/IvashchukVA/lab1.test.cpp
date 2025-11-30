// RUN: %clang_cc1 -load %llvmshlibdir/lab1_IvashchukVA_FIIT2_ClangAST%pluginext -plugin lab1_IvashchukVA_FIIT2_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// Основной тест
int main() {
  int unused_var = 42;
  int used_var = 43;
  return used_var;
}

// CHECK: int main() {
// CHECK-NEXT: [[maybe_unused]] int unused_var = 42;
// CHECK-NEXT: int used_var = 43;
// CHECK-NEXT: return used_var;
// CHECK-NEXT: }

// Тест 1: Функция с разными типами
static void helper() {
  double unused_double = 3.14;
  char normal_char = 'a';
  int result = (int)unused_double + normal_char;
}

// CHECK: static void helper() {
// CHECK-NEXT: [[maybe_unused]] double unused_double = 3.14;
// CHECK-NEXT: char normal_char = 'a';
// CHECK-NEXT: int result = (int)unused_double + normal_char;
// CHECK-NEXT: }

// Тест 2: Несколько unused переменных в одной функции
static void multiple_unused() {
  int unused_first = 1;
  int normal_var = 2; 
  int unused_second = 3;
  int sum = normal_var + unused_first + unused_second;
}

// CHECK: static void multiple_unused() {
// CHECK-NEXT: [[maybe_unused]] int unused_first = 1;
// CHECK-NEXT: int normal_var = 2;
// CHECK-NEXT: [[maybe_unused]] int unused_second = 3;
// CHECK-NEXT: int sum = normal_var + unused_first + unused_second;
// CHECK-NEXT: }

// Тест 3: Const переменные
static void test_const() {
  const int unused_const = 100;
  const int normal_const = 200;
  int total = normal_const + unused_const;
}

// CHECK: static void test_const() {
// CHECK-NEXT: [[maybe_unused]] const int unused_const = 100;
// CHECK-NEXT: const int normal_const = 200;
// CHECK-NEXT: int total = normal_const + unused_const;
// CHECK-NEXT: }