// RUN: %clang_cc1 -load %llvmshlibdir/UnusedVariable_Solovyev_Danila_FIIT3_ClangAST%pluginext -plugin unused_variable_plugin -fsyntax-only %s 2>&1 | FileCheck %s

//CHECK: |-ParmVarDecl {{0x[0-9a-fA-F]+}} <col:23, col:27> col:27 c 'int'
//CHECK-NEXT: | `-UnusedAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> maybe_unused

//CHECK: `-VarDecl {{0x[0-9a-fA-F]+}} <col:5, col:20> col:12 value 'double' cinit
//CHECK-NEXT:  |   |-FloatingLiteral {{0x[0-9a-fA-F]+}} <col:20> 'double' 0.000000e+00
//CHECK-NEXT:  |   `-UnusedAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> maybe_unused

int foo(int a, int b, int c) {
    double value = 0.0;
    return a + b;
}

//CHECK: |-ParmVarDecl {{0x[0-9a-fA-F]+}} <col:25, col:34> col:32 c 'double' cinit
//CHECK-NEXT: | |-FloatingLiteral {{0x[0-9a-fA-F]+}} <col:34> 'double' 3.000000e+00
//CHECK-NEXT: | `-UnusedAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> maybe_unused

bool bar(int a, bool b, double c=3.0){
    b=a++;
    return true;
}
