; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacePass_Mamaeva_Olga_FIIT3_LLVM_IR%pluginext -passes="add-replace" -S %s | FileCheck %s

; CHECK-LABEL: define i32 @test_no_add
; CHECK: add i32 %x, %y
; CHECK-NOT: call i32 @add
; CHECK: ret i32

define i32 @test_no_add(i32 %x, i32 %y) {
  %1 = add i32 %x, %y
  ret i32 %1
}

; CHECK-LABEL: define i32 @add
; CHECK: add i32 %a, %b
; CHECK: ret i32

; CHECK-LABEL: define i32 @test_with_add
; CHECK: call i32 @add(i32 %x, i32 %y)
; CHECK: ret i32

define i32 @add(i32 %a, i32 %b) {
  %1 = add i32 %a, %b
  ret i32 %1
}

define i32 @test_with_add(i32 %x, i32 %y) {
  %1 = add i32 %x, %y
  ret i32 %1
}

; CHECK-LABEL: define i64 @test_mismatch
; CHECK: add i64 %a, %b
; CHECK-NOT: call i32 @add
; CHECK: ret i64

define i64 @test_mismatch(i64 %a, i64 %b) {
  %1 = add i64 %a, %b
  ret i64 %1
}
