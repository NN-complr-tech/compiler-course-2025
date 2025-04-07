; RUN: opt -load-pass-plugin %llvmshlibdir/DivOptPass_KholinKirill_FIIT3_LLVM_IR%pluginext -passes=div-opt-pass -S %s | FileCheck %s

; CHECK-LABEL: define dso_local noundef i32 @_Z18unsigned_part_testjj(i32 noundef %lhs1, i32 noundef %lhs2) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = lshr i32 %lhs1, 1
; CHECK-NEXT: %div1 = udiv i32 %lhs1, -2
; CHECK-NEXT: %1 = lshr i32 %lhs2, 3
; CHECK-NEXT: %add = add i32 %0, %div1
; CHECK-NEXT: %add3 = add i32 %add, %1


define dso_local noundef i32 @_Z18unsigned_part_testjj(i32 noundef %lhs1, i32 noundef %lhs2) #0 {
entry:
  %div = udiv i32 %lhs1, 2
  %div1 = udiv i32 %lhs1, -2
  %div2 = udiv i32 %lhs2, 8
  %add = add i32 %div, %div1
  %add3 = add i32 %add, %div2
  ret i32 %add3
}

;unsigned unsigned_part_test(unsigned lhs1, unsigned lhs2) {
;  unsigned int result_type_OK1 = lhs1 / 2;
;  unsigned int UB1 = lhs1 / (-2);
;  unsigned int result_type_OK2 = lhs2 / 8u;
;
;  return result_type_OK1 + UB1 + result_type_OK2;
;}

; CHECK-LABEL: define dso_local noundef i32 @_Z17signed_part1_testii(i32 noundef %lhs1, i32 noundef %lhs2) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %0 = ashr i32 %lhs1, 6
; CHECK-NEXT: %div1 = sdiv i32 %lhs1, -64
; CHECK: %1 = ashr i32 %sub, 6
; CHECK-NEXT: %div4 = sdiv i32 %sub, -64
; CHECK-NEXT: %2 = lshr i32 %lhs2, 5
; CHECK: %3 = lshr i32 %sub6, 5
; CHECK-NEXT: %add = add nsw i32 %0, %div1
; CHECK-NEXT: %add8 = add nsw i32 %add, %1
; CHECK-NEXT: %add9 = add nsw i32 %add8, %div4
; CHECK-NEXT: %add10 = add nsw i32 %add9, %2
; CHECK-NEXT: %add11 = add nsw i32 %add10, %3

define dso_local noundef i32 @_Z17signed_part1_testii(i32 noundef %lhs1, i32 noundef %lhs2) #0 {
entry:
  %div = sdiv i32 %lhs1, 64
  %div1 = sdiv i32 %lhs1, -64
  %sub = sub nsw i32 0, %lhs1
  %div2 = sdiv i32 %sub, 64
  %div4 = sdiv i32 %sub, -64
  %div5 = udiv i32 %lhs2, 32
  %sub6 = sub nsw i32 0, %lhs2
  %div7 = udiv i32 %sub6, 32
  %add = add nsw i32 %div, %div1
  %add8 = add nsw i32 %add, %div2
  %add9 = add nsw i32 %add8, %div4
  %add10 = add nsw i32 %add9, %div5
  %add11 = add nsw i32 %add10, %div7
  ret i32 %add11
}

;signed int signed_part1_test(signed int lhs1, signed int lhs2) {
;  signed int result_type_OK3 = lhs1 / 64;
;  signed int Not_UB1 = lhs1 / (-64);
;  signed int result_type_OK4 = (-lhs1) / 64;
;  signed int Not_UB2 = (-lhs1) / (-64);
;  signed int result_type_OK5 = lhs2 / 32u;
;  signed int UB2 = (-lhs2) / 32u;
;
;  return result_type_OK3 + Not_UB1 + result_type_OK4 +
;         Not_UB2 + result_type_OK5 + UB2;
;}

; CHECK-LABEL: define dso_local noundef i32 @_Z17signed_part2_testii(i32 noundef %lhs1, i32 noundef %lhs2) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %div = udiv i32 %lhs1, 3
; CHECK: %div1 = udiv i32 %sub, 3
; CHECK-NEXT: %0 = ashr i32 %lhs2, 2
; CHECK: %1 = lshr i32 %sub3, 2
; CHECK-NEXT: %add = add nsw i32 %div, %div1
; CHECK-NEXT: %add5 = add nsw i32 %add, %0
; CHECK-NEXT: %add6 = add nsw i32 %add5, %1

define dso_local noundef i32 @_Z17signed_part2_testii(i32 noundef %lhs1, i32 noundef %lhs2) #0 {
entry:
  %div = udiv i32 %lhs1, 3
  %sub = sub nsw i32 0, %lhs1
  %div1 = udiv i32 %sub, 3
  %div2 = sdiv i32 %lhs2, 4
  %sub3 = sub nsw i32 0, %lhs2
  %div4 = udiv i32 %sub3, 4
  %add = add nsw i32 %div, %div1
  %add5 = add nsw i32 %add, %div2
  %add6 = add nsw i32 %add5, %div4
  ret i32 %add6
}

;signed int signed_part2_test(signed int lhs1, signed int lhs2) {
;  signed int result_type_OK7 = lhs1 / 3u;
;  signed int UB3 = (-lhs1) / 3u;
;  signed int result_type_OK9 = lhs2 / 4;
;  signed int UB4 = (-lhs2) / 4u;
;
;  return result_type_OK7 + UB3 + result_type_OK9 + UB4;
;}

; CHECK-LABEL: define dso_local noundef i32 @_Z11other_typesdix(double noundef %lhs1, i32 noundef %lhs2, i64 noundef %lhs3) {
; CHECK-NEXT: entry:
; CHECK-NEXT: %div = fdiv double %lhs1, 4.000000e+00
; CHECK-NEXT: %div1 = fdiv double %lhs1, -4.000000e+00
; CHECK-NEXT: %conv = sitofp i32 %lhs2 to float
; CHECK-NEXT: %div2 = fdiv float %conv, 0x4042BC2900000000
; CHECK-NEXT: %conv3 = fptosi float %div2 to i32
; CHECK-NEXT: %div5 = fdiv float %conv, 0xC042BC2900000000
; CHECK-NEXT: %conv6 = fptosi float %div5 to i32
; CHECK-NEXT: %0 = ashr i64 %lhs3, 5
; CHECK-NEXT: %div8 = sdiv i64 %lhs3, -32
; CHECK-NEXT: %add = fadd double %div, %div1
; CHECK-NEXT: %conv9 = sitofp i32 %conv3 to double
; CHECK-NEXT: %add10 = fadd double %add, %conv9
; CHECK-NEXT: %conv11 = sitofp i32 %conv6 to double
; CHECK-NEXT: %add12 = fadd double %add10, %conv11
; CHECK-NEXT: %conv13 = sitofp i64 %0 to double
; CHECK-NEXT: %add14 = fadd double %add12, %conv13
; CHECK-NEXT: %conv15 = sitofp i64 %div8 to double
; CHECK-NEXT: %add16 = fadd double %add14, %conv15
; CHECK-NEXT: %conv17 = fptosi double %add16 to i32

define dso_local noundef i32 @_Z11other_typesdix(double noundef %lhs1, i32 noundef %lhs2, i64 noundef %lhs3) #0 {
entry:
  %div = fdiv double %lhs1, 4.000000e+00
  %div1 = fdiv double %lhs1, -4.000000e+00
  %conv = sitofp i32 %lhs2 to float
  %div2 = fdiv float %conv, 0x4042BC2900000000
  %conv3 = fptosi float %div2 to i32
  %div5 = fdiv float %conv, 0xC042BC2900000000
  %conv6 = fptosi float %div5 to i32
  %div7 = sdiv i64 %lhs3, 32
  %div8 = sdiv i64 %lhs3, -32
  %add = fadd double %div, %div1
  %conv9 = sitofp i32 %conv3 to double
  %add10 = fadd double %add, %conv9
  %conv11 = sitofp i32 %conv6 to double
  %add12 = fadd double %add10, %conv11
  %conv13 = sitofp i64 %div7 to double
  %add14 = fadd double %add12, %conv13
  %conv15 = sitofp i64 %div8 to double
  %add16 = fadd double %add14, %conv15
  %conv17 = fptosi double %add16 to i32
  ret i32 %conv17
}

;int other_types(double lhs1, int lhs2, long long lhs3) {
;  double result_type_OK11 = lhs1 / 4;
;  double result_type_OK12 = lhs1 / (-4);
;  int result_type_OK13 = lhs2 / 37.47f;
;  int result_type_OK14 = lhs2 / (-37.47f);
;  long long result_type_OK15 = lhs3 / 32;
;  long long result_type_OK16 = lhs3 / (-32);
;  int over_bits = 32 / 64;
;
;  return result_type_OK11 + result_type_OK12 + result_type_OK13 +
;         result_type_OK14 + result_type_OK15 + result_type_OK16;
;}
