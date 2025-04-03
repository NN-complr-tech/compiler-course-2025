; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacePass_Mamaeva_Olga_FIIT3_LLVM_IR%pluginext\
; RUN: -passes=ChangeADD -S %s | FileCheck %s

; CHECK: define i32 @add(i32 %a, i32 %b)
; CHECK: %result = add i32 %a, %b
; CHECK: ret i32 %result

; CHECK: define i32 @foo(i32 %x, i32 %y)
; CHECK: result1 = call i32 @add(i32 %x, i32 %y)
; CHECK: ret i32 %result1

define i32 @add(i32 %a, i32 %b) {
    %result = add i32 %a, %b
    ret i32 %result
}

define i32 @foo(i32 %x, i32 %y) {
    %result = add i32 %x, %y
    ret i32 %result
}
