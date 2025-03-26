; RUN: opt -load-pass-plugin %llvmshlibdir/DivOptPass_KholinKirill_FIIT3_LLVM_IR%pluginext -passes=div-opt-pass -S %s | FileCheck %s

; CHECK: define dso_local void @_Z18unsigned_part_testv() {
; CHECK-NEXT: entry:
; CHECK: %1 = lshr i32 %0, 1
; CHECK-NEXT: store i32 %1, ptr %result_type_OK1, align 4
; CHECK: %div1 = udiv i32 %2, -2
; CHECK-NEXT: store i32 %div1, ptr %UB1, align 4
; CHECK: %4 = lshr i32 %3, 3
; CHECK-NEXT: store i32 %4, ptr %result_type_OK2, align 4
; CHECK-NEXT: ret void

;void unsigned_part_test() {
;  unsigned int lhs1 = 1024;
;
;  unsigned int result_type_OK1 = lhs1 / 2;
;  unsigned int UB1 = lhs1 / (-2);
;
;  unsigned int lhs2 = 512;
;
;  unsigned int result_type_OK2 = lhs2 / 8u;
;}

define dso_local void @_Z18unsigned_part_testv() #0 {
entry:
  %lhs1 = alloca i32, align 4
  %result_type_OK1 = alloca i32, align 4
  %UB1 = alloca i32, align 4
  %lhs2 = alloca i32, align 4
  %result_type_OK2 = alloca i32, align 4
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
  ret void
}

; CHECK: define dso_local void @_Z17signed_part1_testv()
; entry:
; CHECK: %1 = ashr i32 %0, 6
; CHECK-NEXT: store i32 %1, ptr %result_type_OK3, align 4
; CHECK: %div1 = sdiv i32 %2, -64
; CHECK-NEXT: store i32 %div1, ptr %Not_UB1, align 4
; CHECK: %4 = ashr i32 %sub, 6
; CHECK-NEXT: store i32 %4, ptr %result_type_OK4, align 4
; CHECK: %div4 = sdiv i32 %sub3, -64
; CHECK-NEXT: store i32 %div4, ptr %Not_UB2, align 4
; CHECK: %7 = lshr i32 %6, 5
; CHECK-NEXT: store i32 %7, ptr %result_type_OK5, align 4
; CHECK: %9 = lshr i32 %sub6, 5
; CHECK-NEXT: store i32 %9, ptr %UB2, align 4
; CHECK-NEXT: ret void

define dso_local void @_Z17signed_part1_testv() #0 {
entry:
  %lhs3 = alloca i32, align 4
  %result_type_OK3 = alloca i32, align 4
  %Not_UB1 = alloca i32, align 4
  %result_type_OK4 = alloca i32, align 4
  %Not_UB2 = alloca i32, align 4
  %lhs4 = alloca i32, align 4
  %result_type_OK5 = alloca i32, align 4
  %UB2 = alloca i32, align 4
  store i32 256, ptr %lhs3, align 4
  %0 = load i32, ptr %lhs3, align 4
  %div = sdiv i32 %0, 64
  store i32 %div, ptr %result_type_OK3, align 4
  %1 = load i32, ptr %lhs3, align 4
  %div1 = sdiv i32 %1, -64
  store i32 %div1, ptr %Not_UB1, align 4
  %2 = load i32, ptr %lhs3, align 4
  %sub = sub nsw i32 0, %2
  %div2 = sdiv i32 %sub, 64
  store i32 %div2, ptr %result_type_OK4, align 4
  %3 = load i32, ptr %lhs3, align 4
  %sub3 = sub nsw i32 0, %3
  %div4 = sdiv i32 %sub3, -64
  store i32 %div4, ptr %Not_UB2, align 4
  store i32 128, ptr %lhs4, align 4
  %4 = load i32, ptr %lhs4, align 4
  %div5 = udiv i32 %4, 32
  store i32 %div5, ptr %result_type_OK5, align 4
  %5 = load i32, ptr %lhs4, align 4
  %sub6 = sub nsw i32 0, %5
  %div7 = udiv i32 %sub6, 32
  store i32 %div7, ptr %UB2, align 4
  ret void
}

;void signed_part1_test() {
;  signed int lhs3 = 256;
;
; signed int result_type_OK3 = lhs3 / 64;
; signed int Not_UB1 = lhs3 / (- 64);
;  signed int result_type_OK4 = (-lhs3) / 64;
;  signed int Not_UB2 = (-lhs3) / (-64); 
;
;  signed int lhs4 = 128;
;
;  signed int result_type_OK5 = lhs4 / 32u;
;  signed int UB2 = (-lhs4) / 32u;
;}

; CHECK: define dso_local void @_Z17signed_part2_testv(
; CHECK-NEXT: entry:
; CHECK: %div = udiv i32 %0, 3
; CHECK-NEXT: store i32 %div, ptr %result_type_OK7, align 4
; CHECK: %div1 = udiv i32 %sub, 3
; CHECK-NEXT: store i32 %div1, ptr %UB3, align 4
; CHECK: %3 = ashr i32 %2, 2
; CHECK-NEXT: store i32 %3, ptr %result_type_OK9, align 4
; CHECK: %5 = lshr i32 %sub3, 2
; CHECK-NEXT: store i32 %5, ptr %UB4, align 4
; CHECK-NEXT: ret void

define dso_local void @_Z17signed_part2_testv() #0 {
entry:
  %lhs5 = alloca i32, align 4
  %result_type_OK7 = alloca i32, align 4
  %UB3 = alloca i32, align 4
  %lhs6 = alloca i32, align 4
  %result_type_OK9 = alloca i32, align 4
  %UB4 = alloca i32, align 4
  store i32 128, ptr %lhs5, align 4
  %0 = load i32, ptr %lhs5, align 4
  %div = udiv i32 %0, 3
  store i32 %div, ptr %result_type_OK7, align 4
  %1 = load i32, ptr %lhs5, align 4
  %sub = sub nsw i32 0, %1
  %div1 = udiv i32 %sub, 3
  store i32 %div1, ptr %UB3, align 4
  store i32 126, ptr %lhs6, align 4
  %2 = load i32, ptr %lhs6, align 4
  %div2 = sdiv i32 %2, 4
  store i32 %div2, ptr %result_type_OK9, align 4
  %3 = load i32, ptr %lhs6, align 4
  %sub3 = sub nsw i32 0, %3
  %div4 = udiv i32 %sub3, 4
  store i32 %div4, ptr %UB4, align 4
  ret void
}

;void signed_part2_test() {
;  signed int lhs5 = 128;
;
;  signed int result_type_OK7 = lhs5 / 3u;
;  signed int UB3 = (-lhs5) / 3u;
;
;  signed int lhs6 = 126;
;
;  signed int result_type_OK9 = lhs6 / 4;
;  signed int UB4 = (-lhs6) / 4u;
;}

; CHECK: define dso_local noundef i32 @main()
; CHECK-NEXT: entry:
; CHECK: %div = fdiv double %0, 4.000000e+00
; CHECK: %div1 = fdiv double %1, -4.000000e+00
; CHECK: %div2 = fdiv float %conv, 0x4042BC2900000000
; CHECK: %div5 = fdiv float %conv4, 0xC042BC2900000000
; CHECK: %5 = ashr i64 %4, 5
; CHECK-NEXT: store i64 %5, ptr %result_type_OK15, align 8
; CHECK: %div8 = sdiv i64 %6, -32
; CHECK: store i32 0, ptr %over_bits, align 4
; CHECK: ret i32 0

define dso_local noundef i32 @main() #1 {
entry:
  %retval = alloca i32, align 4
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
  store double 0x408BF770A3D70A3D, ptr %lhs7, align 8
  %0 = load double, ptr %lhs7, align 8
  %div = fdiv double %0, 4.000000e+00
  store double %div, ptr %result_type_OK11, align 8
  %1 = load double, ptr %lhs7, align 8
  %div1 = fdiv double %1, -4.000000e+00
  store double %div1, ptr %result_type_OK12, align 8
  store i32 6, ptr %lhs8, align 4
  %2 = load i32, ptr %lhs8, align 4
  %conv = sitofp i32 %2 to float
  %div2 = fdiv float %conv, 0x4042BC2900000000
  %conv3 = fptosi float %div2 to i32
  store i32 %conv3, ptr %result_type_OK13, align 4
  %3 = load i32, ptr %lhs8, align 4
  %conv4 = sitofp i32 %3 to float
  %div5 = fdiv float %conv4, 0xC042BC2900000000
  %conv6 = fptosi float %div5 to i32
  store i32 %conv6, ptr %result_type_OK14, align 4
  store i64 312345, ptr %lhs9, align 8
  %4 = load i64, ptr %lhs9, align 8
  %div7 = sdiv i64 %4, 32
  store i64 %div7, ptr %result_type_OK15, align 8
  %5 = load i64, ptr %lhs9, align 8
  %div8 = sdiv i64 %5, -32
  store i64 %div8, ptr %result_type_OK16, align 8
  store i32 0, ptr %over_bits, align 4
  ret i32 0
}

;int main() {  
;  double lhs7 = 894.93;
;
;  double result_type_OK11 = lhs7 / 4;
;  double result_type_OK12 = lhs7 / (-4);
;
;  int lhs8 = 6;
;
;  int result_type_OK13 = lhs8 / 37.47f;
;  int result_type_OK14 = lhs8 / (-37.47f);
;
;  long long lhs9 = 312345LL;
;
;  long long result_type_OK15 = lhs9 / 32;
;  long long result_type_OK16 = lhs9 / (-32);
;
;  int over_bits = 32 / 64;
;
;  return 0;
;}
