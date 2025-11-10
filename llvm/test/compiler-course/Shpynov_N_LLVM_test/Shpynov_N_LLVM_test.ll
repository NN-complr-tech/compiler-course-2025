; RUN: opt -load-pass-plugin %llvmshlibdir/PureFunctionPass_Shpynov_Nikita_FIIT1_LLVM_IR%pluginext\
; RUN: -passes=purefunctionpass  -S %s | FileCheck %s
; c++ tests: https://compiler-explorer.com/z/nMhfz96oW

module asm ".globl _ZSt21ios_base_library_initv"

%"class.std::basic_ostream" = type { ptr, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", ptr, i8, i8, ptr, ptr, ptr, ptr }
%"class.std::ios_base" = type { ptr, i64, i64, i32, i32, i32, ptr, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, ptr, %"class.std::locale" }
%"struct.std::ios_base::_Words" = type { ptr, i64 }
%"class.std::locale" = type { ptr }

$__clang_call_terminate = comdat any

@glob_value = dso_local local_unnamed_addr global i32 0, align 4
@atomicVar = dso_local local_unnamed_addr global { i32 } zeroinitializer, align 4
@_ZSt4cout = external global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr constant [7 x i8] c"Hello\0A\00", align 1
@_ZZ19impureStaticCountervE7counter = internal unnamed_addr global i32 0, align 4
@.str.1 = private unnamed_addr constant [4 x i8] c"---\00", align 1
@_ZTISt13runtime_error = external constant ptr
@_ZSt4cerr = external global %"class.std::basic_ostream", align 8
@.str.2 = private unnamed_addr constant [11 x i8] c"Exception\0A\00", align 1
; CHECK: define dso_local noundef i32 @_Z7pureAddii(i32 noundef %a, i32 noundef %b) local_unnamed_addr #0 {
define dso_local noundef i32 @_Z7pureAddii(i32 noundef %a, i32 noundef %b) local_unnamed_addr {
entry:
  %add = add nsw i32 %b, %a
  ret i32 %add
}
; CHECK: define dso_local void @_Z9pureEmptyv() local_unnamed_addr #0 {
define dso_local void @_Z9pureEmptyv() local_unnamed_addr {
entry:
  ret void
}
; CHECK: define dso_local noundef i32 @_Z20pureCallPureFunctionii(i32 noundef %x, i32 noundef %y) local_unnamed_addr #0 {
define dso_local noundef i32 @_Z20pureCallPureFunctionii(i32 noundef %x, i32 noundef %y) local_unnamed_addr {
entry:
  %add.i = add nsw i32 %y, %x
  ret i32 %add.i
}
; CHECK: define dso_local noundef i32 @_Z16impureAtomicReadv() local_unnamed_addr {
define dso_local noundef i32 @_Z16impureAtomicReadv() local_unnamed_addr {
entry:
  %0 = load atomic i32, ptr @atomicVar seq_cst, align 4
  ret i32 %0
}
; CHECK: define dso_local noundef i32 @_Z16impureGlobalReadv() local_unnamed_addr {
define dso_local noundef i32 @_Z16impureGlobalReadv() local_unnamed_addr {
entry:
    %0 = load float, ptr @glob_value, align 4
    %conv = fptosi float %0 to i32
    ret i32 %conv
}
; CHECK: define dso_local void @_Z8impureIOv() local_unnamed_addr {
define dso_local void @_Z8impureIOv() local_unnamed_addr {
entry:
  %call1.i = tail call noundef nonnull align 8 dereferenceable(8) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr noundef nonnull align 8 dereferenceable(8) @_ZSt4cout, ptr noundef nonnull @.str, i64 noundef 6)
  ret void
}
; CHECK: define dso_local void @_Z21impureIncrementGlobalv() local_unnamed_addr {
define dso_local void @_Z21impureIncrementGlobalv() local_unnamed_addr {
entry:
  %0 = load i32, ptr @glob_value, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, ptr @glob_value, align 4
  ret void
}
; CHECK: define dso_local noundef float @_Z16impureReadGlobalv() local_unnamed_addr {
define dso_local noundef float @_Z16impureReadGlobalv() local_unnamed_addr {
entry:
  %0 = load i32, ptr @glob_value, align 4
  %conv = sitofp i32 %0 to float
  ret float %conv
}
; CHECK: define dso_local noundef range(i32 -2147483647, -2147483648) i32 @_Z19impureStaticCounterv() local_unnamed_addr {
define dso_local noundef range(i32 -2147483647, -2147483648) i32 @_Z19impureStaticCounterv() local_unnamed_addr {
entry:
  %0 = load i32, ptr @_ZZ19impureStaticCountervE7counter, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, ptr @_ZZ19impureStaticCountervE7counter, align 4
  ret i32 %add
}
; CHECK: define dso_local void @_Z19impureTryCatchBlocki(i32 noundef %x) local_unnamed_addr personality ptr @__gxx_personality_v0 {
define dso_local void @_Z19impureTryCatchBlocki(i32 noundef %x) local_unnamed_addr personality ptr @__gxx_personality_v0 {
entry:
  %cmp = icmp eq i32 %x, 0
  br i1 %cmp, label %if.then, label %try.cont

if.then:
  %exception = tail call ptr @__cxa_allocate_exception(i64 16) #10
  invoke void @_ZNSt13runtime_errorC1EPKc(ptr noundef nonnull align 8 dereferenceable(16) %exception, ptr noundef nonnull @.str.1)
          to label %invoke.cont unwind label %lpad

invoke.cont:
  invoke void @__cxa_throw(ptr nonnull %exception, ptr nonnull @_ZTISt13runtime_error, ptr nonnull @_ZNSt13runtime_errorD1Ev) #11
          to label %unreachable unwind label %lpad1

lpad:
  %0 = landingpad { ptr, i32 }
          catch ptr null
  tail call void @__cxa_free_exception(ptr %exception) #10
  br label %catch

lpad1:
  %1 = landingpad { ptr, i32 }
          catch ptr null
  br label %catch

catch:
  %.pn = phi { ptr, i32 } [ %1, %lpad1 ], [ %0, %lpad ]
  %exn.slot.0 = extractvalue { ptr, i32 } %.pn, 0
  %2 = tail call ptr @__cxa_begin_catch(ptr %exn.slot.0) #10
  %call1.i7 = invoke noundef nonnull align 8 dereferenceable(8) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr noundef nonnull align 8 dereferenceable(8) @_ZSt4cerr, ptr noundef nonnull @.str.2, i64 noundef 10)
          to label %invoke.cont3 unwind label %lpad2

invoke.cont3:
  tail call void @__cxa_end_catch()
  br label %try.cont

try.cont:
  ret void

lpad2:
  %3 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %eh.resume unwind label %terminate.lpad

eh.resume:
  resume { ptr, i32 } %3

terminate.lpad:
  %4 = landingpad { ptr, i32 }
          catch ptr null
  %5 = extractvalue { ptr, i32 } %4, 0
  tail call void @__clang_call_terminate(ptr %5) #12
  unreachable

unreachable:
  unreachable
}

declare ptr @__cxa_allocate_exception(i64) local_unnamed_addr

declare void @_ZNSt13runtime_errorC1EPKc(ptr noundef nonnull align 8 dereferenceable(16), ptr noundef) unnamed_addr #5

declare i32 @__gxx_personality_v0(...)

declare void @__cxa_free_exception(ptr) local_unnamed_addr

declare void @_ZNSt13runtime_errorD1Ev(ptr noundef nonnull align 8 dereferenceable(16)) unnamed_addr #6

declare void @__cxa_throw(ptr, ptr, ptr) local_unnamed_addr #7

declare ptr @__cxa_begin_catch(ptr) local_unnamed_addr

declare void @__cxa_end_catch() local_unnamed_addr

define linkonce_odr hidden void @__clang_call_terminate(ptr noundef %0) local_unnamed_addr comdat {
  %2 = tail call ptr @__cxa_begin_catch(ptr %0) #10
  tail call void @_ZSt9terminatev() #12
  unreachable
}

declare void @_ZSt9terminatev() local_unnamed_addr #9
; CHECK: define dso_local noundef i32 @_Z25impureIndirectCallExamplePFiiiEii(ptr noundef readonly %func, i32 noundef %a, i32 noundef %b) local_unnamed_addr {
define dso_local noundef i32 @_Z25impureIndirectCallExamplePFiiiEii(ptr noundef readonly %func, i32 noundef %a, i32 noundef %b) local_unnamed_addr {
entry:
  %func_cast = bitcast ptr %func to i32 (i32, i32)*
  %call = tail call i32 %func_cast(i32 %a, i32 %b)
  ret i32 %call
}
; CHECK: define dso_local void @_Z24impureCallImpureFunctioni(i32 noundef %x) local_unnamed_addr {
define dso_local void @_Z24impureCallImpureFunctioni(i32 noundef %x) local_unnamed_addr {
entry:
  %0 = load i32, ptr @glob_value, align 4
  %add.i = add nsw i32 %0, 1
  store i32 %add.i, ptr @glob_value, align 4
  ret void
}

declare noundef nonnull align 8 dereferenceable(8) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr noundef nonnull align 8 dereferenceable(8), ptr noundef, i64 noundef) local_unnamed_addr #5


attributes #0 = { "pure" }