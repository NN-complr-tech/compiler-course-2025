; RUN: opt -load-pass-plugin %llvmshlibdir/PureFunctionPass_Kabalova_Valeria_FIIT1_LLVM_IR%pluginext\
; RUN: -passes=purefunctionpass -S %s | FileCheck %s
; Tests: https://compiler-explorer.com/z/xxbnvnaj8

module asm ".globl _ZSt21ios_base_library_initv"

%"class.std::basic_istream" = type { ptr, i64, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", ptr, i8, i8, ptr, ptr, ptr, ptr }
%"class.std::ios_base" = type { ptr, i64, i64, i32, i32, i32, ptr, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, ptr, %"class.std::locale" }
%"struct.std::ios_base::_Words" = type { ptr, i64 }
%"class.std::locale" = type { ptr }
%"class.std::basic_ostream" = type { ptr, %"class.std::basic_ios" }
%"struct.std::__atomic_base" = type { i32 }

$_ZNSt13__atomic_baseIjEppEv = comdat any

@value = dso_local global float 0.000000e+00, align 4
@_ZZ2p2vE1x = internal global { i32 } zeroinitializer, align 4
@_ZZ2f1vE1x = internal global i32 0, align 4
@g_value = external global float, align 4
@_ZSt3cin = external global %"class.std::basic_istream", align 8
@_ZZ2f5vE1x = internal global i32 0, align 4
@_ZSt4cout = external global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr constant [14 x i8] c"Hello, world!\00", align 1

; Pure function
; CHECK: define dso_local noundef i32 @_Z2p1ii(i32 noundef %a, i32 noundef %b) #0 {
define dso_local noundef i32 @_Z2p1ii(i32 noundef %a, i32 noundef %b) {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %0 = load i32, ptr %a.addr, align 4
  %1 = load i32, ptr %b.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

; Pure function
; CHECK: define dso_local void @_Z2p2v() #0 {
define dso_local void @_Z2p2v() {
entry:
  %call = call noundef i32 @_ZNSt13__atomic_baseIjEppEv(ptr noundef nonnull align 4 dereferenceable(4) @_ZZ2p2vE1x)
  ret void
}

; Help-function for previous (atomic)
define linkonce_odr dso_local noundef i32 @_ZNSt13__atomic_baseIjEppEv(ptr noundef nonnull align 4 dereferenceable(4) %this) #0 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  %.atomictmp = alloca i32, align 4
  %atomic-temp = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %_M_i = getelementptr inbounds %"struct.std::__atomic_base", ptr %this1, i32 0, i32 0
  store i32 1, ptr %.atomictmp, align 4
  %0 = load i32, ptr %.atomictmp, align 4
  %1 = atomicrmw add ptr %_M_i, i32 %0 seq_cst, align 4
  %2 = add i32 %1, %0
  store i32 %2, ptr %atomic-temp, align 4
  %3 = load i32, ptr %atomic-temp, align 4
  ret i32 %3
}

; Pure function - empty
; CHECK: define dso_local void @_Z5emptyv() #0 {
define dso_local void @_Z5emptyv() {
entry:
  ret void
}

; Impure function case 1: return value variation with a static variable
; CHECK: define dso_local noundef i32 @_Z2f1v() {
define dso_local noundef i32 @_Z2f1v() {
entry:
  %0 = load i32, ptr @_ZZ2f1vE1x, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr @_ZZ2f1vE1x, align 4
  %1 = load i32, ptr @_ZZ2f1vE1x, align 4
  ret i32 %1
}

; Impure function case 2: return value variation with a non-local variable
; CHECK: define dso_local noundef i32 @_Z2f2v() {
define dso_local noundef i32 @_Z2f2v() {
entry:
  %0 = load float, ptr @g_value, align 4
  %conv = fptosi float %0 to i32
  ret i32 %conv
}

; Impure function case 2: return value variation with a non-local variable
; CHECK: define dso_local noundef i32 @_Z3f22v() {
define dso_local noundef i32 @_Z3f22v() {
entry:
  %0 = load float, ptr @value, align 4
  %conv = fptosi float %0 to i32
  ret i32 %conv
}

; Impure function case 3: return value variation with a mutable reference argument
; CHECK: define dso_local noundef i32 @_Z2f3Pi(ptr noundef %x) { 
define dso_local noundef i32 @_Z2f3Pi(ptr noundef %x) {
entry:
  %x.addr = alloca ptr, align 8
  store ptr %x, ptr %x.addr, align 8
  %0 = load ptr, ptr %x.addr, align 8
  %1 = load i32, ptr %0, align 4
  ret i32 %1
}

; Impure function case 4: return value variation with an input stream
; CHECK: define dso_local noundef i32 @_Z2f4v() {
define dso_local noundef i32 @_Z2f4v() {
entry:
  %x = alloca i32, align 4
  store i32 0, ptr %x, align 4
  %call = call noundef nonnull align 8 dereferenceable(16) ptr @_ZNSirsERi(ptr noundef nonnull align 8 dereferenceable(16) @_ZSt3cin, ptr noundef nonnull align 4 dereferenceable(4) %x)
  %0 = load i32, ptr %x, align 4
  ret i32 %0
}

declare noundef nonnull align 8 dereferenceable(16) ptr @_ZNSirsERi(ptr noundef nonnull align 8 dereferenceable(16), ptr noundef nonnull align 4 dereferenceable(4))

; Impure function case 5: mutation of a local static variable
; CHECK: define dso_local void @_Z2f5v() {
define dso_local void @_Z2f5v() {
entry:
  %0 = load i32, ptr @_ZZ2f5vE1x, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr @_ZZ2f5vE1x, align 4
  ret void
}

; Impure function case 6: mutation of a non-local variable
; CHECK: define dso_local void @_Z2f6v() {
define dso_local void @_Z2f6v() {
entry:
  %0 = load float, ptr @value, align 4
  %inc = fadd float %0, 1.000000e+00
  store float %inc, ptr @value, align 4
  ret void
}

; Impure function case 7: mutation of a mutable reference argument
; CHECK: define dso_local void @_Z2f7Pi(ptr noundef %x) {
define dso_local void @_Z2f7Pi(ptr noundef %x) {
entry:
  %x.addr = alloca ptr, align 8
  store ptr %x, ptr %x.addr, align 8
  %0 = load ptr, ptr %x.addr, align 8
  %1 = load i32, ptr %0, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %0, align 4
  ret void
}

; Impure function case 8: mutation of an output stream
; CHECK: define dso_local void @_Z2f8v() {
define dso_local void @_Z2f8v() {
entry:
  %call = call noundef nonnull align 8 dereferenceable(8) ptr @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(ptr noundef nonnull align 8 dereferenceable(8) @_ZSt4cout, ptr noundef @.str)
  %call1 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSolsEPFRSoS_E(ptr noundef nonnull align 8 dereferenceable(8) %call, ptr noundef @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_)
  ret void
}

declare noundef nonnull align 8 dereferenceable(8) ptr @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(ptr noundef nonnull align 8 dereferenceable(8), ptr noundef) #2

declare noundef nonnull align 8 dereferenceable(8) ptr @_ZNSolsEPFRSoS_E(ptr noundef nonnull align 8 dereferenceable(8), ptr noundef) #2

declare noundef nonnull align 8 dereferenceable(8) ptr @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(ptr noundef nonnull align 8 dereferenceable(8)) #2

; CHECK: attributes #0 = { "pure" }
attributes #0 = { "pure" }
