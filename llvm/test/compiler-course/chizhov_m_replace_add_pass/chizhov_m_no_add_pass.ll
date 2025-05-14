; RUN: opt -load-pass-plugin %llvmshlibdir/ReplaceAddPass_Chizhov_Maxim_FIIT3_LLVM_IR%pluginext\

; RUN: -passes=ReplaceAddPass -S %s | FileCheck %s


; CHECK-NOT: add i32

define i32 @boo(i32 %x, i32 %y) {

entry:
  %sum = sub i32 %x, %y
  ret i32 %sum
}

; CHECK-LABEL: define i32 @sub(i32 %a, i32 %b)
; CHECK: entry:
; CHECK-NEXT: %result = sub i32 %a, %b
; CHECK-NEXT: ret i32 %result

define i32 @sub(i32 %a, i32 %b) {

entry:
  %result = sub i32 %a, %b
  ret i32 %result
}

; CHECK-NOT: add i32

define i32 @goo(i32 %x, i32 %y) {

entry:
  %result = mul i32 %x, %y
  ret i32 %result
}


; CHECK-LABEL: define i32 @mul(i32 %a, i32 %b)
; CHECK: entry:
; CHECK-NEXT: %result = mul i32 %a, %b
; CHECK-NEXT: ret i32 %result

define i32 @mul(i32 %a, i32 %b) {

entry:
  %result = mul i32 %a, %b
  ret i32 %result
}