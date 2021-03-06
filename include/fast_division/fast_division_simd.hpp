﻿/**
*  Fast Division Library
*  Created by Stefan Ivanov
*
*  Using ideas from
*  Division by Invariant Integers Using Multiplication (1994)
*  by Torbjörn Granlund, Peter L. Montgomery
*/
#pragma once

#include <immintrin.h>
#include <cstdint>

#include <fast_division/fast_division_base.hpp>
#include <fast_division/division_policy.hpp>
#include <fast_division/utility/associated_types.hpp>

namespace fast_division {

    /// Specializations for various simd types.
    template<> template<>
    inline
    __m128i constant_divider_base<uint32_t, false, promotion_policy>::operator()<> (__m128i input) const
    {
        __m128i m = _mm_set1_epi32(multiplier_);
        __m128i s1 = _mm_setr_epi32(shift_1_, 0, 0, 0);
        __m128i s2 = _mm_setr_epi32(shift_2_, 0, 0, 0);
        // Multiply unsigned integers at positions 0 and 2 in n with the multiplier.
        __m128i batch_1_unshifted = _mm_mul_epu32(input, m);
        // Store the high bits of the results.
        __m128i batch_1 = _mm_srli_epi64(batch_1_unshifted, 32);
        // Shift the input in order to compute the product of the integers at positions 1 and 3.
        __m128i n_shift = _mm_srli_epi64(input, 32);
        __m128i batch_2 = _mm_mul_epu32(n_shift, m);
        // Create a mask to extract the correct bits in the two batches.
        __m128i mask = _mm_set_epi32(-1, 0, -1, 0);
        __m128i mult_result = _mm_blendv_epi8(batch_1, batch_2, mask);
        // Continue the algorithm normally, i.e
        // (((input - mult_result) >> s1) + mult_result) >> s2
        __m128i minus_result = _mm_sub_epi32(input, mult_result);
        __m128i first_shift = _mm_srl_epi32(minus_result, s1);
        __m128i add_result = _mm_add_epi32(first_shift, mult_result);
        __m128i second_shift = _mm_srl_epi32(add_result, s2);
        return second_shift;
    }

    template<> template<>
    inline
    __m256i constant_divider_base<uint32_t, false, promotion_policy>::operator()<> (__m256i input) const
    {
        __m256i m = _mm256_set1_epi32(multiplier_);
        __m128i s1 = _mm_setr_epi32(shift_1_, 0, 0, 0);
        __m128i s2 = _mm_setr_epi32(shift_2_, 0, 0, 0);
        // Multiply unsigned integers at positions 0 and 2 in n with the multiplier.
        __m256i batch_1_unshifted = _mm256_mul_epu32(input, m);
        // Store the high bits of the results.
        __m256i batch_1 = _mm256_srli_epi64(batch_1_unshifted, 32);
        // Shift the input in order to compute the product of the integers at positions 1 and 3.
        __m256i n_shift = _mm256_srli_epi64(input, 32);
        __m256i batch_2 = _mm256_mul_epu32(n_shift, m);
        // Create a mask to extract the correct bits in the two batches.
        __m256i mask = _mm256_set_epi32(-1, 0, -1, 0, -1, 0, 0, 1);
        __m256i mult_result = _mm256_blendv_epi8(batch_1, batch_2, mask);
        // Continue the algorithm normally, i.e
        // (((input - mult_result) >> s1) + mult_result) >> s2
        __m256i minus_result = _mm256_sub_epi32(input, mult_result);
        __m256i first_shift = _mm256_srl_epi32(minus_result, s1);
        __m256i add_result = _mm256_add_epi32(first_shift, mult_result);
        __m256i second_shift = _mm256_srl_epi32(add_result, s2);
        return second_shift;
    }

    template <typename Integer, template <typename, bool> class P, typename Simd, typename = std::enable_if_t<utility::is_simd<Simd>>>
    inline
    Simd operator/ (Simd divident, const constant_divider_base<Integer, std::is_signed<Integer>::value, P>& divisor)
    {
        return divisor(divident);
    }

}