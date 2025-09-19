; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacePass_Mamaeva_Olga_FIIT3_LLVM_IR%pluginext -passes="add-replace" -S %s | FileCheck %s

; CHECK-LABEL: @test_no_add
; CHECK: %res = add i32 %x, %y
; CHECK-NOT: call i32 @add
; CHECK: ret i32 %res

define i32 @test_no_add(i32 %x, i32 %y) {
  %res = add i32 %x, %y
  ret i32 %res
}
