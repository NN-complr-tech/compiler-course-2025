#include <immintrin.h>

// Test 1: Basic FMA-operation (a * b + c)
float test_fma_basic(float a, float b, float c) {
    return _mm_cvtss_f32(_mm_fmadd_ss(_mm_set_ss(a), _mm_set_ss(b), _mm_set_ss(c)));
}

// Test 2: FMA with same operand
float test_fma_same_reg(float a, float b) {
    return _mm_cvtss_f32(_mm_fmadd_ss(_mm_set_ss(a), _mm_set_ss(a), _mm_set_ss(b)));
}

// Test 3: No FMA
float test_no_fma(float a, float b, float c) {
    __m128 mul = _mm_mul_ss(_mm_set_ss(a), _mm_set_ss(b));
    return _mm_cvtss_f32(mul);
}

// Test 4: No FMA, but mul + add separately
float test_mul_add(float a, float b, float c) {
    __m128 mul = _mm_mul_ss(_mm_set_ss(a), _mm_set_ss(b));
    __m128 res = _mm_add_ss(mul, _mm_set_ss(c));
    return _mm_cvtss_f32(res);
}