; RUN: opt -load-pass-plugin %llvmshlibdir/AddReplacing_Polyakov_Alexey_FIIT2_LLVM_IR%pluginext -passes="AddReplacing" -S %s | FileCheck %s

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