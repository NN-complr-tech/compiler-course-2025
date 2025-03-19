//// -h arg test ////
// RUN: %clang_cc1 -load %llvmshlibdir/UnusedVariables_Savchenko_Maxim_FIIT1_ClangAST%pluginext -plugin savchenko_m_UnusedVars_plugin -plugin-arg-savchenko_m_UnusedVars_plugin -h 2>&1 | FileCheck --check-prefix=HELP %s
// HELP: The plugin adds the {{\[\[maybe_unused\]\]}} flag to variables and parameters that are not in use.
