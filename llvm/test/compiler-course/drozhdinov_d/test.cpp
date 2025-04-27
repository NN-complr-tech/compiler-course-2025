#include <immintrin.h>

float fma_ss(float a, float b, float c) {
	return _mm_cvtss_f32(_mm_fmadd_ss(_mm_set_ss(a), _mm_set_ss(b), _mm_set_ss(c)));
}

double fma_sd(double a, double b, double c) {
	return _mm_cvtsd_f64(_mm_fmadd_sd(_mm_set_sd(a), _mm_set_sd(b), _mm_set_sd(c)));
}

__m128 fma_ps(__m128 a, __m128 b, __m128 c) {
	return _mm_fmadd_ps(a, b, c);
}

__m256d fma_pd(__m256d a, __m256d b, __m256d c) {
	return _mm256_fmadd_pd(a, b, c);
}
