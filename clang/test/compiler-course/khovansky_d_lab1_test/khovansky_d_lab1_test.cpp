// RUN: %clang_cc1 -load %llvmshlibdir/ImplicitConvPlugin_Khovansky_Dmitry_FIIT2_ClangAST%pluginext -plugin ImplicitConvPlugin -fsyntax-only %s 2>&1 | FileCheck %s

// Проверка глобального скоупа
// CHECK: In global scope:
// CHECK-NEXT: int -> double: 1
double global_par = 42;

// Проверка задач варианта
// CHECK: Function: sum
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1

// CHECK: Function: mul
// CHECK-NEXT: double -> int: 1
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: float -> int: 1

double sum(int a, float b) {
    return a + b;
}

int mul(float a, float b) {
    return a + sum(a, b);
}

// Проверка обычной функции с несколькими преобразованиями
// CHECK: Function: standart_func
// CHECK-NEXT: float -> double: 2
// CHECK-NEXT: int -> float: 1
// CHECK-NEXT: int -> bool: 1

void standart_func(float f) {
  double d = f;
  float x = 3;
  bool flag = 10;
  double k = f;
}

// Проверка конструктора с неявным преобразованием
class U {
public:
  U(int val) {}
};

// CHECK: Function: construct_u
// CHECK-NEXT: int -> U: 1
// CHECK-NEXT: double -> int: 1

void construct_u() {
  U u = 5.6;
}

// Проверка return с преобразованием
// CHECK: Function: make_bool
// CHECK-NEXT: int -> bool: 1

bool make_bool() {
  return 100;
}

// Проверка указателей
// CHECK: Function: pointer_test
// CHECK-NEXT: nullptr_t -> char *: 1
// CHECK-NEXT: char * -> void *: 1

void pointer_test() {
  char* p = nullptr;
  void* vp = p;
}

// Проверка неявного преобразования через пользовательский конструктор
class X {
public:
  X(int x) {}
};

// CHECK: Function: create_x
// CHECK-NEXT: int -> X: 1

void create_x() {
  X obj = 10;
}

// Несколько преобразований
// CHECK: Function: create_x_chain
// CHECK-NEXT: int -> X: 1
// CHECK-NEXT: double -> int: 1

void create_x_chain() {
  X obj = 5.5;
}

// Неявный вызов конструктора в параметрах функции
void takes_x(X x) {}

// CHECK: Function: call_takes_x
// CHECK-NEXT: int -> X: 1

void call_takes_x() {
  takes_x(7);
}
//

