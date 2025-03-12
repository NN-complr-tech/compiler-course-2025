// RUN: %clang_cc1 -load %llvmshlibdir/ExamplePlugin_Ivanov_Ivan_FIIT0_ClangAST%pluginext -plugin example_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: FunctionDecl {{0x[0-9a-fA-F]+}} <{{.*}}> col:20 isEven 'bool (int) noexcept'
// CHECK-NEXT: |-ParmVarDecl {{0x[0-9a-fA-F]+}} <col:27, col:31> col:31 used value 'int'
// CHECK-NEXT: |-CompoundStmt {{0x[0-9a-fA-F]+}} <col:47, col:72>
// CHECK-NEXT: | `-ReturnStmt {{0x[0-9a-fA-F]+}} <col:49, col:69>
// CHECK-NEXT: |   `-BinaryOperator {{0x[0-9a-fA-F]+}} <col:56, col:69> 'bool' '=='
// CHECK-NEXT: |     |-BinaryOperator {{0x[0-9a-fA-F]+}} <col:56, col:64> 'int' '%'
// CHECK-NEXT: |     | |-ImplicitCastExpr {{0x[0-9a-fA-F]+}} <col:56> 'int' <LValueToRValue>
// CHECK-NEXT: |     | | `-DeclRefExpr {{0x[0-9a-fA-F]+}} <col:56> 'int' lvalue ParmVar {{0x[0-9a-fA-F]+}} 'value' 'int'
// CHECK-NEXT: |     | `-IntegerLiteral {{0x[0-9a-fA-F]+}} <col:64> 'int' 2
// CHECK-NEXT: |     `-IntegerLiteral {{0x[0-9a-fA-F]+}} <col:69> 'int' 0
// CHECK-NEXT: `-WarnUnusedResultAttr {{0x[0-9a-fA-F]+}} <col:3> nodiscard ""

[[nodiscard]] bool isEven(int value) noexcept { return value % 2 == 0; }
