; RUN: opt -load-pass-plugin %llvmshlibdir/ReplaceAddWithCall_Sedova_Olga_FIIT1_LLVM_IR%pluginext\
; RUN: -passes=replace-add -S %s | FileCheck %s

; CHECK-LABEL: define i32 @no_add_decl(i32 %x, i32 %y)
; CHECK:         %sum = add i32 %x, %y
; CHECK-NOT:     call i32 @add

define i32 @no_add_decl(i32 %x, i32 %y) {
  %sum = add i32 %x, %y
  ret i32 %sum
}