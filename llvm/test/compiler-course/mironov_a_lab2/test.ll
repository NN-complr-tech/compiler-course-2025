; RUN: opt -load-pass-plugin %llvmshlibdir/Add_Pass_Mironov_Arseniy_FIIT1_LLVM_IR%pluginext\
; RUN: -passes=Add_Pass -S %s 2>&1 | FileCheck %s -dump-input=always


; CHECK-LABEL: define i32 @add(i32 %a, i32 %b)
; CHECK-NEXT: %result = add i32 %a, %b
; CHECK-NEXT: ret i32 %result

; CHECK-LABEL: define i32 @test_case1(i32 %x, i32 %y)
; CHECK-NEXT: %1 = call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT: ret i32 %1
; CHECK-NOT: %sum = add i32 %x, %y

; CHECK-LABEL: define i32 @test_case2(i32 %x, i32 %y, i32 %z)
; CHECK-NEXT: %1 = call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT: %2 = call i32 @add(i32 %1, i32 %z)
; CHECK-NEXT: ret i32 %2
; CHECK-NOT: %res1 = add i32 %x, %y
; CHECK-NOT: %res2 = add i32 %res1, %z

define i32 @add(i32 %a, i32 %b) {
    %result = add i32 %a, %b
    ret i32 %result
}

define i32 @test_case1(i32 %x, i32 %y) {
    %sum = add i32 %x, %y
    ret i32 %sum
}

define i32 @test_case2(i32 %x, i32 %y, i32 %z) {
    %res1 = add i32 %x, %y
    %res2 = add i32 %res1, %z
    ret i32 %res2
}
