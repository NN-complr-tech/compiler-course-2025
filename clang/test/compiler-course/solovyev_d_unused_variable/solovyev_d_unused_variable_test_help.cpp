// RUN: %clang_cc1 -load %llvmshlibdir/UnusedVariable_Solovyev_Danila_FIIT3_ClangAST%pluginext -plugin unused_variable_plugin -plugin-arg-unused_variable_plugin --help 2>&1 | FileCheck %s

// CHECK:Marking unused variables and function parameters as {{\[\[maybe_unused\]\]}}
