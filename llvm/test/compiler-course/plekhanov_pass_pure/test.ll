; RUN: opt -load-pass-plugin %llvmshlibdir/PureFunc_Plekhanov_Daniil_FIIT2_LLVM_IR%pluginext\
; RUN: -passes=pure-func-pass -S %s | FileCheck %s

; int mul(int a, int b) { return a * b; }
; CHECK: define i32 @_Z3muli(i32 %a, i32 %b) #0
define i32 @_Z3muli(i32 %a, i32 %b) {
entry:
  %res = mul i32 %a, %b
  ret i32 %res
}

@counter = global i32 0

; void inc() { counter++; }
; CHECK: define void @_Z3incv()
; CHECK-NOT: #0
define void @_Z3incv() {
entry:
  %val = load i32, ptr @counter
  %inc = add i32 %val, 1
  store i32 %inc, ptr @counter
  ret void
}

; int identity(int x) { return x; }
; CHECK: define i32 @_Z8identityi(i32 %x) #0
define i32 @_Z8identityi(i32 %x) {
entry:
  ret i32 %x
}

declare void @external_impure()

; void wrapper() { external_impure(); }
; CHECK: define void @_Z7wrapperv()
; CHECK-NOT: #0
define void @_Z7wrapperv() {
entry:
  call void @external_impure()
  ret void
}

; int abs(int x) { return x < 0 ? -x : x; }
; CHECK: define i32 @_Z3absi(i32 %x) #0
define i32 @_Z3absi(i32 %x) {
entry:
  %cmp = icmp slt i32 %x, 0
  %neg = sub i32 0, %x
  %res = select i1 %cmp, i32 %neg, i32 %x
  ret i32 %res
}

@gptr = global ptr null

; int read_gptr() { return ptrtoint(ptr @gptr to i32); }
; CHECK: define i32 @_Z9read_gptrv()
; CHECK-NOT: #0
define i32 @_Z9read_gptrv() {
entry:
  %p = load ptr, ptr @gptr
  %i = ptrtoint ptr %p to i32
  ret i32 %i
}

; CHECK: attributes #0 = { "pure" }
attributes #0 = { "pure" }
