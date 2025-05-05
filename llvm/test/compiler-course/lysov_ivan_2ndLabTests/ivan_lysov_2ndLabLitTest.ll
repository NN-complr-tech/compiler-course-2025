; RUN: opt -load-pass-plugin %llvmshlibdir/pureFunctionPass_Lysov_Ivan_FIIT3_LLVM_IR%pluginext\
; RUN: -passes=pure-func-pass -S %s | FileCheck %s

; double x;
@x = dso_local local_unnamed_addr global double 0.000000e+00, align 8

; int add(int a, int b) { return a + b; }
; CHECK: define dso_local noundef i32 @_Z3addii(i32 noundef %a, i32 noundef %b) local_unnamed_addr #0 
define dso_local noundef i32 @_Z3addii(i32 noundef %a, i32 noundef %b) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %b, %a
  ret i32 %add
}

; int f2() { return 0; }
; CHECK: define dso_local noundef i32 @_Z2f2v() local_unnamed_addr #0 
define dso_local noundef i32 @_Z2f2v() local_unnamed_addr #0 {
entry:
  ret i32 0
}

; int f1() { return add(2, 3); }
; CHECK: define dso_local noundef i32 @_Z2f1v()
; CHECK-NOT: #0
define dso_local noundef i32 @_Z2f1v() #1 {
entry:
  %call = call noundef i32 @_Z3addii(i32 noundef 2, i32 noundef 3)
  ret i32 %call
}


; double h() { double f = x; return f; }
; CHECK: define dso_local noundef double @_Z1hv()
; CHECK-NOT: #0
define dso_local noundef double @_Z1hv() local_unnamed_addr #1 {
entry:
  %0 = load double, ptr @x, align 8
  ret double %0
}

; CHECK: attributes #0 = { "pure" }
attributes #0 = { "pure" }
