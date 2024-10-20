#pragma once

#include <immintrin.h>

class SIMDUtils {
public:
    static void vectorized_add(float* a, float* b, float* result, int size) {
        for (int i = 0; i < size; i += 8) {
            __m256 va = _mm256_loadu_ps(a + i);
            __m256 vb = _mm256_loadu_ps(b + i);
            __m256 vr = _mm256_add_ps(va, vb);
            _mm256_storeu_ps(result + i, vr);
        }
    }
};
