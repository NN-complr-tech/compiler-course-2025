; RUN: opt -load-pass-plugin %llvmshlibdir/ReplacerPass_ShulpinIlya_FIIT1_LLVM_IR%pluginext \
; RUN: -passes="ReplacerPass" -S %s | FileCheck %s

;===------------------------------------------------------------------------===;
; Test 1: @add - Reference implementation of add function (should not change)
; Checks that function @add itself is not transformed by the pass
;===------------------------------------------------------------------------===;

; CHECK: define i32 @add(i32 %a, i32 %b)
; CHECK: %result = add i32 %a, %b
; CHECK: ret i32 %result
; CHECK-NOT: add i32

define i32 @add(i32 %a, i32 %b) {
  %result = add i32 %a, %b
  ret i32 %result
}

;===------------------------------------------------------------------------===;
; Test 2: @foo - Matching types, call to @add should replace the add instruction
; This should be transformed to a call to @add because the operand types match
;===------------------------------------------------------------------------===;

; CHECK-LABEL: define i32 @foo(i32 %x, i32 %y)
; CHECK-NEXT: call i32 @add(i32 %x, i32 %y)
; CHECK-NEXT: ret i32 %sum
; CHECK-NOT: add i32

define i32 @foo(i32 %x, i32 %y) {
  %sum = add i32 %x, %y
  ret i32 %sum
}


;===------------------------------------------------------------------------===;
; Test 3: @bar - Same as @foo, another usage of i32 add replaced with @add
; Verifies that the pass replaces all i32 add instructions consistently
;===------------------------------------------------------------------------===;

; CHECK-LABEL: define i32 @bar(i32 %m, i32 %n)
; CHECK-NEXT: %sum1 = call i32 @add(i32 %m, i32 %n)
; CHECK-NEXT: ret i32 %sum
; CHECK-NOT: call i32 @add

define i32 @bar(i32 %m, i32 %n) {
  %sum = add i32 %m, %n
  ret i32 %sum
}

;===------------------------------------------------------------------------===;
; Test 4: @goo - i64 add should not be replaced, because @add only accepts i32
; Ensures type mismatch prevents transformation
;===------------------------------------------------------------------------===;

; CHECK-LABEL: define i64 @goo(i64 %x, i64 %y)
; CHECK-NEXT: %sum = add i64 %x, %y
; CHECK-NEXT: ret i64 %sum

define i64 @goo(i64 %x, i64 %y) {
  %sum = add i64 %x, %y
  ret i64 %sum
}

;===------------------------------------------------------------------------===;
; Test 5: @foo_alt - No add instruction here, should remain unchanged
; Also checks that unrelated functions are not transformed
;===------------------------------------------------------------------------===;


; CHECK-NOT: define i64 @add(
; CHECK-LABEL: define i64 @foo_alt(i64 %x)

define i64 @foo_alt(i64 %x) {
  ret i64 %x
}

;===------------------------------------------------------------------------===;
; Test 6: @baz - add of mismatched type (i1), should not be replaced
; Verifies the pass skips non-i32 instructions (e.g. bitwise bools)
;===------------------------------------------------------------------------===;

; CHECK-LABEL: define i1 @baz(i1 %a, i1 %b)
; CHECK: %r = add i1 %a, %b
; CHECK-NOT: call i32 @add

define i1 @baz(i1 %a, i1 %b) {
  %r = add i1 %a, %b
  ret i1 %r
}

;===------------------------------------------------------------------------===;
; Test 7: @multiple_adds - Only i32 adds should be replaced, others kept
; This mixes types to ensure partial transformation
;===------------------------------------------------------------------------===;

; CHECK-LABEL: define void @multiple_adds(i32 %a, i32 %b, i64 %x, i64 %y)
; CHECK: call i32 @add(i32 %a, i32 %b)
; CHECK: %b2 = add i64 %x, %y

define void @multiple_adds(i32 %a, i32 %b, i64 %x, i64 %y) {
  %b1 = add i32 %a, %b
  %b2 = add i64 %x, %y
  call void @use(i32 %b1, i64 %b2)
  ret void
}

declare void @use(i32, i64)
