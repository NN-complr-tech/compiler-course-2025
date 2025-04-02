// RUN: %clang_cc1 -load %llvmshlibdir/UserDataTypePlugin_beresnev_a_FIIT1_ClangAST%pluginext -plugin UserDataTypePlugin_beresnev_a_clang -plugin-arg-UserDataTypePlugin_beresnev_a_clang --c++17 -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: /[/[maybe_unused/]/] extern int gValue;
// CHECK: extern int gValue1;
// CHECK: int foo(int a, int b, /[/[maybe_unused/]/] int c) {
// CHECK: /[/[maybe_unused/]/] double value=0.0;
// CHECK: return a + b;

// CHECK: int main2() {
// CHECK: /[/[maybe_unused/]/] float d;
// CHECK: return gValue1;

extern int gValue;
extern int gValue1;

int foo(int a, int b, int c) {
    double value = 0.0;
    return a + b;
}

int main2() {
    float d;
    return gValue1;
}

// RUN: %clang_cc1 -load %llvmshlibdir/UserDataTypePlugin_beresnev_a_FIIT1_ClangAST%pluginext -plugin UserDataTypePlugin_beresnev_a_clang -plugin-arg-UserDataTypePlugin_beresnev_a_clang --gcc -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: __attribute__((unused)) extern int gValue2;
// CHECK: extern int gValue3;
// CHECK: int foo1(int a, int b, __attribute__((unused)) int c) {
// CHECK: __attribute__((unused)) double value=0.0;
// CHECK: return a + b;

// CHECK: int main1() {
// CHECK: __attribute__((unused)) float d;
// CHECK: return gValue3;

extern int gValue2;
extern int gValue3;

int foo1(int a, int b, int c) {
    double value = 0.0;
    return a + b;
}

int main1() {
    float d;
    return gValue3;
}
