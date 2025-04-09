; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacePass_Mamaeva_Olga_FIIT3_LLVM_IR%pluginext -passes="add-replace" -S %s | FileCheck %s

; Test 1: No @add function - must remain unchanged
; CHECK-LABEL: define i32 @test_no_add
; CHECK-NEXT: %res = add i32 %x, %y
; CHECK-NEXT: ret i32 %res
; CHECK-NOT: call i32 @add

define i32 @test_no_add(i32 %x, i32 %y) {
  %res = add i32 %x, %y
  ret i32 %res
}

; Test 2: With @add function - should be replaced
; CHECK-LABEL: define i32 @add
; CHECK-NEXT: %result = add i32 %a, %b
; CHECK-NEXT: ret i32 %result

; CHECK-LABEL: define i32 @test_with_add
; CHECK-NEXT: %result = call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT: ret i32 %result

define i32 @add(i32 %a, i32 %b) {
  %result = add i32 %a, %b
  ret i32 %result
}

define i32 @test_with_add(i32 %x, i32 %y) {
  %result = add i32 %x, %y
  ret i32 %result
}

; Test 3: Mismatched types - must remain unchanged
; CHECK-LABEL: define i64 @test_mismatch
; CHECK-NEXT: %sum = add i64 %a, %b
; CHECK-NEXT: ret i64 %sum
; CHECK-NOT: call i32 @add

define i64 @test_mismatch(i64 %a, i64 %b) {
  %sum = add i64 %a, %b
  ret i64 %sum
}
