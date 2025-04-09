; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacePass_Mamaeva_Olga_FIIT3_LLVM_IR%pluginext -passes="add-replace" -S %s | FileCheck %s

; CHECK-LABEL: define i32 @test_no_add
; CHECK-NEXT: %1 = add i32 %x, %y
; CHECK-NEXT: ret i32 %1

define i32 @test_no_add(i32 %x, i32 %y) {
  %1 = add i32 %x, %y
  ret i32 %1
}

; CHECK-LABEL: define i32 @add
; CHECK-NEXT: %1 = add i32 %a, %b
; CHECK-NEXT: ret i32 %1

; CHECK-LABEL: define i32 @test_with_add
; CHECK-NEXT: %1 = call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT: ret i32 %1

define i32 @add(i32 %a, i32 %b) {
  %1 = add i32 %a, %b
  ret i32 %1
}

define i32 @test_with_add(i32 %x, i32 %y) {
  %1 = add i32 %x, %y
  ret i32 %1
}

; CHECK-LABEL: define i64 @test_mismatch
; CHECK-NEXT: %1 = add i64 %a, %b
; CHECK-NEXT: ret i64 %1

define i64 @test_mismatch(i64 %a, i64 %b) {
  %1 = add i64 %a, %b
  ret i64 %1
}

; CHECK-NOT: call i32 @add(i64
