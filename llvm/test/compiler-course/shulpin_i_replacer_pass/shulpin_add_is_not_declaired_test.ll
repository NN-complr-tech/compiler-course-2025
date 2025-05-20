; RUN: opt -load-pass-plugin %llvmshlibdir/ReplacerPass_ShulpinIlya_FIIT1_LLVM_IR%pluginext \
; RUN: -passes="ReplacerPass" -S %s | FileCheck %s

;===------------------------------------------------------------------------===;
; Test 8: @no_add_decl — No @add in module, i32 add instructions should remain
; Verifies that when the add helper function is absent, the pass does not
; replace `add i32` with a call to @add
;===------------------------------------------------------------------------===;

; CHECK-LABEL: define i32 @no_add_decl(i32 %x, i32 %y)
; CHECK:         %sum = add i32 %x, %y
; CHECK-NOT:     call i32 @add

define i32 @no_add_decl(i32 %x, i32 %y) {
  %sum = add i32 %x, %y
  ret i32 %sum
}