; ModuleID = 'test'
source_filename = "test"

define i32 @pure_add(i32 %a, i32 %b) {
entry:
  %0 = add i32 %a, %b
  ret i32 %0
}

define void @impure_store(i32* %ptr, i32 %val) {
entry:
  store i32 %val, i32* %ptr
  ret void
}

define i32 @call_impure(i32 %x) {
entry:
  %call = call void @impure_store(i32* null, i32 %x)
  ret i32 %x
}

declare void @impure_store(i32*, i32)