// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_4_Shkurinskaya_Elena_FIIT2_ClangAST%pluginext -plugin ClangAST_4 -fsyntax-only %s 2>&1 | FileCheck %s

// Проверка глобальных переменных
// CHECK: int global_var = 42;
// CHECK: float global_pi = 3.1415;
// CHECK: static double global_static_value = 2.71828;

// Проверка параметров функций
// CHECK: void process(param_int a, param_float b, param_char c)

// Проверка локальных и статических переменных внутри функций
// CHECK: int local_sum = a + b;
// CHECK: static int static_counter = 0;
// CHECK: for (int local_i = 0; local_i < 5; ++local_i)
// CHECK: char local_buf[10];

int var = 42;
float pi = 3.1415;
static double static_value = 2.71828;

void process(int a, float b, char c) {
    int sum = a + b;
    static int counter = 0;

    for (int i = 0; i < 5; ++i) {
        char buf[10];
        buf[0] = c;
    }
}

void anotherFunction() {
    static int flag = 1;
    int value = flag + 10;
}

void testLoop() {
    for (int j = 0; j < 3; ++j) {
        int temp = j * 2;
    }
}