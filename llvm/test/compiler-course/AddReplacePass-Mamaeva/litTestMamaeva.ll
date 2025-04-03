; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacePass_Mamaeva_Olga_FIIT3_LLVM_IR%pluginext \
; RUN: -passes="add-replace" -S %s | FileCheck %s

; CHECK-LABEL: define i32 @add(
; CHECK: %result = add i32 %a, %b
; CHECK: ret i32 %result

; CHECK-LABEL: define i32 @foo(
; CHECK: %result = call i32 @add(i32 %x, i32 %y)
; CHECK: ret i32 %result

define i32 @add(i32 %a, i32 %b) {
    %result = add i32 %a, %b
    ret i32 %result
}

define i32 @foo(i32 %x, i32 %y) {
    %result = add i32 %x, %y
    ret i32 %result
}
