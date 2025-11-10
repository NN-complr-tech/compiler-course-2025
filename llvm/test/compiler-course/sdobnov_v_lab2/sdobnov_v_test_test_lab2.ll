; RUN: opt -load-pass-plugin %llvmshlibdir/DivisionOptimizer_SdobnovV_FIIT2_LLVM_IR%pluginext \
; RUN: -passes=optimize-divisions -S %s | FileCheck %s

; Тест оптимизации знакового деления на степень двойки
; Исходная операция: value / 8
; Ожидаемая оптимизация: value >> 3
; CHECK-LABEL: @test_signed_division_power_of_two
; CHECK-NOT: sdiv i32
; CHECK: ashr i32 %input_value, 3
; CHECK: ret i32

define i32 @test_signed_division_power_of_two(i32 %input_value) {
entry:
  %division_result = sdiv i32 %input_value, 8
  ret i32 %division_result
}

; Тест сохранения деления на не-степень двойки
; Исходная операция: value / 5
; Ожидается: сохранение операции деления
; CHECK-LABEL: @test_division_non_power_of_two
; CHECK: sdiv i32 %input_value, 5
; CHECK: ret i32

define i32 @test_division_non_power_of_two(i32 %input_value) {
entry:
  %division_result = sdiv i32 %input_value, 5
  ret i32 %division_result
}

; Тест оптимизации беззнакового деления
; Исходная операция: value / 16 (беззнаковое)
; Ожидаемая оптимизация: value >>> 4
; CHECK-LABEL: @test_unsigned_division_optimization
; CHECK-NOT: udiv i32
; CHECK: lshr i32 %input_value, 4
; CHECK: ret i32

define i32 @test_unsigned_division_optimization(i32 %input_value) {
entry:
  %division_result = udiv i32 %input_value, 16
  ret i32 %division_result
}

; Тест игнорирования других арифметических операций
; Исходная операция: value * 2
; Ожидается: сохранение умножения
; CHECK-LABEL: @test_multiplication_untouched
; CHECK: mul i32 %input_value, 2
; CHECK: ret i32

define i32 @test_multiplication_untouched(i32 %input_value) {
entry:
  %multiplication_result = mul i32 %input_value, 2
  ret i32 %multiplication_result
}

; Тест деления на простое число
; Исходная операция: value / 17
; Ожидается: сохранение деления
; CHECK-LABEL: @test_prime_number_division
; CHECK: sdiv i32 %input_value, 17
; CHECK: ret i32

define i32 @test_prime_number_division(i32 %input_value) {
entry:
  %division_result = sdiv i32 %input_value, 17
  ret i32 %division_result
}

; Тест минимальной степени двойки
; Исходная операция: value / 2
; Ожидаемая оптимизация: value >> 1
; CHECK-LABEL: @test_minimal_power_of_two
; CHECK-NOT: sdiv i32
; CHECK: ashr i32 %input_value, 1
; CHECK: ret i32

define i32 @test_minimal_power_of_two(i32 %input_value) {
entry:
  %division_result = sdiv i32 %input_value, 2
  ret i32 %division_result
}

; Тест обработки деления на ноль
; Исходная операция: value / 0
; Ожидается: сохранение (undefined behavior)
; CHECK-LABEL: @test_division_by_zero
; CHECK: sdiv i32 %input_value, 0
; CHECK: ret i32

define i32 @test_division_by_zero(i32 %input_value) {
entry:
  %division_result = sdiv i32 %input_value, 0
  ret i32 %division_result
}

; Тест деления на переменную величину
; Исходная операция: value / divisor
; Ожидается: сохранение динамического деления
; CHECK-LABEL: @test_dynamic_division
; CHECK: sdiv i32 %input_value, %divisor_value
; CHECK: ret i32

define i32 @test_dynamic_division(i32 %input_value, i32 %divisor_value) {
entry:
  %division_result = sdiv i32 %input_value, %divisor_value
  ret i32 %division_result
}

; Тест оптимизации отрицательного делителя (степень двойки)
; Исходная операция: value / -4
; Ожидаемая оптимизация: -(value >> 2)
; CHECK-LABEL: @test_negative_power_of_two
; CHECK-NOT: sdiv i32
; CHECK: %signed_shift_div = ashr i32 %input_value, 2
; CHECK: %negated_shift = sub i32 0, %signed_shift_div
; CHECK: ret i32 %negated_shift

define i32 @test_negative_power_of_two(i32 %input_value) {
entry:
  %division_result = sdiv i32 %input_value, -4
  ret i32 %division_result
}

; Тест отрицательного не-степени двойки
; Исходная операция: value / -11
; Ожидается: сохранение деления
; CHECK-LABEL: @test_negative_non_power
; CHECK: sdiv i32 %input_value, -11
; CHECK: ret i32

define i32 @test_negative_non_power(i32 %input_value) {
entry:
  %division_result = sdiv i32 %input_value, -11
  ret i32 %division_result
}

; Тест большой степени двойки
; Исходная операция: value / 1024
; Ожидаемая оптимизация: value >> 10
; CHECK-LABEL: @test_large_power_of_two
; CHECK-NOT: sdiv i32
; CHECK: ashr i32 %input_value, 10
; CHECK: ret i32

define i32 @test_large_power_of_two(i32 %input_value) {
entry:
  %division_result = sdiv i32 %input_value, 1024
  ret i32 %division_result
}

; Тест граничного случая (деление на 1)
; Исходная операция: value / 1
; Ожидаемая оптимизация: value >> 0 (или value)
; CHECK-LABEL: @test_division_by_one
; CHECK-NOT: sdiv i32
; CHECK: ashr i32 %input_value, 0
; CHECK: ret i32

define i32 @test_division_by_one(i32 %input_value) {
entry:
  %division_result = sdiv i32 %input_value, 1
  ret i32 %division_result
}

; Тест смешанных операций в функции
; Проверка, что пасс корректно работает в контексте других инструкций
; CHECK-LABEL: @test_mixed_operations
; CHECK: mul i32 %input_value, 3
; CHECK-NOT: sdiv i32 {{.*}}, 4
; CHECK: ashr i32 {{.*}}, 2
; CHECK: add i32 {{.*}}, 10
; CHECK: ret i32

define i32 @test_mixed_operations(i32 %input_value) {
entry:
  %mul_result = mul i32 %input_value, 3
  %div_result = sdiv i32 %mul_result, 4
  %final_result = add i32 %div_result, 10
  ret i32 %final_result
}