; RUN: opt -load-pass-plugin %llvmshlibdir/DivToShiftPass_Korobeinikov_Arseny_FIIT1_LLVM_IR%pluginext -passes=div-to-shift -S %s | FileCheck %s

; Проверка знаковый i32, 8 степень двойки, должны поменять на сдвиг
; CHECK-LABEL: @test_div_by_8
; CHECK-NEXT: ashr i32 %x, 3
define i32 @test_div_by_8(i32 %x) {
  %div = sdiv i32 %x, 8
  ret i32 %div
}

; Проверка знаковый i32, -4 степень двойки, должны поменять на сдвиг и вычесть из нуля
; CHECK-LABEL: @test_div_by_neg4
; CHECK-NEXT: ashr i32 %x, 2
; CHECK-NEXT: sub i32 0, %{{.*}}
define i32 @test_div_by_neg4(i32 %x) {
  %div = sdiv i32 %x, -4
  ret i32 %div
}

; Проверка беззнаковый i32, 16 степень двойки, должны поменять на сдвиг
; CHECK-LABEL: @test_udiv_by_16
; CHECK-NEXT: lshr i32 %x, 4
define i32 @test_udiv_by_16(i32 %x) {
  %div = udiv i32 %x, 16
  ret i32 %div
}

; Проверка знаковый i32, 3 не степень двойки, ничего не меняем
; CHECK-LABEL: @test_div_by_3
; CHECK-NEXT: sdiv i32 %x, 3
define i32 @test_div_by_3(i32 %x) {
  %div = sdiv i32 %x, 3
  ret i32 %div
}

; Проверка знаковый i32, 1 возвращаем само число
; CHECK-LABEL: @test_div_by_1
; CHECK-NEXT: ret i32 %x
define i32 @test_div_by_1(i32 %x) {
  %div = sdiv i32 %x, 1
  ret i32 %div
}

; Проверка знаковый i32, -1 возвращаем само число с другим знаком
; CHECK-LABEL: @test_div_by_minus1
; CHECK-NEXT: sub i32 0, %x
define i32 @test_div_by_minus1(i32 %x) {
  %div = sdiv i32 %x, -1
  ret i32 %div
}

; Проверка знаковый i32, 0 ничего не меняем
; CHECK-LABEL: @test_div_by_0
; CHECK-NEXT: sdiv i32 %x, 0
define i32 @test_div_by_0(i32 %x) {
  %div = sdiv i32 %x, 0
  ret i32 %div
}

; Проверка миксуем операции, смотрим что отрабатывает корректно на нескольких операциях
; CHECK-LABEL: @test_mixed_sequence
; CHECK: ashr i32 %x, 2
; CHECK: sdiv i32 %x, 6
; CHECK: lshr i32 %x, 3
define i32 @test_mixed_sequence(i32 %x) {
  %div1 = sdiv i32 %x, 4
  %div2 = sdiv i32 %x, 6
  %div3 = udiv i32 %x, 8
  %sum1 = add i32 %div1, %div2
  %sum2 = add i32 %sum1, %div3
  ret i32 %sum2
}

; Проверка работы с i64
; CHECK-LABEL: @test_i64_div_by_8
; CHECK-NEXT: ashr i64 %x, 3
define i64 @test_i64_div_by_8(i64 %x) {
  %div = sdiv i64 %x, 8
  ret i64 %div
}

; Проверка деления на -1 (i64)
; CHECK-LABEL: @test_i64_div_by_neg1
; CHECK-NEXT: sub i64 0, %x
define i64 @test_i64_div_by_neg1(i64 %x) {
  %div = sdiv i64 %x, -1
  ret i64 %div
}

