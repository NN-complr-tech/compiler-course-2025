// RUN: %clang_cc1 -load %llvmshlibdir/AddingPrefixesPlugin_Suvorov_Dmitrii_FIIT1_ClangAST%pluginext -plugin AddingPrefixesPlugin_Suvorov_Dmitrii_FIIT1_ClangAST %s -fsyntax-only 2>&1 | FileCheck --match-full-lines %s

// CHECK: int global_v1 = -10;
// CHECK-NEXT: extern float global_v3;
// CHECK-NEXT: long long global_ll = 1L;
// CHECK-NEXT: int static global_v4 = 52;
// CHECK-NEXT: int test_func(int param_a, int param_b) {
// CHECK-NEXT:   static int static_v2 = 0;
// CHECK-NEXT:   int local_v3 = 1;
// CHECK-NEXT:   static_v2 = local_v3 - ++static_v2;
// CHECK-NEXT:   return  global_v1 + param_a + param_b + static_v2 + local_v3 + global_ll;
// CHECK-NEXT: }
// CHECK-NEXT:  int static global_v5 = test_func(43, global_v4);

int v1 = -10;
extern float v3;
long long ll = 1L;
int static v4 = 52;
int test_func(int a, int b) {
  static int v2 = 0;
  int v3 = 1;
  v2 = v3 - ++v2;
  return v1 + a + b + v2 + v3 + ll;
}
int static v5 = test_func(43, v4);

// RUN: %clang_cc1 -load %llvmshlibdir/AddingPrefixesPlugin_Suvorov_Dmitrii_FIIT1_ClangAST%pluginext -plugin AddingPrefixesPlugin_Suvorov_Dmitrii_FIIT1_ClangAST %s -fsyntax-only 2>&1 | FileCheck -v %s -check-prefix=CHECK-LOGS

// CHECK-LOGS: Renamed variable at test.cpp:15:5: v1 -> global_v1
// CHECK-LOGS-NEXT: Renamed variable at test.cpp:16:14: v3 -> global_v3
// CHECK-LOGS-NEXT: Renamed variable at test.cpp:17:13: ll -> global_ll
// CHECK-LOGS-NEXT: Renamed variable at test.cpp:18:12: v4 -> static_v4
// CHECK-LOGS-NEXT: Renamed parameter at test.cpp:19:16: a -> param_a
// CHECK-LOGS-NEXT: Renamed parameter at test.cpp:19:23: b -> param_b
// CHECK-LOGS-NEXT: Renamed variable at test.cpp:20:14: v2 -> static_v2
// CHECK-LOGS-NEXT: Renamed variable at test.cpp:21:7: v3 -> local_v3
// CHECK-LOGS-NEXT: Updated reference at test.cpp:22:5: v2 -> static_v2
// CHECK-LOGS-NEXT: Updated reference at test.cpp:22:10: v3 -> local_v3
// CHECK-LOGS-NEXT: Updated reference at test.cpp:22:16: v2 -> static_v2
// CHECK-LOGS-NEXT: Updated reference at test.cpp:23:10: v1 -> global_v1
// CHECK-LOGS-NEXT: Updated reference at test.cpp:23:15: a -> param_a
// CHECK-LOGS-NEXT: Updated reference at test.cpp:23:19: b -> param_b
// CHECK-LOGS-NEXT: Updated reference at test.cpp:23:23: v2 -> static_v2
// CHECK-LOGS-NEXT: Updated reference at test.cpp:23:27: v3 -> local_v3
// CHECK-LOGS-NEXT: Updated reference at test.cpp:23:31: ll -> global_ll
// CHECK-LOGS-NEXT: Renamed variable at test.cpp:25:12: v5 -> static_v5
// CHECK-LOGS-NEXT: Updated reference at test.cpp:25:35: v4 -> static_v4

// RUN: %clang_cc1 -load %llvmshlibdir/AddingPrefixesPlugin_Suvorov_Dmitrii_FIIT1_ClangAST%pluginext -plugin AddingPrefixesPlugin_Suvorov_Dmitrii_FIIT1_ClangAST -plugin-arg-AddingPrefixesPlugin_Suvorov_Dmitrii_FIIT1_ClangAST --help -fsyntax-only %s 2>&1 | FileCheck --match-full-lines %s --check-prefix=CHECK-HELP-ARG
// CHECK-HELP-ARG: Rename Plugin - Adds prefixes to variables and parameters.
// CHECK-HELP-ARG-NEXT: Usage:
// CHECK-HELP-ARG-NEXT:   --no-log         Disable logging of changes
// CHECK-HELP-ARG-NEXT:   --help           Show this help message