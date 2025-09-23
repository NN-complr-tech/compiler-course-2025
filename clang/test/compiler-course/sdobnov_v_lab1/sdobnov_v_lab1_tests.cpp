// RUN: %clang_cc1 -load %llvmshlibdir/variable_prefixer_SdobnovV_FIIT2_ClangAST%pluginext -plugin variable_prefixer -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: double static global_Value = 3.14;
// CHECK-NEXT: extern char global_Symbol;
// CHECK-NEXT: float processData(float param_Input) {
// CHECK-NEXT:   return param_Input * 2.0f;
// CHECK-NEXT: }
// CHECK-NEXT: long static calculate(long param_Base, long param_Offset) {
// CHECK-NEXT:   static long static_Counter = 0;
// CHECK-NEXT:   long local_Result = 100;
// CHECK-NEXT:   long &local_Ref = param_Base;
// CHECK-NEXT:   ++static_Counter;
// CHECK-NEXT:   return local_Ref * param_Offset + global_Value + static_Counter + local_Result;
// CHECK-NEXT: }
// CHECK-NEXT: int static global_Total = calculate(5, processData(global_Symbol));

double static Value = 3.14;
extern char Symbol;
float processData(float Input) {
  return Input * 2.0f;
}
long static calculate(long Base, long Offset) {
  static long Counter = 0;
  long Result = 100;
  long &Ref = Base;
  ++Counter;
  return Ref * Offset + Value + Counter + Result;
}
int static Total = calculate(5, processData(Symbol));