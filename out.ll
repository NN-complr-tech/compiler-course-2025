; ModuleID = 'llvm/test/compiler-course/plekhanov_pass_pure/test.ll'
source_filename = "llvm/test/compiler-course/plekhanov_pass_pure/test.ll"

@counter = global i32 0
@gptr = global ptr null

define i32 @_Z3muli(i32 %a, i32 %b) #0 {
entry:
  %res = mul i32 %a, %b
  ret i32 %res
}

define void @_Z3incv() {
entry:
  %val = load i32, ptr @counter, align 4
  %inc = add i32 %val, 1
  store i32 %inc, ptr @counter, align 4
  ret void
}

define i32 @_Z8identityi(i32 %x) #0 {
entry:
  ret i32 %x
}

declare void @external_impure()

define void @_Z7wrapperv() {
entry:
  call void @external_impure()
  ret void
}

define i32 @_Z3absi(i32 %x) #0 {
entry:
  %cmp = icmp slt i32 %x, 0
  %neg = sub i32 0, %x
  %res = select i1 %cmp, i32 %neg, i32 %x
  ret i32 %res
}

define i32 @_Z9read_gptrv() {
entry:
  %p = load ptr, ptr @gptr, align 8
  %i = ptrtoint ptr %p to i32
  ret i32 %i
}

attributes #0 = { "pure" }
