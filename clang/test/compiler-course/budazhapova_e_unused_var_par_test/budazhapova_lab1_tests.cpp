// RUN: %clang_cc1 -load %llvmshlibdir/UnusedVP_Budazhapova_Ekaterina_FIIT3_ClangAST%pluginext -plugin unused_var_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: int x = 10;
// CHECK-NEXT: {{\[\[maybe_unused\]\]}} int y = 20;
int x = 10;
int y = 20;

// CHECK: int sum(int a, int b, {{\[\[maybe_unused\]\]}} int c) {
// CHECK-NEXT: int result = a + b;
// CHECK-NEXT: {{\[\[maybe_unused\]\]}} int temp = a * b;
// CHECK-NEXT: return result;
int sum(int a, int b, int c) {
    int result = a + b;
    int temp = a * b;
    return result;
}

// CHECK: int mul(int p, int q) {
// CHECK-NEXT: return p * q;
int mul(int p, int q) {
    return p * q;
}

// CHECK: void init() {
// CHECK-NEXT: {{\[\[maybe_unused\]\]}} int count = 0;
// CHECK-NEXT: {{\[\[maybe_unused\]\]}} float pi = 3.14;
void init() {
    int count = 0;
    float pi = 3.14;
}

// CHECK: double calc({{\[\[maybe_unused\]\]}} char ch, double d) {
// CHECK-NEXT: {{\[\[maybe_unused\]\]}} int unused = 5;
// CHECK-NEXT: return d * x;
double calc(char ch, double d) {
    int unused = 5;
    return d * x;
}

// CHECK: int main() {
// CHECK-NEXT: sum(1, 2, 3);
// CHECK-NEXT: mul(4, 5);
// CHECK-NEXT: init();
// CHECK-NEXT: calc('A', 2.5);
// CHECK-NEXT: return x;
int main() {
    sum(1, 2, 3);
    mul(4, 5);
    init();
    calc('A', 2.5);
    return x;
}
