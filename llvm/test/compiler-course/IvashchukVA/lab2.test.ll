; RUN: opt -passes=mark-pure-functions -S %s | FileCheck %s

define void @pure_function() {
; CHECK: define void @pure_function() #0
  ret void
}

define void @impure_function() {
; CHECK: define void @impure_function() {
  call void @external_function()
  ret void
}

declare void @external_function()

; CHECK: attributes #0 = { memory(none) }