; RUN: split-file %s %t
; RUN: opt -load-pass-plugin %llvmshlibdir/add_replace_pass_Shishkarev_Andrey_FIIT2_LLVM_IR%pluginext -passes="add_replace_pass" -S %t/a.ll | FileCheck %t/a.ll --check-prefix=CHECK-A
; RUN: opt -load-pass-plugin %llvmshlibdir/add_replace_pass_Shishkarev_Andrey_FIIT2_LLVM_IR%pluginext -passes="add_replace_pass" -S %t/a.ll | FileCheck %t/b.ll --check-prefix=CHECK-B

;--- a.ll
; CHECK-A-LABEL: @add
define i32 @add(i32 %a, i32 %b) {
  %result = add i32 %a, %b
  ret i32 %result
}

; CHECK-A-LABEL: @foo
; CHECK-A-NOT: add i32 %x, %y
; CHECK-A: call i32 @add(i32 %x, i32 %y)
define i32 @foo(i32 %x, i32 %y) {
  %sum = add i32 %x, %y
  ret i32 %sum
}

;--- b.ll
; CHECK-B-LABEL: @add
define i32 @add(i32 %a, i32 %b) {
  %result = add i32 %a, %b
  ret i32 %result
}

; CHECK-B-LABEL: @foo
; CHECK-B-NOT: add i32 %x, %y  
; CHECK-B: call i32 @add(i32 %x, i32 %y)
define i32 @foo(i32 %x, i32 %y) {
  %sum = add i32 %x, %y
  ret i32 %sum
}