; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacePass_Mamaeva_Olga_FIIT3_LLVM_IR%pluginext \
; RUN: -passes="add-replace" -S %s | FileCheck %s

; CHECK-LABEL: @add
; CHECK: add i32 %a, %b
; CHECK: ret i32 %result

; CHECK-LABEL: @foo
; CHECK: %[[RES:.*]] = call i32 @add(i32 %x, i32 %y)
; CHECK: ret i32 %[[RES]]

; CHECK-LABEL: @bar
; CHECK: %sum = add i64 %a, %b
; CHECK: ret i64 %sum

; CHECK-LABEL: @test_no_add
; CHECK: %res = add i32 %x, %y
; CHECK-NOT: call i32 @add
; CHECK: ret i32 %res

define i32 @add(i32 %a, i32 %b) {
    %result = add i32 %a, %b
    ret i32 %result
}

define i32 @foo(i32 %x, i32 %y) {
    %result = add i32 %x, %y
    ret i32 %result
}

define i64 @bar(i64 %a, i64 %b) {
    %sum = add i64 %a, %b
    ret i64 %sum
}

define i32 @test_no_add(i32 %x, i32 %y) {
  %res = add i32 %x, %y
  ret i32 %res
}
