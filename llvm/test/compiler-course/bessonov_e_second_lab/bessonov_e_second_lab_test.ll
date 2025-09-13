; RUN: opt -load-pass-plugin %llvmshlibdir/PureFuncPass_Bessonov_Egor_FIIT2_LLVM_IR%pluginext -passes="mark-pure" -S %s | FileCheck %s

@global_f = external global float
@sync_var = global i32 0


; CHECK: @add_ints
; CHECK-SAME: #0
define i32 @add_ints(i32 %x, i32 %y) {
entry:
  %sum = add i32 %x, %y
  ret i32 %sum
}

; CHECK: @division
; CHECK-SAME: #0
define float @division(i32 %a, float %b) {
entry:
  %fa = sitofp i32 %a to float
  %res = fdiv float %fa, %b
  ret float %res
}


; CHECK: @multiply
; CHECK-NOT: #0
define float @multiply(float %val) {
entry:
  %g = load float, ptr @global_f
  %res = fmul float %g, %val
  ret float %res
}

; CHECK: @convert
; CHECK-NOT: #0
define float @convert(ptr %ptr) {
entry:
  %v = load i32, ptr %ptr
  %c = sitofp i32 %v to float
  ret float %c
}

; CHECK: @volatile_test
; CHECK-NOT: #0
define float @volatile_test() {
entry:
  %tmp = alloca i32
  %val1 = load volatile i32, ptr %tmp
  %next = add i32 %val1, 5
  store volatile i32 %next, ptr %tmp
  %val2 = load volatile i32, ptr %tmp
  %conv = sitofp i32 %val2 to float
  ret float %conv
}

; CHECK: @atomic_op
; CHECK-NOT: #0
define void @atomic_op() {
entry:
  store atomic i32 123, ptr @sync_var monotonic, align 4
  ret void
}

; CHECK: @atomic_read
; CHECK-NOT: #0
define i32 @atomic_read() {
entry:
  %r = load atomic i32, ptr @sync_var monotonic, align 4
  ret i32 %r
}

; CHECK: #0
; CHECK-SAME: pure
attributes #0 = { "pure" }