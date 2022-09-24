// -----------------------------------------------------------------------------------------
// x264guiEx/x265guiEx/svtAV1guiEx/ffmpegOut/QSVEnc/NVEnc/VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2010-2022 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// --------------------------------------------------------------------------------------------

//AVX2用コード
#include <immintrin.h> //イントリンシック命令 AVX / AVX2

#include "convert.h"
#include "convert_const.h"

#if _MSC_VER >= 1800 && !defined(__AVX512BW__) && !defined(_DEBUG)
static_assert(false, "do not forget to set /arch:AVX512 for this file.");
#endif

#define _mm512_all_one (_mm512_ternarylogic_epi32(_mm512_setzero_si512(), _mm512_setzero_si512(), _mm512_setzero_si512(), 0xff))
#define _mm512_set_epi16_one (_mm512_sub_epi16(_mm512_setzero_si512(), _mm512_all_one))

alignas(64) static const long long PACKUS_SHFFLE_BACK[8] = { 0, 2, 4, 6, 1, 3, 5, 7 };

#define zC_packus_shuffle (_mm512_load_si512(PACKUS_SHFFLE_BACK))

template <typename T, size_t size>
void SET_ARRAY(T(&ARRY)[size], int value) {
    for (size_t iarr = 0; iarr < size; iarr++) {
        ARRY[iarr] = (T)value;
    }
}

static const BYTE INTERLACE_WEIGHT[2][4] = {
    {1, 3, 1, 3},
    {3, 1, 3, 1}
};

#define zC_INTERLACE_WEIGHT(i) _mm512_broadcast_int(*(int *)INTERLACE_WEIGHT[(i)])

static __forceinline void separate_low_up(__m512i& z0_return_lower, __m512i& z1_return_upper) {
    const __m512i z2 = z0_return_lower; // 384, 256, 128,   0
    const __m512i z1 = z1_return_upper; // 896, 768, 640, 512
    z0_return_lower = _mm512_shuffle_i64x2(z2, z1, _MM_SHUFFLE(2, 0, 2, 0)); // 768, 512, 256,   0
    z1_return_upper = _mm512_shuffle_i64x2(z2, z1, _MM_SHUFFLE(3, 1, 3, 1)); // 896, 640, 384, 128

    const __m512i zMaskLowByte = _mm512_srli_epi16(_mm512_all_one, 8);
    const __m512i z4 = _mm512_srli_epi16(z0_return_lower, 8);
    const __m512i z5 = _mm512_srli_epi16(z1_return_upper, 8);

    z0_return_lower = _mm512_and_si512(z0_return_lower, zMaskLowByte);
    z1_return_upper = _mm512_and_si512(z1_return_upper, zMaskLowByte);

    z0_return_lower = _mm512_packus_epi16(z0_return_lower, z1_return_upper);
    z1_return_upper = _mm512_packus_epi16(z4, z5);
}

static __forceinline void separate_low_up_16bit(__m512i& z0_return_lower, __m512i& z1_return_upper) {
    const __m512i z2 = z0_return_lower; // 384, 256, 128,   0
    const __m512i z1 = z1_return_upper; // 896, 768, 640, 512
    z0_return_lower = _mm512_shuffle_i64x2(z2, z1, _MM_SHUFFLE(2, 0, 2, 0)); // 768, 512, 256,   0
    z1_return_upper = _mm512_shuffle_i64x2(z2, z1, _MM_SHUFFLE(3, 1, 3, 1)); // 896, 640, 384, 128

    const __m512i zMaskLowByte = _mm512_srli_epi32(_mm512_all_one, 16);

    const __m512i z4 = _mm512_srli_epi32(z0_return_lower, 16);
    const __m512i z5 = _mm512_srli_epi32(z1_return_upper, 16);

    z0_return_lower = _mm512_and_si512(z0_return_lower, zMaskLowByte);
    z1_return_upper = _mm512_and_si512(z1_return_upper, zMaskLowByte);

    z0_return_lower = _mm512_packus_epi32(z0_return_lower, z1_return_upper);
    z1_return_upper = _mm512_packus_epi32(z4, z5);
}

void convert_yuy2_to_nv12_avx512(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y;
    BYTE *p, *pw, *Y, *C;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_C = pixel_data->data[1];
    for (y = 0; y < height; y += 2) {
        x  = y * width;
        p  = (BYTE *)frame + (x<<1);
        pw = p + (width<<1);
        Y  = (BYTE *)dst_Y +  x;
        C  = (BYTE *)dst_C + (x>>1);
        for (x = 0; x < width; x += 64, p += 128, pw += 128) {
            __m512i z0, z1, z3;
            //-----------1行目---------------
            z0 = _mm512_loadu_si512((const __m512i*)(p+ 0)); // 384, 256, 128,   0
            z1 = _mm512_loadu_si512((const __m512i*)(p+64)); // 896, 768, 640, 512,

            separate_low_up(z0, z1);
            z3 = z1;

            _mm512_storeu_si512((__m512i *)(Y + x), z0);
            //-----------1行目終了---------------

            //-----------2行目---------------
            z0 = _mm512_loadu_si512((const __m512i*)(pw+ 0));
            z1 = _mm512_loadu_si512((const __m512i*)(pw+64));

            separate_low_up(z0, z1);

            _mm512_storeu_si512((__m512i *)(Y + width + x), z0);
            //-----------2行目終了---------------

            z1 = _mm512_avg_epu8(z1, z3);  //VUVUVUVUVUVUVUVU
            _mm512_storeu_si512((__m512i *)(C + x), z1);
        }
    }
}

void convert_yuy2_to_yv12_avx512(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y;
    BYTE *p, *pw, *Y, *U, *V;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_U = pixel_data->data[1];
    BYTE *dst_V = pixel_data->data[2];
    for (y = 0; y < height; y += 2) {
        x  = y * width;
        p  = (BYTE *)frame + (x<<1);
        pw = p + (width<<1);
        Y  = (BYTE *)dst_Y +  x;
        U  = (BYTE *)dst_U + (x>>2);
        V  = (BYTE *)dst_V + (x>>2);
        for (x = 0; x < width; x += 64, p += 128, pw += 128) {
            __m512i z0, z1, z3, z6;
            //-----------1行目---------------
            z0 = _mm512_loadu_si512((const __m512i*)(p+ 0));
            z1 = _mm512_loadu_si512((const __m512i*)(p+64));

            separate_low_up(z0, z1);
            z3 = z1;

            _mm512_storeu_si512((__m512i *)(Y + x), z0);

            z0 = _mm512_loadu_si512((const __m512i*)(p+128));
            z1 = _mm512_loadu_si512((const __m512i*)(p+192));

            separate_low_up(z0, z1);
            z6 = z1;

            _mm512_storeu_si512((__m512i *)(Y + x + 64), z0);
            //-----------1行目終了---------------

            //-----------2行目---------------
            z0 = _mm512_loadu_si512((const __m512i*)(pw+ 0));
            z1 = _mm512_loadu_si512((const __m512i*)(pw+64));

            separate_low_up(z0, z1);

            _mm512_storeu_si512((__m512i *)(Y + width + x), z0);

            z3 = _mm512_avg_epu8(z1, z3);

            z0 = _mm512_loadu_si512((const __m512i*)(pw+128));
            z1 = _mm512_loadu_si512((const __m512i*)(pw+192));

            separate_low_up(z0, z1);

            _mm512_storeu_si512((__m512i *)(Y + width + x + 64), z0);
            //-----------2行目終了---------------

            z1 = _mm512_avg_epu8(z1, z6);  //VUVUVUVUVUVUVUVU

            z0 = z3;
            separate_low_up(z0, z1);

            _mm512_storeu_si512((__m512i *)(U + (x>>1)), z0);
            _mm512_storeu_si512((__m512i *)(V + (x>>1)), z1);
        }
    }
}

static __forceinline __m512i yuv422_to_420_i_interpolate(__m512i z_up, __m512i z_down, int i) {
    __m512i z0, z1;
    z0 = _mm512_unpacklo_epi8(z_down, z_up);
    z1 = _mm512_unpackhi_epi8(z_down, z_up);
    z0 = _mm512_maddubs_epi16(z0, zC_INTERLACE_WEIGHT(i));
    z1 = _mm512_maddubs_epi16(z1, zC_INTERLACE_WEIGHT(i));
    z0 = _mm512_add_epi16(z0, _mm512_set1_epi16(2));
    z1 = _mm512_add_epi16(z1, _mm512_set1_epi16(2));
    z0 = _mm512_srai_epi16(z0, 2);
    z1 = _mm512_srai_epi16(z1, 2);
    z0 = _mm512_packus_epi16(z0, z1);
    return z0;
}

void convert_yuy2_to_nv12_i_avx512(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y, i;
    BYTE *p, *pw, *Y, *C;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_C = pixel_data->data[1];
    for (y = 0; y < height; y += 4) {
        for (i = 0; i < 2; i++) {
            x  = (y + i) * width;
            p  = (BYTE *)frame + (x<<1);
            pw = p + (width<<2);
            Y  = (BYTE *)dst_Y +  x;
            C  = (BYTE *)dst_C + ((x+width*i)>>1);
            for (x = 0; x < width; x += 64, p += 128, pw += 128) {
                __m512i z0, z1, z3;
                //-----------    1+i行目   ---------------
                z0 = _mm512_loadu_si512((const __m512i*)(p+ 0));
                z1 = _mm512_loadu_si512((const __m512i*)(p+64));

                separate_low_up(z0, z1);
                z3 = z1;

                _mm512_storeu_si512((__m512i *)(Y + x), z0);
                //-----------1+i行目終了---------------

                //-----------3+i行目---------------
                z0 = _mm512_loadu_si512((const __m512i*)(pw+ 0));
                z1 = _mm512_loadu_si512((const __m512i*)(pw+64));

                separate_low_up(z0, z1);

                _mm512_storeu_si512((__m512i *)(Y + (width<<1) + x), z0);
                //-----------3+i行目終了---------------

                z0 = yuv422_to_420_i_interpolate(z3, z1, i);

                _mm512_storeu_si512((__m512i *)(C + x), z0);
            }
        }
    }
}

void convert_yuy2_to_yv12_i_avx512(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y, i;
    BYTE *p, *pw, *Y, *U, *V;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_U = pixel_data->data[1];
    BYTE *dst_V = pixel_data->data[2];
    for (y = 0; y < height; y += 4) {
        for (i = 0; i < 2; i++) {
            x  = (y + i) * width;
            p  = (BYTE *)frame + (x<<1);
            pw = p + (width<<2);
            Y  = (BYTE *)dst_Y +  x;
            U  = (BYTE *)dst_U + ((x+width*i)>>2);
            V  = (BYTE *)dst_V + ((x+width*i)>>2);
            for (x = 0; x < width; x += 64, p += 128, pw += 128) {
                __m512i z0, z1, z3, z6;
                //-----------    1+i行目   ---------------
                z0 = _mm512_loadu_si512((__m128i*)(p+ 0));
                z1 = _mm512_loadu_si512((__m128i*)(p+64));

                separate_low_up(z0, z1);
                z3 = z1;

                _mm512_storeu_si512((__m512i *)(Y + x), z0);

                z0 = _mm512_loadu_si512((__m128i*)(p+128));
                z1 = _mm512_loadu_si512((__m128i*)(p+192));

                separate_low_up(z0, z1);
                z6 = z1;

                _mm512_storeu_si512((__m512i *)(Y + x + 64), z0);
                //-----------1+i行目終了---------------

                //-----------3+i行目---------------
                z0 = _mm512_loadu_si512((__m128i*)(pw+ 0));
                z1 = _mm512_loadu_si512((__m128i*)(pw+64));

                separate_low_up(z0, z1);

                _mm512_storeu_si512((__m512i *)(Y + (width<<1) + x), z0);

                z3 = yuv422_to_420_i_interpolate(z3, z1, i);

                z0 = _mm512_loadu_si512((__m128i*)(pw+128));
                z1 = _mm512_loadu_si512((__m128i*)(pw+192));

                separate_low_up(z0, z1);

                _mm512_storeu_si512((__m512i *)(Y + (width<<1) + x + 64), z0);
                //-----------3+i行目終了---------------

                z1 = yuv422_to_420_i_interpolate(z6, z1, i);

                z0 = z3;
                separate_low_up(z0, z1);

                _mm512_storeu_si512((__m512i *)(U + (x>>1)), z0);
                _mm512_storeu_si512((__m512i *)(V + (x>>1)), z1);
            }
        }
    }
}

template<bool avx512vbmi>
static __forceinline void gather_y_uv_from_yc48(__m512i& z0, __m512i& z1, __m512i z2) {
    __m512i z5 = z0;
    __m512i z4 = z1;
    __m512i z3 = z2;
    if (avx512vbmi) {
        __mmask32 k7 = 0xffc00000;

        alignas(64) static const char PACK_YC48_SHUFFLE_Y_AVX512_VBMI[64] = {
             0,   1,   6,   7,  12,  13,  18,  19,  24,  25,  30,  31, 36, 37, 42, 43, 48, 49, 54, 55, 60, 61, 66, 67, 72, 73, 78, 79, 84, 85, 90, 91,
            96,  97, 102, 103, 108, 109, 114, 115, 120, 121, 126, 127,  4,  5, 10, 11, 16, 17, 22, 23, 28, 29, 34, 35, 40, 41, 46, 47, 52, 53, 58, 59
        };
        alignas(64) static const char PACK_YC48_SHUFFLE_UV_AVX512_VBMI[64] = {
             2,   3,   4,   5,  14,  15,  16,  17,  26,  27,  28,  29,  38,  39,  40,  41,  50,  51,  52,  53,  62,  63,  64,  65,  74,  75,  76,  77,  86,  87,  88,  89,
            98,  99, 100, 101, 110, 111, 112, 113, 122, 123, 124, 125,   6,   7,   8,   9,  18,  19,  20,  21,  30,  31,  32,  33,  42,  43,  44,  45,  54,  55,  56,  57
        };
        z0 = _mm512_load_si512((const __m512i *)PACK_YC48_SHUFFLE_Y_AVX512_VBMI);
        z1 = z0;
        z1 = _mm512_permutex2var_epi8(z5/*a*/, z1/*idx*/, z4/*b*/);
        z1 = _mm512_mask_mov_epi16(z1, k7, _mm512_permutexvar_epi8(z0/*idx*/, z3));

        z0 = _mm512_load_si512((const __m512i *)PACK_YC48_SHUFFLE_UV_AVX512_VBMI);
        z2 = _mm512_permutex2var_epi8(z5/*a*/, z0/*idx*/, z4/*b*/);
        z2 = _mm512_mask_mov_epi16(z2, k7, _mm512_permutexvar_epi8(z0/*idx*/, z3));
    } else {
        alignas(64) static const short PACK_YC48_SHUFFLE_Y_AVX512[32] = {
             0,  3,  6,  9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45,
            48, 51, 54, 57, 60, 63,  2,  5,  8, 11, 14, 17, 20, 23, 26, 29
        };
        alignas(64) static const short PACK_YC48_SHUFFLE_UV_AVX512[32] = {
             1,  2,  7,  8, 13, 14, 19, 20, 25, 26, 31, 32, 37, 38, 43, 44,
            49, 50, 55, 56, 61, 62,  3,  4,  9, 10, 15, 16, 21, 22, 27, 28,
        };

        __mmask32 k7 = 0xffc00000;

        z0 = _mm512_load_si512((const __m512i *)PACK_YC48_SHUFFLE_Y_AVX512);
        z1 = z0;
        z1 = _mm512_permutex2var_epi16(z5/*a*/, z1/*idx*/, z4/*b*/);
#if 0 //どちらでもあまり速度は変わらない
        z1 = _mm512_mask_permutexvar_epi16(z1/*src*/, k7, z0/*idx*/, z3);
#else
        z1 = _mm512_mask_mov_epi16(z1, k7, _mm512_permutexvar_epi16(z0/*idx*/, z3));
#endif

        z0 = _mm512_load_si512((const __m512i *)PACK_YC48_SHUFFLE_UV_AVX512);
        z2 = _mm512_permutex2var_epi16(z5/*a*/, z0/*idx*/, z4/*b*/);
#if 0 //どちらでもあまり速度は変わらない
        z2 = _mm512_mask_permutexvar_epi16(z2/*src*/, k7, z0/*idx*/, z3);
#else
        z2 = _mm512_mask_mov_epi16(z2, k7, _mm512_permutexvar_epi16(z0/*idx*/, z3));
#endif
    }
    z0 = z1;
    z1 = z2;
}


template<bool aligned, bool avx512vbmi>
void __forceinline gather_y_u_v_from_yc48(__m512i& y, __m512i& cb, __m512i& cr, const char *src) {
    alignas(64) static const unsigned short PACK_YC48_SHUFFLE_AVX512[32] = {
         0,  3,  6,  9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45,
        48, 51, 54, 57, 60, 63,  2,  5,  8, 11, 14, 17, 20, 23, 26, 29
    };
    alignas(64) static const unsigned char PACK_YC48_SHUFFLE_AVX512_VBMI[64] = {
         0,   1,   6,   7,  12,  13,  18,  19,  24,  25,  30,  31, 36, 37, 42, 43, 48, 49, 54, 55, 60, 61, 66, 67, 72, 73, 78, 79, 84, 85, 90, 91,
        96,  97, 102, 103, 108, 109, 114, 115, 120, 121, 126, 127,  4,  5, 10, 11, 16, 17, 22, 23, 28, 29, 34, 35, 40, 41, 46, 47, 52, 53, 58, 59
    };
    __m512i z0 = _mm512_load_si512(avx512vbmi ? (__m512i *)PACK_YC48_SHUFFLE_AVX512_VBMI : (__m512i *)PACK_YC48_SHUFFLE_AVX512);
    __m512i z5 = (aligned) ? _mm512_load_si512((__m512i *)(src +   0)) : _mm512_loadu_si512((__m512i *)(src +   0));
    __m512i z4 = (aligned) ? _mm512_load_si512((__m512i *)(src +  64)) : _mm512_loadu_si512((__m512i *)(src +  64));
    __m512i z3 = (aligned) ? _mm512_load_si512((__m512i *)(src + 128)) : _mm512_loadu_si512((__m512i *)(src + 128));

    __m512i z1, z2;
    __m512i z6 = _mm512_ternarylogic_epi64(_mm512_setzero_si512(), _mm512_setzero_si512(), _mm512_setzero_si512(), 0xff);
    if (avx512vbmi) {
        z6 = _mm512_add_epi8(z6, z6);
#if 0
        __mmask64 k6 = _cvtu64_mask64(0xFFFFFC0000000000);
        __mmask64 k7 = _cvtu64_mask64(0xFFFFF00000000000);

        z1 = z0;
        z1 = _mm512_permutex2var_epi8(z5/*a*/, z1/*idx*/, z4/*b*/);
        z1 = _mm512_mask_permutexvar_epi8(z1/*src*/, k7, z0/*idx*/, z3);
        z0 = _mm512_sub_epi16(z0, z6);

        z2 = z0;
        z2 = _mm512_permutex2var_epi8(z5/*a*/, z2/*idx*/, z4/*b*/);
        z2 = _mm512_mask_permutexvar_epi8(z2/*src*/, k6, z0/*idx*/, z3);
        z0 = _mm512_sub_epi16(z0, z6);

        z6 = z0;
        z6 = _mm512_permutex2var_epi8(z5/*a*/, z6/*idx*/, z4/*b*/);
        z6 = _mm512_mask_permutexvar_epi8(z6/*src*/, k6, z0/*idx*/, z3);
#else
        __mmask32 k7 = 0xffc00000;
        __mmask32 k6 = 0xffe00000;

        z1 = z0;
        z1 = _mm512_permutex2var_epi8(z5/*a*/, z1/*idx*/, z4/*b*/);
        z1 = _mm512_mask_mov_epi16(z1, k7, _mm512_permutexvar_epi8(z0/*idx*/, z3));
        z0 = _mm512_sub_epi8(z0, z6);

        z2 = z0;
        z2 = _mm512_permutex2var_epi8(z5/*a*/, z2/*idx*/, z4/*b*/);
        z2 = _mm512_mask_mov_epi16(z2, k6, _mm512_permutexvar_epi8(z0/*idx*/, z3));
        z0 = _mm512_sub_epi8(z0, z6);

        z6 = z0;
        z6 = _mm512_permutex2var_epi8(z5/*a*/, z6/*idx*/, z4/*b*/);
        z6 = _mm512_mask_mov_epi16(z6, k6, _mm512_permutexvar_epi8(z0/*idx*/, z3));
#endif
    } else {
        __mmask32 k7 = 0xffc00000;
        __mmask32 k6 = 0xffe00000;

        z1 = z0;
        z1 = _mm512_permutex2var_epi16(z5/*a*/, z1/*idx*/, z4/*b*/);
#if 1 //どちらでもあまり速度は変わらない
        z1 = _mm512_mask_permutexvar_epi16(z1/*src*/, k7, z0/*idx*/, z3);
#else
        z1 = _mm512_mask_mov_epi16(z1, k7, _mm512_permutexvar_epi16(z0/*idx*/, z3));
#endif
        z0 = _mm512_sub_epi16(z0, z6);

        z2 = z0;
        z2 = _mm512_permutex2var_epi16(z5/*a*/, z2/*idx*/, z4/*b*/);
#if 1 //どちらでもあまり速度は変わらない
        z2 = _mm512_mask_permutexvar_epi16(z2/*src*/, k6, z0/*idx*/, z3);
#else
        z1 = _mm512_mask_mov_epi16(z2, k6, _mm512_permutexvar_epi16(z0/*idx*/, z3));
#endif
        z0 = _mm512_sub_epi16(z0, z6);

        z6 = z0;
        z6 = _mm512_permutex2var_epi16(z5/*a*/, z6/*idx*/, z4/*b*/);
#if 1 //どちらでもあまり速度は変わらない
        z6 = _mm512_mask_permutexvar_epi16(z6/*src*/, k6, z0/*idx*/, z3);
#else
        z6 = _mm512_mask_mov_epi16(z6, k6, _mm512_permutexvar_epi16(z0/*idx*/, z3));
#endif
    }
    y = z1;
    cb = z2;
    cr = z6;
}

static __forceinline __m512i convert_z_range_from_yc48(__m512i z0, __m512i zC_Y_MA_16, int Y_RSH_16, const __m512i& zC_YCC, const __m512i& zC_pw_one, const __m512i& zC_max) {
    __m512i z7;

    z7 = _mm512_unpackhi_epi16(z0, zC_pw_one);
    z0 = _mm512_unpacklo_epi16(z0, zC_pw_one);

    z0 = _mm512_madd_epi16(z0, zC_Y_MA_16);
    z7 = _mm512_madd_epi16(z7, zC_Y_MA_16);
    z0 = _mm512_srai_epi32(z0, Y_RSH_16);
    z7 = _mm512_srai_epi32(z7, Y_RSH_16);
    z0 = _mm512_add_epi32(z0, zC_YCC);
    z7 = _mm512_add_epi32(z7, zC_YCC);

    z0 = _mm512_packus_epi32(z0, z7);

    z0 = _mm512_min_epu16(z0, zC_max);

    return z0;
}
static __forceinline __m512i convert_uv_range_after_adding_offset(__m512i z0, const __m512i& zC_UV_MA_16, int UV_RSH_16, const __m512i& zC_YCC, const __m512i& zC_pw_one, const __m512i& zC_max) {
    __m512i z7;
    z7 = _mm512_unpackhi_epi16(z0, zC_pw_one);
    z0 = _mm512_unpacklo_epi16(z0, zC_pw_one);

    z0 = _mm512_madd_epi16(z0, zC_UV_MA_16);
    z7 = _mm512_madd_epi16(z7, zC_UV_MA_16);
    z0 = _mm512_srai_epi32(z0, UV_RSH_16);
    z7 = _mm512_srai_epi32(z7, UV_RSH_16);
    z0 = _mm512_add_epi32(z0, zC_YCC);
    z7 = _mm512_add_epi32(z7, zC_YCC);

    z0 = _mm512_packus_epi32(z0, z7);

    z0 = _mm512_min_epu16(z0, zC_max);

    return z0;
}
static __forceinline __m512i convert_uv_range_from_yc48(__m512i z0, const __m512i& zC_UV_OFFSET_x1, const __m512i& zC_UV_MA_16, int UV_RSH_16, const __m512i& zC_YCC, const __m512i& zC_pw_one, const __m512i& zC_max) {
    z0 = _mm512_add_epi16(z0, zC_UV_OFFSET_x1);

    return convert_uv_range_after_adding_offset(z0, zC_UV_MA_16, UV_RSH_16, zC_YCC, zC_pw_one, zC_max);
}
static __forceinline __m512i convert_uv_range_from_yc48_yuv420p(__m512i z0, __m512i z1, const __m512i& zC_UV_OFFSET_x2, __m512i zC_UV_MA_16, int UV_RSH_16, const __m512i& zC_YCC, const __m512i& zC_pw_one, const __m512i& zC_max) {
    z0 = _mm512_add_epi16(z0, z1);
    z0 = _mm512_add_epi16(z0, zC_UV_OFFSET_x2);

    return convert_uv_range_after_adding_offset(z0, zC_UV_MA_16, UV_RSH_16, zC_YCC, zC_pw_one, zC_max);
}

static __forceinline __m512i convert_uv_range_from_yc48_420i(__m512i z0, __m512i z1, const __m512i& zC_UV_OFFSET_x1, const __m512i& zC_UV_MA_16_0, const __m512i& zC_UV_MA_16_1, int UV_RSH_16, const __m512i& zC_YCC, const __m512i& zC_pw_one, const __m512i& zC_max) {
    __m512i z2, z3, z6, z7;

    z0 = _mm512_add_epi16(z0, zC_UV_OFFSET_x1);
    z1 = _mm512_add_epi16(z1, zC_UV_OFFSET_x1);

    z7 = _mm512_unpackhi_epi16(z0, zC_pw_one);
    z6 = _mm512_unpacklo_epi16(z0, zC_pw_one);
    z3 = _mm512_unpackhi_epi16(z1, zC_pw_one);
    z2 = _mm512_unpacklo_epi16(z1, zC_pw_one);

    z6 = _mm512_madd_epi16(z6, zC_UV_MA_16_0);
    z7 = _mm512_madd_epi16(z7, zC_UV_MA_16_0);
    z2 = _mm512_madd_epi16(z2, zC_UV_MA_16_1);
    z3 = _mm512_madd_epi16(z3, zC_UV_MA_16_1);
    z0 = _mm512_add_epi32(z6, z2);
    z7 = _mm512_add_epi32(z7, z3);
    z0 = _mm512_srai_epi32(z0, UV_RSH_16);
    z7 = _mm512_srai_epi32(z7, UV_RSH_16);
    z0 = _mm512_add_epi32(z0, zC_YCC);
    z7 = _mm512_add_epi32(z7, zC_YCC);

    z0 = _mm512_packus_epi32(z0, z7);

    z0 = _mm512_min_epu16(z0, zC_max);

    return z0;
}

template<bool avx512vbmi>
void __forceinline convert_yc48_to_nv12_highbit_avx512(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height, const int LSFT_YCC, const __m512i& zC_Y_L_MA, const int Y_L_RSH, const __m512i&zC_UV_L_MA_420P, int UV_L_RSH_420P, int bitdepthMax) {
    int x, y;
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_C = (short *)pixel_data->data[1];
    short *ycp, *ycpw;
    short *Y = NULL, *C = NULL;
    alignas(64) short c_max[32];       SET_ARRAY(c_max, bitdepthMax);
    alignas(64) short c_uv_offset[32]; SET_ARRAY(c_uv_offset, UV_OFFSET_x2);
    alignas(64) int c_ycc[16];         SET_ARRAY(c_ycc, 1 << LSFT_YCC);
    const __m512i zC_max = _mm512_load_si512(c_max);
    const __m512i zC_UV_OFFSET_x2 = _mm512_load_si512(c_uv_offset);
    const __m512i zC_YCC = _mm512_load_si512(c_ycc);
    const __m512i zC_pw_one = _mm512_set_epi16_one;

    for (y = 0; y < height; y += 2) {
        ycp = (short*)pixel + width * y * 3;
        ycpw= ycp + width*3;
        Y   = (short*)dst_Y + width * y;
        C   = (short*)dst_C + width * y / 2;
        for (x = 0; x < width; x += 32, ycp += 96, ycpw += 96) {

            __m512i z0, z1, z2, z3;
            z1 = _mm512_loadu_si512((__m512i *)(ycp +  0));
            z2 = _mm512_loadu_si512((__m512i *)(ycp + 32));
            z3 = _mm512_loadu_si512((__m512i *)(ycp + 64));

            gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);
            z0 = z2;

            _mm512_storeu_si512((__m512i *)(Y + x), convert_z_range_from_yc48(z1, zC_Y_L_MA, Y_L_RSH, zC_YCC, zC_pw_one, zC_max));

            z1 = _mm512_loadu_si512((__m512i *)(ycpw +  0));
            z2 = _mm512_loadu_si512((__m512i *)(ycpw + 32));
            z3 = _mm512_loadu_si512((__m512i *)(ycpw + 64));

            gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);

            _mm512_storeu_si512((__m512i *)(Y + x + width), convert_z_range_from_yc48(z1, zC_Y_L_MA, Y_L_RSH, zC_YCC, zC_pw_one, zC_max));

            _mm512_storeu_si512((__m512i *)(C + x), convert_uv_range_from_yc48_yuv420p(z0, z2, zC_UV_OFFSET_x2, zC_UV_L_MA_420P, UV_L_RSH_420P, zC_YCC, zC_pw_one, zC_max));
        }
    }
}

void convert_yc48_to_nv12_10bit_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_nv12_highbit_avx512<false>(pixel, pixel_data, width, height, LSFT_YCC_10, zC_Y_L_MA_10, Y_L_RSH_10, zC_UV_L_MA_10_420P, UV_L_RSH_10_420P, LIMIT_10);
}

void convert_yc48_to_nv12_10bit_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_nv12_highbit_avx512<true>(pixel, pixel_data, width, height, LSFT_YCC_10, zC_Y_L_MA_10, Y_L_RSH_10, zC_UV_L_MA_10_420P, UV_L_RSH_10_420P, LIMIT_10);
}

void convert_yc48_to_nv12_16bit_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_nv12_highbit_avx512<false>(pixel, pixel_data, width, height, LSFT_YCC_16, zC_Y_L_MA_16, Y_L_RSH_16, zC_UV_L_MA_16_420P, UV_L_RSH_16_420P, LIMIT_16);
}

void convert_yc48_to_nv12_16bit_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_nv12_highbit_avx512<true>(pixel, pixel_data, width, height, LSFT_YCC_16, zC_Y_L_MA_16, Y_L_RSH_16, zC_UV_L_MA_16_420P, UV_L_RSH_16_420P, LIMIT_16);
}

template<bool avx512vbmi>
void __forceinline convert_yc48_to_nv12_i_highbit_avx512(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height, const int LSFT_YCC, const __m512i &zC_Y_L_MA, const int Y_L_RSH, const short zC_UV_L_MA_420I[2][16], int UV_L_RSH_420I, int bitdepthMax) {
    int x, y, i;
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_C = (short *)pixel_data->data[1];
    short *ycp, *ycpw;
    short *Y = NULL, *C = NULL;
    alignas(64) short c_max[32];       SET_ARRAY(c_max, bitdepthMax);
    alignas(64) short c_uv_offset[32]; SET_ARRAY(c_uv_offset, UV_OFFSET_x1);
    alignas(64) int c_ycc[16];         SET_ARRAY(c_ycc, 1 << LSFT_YCC);
    const __m512i zC_max = _mm512_load_si512(c_max);
    const __m512i zC_UV_OFFSET_x1 = _mm512_load_si512(c_uv_offset);
    const __m512i zC_YCC = _mm512_load_si512(c_ycc);
    const __m512i zC_pw_one = _mm512_set_epi16_one;
    for (y = 0; y < height; y += 4) {
        for (i = 0; i < 2; i++) {
            ycp = (short*)pixel + width * (y + i) * 3;
            ycpw= ycp + width*2*3;
            Y   = (short*)dst_Y + width * (y + i);
            C   = (short*)dst_C + width * (y + i*2) / 2;
            for (x = 0; x < width; x += 32, ycp += 96, ycpw += 96) {
                __m512i z0, z1, z2, z3;
                z1 = _mm512_loadu_si512((__m512i *)(ycp +  0));
                z2 = _mm512_loadu_si512((__m512i *)(ycp + 32));
                z3 = _mm512_loadu_si512((__m512i *)(ycp + 64));

                gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);
                z0 = z2;

                _mm512_storeu_si512((__m512i *)(Y + x), convert_z_range_from_yc48(z1, zC_Y_L_MA, Y_L_RSH, zC_YCC, zC_pw_one, zC_max));

                z1 = _mm512_loadu_si512((__m512i *)(ycpw +  0));
                z2 = _mm512_loadu_si512((__m512i *)(ycpw + 32));
                z3 = _mm512_loadu_si512((__m512i *)(ycpw + 64));

                gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);

                _mm512_storeu_si512((__m512i *)(Y + x + width*2), convert_z_range_from_yc48(z1, zC_Y_L_MA, Y_L_RSH, zC_YCC, zC_pw_one, zC_max));

                _mm512_storeu_si512((__m512i *)(C + x), convert_uv_range_from_yc48_420i(z0, z2, zC_UV_OFFSET_x1, _mm512_broadcast_int(*(const int *)zC_UV_L_MA_420I[i]), _mm512_broadcast_int(*(const int *)zC_UV_L_MA_420I[(i+1)&0x01]), UV_L_RSH_420I, zC_YCC, zC_pw_one, zC_max));
            }
        }
    }
}

void convert_yc48_to_nv12_i_10bit_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_nv12_i_highbit_avx512<false>(pixel, pixel_data, width, height, LSFT_YCC_10, zC_Y_L_MA_10, Y_L_RSH_10, Array_UV_L_MA_10_420I, UV_L_RSH_10_420I, LIMIT_10);
}

void convert_yc48_to_nv12_i_10bit_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_nv12_i_highbit_avx512<true>(pixel, pixel_data, width, height, LSFT_YCC_10, zC_Y_L_MA_10, Y_L_RSH_10, Array_UV_L_MA_10_420I, UV_L_RSH_10_420I, LIMIT_10);
}

void convert_yc48_to_nv12_i_16bit_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_nv12_i_highbit_avx512<false>(pixel, pixel_data, width, height, LSFT_YCC_16, zC_Y_L_MA_16, Y_L_RSH_16, Array_UV_L_MA_16_420I, UV_L_RSH_16_420I, LIMIT_16);
}

void convert_yc48_to_nv12_i_16bit_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_nv12_i_highbit_avx512<true>(pixel, pixel_data, width, height, LSFT_YCC_16, zC_Y_L_MA_16, Y_L_RSH_16, Array_UV_L_MA_16_420I, UV_L_RSH_16_420I, LIMIT_16);
}

template<bool avx512vbmi>
void __forceinline convert_yc48_to_yv12_highbit_avx512(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height, const int LSFT_YCC, const __m512i &zC_Y_L_MA, const int Y_L_RSH, const __m512i &zC_UV_L_MA_420P, int UV_L_RSH_420P, int bitdepthMax) {
    int x, y;
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_U = (short *)pixel_data->data[1];
    short *dst_V = (short *)pixel_data->data[2];
    short *ycp, *ycpw;
    short *Y = NULL, *U = NULL, *V = NULL;
    alignas(64) short c_max[32];       SET_ARRAY(c_max, bitdepthMax);
    alignas(64) short c_uv_offset[32]; SET_ARRAY(c_uv_offset, UV_OFFSET_x2);
    alignas(64) int c_ycc[16];         SET_ARRAY(c_ycc, 1 << LSFT_YCC);
    const __m512i zC_max = _mm512_load_si512(c_max);
    const __m512i zC_UV_OFFSET_x2 = _mm512_load_si512(c_uv_offset);
    const __m512i zC_YCC = _mm512_load_si512(c_ycc);
    const __m512i zC_pw_one = _mm512_set_epi16_one;
    for (y = 0; y < height; y += 2) {
        ycp = (short*)pixel + width * y * 3;
        ycpw= ycp + width*3;
        Y   = (short*)dst_Y + width * y;
        U   = (short*)dst_U + width * y / 4;
        V   = (short*)dst_V + width * y / 4;
        for (x = 0; x < width; x += 32, ycp += 96, ycpw += 96) {
            __m512i z0, z1, z2, z3, z4;
            z1 = _mm512_loadu_si512((__m512i *)(ycp +  0));
            z2 = _mm512_loadu_si512((__m512i *)(ycp + 32));
            z3 = _mm512_loadu_si512((__m512i *)(ycp + 64));

            gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);
            z0 = z2;

            _mm512_storeu_si512((__m512i *)(Y + x), convert_z_range_from_yc48(z1, zC_Y_L_MA, Y_L_RSH, zC_YCC, zC_pw_one, zC_max));

            z1 = _mm512_loadu_si512((__m512i *)(ycpw +  0));
            z2 = _mm512_loadu_si512((__m512i *)(ycpw + 32));
            z3 = _mm512_loadu_si512((__m512i *)(ycpw + 64));

            gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);

            _mm512_storeu_si512((__m512i *)(Y + x + width), convert_z_range_from_yc48(z1, zC_Y_L_MA, Y_L_RSH, zC_YCC, zC_pw_one, zC_max));

            z4 = convert_uv_range_from_yc48_yuv420p(z0, z2,  zC_UV_OFFSET_x2, zC_UV_L_MA_420P, UV_L_RSH_420P, zC_YCC, zC_pw_one, zC_max);

            z1 = _mm512_loadu_si512((__m512i *)(ycp +  96));
            z2 = _mm512_loadu_si512((__m512i *)(ycp + 128));
            z3 = _mm512_loadu_si512((__m512i *)(ycp + 160));

            gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);
            z0 = z2;

            _mm512_storeu_si512((__m512i *)(Y + x + 32), convert_z_range_from_yc48(z1, zC_Y_L_MA, Y_L_RSH, zC_YCC, zC_pw_one, zC_max));

            z1 = _mm512_loadu_si512((__m512i *)(ycpw +  96));
            z2 = _mm512_loadu_si512((__m512i *)(ycpw + 128));
            z3 = _mm512_loadu_si512((__m512i *)(ycpw + 160));

            gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);

            _mm512_storeu_si512((__m512i *)(Y + x + 32 + width), convert_z_range_from_yc48(z1, zC_Y_L_MA, Y_L_RSH, zC_YCC, zC_pw_one, zC_max));

            z0 = convert_uv_range_from_yc48_yuv420p(z0, z2,  zC_UV_OFFSET_x2, zC_UV_L_MA_420P, UV_L_RSH_420P, zC_YCC, zC_pw_one, zC_max);

            separate_low_up_16bit(z4, z0);

            _mm512_storeu_si512((__m512i *)(U + (x>>1)), z4);
            _mm512_storeu_si512((__m512i *)(V + (x>>1)), z0);
        }
    }
}

void convert_yc48_to_yv12_10bit_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yv12_highbit_avx512<false>(pixel, pixel_data, width, height, LSFT_YCC_10, zC_Y_L_MA_10, Y_L_RSH_10, zC_UV_L_MA_10_420P, UV_L_RSH_10_420P, LIMIT_10);
}

void convert_yc48_to_yv12_10bit_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yv12_highbit_avx512<true>(pixel, pixel_data, width, height, LSFT_YCC_10, zC_Y_L_MA_10, Y_L_RSH_10, zC_UV_L_MA_10_420P, UV_L_RSH_10_420P, LIMIT_10);
}

void convert_yc48_to_yv12_16bit_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yv12_highbit_avx512<false>(pixel, pixel_data, width, height, LSFT_YCC_16, zC_Y_L_MA_16, Y_L_RSH_16, zC_UV_L_MA_16_420P, UV_L_RSH_16_420P, LIMIT_16);
}

void convert_yc48_to_yv12_16bit_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yv12_highbit_avx512<true>(pixel, pixel_data, width, height, LSFT_YCC_16, zC_Y_L_MA_16, Y_L_RSH_16, zC_UV_L_MA_16_420P, UV_L_RSH_16_420P, LIMIT_16);
}

template<bool avx512vbmi>
void __forceinline convert_yc48_to_yv12_i_highbit_avx512(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height, const int LSFT_YCC, const __m512i &zC_Y_L_MA, const int Y_L_RSH, const short zC_UV_L_MA_420I[2][16], int UV_L_RSH_420I, int bitdepthMax) {
    int x, y, i;
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_U = (short *)pixel_data->data[1];
    short *dst_V = (short *)pixel_data->data[2];
    short *ycp, *ycpw;
    short *Y = NULL, *U = NULL, *V = NULL;
    alignas(64) short c_max[32];       SET_ARRAY(c_max, bitdepthMax);
    alignas(64) short c_uv_offset[32]; SET_ARRAY(c_uv_offset, UV_OFFSET_x1);
    alignas(64) int c_ycc[16];         SET_ARRAY(c_ycc, 1 << LSFT_YCC);
    const __m512i zC_max = _mm512_load_si512(c_max);
    const __m512i zC_UV_OFFSET_x1 = _mm512_load_si512(c_uv_offset);
    const __m512i zC_YCC = _mm512_load_si512(c_ycc);
    const __m512i zC_pw_one = _mm512_set_epi16_one;
    for (y = 0; y < height; y += 4) {
        for (i = 0; i < 2; i++) {
            ycp = (short*)pixel + width * (y + i) * 3;
            ycpw= ycp + width*2*3;
            Y   = (short*)dst_Y + width * (y + i);
            U   = (short*)dst_U + width * (y + i*2) / 4;
            V   = (short*)dst_V + width * (y + i*2) / 4;
            for (x = 0; x < width; x += 64, ycp += 192, ycpw += 192) {
                __m512i z0, z1, z2, z3, z4;
                z1 = _mm512_loadu_si512((__m512i *)(ycp +  0));
                z2 = _mm512_loadu_si512((__m512i *)(ycp + 32));
                z3 = _mm512_loadu_si512((__m512i *)(ycp + 64));

                gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);
                z0 = z2;

                _mm512_storeu_si512((__m512i *)(Y + x), convert_z_range_from_yc48(z1, zC_Y_L_MA, Y_L_RSH, zC_YCC, zC_pw_one, zC_max));

                z1 = _mm512_loadu_si512((__m512i *)(ycpw +  0));
                z2 = _mm512_loadu_si512((__m512i *)(ycpw + 32));
                z3 = _mm512_loadu_si512((__m512i *)(ycpw + 64));

                gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);

                _mm512_storeu_si512((__m512i *)(Y + x + width*2), convert_z_range_from_yc48(z1, zC_Y_L_MA, Y_L_RSH, zC_YCC, zC_pw_one, zC_max));

                z4 = convert_uv_range_from_yc48_420i(z0, z2, zC_UV_OFFSET_x1, _mm512_broadcast_int(*(const int *)zC_UV_L_MA_420I[i]), _mm512_broadcast_int(*(const int *)zC_UV_L_MA_420I[(i+1)&0x01]), UV_L_RSH_420I, zC_YCC, zC_pw_one, zC_max);

                z1 = _mm512_loadu_si512((__m512i *)(ycp +  96));
                z2 = _mm512_loadu_si512((__m512i *)(ycp + 128));
                z3 = _mm512_loadu_si512((__m512i *)(ycp + 160));

                gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);
                z0 = z2;

                _mm512_storeu_si512((__m512i *)(Y + x + 16), convert_z_range_from_yc48(z1, zC_Y_L_MA, Y_L_RSH, zC_YCC, zC_pw_one, zC_max));

                z1 = _mm512_loadu_si512((__m512i *)(ycpw +  96));
                z2 = _mm512_loadu_si512((__m512i *)(ycpw + 128));
                z3 = _mm512_loadu_si512((__m512i *)(ycpw + 160));

                gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);

                _mm512_storeu_si512((__m512i *)(Y + x + 16 + width*2), convert_z_range_from_yc48(z1, zC_Y_L_MA, Y_L_RSH, zC_YCC, zC_pw_one, zC_max));

                z0 = convert_uv_range_from_yc48_420i(z0, z2, zC_UV_OFFSET_x1, _mm512_broadcast_int(*(const int *)zC_UV_L_MA_420I[i]), _mm512_broadcast_int(*(const int *)zC_UV_L_MA_420I[(i+1)&0x01]), UV_L_RSH_16_420I, zC_YCC, zC_pw_one, zC_max);

                separate_low_up_16bit(z4, z0);

                _mm512_storeu_si512((__m512i *)(U + (x>>1)), z4);
                _mm512_storeu_si512((__m512i *)(V + (x>>1)), z0);
            }
        }
    }
}

void convert_yc48_to_yv12_i_10bit_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yv12_i_highbit_avx512<false>(pixel, pixel_data, width, height, LSFT_YCC_10, zC_Y_L_MA_10, Y_L_RSH_10, Array_UV_L_MA_10_420I, UV_L_RSH_10_420I, LIMIT_10);
}

void convert_yc48_to_yv12_i_10bit_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yv12_i_highbit_avx512<true>(pixel, pixel_data, width, height, LSFT_YCC_10, zC_Y_L_MA_10, Y_L_RSH_10, Array_UV_L_MA_10_420I, UV_L_RSH_10_420I, LIMIT_10);
}

void convert_yc48_to_yv12_i_16bit_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yv12_i_highbit_avx512<false>(pixel, pixel_data, width, height, LSFT_YCC_16, zC_Y_L_MA_16, Y_L_RSH_16, Array_UV_L_MA_16_420I, UV_L_RSH_16_420I, LIMIT_16);
}

void convert_yc48_to_yv12_i_16bit_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yv12_i_highbit_avx512<true>(pixel, pixel_data, width, height, LSFT_YCC_16, zC_Y_L_MA_16, Y_L_RSH_16, Array_UV_L_MA_16_420I, UV_L_RSH_16_420I, LIMIT_16);
}

template<bool aligned, bool avx512vbmi>
void __forceinline afs_load_yc48(__m512i& y, __m512i& cb, __m512i& cr, const char *src) {
    alignas(64) static const unsigned short PACK_YC48_SHUFFLE_AVX512[32] = {
         0,  3,  6,  9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45,
        48, 51, 54, 57, 60, 63,  2,  5,  8, 11, 14, 17, 20, 23, 26, 29
    };
    alignas(64) static const unsigned char PACK_YC48_SHUFFLE_AVX512_VBMI[64] = {
         0,   1,   6,   7,  12,  13,  18,  19,  24,  25,  30,  31, 36, 37, 42, 43, 48, 49, 54, 55, 60, 61, 66, 67, 72, 73, 78, 79, 84, 85, 90, 91,
        96,  97, 102, 103, 108, 109, 114, 115, 120, 121, 126, 127,  4,  5, 10, 11, 16, 17, 22, 23, 28, 29, 34, 35, 40, 41, 46, 47, 52, 53, 58, 59
    };
    __m512i z0 = _mm512_load_si512(avx512vbmi ? (__m512i *)PACK_YC48_SHUFFLE_AVX512_VBMI : (__m512i *)PACK_YC48_SHUFFLE_AVX512);
    __m512i z5 = (aligned) ? _mm512_load_si512((__m512i *)(src + 0)) : _mm512_loadu_si512((__m512i *)(src + 0));
    __m512i z4 = (aligned) ? _mm512_load_si512((__m512i *)(src + 64)) : _mm512_loadu_si512((__m512i *)(src + 64));
    __m512i z3 = (aligned) ? _mm512_load_si512((__m512i *)(src + 128)) : _mm512_loadu_si512((__m512i *)(src + 128));

    __m512i z1, z2;
    __m512i z6 = _mm512_ternarylogic_epi64(_mm512_setzero_si512(), _mm512_setzero_si512(), _mm512_setzero_si512(), 0xff);
    if (avx512vbmi) {
        z6 = _mm512_add_epi8(z6, z6);
#if 0
        __mmask64 k6 = _cvtu64_mask64(0xFFFFFC0000000000);
        __mmask64 k7 = _cvtu64_mask64(0xFFFFF00000000000);

        z1 = z0;
        z1 = _mm512_permutex2var_epi8(z5/*a*/, z1/*idx*/, z4/*b*/);
        z1 = _mm512_mask_permutexvar_epi8(z1/*src*/, k7, z0/*idx*/, z3);
        z0 = _mm512_sub_epi16(z0, z6);

        z2 = z0;
        z2 = _mm512_permutex2var_epi8(z5/*a*/, z2/*idx*/, z4/*b*/);
        z2 = _mm512_mask_permutexvar_epi8(z2/*src*/, k6, z0/*idx*/, z3);
        z0 = _mm512_sub_epi16(z0, z6);

        z6 = z0;
        z6 = _mm512_permutex2var_epi8(z5/*a*/, z6/*idx*/, z4/*b*/);
        z6 = _mm512_mask_permutexvar_epi8(z6/*src*/, k6, z0/*idx*/, z3);
#else
        __mmask32 k7 = 0xffc00000;
        __mmask32 k6 = 0xffe00000;

        z1 = z0;
        z1 = _mm512_permutex2var_epi8(z5/*a*/, z1/*idx*/, z4/*b*/);
        z1 = _mm512_mask_mov_epi16(z1, k7, _mm512_permutexvar_epi8(z0/*idx*/, z3));
        z0 = _mm512_sub_epi8(z0, z6);

        z2 = z0;
        z2 = _mm512_permutex2var_epi8(z5/*a*/, z2/*idx*/, z4/*b*/);
        z2 = _mm512_mask_mov_epi16(z2, k6, _mm512_permutexvar_epi8(z0/*idx*/, z3));
        z0 = _mm512_sub_epi8(z0, z6);

        z6 = z0;
        z6 = _mm512_permutex2var_epi8(z5/*a*/, z6/*idx*/, z4/*b*/);
        z6 = _mm512_mask_mov_epi16(z6, k6, _mm512_permutexvar_epi8(z0/*idx*/, z3));
#endif
    } else {
        __mmask32 k7 = 0xffc00000;
        __mmask32 k6 = 0xffe00000;

        z1 = z0;
        z1 = _mm512_permutex2var_epi16(z5/*a*/, z1/*idx*/, z4/*b*/);
#if 1 //どちらでもあまり速度は変わらない
        z1 = _mm512_mask_permutexvar_epi16(z1/*src*/, k7, z0/*idx*/, z3);
#else
        z1 = _mm512_mask_mov_epi16(z1, k7, _mm512_permutexvar_epi16(z0/*idx*/, z3));
#endif
        z0 = _mm512_sub_epi16(z0, z6);

        z2 = z0;
        z2 = _mm512_permutex2var_epi16(z5/*a*/, z2/*idx*/, z4/*b*/);
#if 1 //どちらでもあまり速度は変わらない
        z2 = _mm512_mask_permutexvar_epi16(z2/*src*/, k6, z0/*idx*/, z3);
#else
        z1 = _mm512_mask_mov_epi16(z2, k6, _mm512_permutexvar_epi16(z0/*idx*/, z3));
#endif
        z0 = _mm512_sub_epi16(z0, z6);

        z6 = z0;
        z6 = _mm512_permutex2var_epi16(z5/*a*/, z6/*idx*/, z4/*b*/);
#if 1 //どちらでもあまり速度は変わらない
        z6 = _mm512_mask_permutexvar_epi16(z6/*src*/, k6, z0/*idx*/, z3);
#else
        z6 = _mm512_mask_mov_epi16(z6, k6, _mm512_permutexvar_epi16(z0/*idx*/, z3));
#endif
    }
    y = z1;
    cb = z2;
    cr = z6;
}

template<bool avx512vbmi>
void __forceinline convert_yc48_to_yuv444_avx512(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    BYTE *Y = (BYTE *)pixel_data->data[0];
    BYTE *U = (BYTE *)pixel_data->data[1];
    BYTE *V = (BYTE *)pixel_data->data[2];
    short *ycp;
    short *const ycp_fin = (short *)pixel + width * height * 3;

    alignas(64) short c_max[32];       SET_ARRAY(c_max, LIMIT_16);
    alignas(64) short c_uv_offset[32]; SET_ARRAY(c_uv_offset, UV_OFFSET_x1);
    alignas(64) int c_ycc[16];         SET_ARRAY(c_ycc, 1 << LSFT_YCC_16);
    const __m512i zC_max = _mm512_load_si512(c_max);
    const __m512i zC_UV_OFFSET_x1 = _mm512_load_si512(c_uv_offset);
    const __m512i zC_YCC = _mm512_load_si512(c_ycc);
    const __m512i zC_pw_one = _mm512_set_epi16_one;

    for (ycp = (short *)pixel; ycp < ycp_fin; ycp += 192, Y += 64, U += 64, V += 64) {
        __m512i z1, z2, z3, yY, yU, yV;
        afs_load_yc48<false, avx512vbmi>(z1, z2, z3, (const char *)ycp);

        z1 = convert_z_range_from_yc48(z1, zC_Y_L_MA_16, Y_L_RSH_16, zC_YCC, zC_pw_one, zC_max);
        z2 = convert_uv_range_from_yc48(z2, zC_UV_OFFSET_x1, zC_UV_L_MA_16_444, UV_L_RSH_16_444, zC_YCC, zC_pw_one, zC_max);
        z3 = convert_uv_range_from_yc48(z3, zC_UV_OFFSET_x1, zC_UV_L_MA_16_444, UV_L_RSH_16_444, zC_YCC, zC_pw_one, zC_max);
        yY = _mm512_srli_epi16(z1, 8);
        yU = _mm512_srli_epi16(z2, 8);
        yV = _mm512_srli_epi16(z3, 8);

        afs_load_yc48<false, avx512vbmi>(z1, z2, z3, (const char *)(ycp+96));

        z1 = convert_z_range_from_yc48(z1, zC_Y_L_MA_16, Y_L_RSH_16, zC_YCC, zC_pw_one, zC_max);
        z2 = convert_uv_range_from_yc48(z2, zC_UV_OFFSET_x1, zC_UV_L_MA_16_444, UV_L_RSH_16_444, zC_YCC, zC_pw_one, zC_max);
        z3 = convert_uv_range_from_yc48(z3, zC_UV_OFFSET_x1, zC_UV_L_MA_16_444, UV_L_RSH_16_444, zC_YCC, zC_pw_one, zC_max);
        z1 = _mm512_srli_epi16(z1, 8);
        z2 = _mm512_srli_epi16(z2, 8);
        z3 = _mm512_srli_epi16(z3, 8);

        yY = _mm512_packus_epi16(yY, z1);
        yU = _mm512_packus_epi16(yU, z2);
        yV = _mm512_packus_epi16(yV, z3);

        yY = _mm512_permutexvar_epi64(zC_packus_shuffle, yY);
        yU = _mm512_permutexvar_epi64(zC_packus_shuffle, yU);
        yV = _mm512_permutexvar_epi64(zC_packus_shuffle, yV);

        _mm512_storeu_si512((__m512i *)Y, yY);
        _mm512_storeu_si512((__m512i *)U, yU);
        _mm512_storeu_si512((__m512i *)V, yV);
    }
}

void convert_yc48_to_yuv444_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yuv444_avx512<false>(pixel, pixel_data, width, height);
}

void convert_yc48_to_yuv444_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yuv444_avx512<true>(pixel, pixel_data, width, height);
}

template<bool avx512vbmi>
void __forceinline convert_yc48_to_yuv444_16bit_avx512(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    short *Y = (short *)pixel_data->data[0];
    short *U = (short *)pixel_data->data[1];
    short *V = (short *)pixel_data->data[2];
    short *ycp;
    short *const ycp_fin = (short *)pixel + width * height * 3;
    alignas(64) short c_max[32];       SET_ARRAY(c_max, LIMIT_16);
    alignas(64) short c_uv_offset[32]; SET_ARRAY(c_uv_offset, UV_OFFSET_x1);
    alignas(64) int c_ycc[16];         SET_ARRAY(c_ycc, 1 << LSFT_YCC_16);
    const __m512i zC_max = _mm512_load_si512(c_max);
    const __m512i zC_UV_OFFSET_x1 = _mm512_load_si512(c_uv_offset);
    const __m512i zC_YCC = _mm512_load_si512(c_ycc);
    const __m512i zC_pw_one = _mm512_set_epi16_one;

    for (ycp = (short *)pixel; ycp < ycp_fin; ycp += 96, Y += 32, U += 32, V += 32) {
        __m512i z1, z2, z3;
        afs_load_yc48<false, avx512vbmi>(z1, z2, z3, (const char *)ycp);

        _mm512_storeu_si512((__m512i *)Y, convert_z_range_from_yc48(z1, zC_Y_L_MA_16, Y_L_RSH_16, zC_YCC, zC_pw_one, zC_max));
        _mm512_storeu_si512((__m512i *)U, convert_uv_range_from_yc48(z2, zC_UV_OFFSET_x1, zC_UV_L_MA_16_444, UV_L_RSH_16_444, zC_YCC, zC_pw_one, zC_max));
        _mm512_storeu_si512((__m512i *)V, convert_uv_range_from_yc48(z3, zC_UV_OFFSET_x1, zC_UV_L_MA_16_444, UV_L_RSH_16_444, zC_YCC, zC_pw_one, zC_max));
    }
}

void convert_yc48_to_yuv444_16bit_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yuv444_16bit_avx512<false>(pixel, pixel_data, width, height);
}

void convert_yc48_to_yuv444_16bit_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yuv444_16bit_avx512<true>(pixel, pixel_data, width, height);
}

#if 0
void convert_yuy2_to_nv16_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    BYTE *p = (BYTE *)pixel;
    BYTE * const p_fin = p + width * height * 2;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_C = pixel_data->data[1];
    __m512i z0, z1;
    for (; p < p_fin; p += 64, dst_Y += 32, dst_C += 32) {
        z0 = _mm512_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
        z1 = _mm512_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

        separate_low_up(z0, z1);

        _mm512_storeu_si512((__m512i *)dst_Y, z0);
        _mm512_storeu_si512((__m512i *)dst_C, z1);
    }
}

void convert_yc48_to_nv16_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_C = (short *)pixel_data->data[1];
    short *ycp = (short *)pixel;
    short * const ycp_fin = ycp + width * height * 3;
    const __m512i zC_pw_one = _mm512_set_epi16_one;
    const __m512i zC_max = _mm512_set1_epi16((short)LIMIT_16);
    const __m512i zC_YCC = _mm512_set1_epi32(1<<LSFT_YCC_16);
    __m512i z1, z2, z3;
    for (; ycp < ycp_fin; ycp += 48, dst_Y += 16, dst_C += 16) {
        z1 = _mm512_loadu_si512((__m512i *)(ycp +  0));
        z2 = _mm512_loadu_si512((__m512i *)(ycp + 16));
        z3 = _mm512_loadu_si512((__m512i *)(ycp + 32));

        gather_y_uv_from_yc48(z1, z2, z3);

        _mm512_storeu_si512((__m512i *)dst_Y, convert_z_range_from_yc48(z1, zC_Y_L_MA_16, Y_L_RSH_16, zC_YCC, zC_pw_one, zC_max));
        _mm512_storeu_si512((__m512i *)dst_C, convert_uv_range_from_yc48(z2, zC_UV_OFFSET_x1, zC_UV_L_MA_16_444, UV_L_RSH_16_444, zC_YCC, zC_pw_one, zC_max));
    }
}
#endif

template<bool avx512vbmi>
void __forceinline convert_lw48_to_nv12_16bit_avx512(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y;
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_C = (short *)pixel_data->data[1];
    short *ycp, *ycpw;
    short *Y = NULL, *C = NULL;
    __m512i z0, z1, z2, z3;
    for (y = 0; y < height; y += 2) {
        ycp = (short*)pixel + width * y * 3;
        ycpw= ycp + width*3;
        Y   = (short*)dst_Y + width * y;
        C   = (short*)dst_C + width * y / 2;
        for (x = 0; x < width; x += 32, ycp += 96, ycpw += 96) {
            z1 = _mm512_loadu_si512((__m512i *)(ycp +  0));
            z2 = _mm512_loadu_si512((__m512i *)(ycp + 16));
            z3 = _mm512_loadu_si512((__m512i *)(ycp + 32));

            gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);
            z0 = z2;

            _mm512_storeu_si512((__m512i *)(Y + x), z1);

            z1 = _mm512_loadu_si512((__m512i *)(ycpw +  0));
            z2 = _mm512_loadu_si512((__m512i *)(ycpw + 16));
            z3 = _mm512_loadu_si512((__m512i *)(ycpw + 32));

            gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);

            _mm512_storeu_si512((__m512i *)(Y + x + width), z1);

            z0 = _mm512_avg_epu16(z0, z2);

            _mm512_storeu_si512((__m512i *)(C + x), z0);
        }
    }
}

template<bool avx512vbmi>
void __forceinline convert_lw48_to_nv12_i_16bit_avx512(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y, i;
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_C = (short *)pixel_data->data[1];
    short *ycp, *ycpw;
    short *Y = NULL, *C = NULL;
    __m512i z0, z1, z2, z3;
    for (y = 0; y < height; y += 4) {
        for (i = 0; i < 2; i++) {
            ycp = (short*)pixel + width * (y + i) * 3;
            ycpw= ycp + width*2*3;
            Y   = (short*)dst_Y + width * (y + i);
            C   = (short*)dst_C + width * (y + i*2) / 2;
            for (x = 0; x < width; x += 32, ycp += 96, ycpw += 96) {
                z1 = _mm512_loadu_si512((__m512i *)(ycp +  0)); // 128, 0
                z2 = _mm512_loadu_si512((__m512i *)(ycp + 16)); // 384, 256
                z3 = _mm512_loadu_si512((__m512i *)(ycp + 32)); // 640, 512

                gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);
                z0 = z2;

                _mm512_storeu_si512((__m512i *)(Y + x), z1);

                z1 = _mm512_loadu_si512((__m512i *)(ycpw +  0));
                z2 = _mm512_loadu_si512((__m512i *)(ycpw + 16));
                z3 = _mm512_loadu_si512((__m512i *)(ycpw + 32));

                gather_y_uv_from_yc48<avx512vbmi>(z1, z2, z3);

                _mm512_storeu_si512((__m512i *)(Y + x + width*2), z1);

                z1 = _mm512_unpacklo_epi16(z0, z2);
                z0 = _mm512_unpackhi_epi16(z0, z2);
                z1 = _mm512_madd_epi16(z1, zC_INTERLACE_WEIGHT(i));
                z0 = _mm512_madd_epi16(z0, zC_INTERLACE_WEIGHT(i));
                z1 = _mm512_srli_epi32(z1, 2);
                z0 = _mm512_srli_epi32(z0, 2);
                z1 = _mm512_packus_epi32(z1, z0);

                _mm512_storeu_si512((__m512i *)(C + x), z1);
            }
        }
    }
}

void convert_lw48_to_nv12_16bit_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_lw48_to_nv12_16bit_avx512<false>(pixel, pixel_data, width, height);
}

void convert_lw48_to_nv12_16bit_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_lw48_to_nv12_16bit_avx512<true>(pixel, pixel_data, width, height);
}

void convert_lw48_to_nv12_i_16bit_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_lw48_to_nv12_i_16bit_avx512<false>(pixel, pixel_data, width, height);
}

void convert_lw48_to_nv12_i_16bit_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_lw48_to_nv12_i_16bit_avx512<true>(pixel, pixel_data, width, height);
}

#if 0
void convert_lw48_to_nv16_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_C = (short *)pixel_data->data[1];
    short *ycp = (short *)pixel;
    short * const ycp_fin = ycp + width * height * 3;
    __m512i z1, z2, z3;
    for (; ycp < ycp_fin; ycp += 48, dst_Y += 16, dst_C += 16) {
        z1 = _mm512_loadu_si512((__m512i *)(ycp +  0));
        z2 = _mm512_loadu_si512((__m512i *)(ycp + 16));
        z3 = _mm512_loadu_si512((__m512i *)(ycp + 32));

        gather_y_uv_from_yc48(z1, z2, z3);

        _mm512_storeu_si512((__m512i *)dst_Y, z1);
        _mm512_storeu_si512((__m512i *)dst_C, z2);
    }
}
#endif

template<bool avx512vbmi>
void __forceinline convert_lw48_to_yuv444_avx512(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    BYTE *Y = (BYTE *)pixel_data->data[0];
    BYTE *U = (BYTE *)pixel_data->data[1];
    BYTE *V = (BYTE *)pixel_data->data[2];
    short *ycp;
    short *const ycp_fin = (short *)pixel + width * height * 3;
    for (ycp = (short *)pixel; ycp < ycp_fin; ycp += 192, Y += 64, U += 64, V += 64) {
        __m512i z1, z2, z3, yY, yU, yV;
        afs_load_yc48<false, avx512vbmi>(z1, z2, z3, (const char *)ycp);

        yY = _mm512_srli_epi16(z1, 8);
        yU = _mm512_srli_epi16(z2, 8);
        yV = _mm512_srli_epi16(z3, 8);

        afs_load_yc48<false, avx512vbmi>(z1, z2, z3, (const char *)(ycp + 96));

        z1 = _mm512_srli_epi16(z1, 8);
        z2 = _mm512_srli_epi16(z2, 8);
        z3 = _mm512_srli_epi16(z3, 8);

        yY = _mm512_packus_epi16(yY, z1);
        yU = _mm512_packus_epi16(yU, z2);
        yV = _mm512_packus_epi16(yV, z3);

        yY = _mm512_permutexvar_epi64(yY, zC_packus_shuffle);
        yU = _mm512_permutexvar_epi64(yU, zC_packus_shuffle);
        yV = _mm512_permutexvar_epi64(yV, zC_packus_shuffle);

        _mm512_storeu_si512((__m512i *)Y, yY);
        _mm512_storeu_si512((__m512i *)U, yU);
        _mm512_storeu_si512((__m512i *)V, yV);
    }
}

void convert_lw48_to_yuv444_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_lw48_to_yuv444_avx512<false>(pixel, pixel_data, width, height);
}

void convert_lw48_to_yuv444_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_lw48_to_yuv444_avx512<true>(pixel, pixel_data, width, height);
}

template<bool avx512vbmi>
void __forceinline convert_lw48_to_yuv444_16bit_avx512(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    short *Y = (short *)pixel_data->data[0];
    short *U = (short *)pixel_data->data[1];
    short *V = (short *)pixel_data->data[2];
    short *ycp;
    short *const ycp_fin = (short *)pixel + width * height * 3;
    for (ycp = (short *)pixel; ycp < ycp_fin; ycp += 192, Y += 32, U += 32, V += 32) {
        __m512i z1, z2, z3;
        afs_load_yc48<false, avx512vbmi>(z1, z2, z3, (const char *)ycp);

        _mm512_storeu_si512((__m512i *)Y, z1);
        _mm512_storeu_si512((__m512i *)U, z2);
        _mm512_storeu_si512((__m512i *)V, z3);
    }
}

void convert_lw48_to_yuv444_16bit_avx512bw(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_lw48_to_yuv444_16bit_avx512<false>(pixel, pixel_data, width, height);
}

void convert_lw48_to_yuv444_16bit_avx512vbmi(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_lw48_to_yuv444_16bit_avx512<true>(pixel, pixel_data, width, height);
}
