// RUN: %clang_cc1 -load %llvmshlibdir/UnusedVars_Grudzin_Konstantin_FIIT1_ClangAST%pluginext -plugin grudzin_k_UnVars_plugin 2>&1 | FileCheck %s --help

// CHECK:This plugin marks unused variables with \[\[maybe_unused\]\] attribute
