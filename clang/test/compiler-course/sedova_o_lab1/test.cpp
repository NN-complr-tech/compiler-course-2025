// RUN: %clang_cc1 -load %llvmshlibdir/Lab1_Sedova_Olga_FIIT1_ClangAST%pluginext -plugin Lab1 -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK-LABEL: Function 'isPositive' contains 0 implicit casts.

// CHECK-LABEL: Function 'compute' contains 2 implicit casts.

// CHECK-LABEL: Function 'process' contains 3 implicit casts.

// CHECK-LABEL: Function 'identity' contains 0 implicit casts.

// CHECK-LABEL: Function 'explicitCast' contains 1 implicit casts.

// CHECK-LABEL: Function 'returnFloat' contains 0 implicit casts.

bool isPositive(int a) {
  return a > 0;
}

double compute(int x, float y) {
  return x + y;
}

int process(float a, float b) {
  return a + compute(a, b);
}

int identity(int v) {
  return v;
}

float explicitCast(int a) {
  return (float)a;
}

float returnFloat(float f) {
  return f;
}
