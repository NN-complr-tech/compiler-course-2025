// RUN: %clang_cc1 -load %llvmshlibdir/UnusedVars_Grudzin_Konstantin_FIIT1_ClangAST%pluginext -plugin grudzin_k_UnVars_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: |-ParmVarDecl {{0x[0-9a-fA-F]+}} <col:23, col:27> col:27 r 'int'
// CHECK-NEXT: | `-UnusedAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> maybe_unused
// CHECK: `-VarDecl {{0x[0-9a-fA-F]+}} <col:5, col:16> col:9 temp 'int' cinit
// CHECK-NEXT: | |-IntegerLiteral {{0x[0-9a-fA-F]+}} <col:16> 'int' 42
// CHECK-NEXT:  `-UnusedAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> maybe_unused

int baz(int p, int q, int r) {
    int temp = 42;
    return p + q;
}

// CHECK: |-ParmVarDecl {{0x[0-9a-fA-F]+}} <col:30, col:41> col:37 c 'double' cinit
// CHECK-NEXT: | |-FloatingLiteral {{0x[0-9a-fA-F]+}} <col:41> 'double' 5.000000e+00
// CHECK-NEXT: `-UnusedAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> maybe_unused
// CHECK: `-VarDecl {{0x[0-9a-fA-F]+}} <col:5, col:25> col:12 result 'double' cinit
// CHECK-NEXT: -BinaryOperator {{0x[0-9a-fA-F]+}} <col:21, col:25> 'double' '*'
// CHECK-NEXT: | |-ImplicitCastExpr {{0x[0-9a-fA-F]+}} <col:21> 'double' <LValueToRValue>
// CHECK-NEXT: | | `-DeclRefExpr {{0x[0-9a-fA-F]+}} <col:21> 'double' lvalue ParmVar {{0x[0-9a-fA-F]+}} 'a' 'double'
// CHECK-NEXT: | `-ImplicitCastExpr {{0x[0-9a-fA-F]+}} <col:25> 'double' <LValueToRValue> 
// CHECK-NEXT: | `-DeclRefExpr {{0x[0-9a-fA-F]+}} <col:25> 'double' lvalue ParmVar {{0x[0-9a-fA-F]+}} 'b' 'double'
// CHECK-NEXT: `-UnusedAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> maybe_unused

void qux(double a, double b, double c = 5.0) {
    double result = a * b;
}

