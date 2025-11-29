// RUN: %clang_cc1 -load %llvmshlibdir/lab1_IvashchukVA_FIIT2_ClangAST%pluginext -plugin lab1_IvashchukVA_FIIT2_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

// Тест 1: Базовая функциональность
int main() {
  int UnusedVar = 42;
  int UsedVar = 43;
  return 0;
}

// CHECK: int main() {
// CHECK: [[maybe_unused]] int UnusedVar = 42;
// CHECK: int UsedVar = 43;
// CHECK: return 0;

// Тест 2: Несколько unused переменных
static void testMultiple() {
  int UnusedFirst = 1;
  int NormalVar = 2;
  int UnusedSecond = 3;
}

// CHECK: void testMultiple() {
// CHECK: [[maybe_unused]] int UnusedFirst = 1;
// CHECK: int NormalVar = 2;
// CHECK: [[maybe_unused]] int UnusedSecond = 3;

// Тест 3: Переменные без инициализации (не должны обрабатываться)
static void testNoInit() {
  int UnusedNoInit;
  int NormalNoInit;
}

// CHECK: void testNoInit() {
// CHECK: int UnusedNoInit;
// CHECK: int NormalNoInit;

// Тест 4: Разные типы переменных
static void testDifferentTypes() {
  double UnusedDouble = 3.14;
  char UnusedChar = 'a';
  float NormalFloat = 2.71f;
}

// CHECK: void testDifferentTypes() {
// CHECK: [[maybe_unused]] double UnusedDouble = 3.14;
// CHECK: [[maybe_unused]] char UnusedChar = 'a';
// CHECK: float NormalFloat = 2.71f;

// Тест 5: Static переменные
static void testStatic() {
  static int UnusedStatic = 100;
  static int NormalStatic = 200;
}

// CHECK: void testStatic() {
// CHECK: [[maybe_unused]] static int UnusedStatic = 100;
// CHECK: static int NormalStatic = 200;

// Тест 6: Const переменные
static void testConst() {
  const int UnusedConst = 300;
  const int NormalConst = 400;
}

// CHECK: void testConst() {
// CHECK: [[maybe_unused]] const int UnusedConst = 300;
// CHECK: const int NormalConst = 400;