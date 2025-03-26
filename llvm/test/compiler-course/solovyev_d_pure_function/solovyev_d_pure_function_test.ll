; RUN: opt -load-pass-plugin %llvmshlibdir/PureFunctionPass_Solovyev_Danila_FIIT3_LLVM_IR.so -passes="PureFunctionPass" -S %s | FileCheck %s 

@g_value = external local_unnamed_addr global float, align 4

; CHECK: #0
define dso_local noundef i32 @_Z3addii(i32 noundef %a, i32 noundef %b) local_unnamed_addr{
entry:
  %add = add nsw i32 %b, %a
  ret i32 %add
}

; CHECK-NOT: #0
define dso_local noundef float @_Z3foof(float noundef %a) local_unnamed_addr {
entry:
  %0 = load float, ptr @g_value, align 4
  %mul = fmul float %0, %a
  ret float %mul
}

; CHECK-NOT: #0
define dso_local noundef float @_Z3barRi(ptr nocapture noundef nonnull readonly align 4 dereferenceable(4) %c) local_unnamed_addr {
entry:
  %0 = load i32, ptr %c, align 4
  %add = add nsw i32 %0, 3
  %conv = sitofp i32 %add to float
  ret float %conv
}

; CHECK-NOT: #0
define dso_local noundef float @_Z3bazv() local_unnamed_addr {
entry:
  %a = alloca i32, align 4
  %a.0.a.0.a.0.a.0. = load volatile i32, ptr %a, align 4
  %inc = add nsw i32 %a.0.a.0.a.0.a.0., 1
  store volatile i32 %inc, ptr %a, align 4
  %a.0.a.0.a.0.a.0.1 = load volatile i32, ptr %a, align 4
  %conv = sitofp i32 %a.0.a.0.a.0.a.0.1 to float
  ret float %conv
}

; CHECK: #0
define dso_local noundef float @_Z3quxif(i32 noundef %a, float noundef %c) local_unnamed_addr {
entry:
  %conv = sitofp i32 %a to float
  %div = fdiv float %conv, %c
  ret float %div
}

; CHECK-NOT: #0
define dso_local noundef float @_Z5corgev() local_unnamed_addr {
entry:
  %a.i = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %a.i)
  %a.i.0.a.i.0.a.i.0.a.0.a.0.a.0..i = load volatile i32, ptr %a.i, align 4
  %inc.i = add nsw i32 %a.i.0.a.i.0.a.i.0.a.0.a.0.a.0..i, 1
  store volatile i32 %inc.i, ptr %a.i, align 4
  %a.i.0.a.i.0.a.i.0.a.0.a.0.a.0.1.i = load volatile i32, ptr %a.i, align 4
  %conv.i = sitofp i32 %a.i.0.a.i.0.a.i.0.a.0.a.0.a.0.1.i to float
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %a.i)
  ret float %conv.i
}

; CHECK: #0
attributes #0 = {"pure"}
