// RUN: %clang_cc1 -load %llvmshlibdir/lab1_IvashchukVA_FIIT2_ClangAST%pluginext -plugin lab1_IvashchukVA_FIIT2_ClangAST %s

int main() {
  int unused_var = 42;
  // CHECK: [[maybe_unused]] int unused_var
  int used_var = 43;
  return 0;
}