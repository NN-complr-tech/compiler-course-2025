; RUN: split-file %s %t
; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacePass_Kudryashova_Irina_FIIT3_LLVM_IR%pluginext -passes="AddReplacePass" -S %t/a.ll | FileCheck %t/a.ll
; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacePass_Kudryashova_Irina_FIIT3_LLVM_IR%pluginext -passes="AddReplacePass" -S %t/b.ll | FileCheck %t/b.ll

;--- a.ll
; CHECK-LABEL: @add
; CHECK: add i32 %a, %b
define i32 @add(i32 %a, i32 %b) {
  %result = add i32 %a, %b
  ret i32 %result
}

; CHECK-LABEL: @foo
; CHECK: call i32 @add(i32 %x, i32 %y)
define i32 @foo(i32 %x, i32 %y) {
  %sum = add i32 %x, %y
  ret i32 %sum
}

; CHECK-LABEL: @bar
; CHECK: add i64 %x, %y
define i64 @bar(i64 %x, i64 %y) {
  %sum = add i64 %x, %y
  ret i64 %sum
}

;--- b.ll
; CHECK-LABEL: @far
; CHECK: add i64 %x, %y
define i64 @far(i64 %x, i64 %y) {
  %sum = add i64 %x, %y
  ret i64 %sum
}
