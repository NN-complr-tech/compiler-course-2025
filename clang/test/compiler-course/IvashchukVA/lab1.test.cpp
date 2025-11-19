// RUN: %clang_cc1 -load %llvmshlibdir/lab1_IvashchukVA_FIIT2_ClangAST%pluginext -plugin lab1_IvashchukVA_FIIT2_ClangAST %s -fsyntax-only 2>&1 | FileCheck %s

int main() {
  int unused_var = 42;
  int used_var = 43;
  return 0;
}

// CHECK: int main() {
// CHECK: int unused_var = 42;
// CHECK: int used_var = 43;
// CHECK: return 0;