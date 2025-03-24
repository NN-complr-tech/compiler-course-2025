; RUN: opt -load-pass-plugin %llvmshlibdir/DivOptPass_KholinKirill_FIIT3_LLVM_IR%pluginext -passes=div-opt-pass -S %s | FileCheck %s

; CHECK: define dso_local noundef i32 @main() #0 {
; CHECK-NEXT: entry:

; CHECK: %1 = lshr i32 %0, 1
; CHECK-NEXT: store i32 %1, ptr %result_type_OK1, align 4
; CHECK: %div1 = udiv i32 %2, -2
; CHECK-NEXT: store i32 %div1, ptr %UB1, align 4
; CHECK: %4 = lshr i32 %3, 3
; CHECK-NEXT: store i32 %4, ptr %result_type_OK2, align 4
; CHECK: %6 = ashr i32 %5, 6
; CHECK-NEXT: store i32 %6, ptr %result_type_OK3, align 4
; CHECK: %div4 = sdiv i32 %7, -64
; CHECK-NEXT: store i32 %div4, ptr %Not_UB1, align 4
; CHECK: %9 = ashr i32 %sub, 6
; CHECK-NEXT: store i32 %9, ptr %result_type_OK4, align 4
; CHECK: %div7 = sdiv i32 %sub6, -64
; CHECK-NEXT: store i32 %div7, ptr %Not_UB2, align 4
; CHECK: %12 = lshr i32 %11, 5
; CHECK-NEXT: store i32 %12, ptr %result_type_OK5, align 4
; CHECK: %14 = lshr i32 %sub9, 5
; CHECK-NEXT: store i32 %14, ptr %UB2, align 4
; CHECK: %div11 = udiv i32 %15, 3
; CHECK-NEXT: store i32 %div11, ptr %result_type_OK7, align 4
; CHECK: %div13 = udiv i32 %sub12, 3
; CHECK-NEXT: store i32 %div13, ptr %UB3, align 4
; CHECK: %18 = ashr i32 %17, 2
; CHECK-NEXT: store i32 %18, ptr %result_type_OK9, align 4
; CHECK: %20 = ashr i32 %sub15, 2
; CHECK-NEXT: store i32 %20, ptr %UB4, align 4
; CHECK: %div17 = fdiv double %21, 4.000000e+00
; CHECK-NEXT: store double %div17, ptr %result_type_OK11, align 8
; CHECK: %div18 = fdiv double %22, -4.000000e+00
; CHECK-NEXT: store double %div18, ptr %result_type_OK12, align 8
; CHECK: %div19 = fdiv float %conv, 0x4042BC2900000000
; CHECK: %div22 = fdiv float %conv21, 0xC042BC2900000000
; CHECK: %26 = ashr i64 %25, 5
; CHECK-NEXT: store i64 %26, ptr %result_type_OK15, align 8
; CHECK: %div25 = sdiv i64 %27, -32
; CHECK-NEXT: store i64 %div25, ptr %result_type_OK16, align 8
; CHECK:  store i32 0, ptr %over_bits, align 4

define dso_local noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %lhs1 = alloca i32, align 4
  %result_type_OK1 = alloca i32, align 4
  %UB1 = alloca i32, align 4
  %lhs2 = alloca i32, align 4
  %result_type_OK2 = alloca i32, align 4
  %lhs3 = alloca i32, align 4
  %result_type_OK3 = alloca i32, align 4
  %Not_UB1 = alloca i32, align 4
  %result_type_OK4 = alloca i32, align 4
  %Not_UB2 = alloca i32, align 4
  %lhs4 = alloca i32, align 4
  %result_type_OK5 = alloca i32, align 4
  %UB2 = alloca i32, align 4
  %lhs5 = alloca i32, align 4
  %result_type_OK7 = alloca i32, align 4
  %UB3 = alloca i32, align 4
  %lhs6 = alloca i32, align 4
  %result_type_OK9 = alloca i32, align 4
  %UB4 = alloca i32, align 4
  %lhs7 = alloca double, align 8
  %result_type_OK11 = alloca double, align 8
  %result_type_OK12 = alloca double, align 8
  %lhs8 = alloca i32, align 4
  %result_type_OK13 = alloca i32, align 4
  %result_type_OK14 = alloca i32, align 4
  %lhs9 = alloca i64, align 8
  %result_type_OK15 = alloca i64, align 8
  %result_type_OK16 = alloca i64, align 8
  %over_bits = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 1024, ptr %lhs1, align 4
  %0 = load i32, ptr %lhs1, align 4
  %div = udiv i32 %0, 2
  store i32 %div, ptr %result_type_OK1, align 4
  %1 = load i32, ptr %lhs1, align 4
  %div1 = udiv i32 %1, -2
  store i32 %div1, ptr %UB1, align 4
  store i32 512, ptr %lhs2, align 4
  %2 = load i32, ptr %lhs2, align 4
  %div2 = udiv i32 %2, 8
  store i32 %div2, ptr %result_type_OK2, align 4
  store i32 256, ptr %lhs3, align 4
  %3 = load i32, ptr %lhs3, align 4
  %div3 = sdiv i32 %3, 64
  store i32 %div3, ptr %result_type_OK3, align 4
  %4 = load i32, ptr %lhs3, align 4
  %div4 = sdiv i32 %4, -64
  store i32 %div4, ptr %Not_UB1, align 4
  %5 = load i32, ptr %lhs3, align 4
  %sub = sub nsw i32 0, %5
  %div5 = sdiv i32 %sub, 64
  store i32 %div5, ptr %result_type_OK4, align 4
  %6 = load i32, ptr %lhs3, align 4
  %sub6 = sub nsw i32 0, %6
  %div7 = sdiv i32 %sub6, -64
  store i32 %div7, ptr %Not_UB2, align 4
  store i32 128, ptr %lhs4, align 4
  %7 = load i32, ptr %lhs4, align 4
  %div8 = udiv i32 %7, 32
  store i32 %div8, ptr %result_type_OK5, align 4
  %8 = load i32, ptr %lhs4, align 4
  %sub9 = sub nsw i32 0, %8
  %div10 = udiv i32 %sub9, 32
  store i32 %div10, ptr %UB2, align 4
  store i32 128, ptr %lhs5, align 4
  %9 = load i32, ptr %lhs5, align 4
  %div11 = udiv i32 %9, 3
  store i32 %div11, ptr %result_type_OK7, align 4
  %10 = load i32, ptr %lhs5, align 4
  %sub12 = sub nsw i32 0, %10
  %div13 = udiv i32 %sub12, 3
  store i32 %div13, ptr %UB3, align 4
  store i32 126, ptr %lhs6, align 4
  %11 = load i32, ptr %lhs6, align 4
  %div14 = sdiv i32 %11, 4
  store i32 %div14, ptr %result_type_OK9, align 4
  %12 = load i32, ptr %lhs6, align 4
  %sub15 = sub nsw i32 0, %12
  %div16 = sdiv i32 %sub15, 4
  store i32 %div16, ptr %UB4, align 4
  store double 0x408BF770A3D70A3D, ptr %lhs7, align 8
  %13 = load double, ptr %lhs7, align 8
  %div17 = fdiv double %13, 4.000000e+00
  store double %div17, ptr %result_type_OK11, align 8
  %14 = load double, ptr %lhs7, align 8
  %div18 = fdiv double %14, -4.000000e+00
  store double %div18, ptr %result_type_OK12, align 8
  store i32 6, ptr %lhs8, align 4
  %15 = load i32, ptr %lhs8, align 4
  %conv = sitofp i32 %15 to float
  %div19 = fdiv float %conv, 0x4042BC2900000000
  %conv20 = fptosi float %div19 to i32
  store i32 %conv20, ptr %result_type_OK13, align 4
  %16 = load i32, ptr %lhs8, align 4
  %conv21 = sitofp i32 %16 to float
  %div22 = fdiv float %conv21, 0xC042BC2900000000
  %conv23 = fptosi float %div22 to i32
  store i32 %conv23, ptr %result_type_OK14, align 4
  store i64 312345, ptr %lhs9, align 8
  %17 = load i64, ptr %lhs9, align 8
  %div24 = sdiv i64 %17, 32
  store i64 %div24, ptr %result_type_OK15, align 8
  %18 = load i64, ptr %lhs9, align 8
  %div25 = sdiv i64 %18, -32
  store i64 %div25, ptr %result_type_OK16, align 8
  store i32 0, ptr %over_bits, align 4
  ret i32 0
}

attributes #0 = { mustprogress noinline norecurse nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 19.1.6 (git@github.com:Kirius257/compiler-course-2025.git 8797851e87b0c3c25247c18dd167cbbea02581cd)"}
