// RUN: %clang_cc1 -load %llvmshlibdir/lab1_IvashchukVA_FIIT2_ClangAST%pluginext -plugin lab1_IvashchukVA_FIIT2_ClangAST -fsyntax-only %s 2>&1 | FileCheck %s

void test_function(int unused, int used) {
  int unused_var = 42;
  int used_var = 43;
}

// CHECK: Applied [[maybe_unused]] to variables