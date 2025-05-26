/*
Вариант 2
Написать пасс, который при обнаружении чистой функции пометит ее атрибутом
"pure" Что такое чистая функция: https://en.m.wikipedia.org/wiki/Pure_function
Больше информации по LLVM IR: https://llvm.org/docs/LangRef.html

Tests: https://compiler-explorer.com/z/xxbnvnaj8
*/

#include <atomic>
#include <iostream>

extern float g_value;
float value;

// Pure functions:
int p1(int a, int b) { return a + b; }
void p2() {
  static std::atomic<unsigned int> x = 0;
  ++x;
}
void empty() {}

// Impure functions:
// 1.
int f1() {
  static int x = 0;
  ++x;
  return x;
}

// 2.
int f2() { return g_value; }
int f22() { return value; }

// 3.
int f3(int *x) { return *x; }

// 4.
int f4() {
  int x = 0;
  std::cin >> x;
  return x;
}

// 5.
void f5() {
  static int x = 0;
  ++x;
}

// 6.
void f6() { ++value; }

// 7.
void f7(int *x) { ++*x; }

// 8.
void f8() { std::cout << "Hello, world!" << std::endl; }