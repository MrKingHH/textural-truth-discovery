//
// Created by anxin on 2020-02-25.
//

#ifndef TEXTTRUTH_UTIL_H
#define TEXTTRUTH_UTIL_H
#include <immintrin.h>
#include <smmintrin.h>
#include <vector>

namespace hpc {
    inline bool array_equal( std::vector<int> &a, std::vector<int> &b);
    inline float hsum_ps_sse3(__m128 v) {
        __m128 shuf = _mm_movehdup_ps(v);        // broadcast elements 3,1 to 2,0
        __m128 sums = _mm_add_ps(v, shuf);
        shuf        = _mm_movehl_ps(shuf, sums); // high half -> low half
        sums        = _mm_add_ss(sums, shuf);
        return        _mm_cvtss_f32(sums);
    }
    inline float dot_product(std::vector<float> &p, std::vector<float>&q) {
        float aux = 0;
        int length = p.size();
        float *a = &p[0];
        float *b = &q[0];
        int i=0;
        for (; i < length-8; i += 8)
        {
            __m256 va = _mm256_load_ps(a + i);
            __m256 vb = _mm256_load_ps(b + i);
            __m256 dp = _mm256_dp_ps(va, vb, 0xf1);
            __m128 m128_low = _mm256_castps256_ps128(dp);
            __m128 m128_high = _mm256_extractf128_ps(dp, 1);
            __m128 m128_sum = _mm_add_ps(m128_low, m128_high);
            aux+= _mm_cvtss_f32(m128_sum);
        }
        for(;i<length-4;i+=4) {
            __m128 va = _mm_loadu_ps(a+i);
            __m128 vb = _mm_loadu_ps(b+i);
            __m128 dprot = _mm_dp_ps(va,vb,0xf1);
            aux+= _mm_cvtss_f32(dprot);
        }
        for(;i<length;i++) {
            aux += p[i] * q[i];
        }
        return aux;
    }
    inline float dot_product(float *p, float *q, int length) {
        float aux = 0;
        int i = 0;
        for (; i < length - 4; i += 4) {
            __m128 va = _mm_loadu_ps(p + i);
            __m128 vb = _mm_loadu_ps(q + i);
            __m128 dprot = _mm_dp_ps(va, vb, 0xf1);
            aux += _mm_cvtss_f32(dprot);
        }
        for (; i < length; i++) {
            aux += p[i] * q[i];
        }
        return aux;
    }
    inline void vector_mul_inplace(std::vector<float> &p, float v) {
        int length = p.size();
        int i=0;
        float * a= &p[0];
        __m128 mul_v =  _mm_load_ps1(&v);
        for(;i<length-4;i+=4) {
            __m128 va = _mm_loadu_ps(a+i);
            __m128 result = _mm_mul_ps(va,mul_v);
            _mm_store_ps(a+i,result);
        }
        for(;i<length;++i) {
            p[i] *= v;
        }
    }
    // add p+q and store result in p
    inline void vector_add_inplace(std::vector<float> &p, std::vector<float>&q) {
        float *a = &p[0];
        float *b = &q[0];
        int i=0;
        int length = p.size();
        for(;i<length-8;i+=8) {
            __m256 va = _mm256_loadu_ps(a+i);
            __m256 vb = _mm256_loadu_ps(b+i);
            __m256 result = _mm256_add_ps(va,vb);
            _mm256_store_ps(a+i, result);
        }
        for(;i<length-4;i+=4) {
            __m128 va = _mm_loadu_ps(a+i);
            __m128 vb = _mm_loadu_ps(b+i);
            __m128 result = _mm_add_ps(va,vb);
            _mm_store_ps(a+i, result);
        }
        for(;i<length;++i) {
           p[i] += q[i];
        }
    }
    inline float hsum256_ps_avx(__m256 v) {
        __m128 vlow  = _mm256_castps256_ps128(v);
        __m128 vhigh = _mm256_extractf128_ps(v, 1); // high 128
        vlow  = _mm_add_ps(vlow, vhigh);     // add the low 128
        return hsum_ps_sse3(vlow);         // and inline the sse3 version, which is optimal for AVX
        // (no wasted instructions, and all of them are the 4B minimum)
    }

    inline bool vector_cmpeq(std::vector<int>&p, std::vector<int>&q) {
        int *a  = &p[0];
        int *b =  &q[0];
        int i=0;
        int length = p.size();
        bool equal = true;
        for(;i<length-8;i+=8) {
            __m256i va = _mm256_load_si256((__m256i *)(a+i));
            __m256i vb = _mm256_load_si256((__m256i *)(b+i));
            __m256 compare_result = _mm256_cmpeq_epi32(va, vb);
            int result = _mm256_movemask_epi8(compare_result);
            equal &= (0xffffffff == result);
            if(!equal) return false;
        }
        for(;i<length;i++) {
            equal &= (p[i]==q[i]);
            if(!equal) return false;
        }
        return true;
    }
}
#endif //TEXTTRUTH_UTIL_H
