; RUN: opt -load-pass-plugin %llvmshlibdir/UserPass_Beresnev_Anton_FIIT1_LLVM_IR%pluginext\
; RUN: -passes=pure_fn -S %s | FileCheck %s

@global_value = dso_local global float 0.000000e+00, align 4

; CHECK: define dso_local noundef i32 @_Z3addii(i32 noundef %a, i32 noundef %b) #0 {
define dso_local noundef i32 @_Z3addii(i32 noundef %a, i32 noundef %b) {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

; CHECK: define dso_local noundef float @_Z3foof(float noundef %a) {
define dso_local noundef float @_Z3foof(float noundef %a) {
entry:
  %a.addr = alloca float, align 4
  store float %a, ptr %a.addr, align 4
  %0 = load float, ptr %a.addr, align 4
  %1 = load float, ptr @global_value, align 4
  %mul = fmul float %0, %1
  ret float %mul
}

; CHECK: define dso_local noundef i32 @_Z14getSecretValuev() {
define dso_local noundef i32 @_Z14getSecretValuev() {
entry:
  %val = alloca i32, align 4
  store volatile i32 0, ptr %val, align 4
  %0 = load volatile i32, ptr %val, align 4
  ret i32 %0
}

; CHECK: attributes #0 = { "pure" }

