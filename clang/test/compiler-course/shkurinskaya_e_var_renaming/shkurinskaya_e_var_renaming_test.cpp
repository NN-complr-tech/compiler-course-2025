// RUN: %clang_cc1 -load %llvmshlibdir/ClangAST_4_Shkurinskaya_Elena_FIIT2_ClangAST%pluginext -plugin ClangAST_4 -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: int global_var = 42;
// CHECK-NEXT: float global_pi = 3.1415;
// CHECK-NEXT: static double global_static_value = 2.71828;
// CHECK-NEXT: void process(int param_a, float param_b, char param_c) {
// CHECK-NEXT:   int local_sum = param_a + param_b;
// CHECK-NEXT:   static int static_counter = 0;
// CHECK-NEXT:   for (int local_i = 0; local_i < 5; ++local_i) {
// CHECK-NEXT:     char local_buf[10];
// CHECK-NEXT:   }
// CHECK-NEXT: }
// CHECK-NEXT: void anotherFunction() {
// CHECK-NEXT:   static int static_flag = 1;
// CHECK-NEXT:   int local_value = static_flag + 10;
// CHECK-NEXT: }
// CHECK-NEXT: void testLoop() {
// CHECK-NEXT:   for (int local_j = 0; local_j < 3; ++local_j) {
// CHECK-NEXT:     int local_temp = local_j * 2;
// CHECK-NEXT:   }
// CHECK-NEXT: }

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
