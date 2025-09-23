; RUN: opt -load-pass-plugin %llvmshlibdir/Add_Pass_Mironov_Arseniy_FIIT1_LLVM_IR%pluginext\
; RUN: -passes=Add_Pass -S %s 2>&1 | FileCheck %s -dump-input=always

; CHECK-LABEL: define i32 @test_case1(i32 %x, i32 %y)
; CHECK-NEXT: %sum = add i32 %x, %y
; CHECK-NEXT: ret i32 %sum

define i32 @test_case1(i32 %x, i32 %y) {
    %sum = add i32 %x, %y
    ret i32 %sum
}
