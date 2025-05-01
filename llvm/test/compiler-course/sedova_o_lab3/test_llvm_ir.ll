; RUN: opt -load-pass-plugin %llvmshlibdir/ExamplePass_Ivanov_Ivan_FIIT0_LLVM_IR%pluginext\
; RUN: -passes=example -S %s | FileCheck %s

; CHECK: _Z6isEveni

define dso_local noundef zeroext i1 @_Z6isEveni(i32 noundef %0) {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = load i32, ptr %2, align 4
  %4 = srem i32 %3, 2
  %5 = icmp eq i32 %4, 0
  ret i1 %5
}
