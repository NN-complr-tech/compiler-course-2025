// RUN: %clang_cc1 -load %llvmshlibdir/ImplicitConvPlugin_Frolova_Elizaveta_FIIT3_ClangAST%pluginext -plugin ImplicitConvPlugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: Function: sum
// CHECK-NEXT: float -> double: 1
// CHECK-NEXT: int -> float: 1

double sum(int a, float b) {
    return a + b; // 'a' (int) -> 'b' (float) -> 'double'
}

