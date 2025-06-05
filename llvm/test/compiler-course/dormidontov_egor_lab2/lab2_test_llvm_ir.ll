; RUN: opt -load-pass-plugin %llvmshlibdir/DivToBitwiseShiftPass_Dormidontov_Egor_FIIT2_LLVM_IR%pluginext \
; RUN: -passes=div-to-shift -S %s | FileCheck %s

; ###Проверка замены деления на степень двойки (8) на сдвиг вправо
; ###Вход:  return value / 8;
; ###После пасса:  return value >> 3;
; CHECK-LABEL: @divide_by_eight
; CHECK: ashr i32 %{{[^,]+}}, 3

define i32 @divide_by_eight(i32 %val) {
entry:
  %div = sdiv i32 %val, 8
  ret i32 %div
}

; ###Проверка, что деление на не-степень двойки (5) НЕ заменяется
; ###Вход:  return value / 5;
; ###После пасса:  return value / 5;  //никаких сдвигов
; CHECK-LABEL: @divide_by_five
; CHECK: sdiv i32 %{{[^,]+}}, 5

define i32 @divide_by_five(i32 %val) {
entry:
  %div = sdiv i32 %val, 5
  ret i32 %div
}

; ###Проверка замены беззнакового деления на степень двойки (16) на логический сдвиг вправо 
; ###Вход:  return value / 16;   //где value - unsigned
; ###После пасса:  return value >> 4;
; CHECK-LABEL: @udivide_by_sixteen
; CHECK: lshr i32 %{{[^,]+}}, 4

define i32 @udivide_by_sixteen(i32 %val) {
entry:
  %div = udiv i32 %val, 16
  ret i32 %div
}

; ###Проверка, что посторонние инструкции (умножение) не затрагиваются пассом
; ###Вход:  return value * 2;
; ###После пасса:  return value * 2; //никаких сдвигов, никаких изменений
; CHECK-LABEL: @multiply_by_two
; CHECK: mul i32 %{{[^,]+}}, 2

define i32 @multiply_by_two(i32 %val) {
entry:
  %mul = mul i32 %val, 2
  ret i32 %mul
}

