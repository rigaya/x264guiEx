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

#if _MSC_VER >= 1800 && !defined(__AVX2__) && !defined(_DEBUG)
static_assert(false, "do not forget to set /arch:AVX2 for this file.");
#endif


#ifndef clamp
#define clamp(x, low, high) (((x) <= (high)) ? (((x) >= (low)) ? (x) : (low)) : (high))
#endif

//本来の256bit alignr
#define MM_ABS(x) (((x) < 0) ? -(x) : (x))
#define _mm256_alignr256_epi8(a, b, i) ((i<=16) ? _mm256_alignr_epi8(_mm256_permute2x128_si256(a, b, (0x00<<4) + 0x03), b, i) : _mm256_alignr_epi8(a, _mm256_permute2x128_si256(a, b, (0x00<<4) + 0x03), MM_ABS(i-16)))

//_mm256_srli_si256, _mm256_slli_si256は
//単に128bitシフト×2をするだけの命令である
#define _mm256_bsrli_epi128 _mm256_srli_si256
#define _mm256_bslli_epi128 _mm256_slli_si256
//本当の256bitシフト
#define _mm256_srli256_si256(a, i) ((i<=16) ? _mm256_alignr_epi8(_mm256_permute2x128_si256(a, a, (0x08<<4) + 0x03), a, i) : _mm256_bsrli_epi128(_mm256_permute2x128_si256(a, a, (0x08<<4) + 0x03), MM_ABS(i-16)))
#define _mm256_slli256_si256(a, i) ((i<=16) ? _mm256_alignr_epi8(a, _mm256_permute2x128_si256(a, a, (0x00<<4) + 0x08), MM_ABS(16-i)) : _mm256_bslli_epi128(_mm256_permute2x128_si256(a, a, (0x00<<4) + 0x08), MM_ABS(i-16)))

void convert_audio_16to8_avx2(BYTE *dst, short *src, int n) {
    BYTE *byte = dst;
    short *sh = src;
    BYTE * const loop_start = (BYTE *)(((size_t)dst + 31) & ~31);
    BYTE * const loop_fin   = (BYTE *)(((size_t)dst + n) & ~31);
    BYTE * const fin = dst + n;
    __m256i ySA, ySB;
    static const __m256i yConst = _mm256_set1_epi16(128);
    //アライメント調整
    while (byte < loop_start) {
        *byte = (*sh >> 8) + 128;
        byte++;
        sh++;
    }
    //メインループ
    while (byte < loop_fin) {
        ySA = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(sh + 16)), _mm_loadu_si128((__m128i*)(sh + 0)));
        ySB = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(sh + 24)), _mm_loadu_si128((__m128i*)(sh + 8)));
        ySA = _mm256_srai_epi16(ySA, 8);
        ySB = _mm256_srai_epi16(ySB, 8);
        ySA = _mm256_add_epi16(ySA, yConst);
        ySB = _mm256_add_epi16(ySB, yConst);
        ySA = _mm256_packus_epi16(ySA, ySB);
        _mm256_stream_si256((__m256i *)byte, ySA);
        sh += 32;
        byte += 32;
    }
    //残り
    while (byte < fin) {
        *byte = (*sh >> 8) + 128;
        byte++;
        sh++;
    }
}

void split_audio_16to8x2_avx2(BYTE *dst, short *src, int n) {
    BYTE *byte0 = dst;
    BYTE *byte1 = dst + n;
    short *sh = src;
    short *sh_fin = src + (n & ~15);
    __m256i y0, y1, y2, y3;
    __m256i yMask = _mm256_srli_epi16(_mm256_cmpeq_epi8(_mm256_setzero_si256(), _mm256_setzero_si256()), 8);
    __m256i yConst = _mm256_set1_epi8(-128);
    for ( ; sh < sh_fin; sh += 32, byte0 += 32, byte1 += 32) {
        y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(sh + 16)), _mm_loadu_si128((__m128i*)(sh + 0)));
        y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(sh + 24)), _mm_loadu_si128((__m128i*)(sh + 8)));
        y2 = _mm256_and_si256(y0, yMask); //Lower8bit
        y3 = _mm256_and_si256(y1, yMask); //Lower8bit
        y0 = _mm256_srli_epi16(y0, 8);    //Upper8bit
        y1 = _mm256_srli_epi16(y1, 8);    //Upper8bit
        y2 = _mm256_packus_epi16(y2, y3);
        y0 = _mm256_packus_epi16(y0, y1);
        y2 = _mm256_add_epi8(y2, yConst);
        y0 = _mm256_add_epi8(y0, yConst);
        _mm256_storeu_si256((__m256i*)byte0, y0);
        _mm256_storeu_si256((__m256i*)byte1, y2);
    }
    sh_fin = sh + (n & 15);
    for ( ; sh < sh_fin; sh++, byte0++, byte1++) {
        *byte0 = (*sh >> 8)   + 128;
        *byte1 = (*sh & 0xff) + 128;
    }
}


static __forceinline void separate_low_up(__m256i& y0_return_lower, __m256i& y1_return_upper) {
    __m256i y4, y5;
    const __m256i xMaskLowByte = _mm256_srli_epi16(_mm256_cmpeq_epi8(_mm256_setzero_si256(), _mm256_setzero_si256()), 8);
    y4 = _mm256_srli_epi16(y0_return_lower, 8);
    y5 = _mm256_srli_epi16(y1_return_upper, 8);

    y0_return_lower = _mm256_and_si256(y0_return_lower, xMaskLowByte);
    y1_return_upper = _mm256_and_si256(y1_return_upper, xMaskLowByte);

    y0_return_lower = _mm256_packus_epi16(y0_return_lower, y1_return_upper);
    y1_return_upper = _mm256_packus_epi16(y4, y5);
}
static __forceinline void separate_low_up_16bit(__m256i& y0_return_lower, __m256i& y1_return_upper) {
    __m256i y4, y5;
    const __m256i xMaskLowByte = _mm256_srli_epi32(_mm256_cmpeq_epi8(_mm256_setzero_si256(), _mm256_setzero_si256()), 16);

    y4 = y0_return_lower; //128,   0
    y5 = y1_return_upper; //384, 256
    y0_return_lower = _mm256_permute2x128_si256(y4, y5, (2<<4)+0); //256,   0
    y1_return_upper = _mm256_permute2x128_si256(y4, y5, (3<<4)+1); //384, 128

    y4 = _mm256_srli_epi32(y0_return_lower, 16);
    y5 = _mm256_srli_epi32(y1_return_upper, 16);

    y0_return_lower = _mm256_and_si256(y0_return_lower, xMaskLowByte);
    y1_return_upper = _mm256_and_si256(y1_return_upper, xMaskLowByte);

    y0_return_lower = _mm256_packus_epi32(y0_return_lower, y1_return_upper);
    y1_return_upper = _mm256_packus_epi32(y4, y5);
}

void convert_yuy2_to_nv12_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y;
    BYTE *p, *pw, *Y, *C;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_C = pixel_data->data[1];
    __m256i y0, y1, y3;
    for (y = 0; y < height; y += 2) {
        x  = y * width;
        p  = (BYTE *)frame + (x<<1);
        pw = p + (width<<1);
        Y  = (BYTE *)dst_Y +  x;
        C  = (BYTE *)dst_C + (x>>1);
        for (x = 0; x < width; x += 32, p += 64, pw += 64) {
            //-----------1行目---------------
            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

            separate_low_up(y0, y1);
            y3 = y1;

            _mm256_storeu_si256((__m256i *)(Y + x), y0);
            //-----------1行目終了---------------

            //-----------2行目---------------
            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+32)), _mm_loadu_si128((__m128i*)(pw+ 0)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+48)), _mm_loadu_si128((__m128i*)(pw+16)));

            separate_low_up(y0, y1);

            _mm256_storeu_si256((__m256i *)(Y + width + x), y0);
            //-----------2行目終了---------------

            y1 = _mm256_avg_epu8(y1, y3);  //VUVUVUVUVUVUVUVU
            _mm256_storeu_si256((__m256i *)(C + x), y1);
        }
    }
    _mm256_zeroupper();
}

void convert_yuy2_to_nv12_16bit_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y;
    BYTE *p, *pw;
    USHORT *Y, *C;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_C = pixel_data->data[1];
    __m256i y0, y1, y3, y0_0, y0_1, y1_0, y1_1;
    for (y = 0; y < height; y += 2) {
        x  = y * width;
        p  = (BYTE *)frame + (x<<1);
        pw = p + (width<<1);
        Y  = (USHORT *)dst_Y +  x;
        C  = (USHORT *)dst_C + (x>>1);
        for (x = 0; x < width; x += 32, p += 64, pw += 64) {
            //-----------1行目---------------
            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

            separate_low_up(y0, y1);
            y3 = y1;

            y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
            y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), 8);
            y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), 8);
            _mm256_storeu_si256((__m256i *)(Y + x +  0), y0_0);
            _mm256_storeu_si256((__m256i *)(Y + x + 16), y0_1);
            //-----------1行目終了---------------

            //-----------2行目---------------
            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+32)), _mm_loadu_si128((__m128i*)(pw+ 0)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+48)), _mm_loadu_si128((__m128i*)(pw+16)));

            separate_low_up(y0, y1);

            y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
            y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), 8);
            y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), 8);
            _mm256_storeu_si256((__m256i *)(Y + width + x +  0), y0_0);
            _mm256_storeu_si256((__m256i *)(Y + width + x + 16), y0_1);
            //-----------2行目終了---------------

            y1 = _mm256_avg_epu8(y1, y3);  //VUVUVUVUVUVUVUVU
            y1 = _mm256_permute4x64_epi64(y1, _MM_SHUFFLE(3,1,2,0));
            y1_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y1, _mm256_setzero_si256()), 8);
            y1_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y1, _mm256_setzero_si256()), 8);
            _mm256_storeu_si256((__m256i *)(C + x +  0), y1_0);
            _mm256_storeu_si256((__m256i *)(C + x + 16), y1_1);
        }
    }
    _mm256_zeroupper();
}

void convert_yuy2_to_yv12_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y;
    BYTE *p, *pw, *Y, *U, *V;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_U = pixel_data->data[1];
    BYTE *dst_V = pixel_data->data[2];
    __m256i y0, y1, y3, y6;
    for (y = 0; y < height; y += 2) {
        x  = y * width;
        p  = (BYTE *)frame + (x<<1);
        pw = p + (width<<1);
        Y  = (BYTE *)dst_Y +  x;
        U  = (BYTE *)dst_U + (x>>2);
        V  = (BYTE *)dst_V + (x>>2);
        for (x = 0; x < width; x += 64, p += 128, pw += 128) {
            //-----------1行目---------------
            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

            separate_low_up(y0, y1);
            y3 = y1;

            _mm256_storeu_si256((__m256i *)(Y + x), y0);

            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+ 96)), _mm_loadu_si128((__m128i*)(p+64)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+112)), _mm_loadu_si128((__m128i*)(p+80)));

            separate_low_up(y0, y1);
            y6 = y1;

            _mm256_storeu_si256((__m256i *)(Y + x + 32), y0);
            //-----------1行目終了---------------

            //-----------2行目---------------
            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+32)), _mm_loadu_si128((__m128i*)(pw+ 0)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+48)), _mm_loadu_si128((__m128i*)(pw+16)));

            separate_low_up(y0, y1);

            _mm256_storeu_si256((__m256i *)(Y + width + x), y0);

            y3 = _mm256_avg_epu8(y1, y3);

            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+ 96)), _mm_loadu_si128((__m128i*)(pw+64)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+112)), _mm_loadu_si128((__m128i*)(pw+80)));

            separate_low_up(y0, y1);

            _mm256_storeu_si256((__m256i *)(Y + width + x + 32), y0);
            //-----------2行目終了---------------

            y6 = _mm256_avg_epu8(y1, y6);  //VUVUVUVUVUVUVUVU

            y0 = _mm256_permute2x128_si256(y3, y6, (0x02<< 4) + 0x00);
            y1 = _mm256_permute2x128_si256(y3, y6, (0x03<< 4) + 0x01);
            separate_low_up(y0, y1);

            _mm256_storeu_si256((__m256i *)(U + (x>>1)), y0);
            _mm256_storeu_si256((__m256i *)(V + (x>>1)), y1);
        }
    }
    _mm256_zeroupper();
}

template<int output_depth>
void convert_yuy2_to_yv12_highbit_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    static_assert(output_depth > 8 && output_depth <= 16, "output_depth must be 9 - 16");
    int x, y;
    BYTE *p, *pw;
    USHORT *Y, *U, *V;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_U = pixel_data->data[1];
    BYTE *dst_V = pixel_data->data[2];
    __m256i y0, y1, y3, y6, y0_0, y0_1, y1_0, y1_1;;
    for (y = 0; y < height; y += 2) {
        x  = y * width;
        p  = (BYTE *)frame + (x<<1);
        pw = p + (width<<1);
        Y  = (USHORT *)dst_Y +  x;
        U  = (USHORT *)dst_U + (x>>2);
        V  = (USHORT *)dst_V + (x>>2);
        for (x = 0; x < width; x += 64, p += 128, pw += 128) {
            //-----------1行目---------------
            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

            separate_low_up(y0, y1);
            y3 = y1;

            y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
            y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
            y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
            _mm256_storeu_si256((__m256i *)(Y + x +  0), y0_0);
            _mm256_storeu_si256((__m256i *)(Y + x + 16), y0_1);

            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+ 96)), _mm_loadu_si128((__m128i*)(p+64)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+112)), _mm_loadu_si128((__m128i*)(p+80)));

            separate_low_up(y0, y1);
            y6 = y1;

            y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
            y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
            y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
            _mm256_storeu_si256((__m256i *)(Y + x + 32), y0_0);
            _mm256_storeu_si256((__m256i *)(Y + x + 48), y0_1);
            //-----------1行目終了---------------

            //-----------2行目---------------
            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+32)), _mm_loadu_si128((__m128i*)(pw+ 0)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+48)), _mm_loadu_si128((__m128i*)(pw+16)));

            separate_low_up(y0, y1);

            y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
            y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
            y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
            _mm256_storeu_si256((__m256i *)(Y + width + x +  0), y0_0);
            _mm256_storeu_si256((__m256i *)(Y + width + x + 16), y0_1);

            y3 = _mm256_avg_epu8(y1, y3);

            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+ 96)), _mm_loadu_si128((__m128i*)(pw+64)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+112)), _mm_loadu_si128((__m128i*)(pw+80)));

            separate_low_up(y0, y1);

            y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
            y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
            y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
            _mm256_storeu_si256((__m256i *)(Y + width + x + 32), y0_0);
            _mm256_storeu_si256((__m256i *)(Y + width + x + 48), y0_1);
            //-----------2行目終了---------------

            y6 = _mm256_avg_epu8(y1, y6);  //VUVUVUVUVUVUVUVU

            y0 = _mm256_permute2x128_si256(y3, y6, (0x02<< 4) + 0x00);
            y1 = _mm256_permute2x128_si256(y3, y6, (0x03<< 4) + 0x01);
            separate_low_up(y0, y1);

            y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
            y1 = _mm256_permute4x64_epi64(y1, _MM_SHUFFLE(3,1,2,0));
            y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
            y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
            y1_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y1, _mm256_setzero_si256()), output_depth - 8);
            y1_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y1, _mm256_setzero_si256()), output_depth - 8);
            _mm256_storeu_si256((__m256i *)(U + (x>>1) +  0), y0_0);
            _mm256_storeu_si256((__m256i *)(U + (x>>1) + 16), y0_1);
            _mm256_storeu_si256((__m256i *)(V + (x>>1) +  0), y1_0);
            _mm256_storeu_si256((__m256i *)(V + (x>>1) + 16), y1_1);
        }
    }
    _mm256_zeroupper();
}

void convert_yuy2_to_yv12_10bit_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yuy2_to_yv12_highbit_avx2<10>(frame, pixel_data, width, height);
}
void convert_yuy2_to_yv12_16bit_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yuy2_to_yv12_highbit_avx2<16>(frame, pixel_data, width, height);
}

static __forceinline __m256i yuv422_to_420_i_interpolate(__m256i y_up, __m256i y_down, int i) {
    __m256i y0, y1;
    y0 = _mm256_unpacklo_epi8(y_down, y_up);
    y1 = _mm256_unpackhi_epi8(y_down, y_up);
    y0 = _mm256_maddubs_epi16(y0, yC_INTERLACE_WEIGHT(i));
    y1 = _mm256_maddubs_epi16(y1, yC_INTERLACE_WEIGHT(i));
    y0 = _mm256_add_epi16(y0, _mm256_set1_epi16(2));
    y1 = _mm256_add_epi16(y1, _mm256_set1_epi16(2));
    y0 = _mm256_srai_epi16(y0, 2);
    y1 = _mm256_srai_epi16(y1, 2);
    y0 = _mm256_packus_epi16(y0, y1);
    return y0;
}

void convert_yuy2_to_nv12_i_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y, i;
    BYTE *p, *pw, *Y, *C;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_C = pixel_data->data[1];
    __m256i y0, y1, y3;
    for (y = 0; y < height; y += 4) {
        for (i = 0; i < 2; i++) {
            x  = (y + i) * width;
            p  = (BYTE *)frame + (x<<1);
            pw = p + (width<<2);
            Y  = (BYTE *)dst_Y +  x;
            C  = (BYTE *)dst_C + ((x+width*i)>>1);
            for (x = 0; x < width; x += 32, p += 64, pw += 64) {
                //-----------    1+i行目   ---------------
                y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
                y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

                separate_low_up(y0, y1);
                y3 = y1;

                _mm256_storeu_si256((__m256i *)(Y + x), y0);
                //-----------1+i行目終了---------------

                //-----------3+i行目---------------
                y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+32)), _mm_loadu_si128((__m128i*)(pw+ 0)));
                y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+48)), _mm_loadu_si128((__m128i*)(pw+16)));

                separate_low_up(y0, y1);

                _mm256_storeu_si256((__m256i *)(Y + (width<<1) + x), y0);
                //-----------3+i行目終了---------------

                y0 = yuv422_to_420_i_interpolate(y3, y1, i);

                _mm256_storeu_si256((__m256i *)(C + x), y0);
            }
        }
    }
    _mm256_zeroupper();
}

void convert_yuy2_to_nv12_i_16bit_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y, i;
    BYTE *p, *pw;
    USHORT *Y, *C;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_C = pixel_data->data[1];
    __m256i y0, y1, y3, y0_0, y0_1;
    for (y = 0; y < height; y += 4) {
        for (i = 0; i < 2; i++) {
            x  = (y + i) * width;
            p  = (BYTE *)frame + (x<<1);
            pw = p + (width<<2);
            Y  = (USHORT *)dst_Y +  x;
            C  = (USHORT *)dst_C + ((x+width*i)>>1);
            for (x = 0; x < width; x += 32, p += 64, pw += 64) {
                //-----------    1+i行目   ---------------
                y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
                y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

                separate_low_up(y0, y1);
                y3 = y1;

                y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
                y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), 8);
                y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), 8);
                _mm256_storeu_si256((__m256i *)(Y + x +  0), y0_0);
                _mm256_storeu_si256((__m256i *)(Y + x + 16), y0_1);
                //-----------1+i行目終了---------------

                //-----------3+i行目---------------
                y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+32)), _mm_loadu_si128((__m128i*)(pw+ 0)));
                y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+48)), _mm_loadu_si128((__m128i*)(pw+16)));

                separate_low_up(y0, y1);

                y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
                y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), 8);
                y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), 8);
                _mm256_storeu_si256((__m256i *)(Y + (width<<1) + x +  0), y0_0);
                _mm256_storeu_si256((__m256i *)(Y + (width<<1) + x + 16), y0_1);
                //-----------3+i行目終了---------------

                y0 = yuv422_to_420_i_interpolate(y3, y1, i);

                y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
                y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), 8);
                y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), 8);
                _mm256_storeu_si256((__m256i *)(C + x +  0), y0_0);
                _mm256_storeu_si256((__m256i *)(C + x + 16), y0_1);
            }
        }
    }
    _mm256_zeroupper();
}

void convert_yuy2_to_yv12_i_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y, i;
    BYTE *p, *pw, *Y, *U, *V;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_U = pixel_data->data[1];
    BYTE *dst_V = pixel_data->data[2];
    __m256i y0, y1, y3, y6;
    for (y = 0; y < height; y += 4) {
        for (i = 0; i < 2; i++) {
            x  = (y + i) * width;
            p  = (BYTE *)frame + (x<<1);
            pw = p + (width<<2);
            Y  = (BYTE *)dst_Y +  x;
            U  = (BYTE *)dst_U + ((x+width*i)>>2);
            V  = (BYTE *)dst_V + ((x+width*i)>>2);
            for (x = 0; x < width; x += 64, p += 128, pw += 128) {
                //-----------    1+i行目   ---------------
                y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
                y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

                separate_low_up(y0, y1);
                y3 = y1;

                _mm256_storeu_si256((__m256i *)(Y + x), y0);

                y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+ 96)), _mm_loadu_si128((__m128i*)(p+64)));
                y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+112)), _mm_loadu_si128((__m128i*)(p+80)));

                separate_low_up(y0, y1);
                y6 = y1;

                _mm256_storeu_si256((__m256i *)(Y + x + 32), y0);
                //-----------1+i行目終了---------------

                //-----------3+i行目---------------
                y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+32)), _mm_loadu_si128((__m128i*)(pw+ 0)));
                y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+48)), _mm_loadu_si128((__m128i*)(pw+16)));

                separate_low_up(y0, y1);

                _mm256_storeu_si256((__m256i *)(Y + (width<<1) + x), y0);

                y3 = yuv422_to_420_i_interpolate(y3, y1, i);

                y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+ 96)), _mm_loadu_si128((__m128i*)(pw+64)));
                y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+112)), _mm_loadu_si128((__m128i*)(pw+80)));

                separate_low_up(y0, y1);

                _mm256_storeu_si256((__m256i *)(Y + (width<<1) + x + 32), y0);
                //-----------3+i行目終了---------------

                y1 = yuv422_to_420_i_interpolate(y6, y1, i);

                y0 = _mm256_permute2x128_si256(y3, y1, (2<<4)|0);
                y1 = _mm256_permute2x128_si256(y3, y1, (3<<4)|1);
                separate_low_up(y0, y1);

                _mm256_storeu_si256((__m256i *)(U + (x>>1)), y0);
                _mm256_storeu_si256((__m256i *)(V + (x>>1)), y1);
            }
        }
    }
    _mm256_zeroupper();
}

template<int output_depth>
void convert_yuy2_to_yv12_i_highbit_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    static_assert(output_depth > 8 && output_depth <= 16, "output_depth must be 9 - 16");
    int x, y, i;
    BYTE *p, *pw;
    USHORT *Y, *U, *V;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_U = pixel_data->data[1];
    BYTE *dst_V = pixel_data->data[2];
    __m256i y0, y1, y3, y6, y0_0, y0_1, y1_0, y1_1;;
    for (y = 0; y < height; y += 4) {
        for (i = 0; i < 2; i++) {
            x  = (y + i) * width;
            p  = (BYTE *)frame + (x<<1);
            pw = p + (width<<2);
            Y  = (USHORT *)dst_Y +  x;
            U  = (USHORT *)dst_U + ((x+width*i)>>2);
            V  = (USHORT *)dst_V + ((x+width*i)>>2);
            for (x = 0; x < width; x += 64, p += 128, pw += 128) {
                //-----------    1+i行目   ---------------
                y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
                y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

                separate_low_up(y0, y1);
                y3 = y1;

                y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
                y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
                y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
                _mm256_storeu_si256((__m256i *)(Y + x +  0), y0_0);
                _mm256_storeu_si256((__m256i *)(Y + x + 16), y0_1);

                y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+ 96)), _mm_loadu_si128((__m128i*)(p+64)));
                y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+112)), _mm_loadu_si128((__m128i*)(p+80)));

                separate_low_up(y0, y1);
                y6 = y1;

                y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
                y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
                y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
                _mm256_storeu_si256((__m256i *)(Y + x + 32), y0_0);
                _mm256_storeu_si256((__m256i *)(Y + x + 48), y0_1);
                //-----------1+i行目終了---------------

                //-----------3+i行目---------------
                y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+32)), _mm_loadu_si128((__m128i*)(pw+ 0)));
                y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+48)), _mm_loadu_si128((__m128i*)(pw+16)));

                separate_low_up(y0, y1);

                y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
                y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
                y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
                _mm256_storeu_si256((__m256i *)(Y + (width<<1) + x +  0), y0_0);
                _mm256_storeu_si256((__m256i *)(Y + (width<<1) + x + 16), y0_1);

                y3 = yuv422_to_420_i_interpolate(y3, y1, i);

                y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+ 96)), _mm_loadu_si128((__m128i*)(pw+64)));
                y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+112)), _mm_loadu_si128((__m128i*)(pw+80)));

                separate_low_up(y0, y1);

                y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
                y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
                y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
                _mm256_storeu_si256((__m256i *)(Y + (width<<1) + x + 32), y0_0);
                _mm256_storeu_si256((__m256i *)(Y + (width<<1) + x + 48), y0_1);
                //-----------3+i行目終了---------------

                y1 = yuv422_to_420_i_interpolate(y6, y1, i);

                y0 = _mm256_permute2x128_si256(y3, y1, (2<<4)|0);
                y1 = _mm256_permute2x128_si256(y3, y1, (3<<4)|1);
                separate_low_up(y0, y1);

                y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
                y1 = _mm256_permute4x64_epi64(y1, _MM_SHUFFLE(3,1,2,0));
                y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
                y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), output_depth - 8);
                y1_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y1, _mm256_setzero_si256()), output_depth - 8);
                y1_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y1, _mm256_setzero_si256()), output_depth - 8);
                _mm256_storeu_si256((__m256i *)(U + (x>>1) + 0 ), y0_0);
                _mm256_storeu_si256((__m256i *)(U + (x>>1) + 16), y0_1);
                _mm256_storeu_si256((__m256i *)(V + (x>>1) + 0 ), y1_0);
                _mm256_storeu_si256((__m256i *)(V + (x>>1) + 16), y1_1);
            }
        }
    }
    _mm256_zeroupper();
}

void convert_yuy2_to_yv12_i_10bit_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yuy2_to_yv12_i_highbit_avx2<10>(frame, pixel_data, width, height);
}
void convert_yuy2_to_yv12_i_16bit_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yuy2_to_yv12_i_highbit_avx2<16>(frame, pixel_data, width, height);
}

static __forceinline void gather_y_uv_from_yc48(__m256i& y0, __m256i& y1, __m256i y2) {
    const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
    const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
    __m256i y3 = y0;
    __m256i y4 = y1;
    __m256i y5 = y2;

    y0 = _mm256_blend_epi32(y3, y4, 0xf0);                    // 384, 0
    y1 = _mm256_permute2x128_si256(y3, y5, (0x02<<4) + 0x01); // 512, 128
    y2 = _mm256_blend_epi32(y4, y5, 0xf0);                    // 640, 256

    y3 = _mm256_blend_epi16(y0, y1, MASK_INT_Y);
    y3 = _mm256_blend_epi16(y3, y2, MASK_INT_Y>>2);

    y1 = _mm256_blend_epi16(y0, y1, MASK_INT_UV);
    y1 = _mm256_blend_epi16(y1, y2, MASK_INT_UV>>2);
    y1 = _mm256_alignr_epi8(y1, y1, 2);
    y1 = _mm256_shuffle_epi32(y1, _MM_SHUFFLE(1, 2, 3, 0));//UV1行目

    y0 = _mm256_shuffle_epi8(y3, yC_SUFFLE_YCP_Y);
}

static __forceinline __m256i convert_y_range_from_yc48(__m256i y0, __m256i yC_Y_MA_16, int Y_RSH_16, const __m256i& yC_YCC, const __m256i& yC_pw_one, const __m256i& yC_max) {
    __m256i y7;

    y7 = _mm256_unpackhi_epi16(y0, yC_pw_one);
    y0 = _mm256_unpacklo_epi16(y0, yC_pw_one);

    y0 = _mm256_madd_epi16(y0, yC_Y_MA_16);
    y7 = _mm256_madd_epi16(y7, yC_Y_MA_16);
    y0 = _mm256_srai_epi32(y0, Y_RSH_16);
    y7 = _mm256_srai_epi32(y7, Y_RSH_16);
    y0 = _mm256_add_epi32(y0, yC_YCC);
    y7 = _mm256_add_epi32(y7, yC_YCC);

    y0 = _mm256_packus_epi32(y0, y7);

    y0 = _mm256_min_epu16(y0, yC_max);

    return y0;
}
static __forceinline __m256i convert_uv_range_after_adding_offset(__m256i y0, const __m256i& yC_UV_MA_16, int UV_RSH_16, const __m256i& yC_YCC, const __m256i& yC_pw_one, const __m256i& yC_max) {
    __m256i y7;
    y7 = _mm256_unpackhi_epi16(y0, yC_pw_one);
    y0 = _mm256_unpacklo_epi16(y0, yC_pw_one);

    y0 = _mm256_madd_epi16(y0, yC_UV_MA_16);
    y7 = _mm256_madd_epi16(y7, yC_UV_MA_16);
    y0 = _mm256_srai_epi32(y0, UV_RSH_16);
    y7 = _mm256_srai_epi32(y7, UV_RSH_16);
    y0 = _mm256_add_epi32(y0, yC_YCC);
    y7 = _mm256_add_epi32(y7, yC_YCC);

    y0 = _mm256_packus_epi32(y0, y7);

    y0 = _mm256_min_epu16(y0, yC_max);

    return y0;
}
static __forceinline __m256i convert_uv_range_from_yc48(__m256i y0, const __m256i& yC_UV_OFFSET_x1, const __m256i& yC_UV_MA_16, int UV_RSH_16, const __m256i& yC_YCC, const __m256i& yC_pw_one, const __m256i& yC_max) {
    y0 = _mm256_add_epi16(y0, yC_UV_OFFSET_x1);

    return convert_uv_range_after_adding_offset(y0, yC_UV_MA_16, UV_RSH_16, yC_YCC, yC_pw_one, yC_max);
}
static __forceinline __m256i convert_uv_range_from_yc48_yuv420p(__m256i y0, __m256i y1, const __m256i& yC_UV_OFFSET_x2, __m256i yC_UV_MA_16, int UV_RSH_16, const __m256i& yC_YCC, const __m256i& yC_pw_one, const __m256i& yC_max) {
    y0 = _mm256_add_epi16(y0, y1);
    y0 = _mm256_add_epi16(y0, yC_UV_OFFSET_x2);

    return convert_uv_range_after_adding_offset(y0, yC_UV_MA_16, UV_RSH_16, yC_YCC, yC_pw_one, yC_max);
}
static __forceinline __m256i convert_uv_range_from_yc48_420i(__m256i y0, __m256i y1, const __m256i& yC_UV_OFFSET_x1, const __m256i& yC_UV_MA_16_0, const __m256i& yC_UV_MA_16_1, int UV_RSH_16, const __m256i& yC_YCC, const __m256i& yC_pw_one, const __m256i& yC_max) {
    __m256i y2, y3, y6, y7;

    y0 = _mm256_add_epi16(y0, yC_UV_OFFSET_x1);
    y1 = _mm256_add_epi16(y1, yC_UV_OFFSET_x1);

    y7 = _mm256_unpackhi_epi16(y0, yC_pw_one);
    y6 = _mm256_unpacklo_epi16(y0, yC_pw_one);
    y3 = _mm256_unpackhi_epi16(y1, yC_pw_one);
    y2 = _mm256_unpacklo_epi16(y1, yC_pw_one);

    y6 = _mm256_madd_epi16(y6, yC_UV_MA_16_0);
    y7 = _mm256_madd_epi16(y7, yC_UV_MA_16_0);
    y2 = _mm256_madd_epi16(y2, yC_UV_MA_16_1);
    y3 = _mm256_madd_epi16(y3, yC_UV_MA_16_1);
    y0 = _mm256_add_epi32(y6, y2);
    y7 = _mm256_add_epi32(y7, y3);
    y0 = _mm256_srai_epi32(y0, UV_RSH_16);
    y7 = _mm256_srai_epi32(y7, UV_RSH_16);
    y0 = _mm256_add_epi32(y0, yC_YCC);
    y7 = _mm256_add_epi32(y7, yC_YCC);

    y0 = _mm256_packus_epi32(y0, y7);

    y0 = _mm256_min_epu16(y0, yC_max);

    return y0;
}

void convert_yc48_to_nv12_highbit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height, const int LSFT_YCC, const __m256i& yC_Y_L_MA, const int Y_L_RSH, const __m256i&yC_UV_L_MA_420P, int UV_L_RSH_420P, int bitdepthMax) {
    int x, y;
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_C = (short *)pixel_data->data[1];
    short *ycp, *ycpw;
    short *Y = NULL, *C = NULL;
    const __m256i yC_pw_one = _mm256_set1_epi16(1);
    const __m256i yC_max = _mm256_set1_epi16((short)bitdepthMax);
    const __m256i yC_YCC = _mm256_set1_epi32(1<<LSFT_YCC);
    __m256i y0, y1, y2, y3;
    for (y = 0; y < height; y += 2) {
        ycp = (short*)pixel + width * y * 3;
        ycpw= ycp + width*3;
        Y   = (short*)dst_Y + width * y;
        C   = (short*)dst_C + width * y / 2;
        for (x = 0; x < width; x += 16, ycp += 48, ycpw += 48) {
            y1 = _mm256_loadu_si256((__m256i *)(ycp +  0)); // 128, 0
            y2 = _mm256_loadu_si256((__m256i *)(ycp + 16)); // 384, 256
            y3 = _mm256_loadu_si256((__m256i *)(ycp + 32)); // 640, 512

            gather_y_uv_from_yc48(y1, y2, y3);
            y0 = y2;

            _mm256_storeu_si256((__m256i *)(Y + x), convert_y_range_from_yc48(y1, yC_Y_L_MA, Y_L_RSH, yC_YCC, yC_pw_one, yC_max));

            y1 = _mm256_loadu_si256((__m256i *)(ycpw +  0));
            y2 = _mm256_loadu_si256((__m256i *)(ycpw + 16));
            y3 = _mm256_loadu_si256((__m256i *)(ycpw + 32));

            gather_y_uv_from_yc48(y1, y2, y3);

            _mm256_storeu_si256((__m256i *)(Y + x + width), convert_y_range_from_yc48(y1, yC_Y_L_MA, Y_L_RSH, yC_YCC, yC_pw_one, yC_max));

            _mm256_storeu_si256((__m256i *)(C + x), convert_uv_range_from_yc48_yuv420p(y0, y2,  _mm256_set1_epi16(UV_OFFSET_x2), yC_UV_L_MA_420P, UV_L_RSH_420P, yC_YCC, yC_pw_one, yC_max));
        }
    }
    _mm256_zeroupper();
}

void convert_yc48_to_nv12_10bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_nv12_highbit_avx2(pixel, pixel_data, width, height, LSFT_YCC_10, yC_Y_L_MA_10, Y_L_RSH_10, yC_UV_L_MA_10_420P, UV_L_RSH_10_420P, LIMIT_10);
}

void convert_yc48_to_nv12_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_nv12_highbit_avx2(pixel, pixel_data, width, height, LSFT_YCC_16, yC_Y_L_MA_16, Y_L_RSH_16, yC_UV_L_MA_16_420P, UV_L_RSH_16_420P, LIMIT_16);
}

void convert_yc48_to_nv12_i_highbit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height, const int LSFT_YCC, const __m256i &yC_Y_L_MA, const int Y_L_RSH, const __m256i yC_UV_L_MA_420I0[2], int UV_L_RSH_420I, int bitdepthMax) {
    int x, y, i;
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_C = (short *)pixel_data->data[1];
    short *ycp, *ycpw;
    short *Y = NULL, *C = NULL;
    const __m256i yC_pw_one = _mm256_set1_epi16(1);
    const __m256i yC_max = _mm256_set1_epi16((short)bitdepthMax);
    const __m256i yC_YCC = _mm256_set1_epi32(1<<LSFT_YCC);
    __m256i y0, y1, y2, y3;
    for (y = 0; y < height; y += 4) {
        for (i = 0; i < 2; i++) {
            ycp = (short*)pixel + width * (y + i) * 3;
            ycpw= ycp + width*2*3;
            Y   = (short*)dst_Y + width * (y + i);
            C   = (short*)dst_C + width * (y + i*2) / 2;
            for (x = 0; x < width; x += 16, ycp += 48, ycpw += 48) {
                y1 = _mm256_loadu_si256((__m256i *)(ycp +  0)); // 128, 0
                y2 = _mm256_loadu_si256((__m256i *)(ycp + 16)); // 384, 256
                y3 = _mm256_loadu_si256((__m256i *)(ycp + 32)); // 640, 512

                gather_y_uv_from_yc48(y1, y2, y3);
                y0 = y2;

                _mm256_storeu_si256((__m256i *)(Y + x), convert_y_range_from_yc48(y1, yC_Y_L_MA, Y_L_RSH, yC_YCC, yC_pw_one, yC_max));

                y1 = _mm256_loadu_si256((__m256i *)(ycpw +  0));
                y2 = _mm256_loadu_si256((__m256i *)(ycpw + 16));
                y3 = _mm256_loadu_si256((__m256i *)(ycpw + 32));

                gather_y_uv_from_yc48(y1, y2, y3);

                _mm256_storeu_si256((__m256i *)(Y + x + width*2), convert_y_range_from_yc48(y1, yC_Y_L_MA, Y_L_RSH, yC_YCC, yC_pw_one, yC_max));

                _mm256_storeu_si256((__m256i *)(C + x), convert_uv_range_from_yc48_420i(y0, y2, _mm256_set1_epi16(UV_OFFSET_x1), yC_UV_L_MA_420I0[i], yC_UV_L_MA_420I0[(i+1)&0x01], UV_L_RSH_420I, yC_YCC, yC_pw_one, yC_max));
            }
        }
    }
    _mm256_zeroupper();
}

void convert_yc48_to_nv12_i_10bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_nv12_i_highbit_avx2(pixel, pixel_data, width, height, LSFT_YCC_10, yC_Y_L_MA_10, Y_L_RSH_10, (const __m256i *)Array_UV_L_MA_10_420I, UV_L_RSH_10_420I, LIMIT_10);
}

void convert_yc48_to_nv12_i_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_nv12_i_highbit_avx2(pixel, pixel_data, width, height, LSFT_YCC_16, yC_Y_L_MA_16, Y_L_RSH_16, (__m256i *)Array_UV_L_MA_16_420I, UV_L_RSH_16_420I, LIMIT_16);
}

void convert_yc48_to_yv12_highbit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height, const int LSFT_YCC, const __m256i &yC_Y_L_MA, const int Y_L_RSH, const __m256i &yC_UV_L_MA_420P, int UV_L_RSH_420P, int bitdepthMax) {
    int x, y;
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_U = (short *)pixel_data->data[1];
    short *dst_V = (short *)pixel_data->data[2];
    short *ycp, *ycpw;
    short *Y = NULL, *U = NULL, *V = NULL;
    const __m256i yC_pw_one = _mm256_set1_epi16(1);
    const __m256i yC_max = _mm256_set1_epi16((short)bitdepthMax);
    const __m256i yC_YCC = _mm256_set1_epi32(1<<LSFT_YCC);
    __m256i y0, y1, y2, y3, y4;
    for (y = 0; y < height; y += 2) {
        ycp = (short*)pixel + width * y * 3;
        ycpw= ycp + width*3;
        Y   = (short*)dst_Y + width * y;
        U   = (short*)dst_U + width * y / 4;
        V   = (short*)dst_V + width * y / 4;
        for (x = 0; x < width; x += 32, ycp += 96, ycpw += 96) {
            y1 = _mm256_loadu_si256((__m256i *)(ycp +  0)); // 128, 0
            y2 = _mm256_loadu_si256((__m256i *)(ycp + 16)); // 384, 256
            y3 = _mm256_loadu_si256((__m256i *)(ycp + 32)); // 640, 512

            gather_y_uv_from_yc48(y1, y2, y3);
            y0 = y2;

            _mm256_storeu_si256((__m256i *)(Y + x), convert_y_range_from_yc48(y1, yC_Y_L_MA, Y_L_RSH, yC_YCC, yC_pw_one, yC_max));

            y1 = _mm256_loadu_si256((__m256i *)(ycpw +  0));
            y2 = _mm256_loadu_si256((__m256i *)(ycpw + 16));
            y3 = _mm256_loadu_si256((__m256i *)(ycpw + 32));

            gather_y_uv_from_yc48(y1, y2, y3);

            _mm256_storeu_si256((__m256i *)(Y + x + width), convert_y_range_from_yc48(y1, yC_Y_L_MA, Y_L_RSH, yC_YCC, yC_pw_one, yC_max));

            y4 = convert_uv_range_from_yc48_yuv420p(y0, y2,  _mm256_set1_epi16(UV_OFFSET_x2), yC_UV_L_MA_420P, UV_L_RSH_420P, yC_YCC, yC_pw_one, yC_max);

            y1 = _mm256_loadu_si256((__m256i *)(ycp + 48)); // 128, 0
            y2 = _mm256_loadu_si256((__m256i *)(ycp + 64)); // 384, 256
            y3 = _mm256_loadu_si256((__m256i *)(ycp + 80)); // 640, 512

            gather_y_uv_from_yc48(y1, y2, y3);
            y0 = y2;

            _mm256_storeu_si256((__m256i *)(Y + x + 16), convert_y_range_from_yc48(y1, yC_Y_L_MA, Y_L_RSH, yC_YCC, yC_pw_one, yC_max));

            y1 = _mm256_loadu_si256((__m256i *)(ycpw + 48));
            y2 = _mm256_loadu_si256((__m256i *)(ycpw + 64));
            y3 = _mm256_loadu_si256((__m256i *)(ycpw + 80));

            gather_y_uv_from_yc48(y1, y2, y3);

            _mm256_storeu_si256((__m256i *)(Y + x + 16 + width), convert_y_range_from_yc48(y1, yC_Y_L_MA, Y_L_RSH, yC_YCC, yC_pw_one, yC_max));

            y0 = convert_uv_range_from_yc48_yuv420p(y0, y2,  _mm256_set1_epi16(UV_OFFSET_x2), yC_UV_L_MA_420P, UV_L_RSH_420P, yC_YCC, yC_pw_one, yC_max);

            separate_low_up_16bit(y4, y0);

            _mm256_storeu_si256((__m256i *)(U + (x>>1)), y4);
            _mm256_storeu_si256((__m256i *)(V + (x>>1)), y0);
        }
    }
    _mm256_zeroupper();
}

void convert_yc48_to_yv12_10bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yv12_highbit_avx2(pixel, pixel_data, width, height, LSFT_YCC_10, yC_Y_L_MA_10, Y_L_RSH_10, yC_UV_L_MA_10_420P, UV_L_RSH_10_420P, LIMIT_10);
}

void convert_yc48_to_yv12_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yv12_highbit_avx2(pixel, pixel_data, width, height, LSFT_YCC_16, yC_Y_L_MA_16, Y_L_RSH_16, yC_UV_L_MA_16_420P, UV_L_RSH_16_420P, LIMIT_16);
}

void convert_yc48_to_yv12_i_highbit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height, const int LSFT_YCC, const __m256i &yC_Y_L_MA, const int Y_L_RSH, const __m256i yC_UV_L_MA_420I0[2], int UV_L_RSH_420I, int bitdepthMax) {
    int x, y, i;
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_U = (short *)pixel_data->data[1];
    short *dst_V = (short *)pixel_data->data[2];
    short *ycp, *ycpw;
    short *Y = NULL, *U = NULL, *V = NULL;
    const __m256i yC_pw_one = _mm256_set1_epi16(1);
    const __m256i yC_max = _mm256_set1_epi16((short)bitdepthMax);
    const __m256i yC_YCC = _mm256_set1_epi32(1<<LSFT_YCC);
    __m256i y0, y1, y2, y3, y4;
    for (y = 0; y < height; y += 4) {
        for (i = 0; i < 2; i++) {
            ycp = (short*)pixel + width * (y + i) * 3;
            ycpw= ycp + width*2*3;
            Y   = (short*)dst_Y + width * (y + i);
            U   = (short*)dst_U + width * (y + i*2) / 4;
            V   = (short*)dst_V + width * (y + i*2) / 4;
            for (x = 0; x < width; x += 32, ycp += 96, ycpw += 96) {
                y1 = _mm256_loadu_si256((__m256i *)(ycp +  0)); // 128, 0
                y2 = _mm256_loadu_si256((__m256i *)(ycp + 16)); // 384, 256
                y3 = _mm256_loadu_si256((__m256i *)(ycp + 32)); // 640, 512

                gather_y_uv_from_yc48(y1, y2, y3);
                y0 = y2;

                _mm256_storeu_si256((__m256i *)(Y + x), convert_y_range_from_yc48(y1, yC_Y_L_MA, Y_L_RSH, yC_YCC, yC_pw_one, yC_max));

                y1 = _mm256_loadu_si256((__m256i *)(ycpw +  0));
                y2 = _mm256_loadu_si256((__m256i *)(ycpw + 16));
                y3 = _mm256_loadu_si256((__m256i *)(ycpw + 32));

                gather_y_uv_from_yc48(y1, y2, y3);

                _mm256_storeu_si256((__m256i *)(Y + x + width*2), convert_y_range_from_yc48(y1, yC_Y_L_MA, Y_L_RSH, yC_YCC, yC_pw_one, yC_max));

                y4 = convert_uv_range_from_yc48_420i(y0, y2, _mm256_set1_epi16(UV_OFFSET_x1), yC_UV_L_MA_420I0[i], yC_UV_L_MA_420I0[(i+1)&0x01], UV_L_RSH_420I, yC_YCC, yC_pw_one, yC_max);

                y1 = _mm256_loadu_si256((__m256i *)(ycp + 48)); // 128, 0
                y2 = _mm256_loadu_si256((__m256i *)(ycp + 64)); // 384, 256
                y3 = _mm256_loadu_si256((__m256i *)(ycp + 80)); // 640, 512

                gather_y_uv_from_yc48(y1, y2, y3);
                y0 = y2;

                _mm256_storeu_si256((__m256i *)(Y + x + 16), convert_y_range_from_yc48(y1, yC_Y_L_MA, Y_L_RSH, yC_YCC, yC_pw_one, yC_max));

                y1 = _mm256_loadu_si256((__m256i *)(ycpw + 48));
                y2 = _mm256_loadu_si256((__m256i *)(ycpw + 64));
                y3 = _mm256_loadu_si256((__m256i *)(ycpw + 80));

                gather_y_uv_from_yc48(y1, y2, y3);

                _mm256_storeu_si256((__m256i *)(Y + x + 16 + width*2), convert_y_range_from_yc48(y1, yC_Y_L_MA, Y_L_RSH, yC_YCC, yC_pw_one, yC_max));

                y0 = convert_uv_range_from_yc48_420i(y0, y2, _mm256_set1_epi16(UV_OFFSET_x1), yC_UV_L_MA_420I0[i], yC_UV_L_MA_420I0[(i+1)&0x01], UV_L_RSH_16_420I, yC_YCC, yC_pw_one, yC_max);

                separate_low_up_16bit(y4, y0);

                _mm256_storeu_si256((__m256i *)(U + (x>>1)), y4);
                _mm256_storeu_si256((__m256i *)(V + (x>>1)), y0);
            }
        }
    }
    _mm256_zeroupper();
}

void convert_yc48_to_yv12_i_10bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yv12_i_highbit_avx2(pixel, pixel_data, width, height, LSFT_YCC_10, yC_Y_L_MA_10, Y_L_RSH_10, (const __m256i *)Array_UV_L_MA_10_420I, UV_L_RSH_10_420I, LIMIT_10);
}

void convert_yc48_to_yv12_i_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    convert_yc48_to_yv12_i_highbit_avx2(pixel, pixel_data, width, height, LSFT_YCC_16, yC_Y_L_MA_16, Y_L_RSH_16, (const __m256i *)Array_UV_L_MA_16_420I, UV_L_RSH_16_420I, LIMIT_16);
}

static __forceinline void gather_y_u_v_from_yc48(__m256i& y0, __m256i& y1, __m256i& y2) {
    __m256i y3, y4, y5;
    const int MASK_INT = 0x40 + 0x08 + 0x01;
    y3 = _mm256_blend_epi32(y0, y1, 0xf0);                    // 384, 0
    y4 = _mm256_permute2x128_si256(y0, y2, (0x02<<4) + 0x01); // 512, 128
    y5 = _mm256_blend_epi32(y1, y2, 0xf0);                    // 640, 256

    y0 = _mm256_blend_epi16(y5, y3, MASK_INT);
    y1 = _mm256_blend_epi16(y4, y5, MASK_INT);
    y2 = _mm256_blend_epi16(y3, y4, MASK_INT);

    y0 = _mm256_blend_epi16(y0, y4, MASK_INT<<1);
    y1 = _mm256_blend_epi16(y1, y3, MASK_INT<<1);
    y2 = _mm256_blend_epi16(y2, y5, MASK_INT<<1);

    y0 = _mm256_shuffle_epi8(y0, yC_SUFFLE_YCP_Y);
    y1 = _mm256_shuffle_epi8(y1, _mm256_alignr_epi8(yC_SUFFLE_YCP_Y, yC_SUFFLE_YCP_Y, 6));
    y2 = _mm256_shuffle_epi8(y2, _mm256_alignr_epi8(yC_SUFFLE_YCP_Y, yC_SUFFLE_YCP_Y, 12));
}
void convert_yc48_to_yuv444_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    BYTE *Y = (BYTE *)pixel_data->data[0];
    BYTE *U = (BYTE *)pixel_data->data[1];
    BYTE *V = (BYTE *)pixel_data->data[2];
    short *ycp;
    short *const ycp_fin = (short *)pixel + width * height * 3;
    const __m256i yC_pw_one = _mm256_set1_epi16(1);
    const __m256i yC_max = _mm256_set1_epi16((short)LIMIT_16);
    const __m256i yC_YCC = _mm256_set1_epi32(1<<LSFT_YCC_16);
    __m256i y1, y2, y3, yY, yU, yV;
    for (ycp = (short *)pixel; ycp < ycp_fin; ycp += 96, Y += 32, U += 32, V += 32) {
        y1 = _mm256_loadu_si256((__m256i *)(ycp +  0));
        y2 = _mm256_loadu_si256((__m256i *)(ycp + 16));
        y3 = _mm256_loadu_si256((__m256i *)(ycp + 32));

        gather_y_u_v_from_yc48(y1, y2, y3);

        y1 = convert_y_range_from_yc48(y1, yC_Y_L_MA_16, Y_L_RSH_16, yC_YCC, yC_pw_one, yC_max);
        y2 = convert_uv_range_from_yc48(y2, _mm256_set1_epi16(UV_OFFSET_x1), yC_UV_L_MA_16_444, UV_L_RSH_16_444, yC_YCC, yC_pw_one, yC_max);
        y3 = convert_uv_range_from_yc48(y3, _mm256_set1_epi16(UV_OFFSET_x1), yC_UV_L_MA_16_444, UV_L_RSH_16_444, yC_YCC, yC_pw_one, yC_max);
        yY = _mm256_srli_epi16(y1, 8);
        yU = _mm256_srli_epi16(y2, 8);
        yV = _mm256_srli_epi16(y3, 8);

        y1 = _mm256_loadu_si256((__m256i *)(ycp + 48));
        y2 = _mm256_loadu_si256((__m256i *)(ycp + 64));
        y3 = _mm256_loadu_si256((__m256i *)(ycp + 80));

        gather_y_u_v_from_yc48(y1, y2, y3);

        y1 = convert_y_range_from_yc48(y1, yC_Y_L_MA_16, Y_L_RSH_16, yC_YCC, yC_pw_one, yC_max);
        y2 = convert_uv_range_from_yc48(y2, _mm256_set1_epi16(UV_OFFSET_x1), yC_UV_L_MA_16_444, UV_L_RSH_16_444, yC_YCC, yC_pw_one, yC_max);
        y3 = convert_uv_range_from_yc48(y3, _mm256_set1_epi16(UV_OFFSET_x1), yC_UV_L_MA_16_444, UV_L_RSH_16_444, yC_YCC, yC_pw_one, yC_max);
        y1 = _mm256_srli_epi16(y1, 8);
        y2 = _mm256_srli_epi16(y2, 8);
        y3 = _mm256_srli_epi16(y3, 8);

        yY = _mm256_packus_epi16(yY, y1);
        yU = _mm256_packus_epi16(yU, y2);
        yV = _mm256_packus_epi16(yV, y3);

        yY = _mm256_permute4x64_epi64(yY, _MM_SHUFFLE(3,1,2,0));
        yU = _mm256_permute4x64_epi64(yU, _MM_SHUFFLE(3,1,2,0));
        yV = _mm256_permute4x64_epi64(yV, _MM_SHUFFLE(3,1,2,0));

        _mm256_storeu_si256((__m256i *)Y, yY);
        _mm256_storeu_si256((__m256i *)U, yU);
        _mm256_storeu_si256((__m256i *)V, yV);
    }
    _mm256_zeroupper();
}
void convert_yc48_to_yuv444_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    short *Y = (short *)pixel_data->data[0];
    short *U = (short *)pixel_data->data[1];
    short *V = (short *)pixel_data->data[2];
    short *ycp;
    short *const ycp_fin = (short *)pixel + width * height * 3;
    const __m256i yC_pw_one = _mm256_set1_epi16(1);
    const __m256i yC_max = _mm256_set1_epi16((short)LIMIT_16);
    const __m256i yC_YCC = _mm256_set1_epi32(1<<LSFT_YCC_16);
    __m256i y1, y2, y3;
    for (ycp = (short *)pixel; ycp < ycp_fin; ycp += 48, Y += 16, U += 16, V += 16) {
        y1 = _mm256_loadu_si256((__m256i *)(ycp +  0));
        y2 = _mm256_loadu_si256((__m256i *)(ycp + 16));
        y3 = _mm256_loadu_si256((__m256i *)(ycp + 32));

        gather_y_u_v_from_yc48(y1, y2, y3);

        _mm256_storeu_si256((__m256i *)Y, convert_y_range_from_yc48(y1, yC_Y_L_MA_16, Y_L_RSH_16, yC_YCC, yC_pw_one, yC_max));
        _mm256_storeu_si256((__m256i *)U, convert_uv_range_from_yc48(y2, _mm256_set1_epi16(UV_OFFSET_x1), yC_UV_L_MA_16_444, UV_L_RSH_16_444, yC_YCC, yC_pw_one, yC_max));
        _mm256_storeu_si256((__m256i *)V, convert_uv_range_from_yc48(y3, _mm256_set1_epi16(UV_OFFSET_x1), yC_UV_L_MA_16_444, UV_L_RSH_16_444, yC_YCC, yC_pw_one, yC_max));
    }
    _mm256_zeroupper();
}

void convert_yuy2_to_nv16_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    BYTE *p = (BYTE *)pixel;
    BYTE * const p_fin = p + width * height * 2;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_C = pixel_data->data[1];
    __m256i y0, y1;
    for (; p < p_fin; p += 64, dst_Y += 32, dst_C += 32) {
        y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
        y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

        separate_low_up(y0, y1);

        _mm256_storeu_si256((__m256i *)dst_Y, y0);
        _mm256_storeu_si256((__m256i *)dst_C, y1);
    }
    _mm256_zeroupper();
}

void convert_yuy2_to_nv16_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    BYTE *p = (BYTE *)pixel;
    BYTE * const p_fin = p + width * height * 2;
    USHORT *dst_Y = (USHORT *)pixel_data->data[0];
    USHORT *dst_C = (USHORT *)pixel_data->data[1];
    __m256i y0, y1, y0_0, y0_1, y1_0, y1_1;
    for (; p < p_fin; p += 64, dst_Y += 32, dst_C += 32) {
        y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
        y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

        separate_low_up(y0, y1);

        y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
        y1 = _mm256_permute4x64_epi64(y1, _MM_SHUFFLE(3,1,2,0));
        y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), 8);
        y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), 8);
        y1_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y1, _mm256_setzero_si256()), 8);
        y1_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y1, _mm256_setzero_si256()), 8);

        _mm256_storeu_si256((__m256i *)(dst_Y +  0), y0_0);
        _mm256_storeu_si256((__m256i *)(dst_Y + 16), y0_1);
        _mm256_storeu_si256((__m256i *)(dst_C +  0), y1_0);
        _mm256_storeu_si256((__m256i *)(dst_C + 16), y1_1);
    }
    _mm256_zeroupper();
}

void convert_yc48_to_nv16_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_C = (short *)pixel_data->data[1];
    short *ycp = (short *)pixel;
    short * const ycp_fin = ycp + width * height * 3;
    const __m256i yC_pw_one = _mm256_set1_epi16(1);
    const __m256i yC_max = _mm256_set1_epi16((short)LIMIT_16);
    const __m256i yC_YCC = _mm256_set1_epi32(1<<LSFT_YCC_16);
    __m256i y1, y2, y3;
    for (; ycp < ycp_fin; ycp += 48, dst_Y += 16, dst_C += 16) {
        y1 = _mm256_loadu_si256((__m256i *)(ycp +  0));
        y2 = _mm256_loadu_si256((__m256i *)(ycp + 16));
        y3 = _mm256_loadu_si256((__m256i *)(ycp + 32));

        gather_y_uv_from_yc48(y1, y2, y3);

        _mm256_storeu_si256((__m256i *)dst_Y, convert_y_range_from_yc48(y1, yC_Y_L_MA_16, Y_L_RSH_16, yC_YCC, yC_pw_one, yC_max));
        _mm256_storeu_si256((__m256i *)dst_C, convert_uv_range_from_yc48(y2, _mm256_set1_epi16(UV_OFFSET_x1), yC_UV_L_MA_16_444, UV_L_RSH_16_444, yC_YCC, yC_pw_one, yC_max));
    }
    _mm256_zeroupper();
}
void convert_lw48_to_nv12_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y;
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_C = (short *)pixel_data->data[1];
    short *ycp, *ycpw;
    short *Y = NULL, *C = NULL;
    __m256i y0, y1, y2, y3;
    for (y = 0; y < height; y += 2) {
        ycp = (short*)pixel + width * y * 3;
        ycpw= ycp + width*3;
        Y   = (short*)dst_Y + width * y;
        C   = (short*)dst_C + width * y / 2;
        for (x = 0; x < width; x += 16, ycp += 48, ycpw += 48) {
            y1 = _mm256_loadu_si256((__m256i *)(ycp +  0)); // 128, 0
            y2 = _mm256_loadu_si256((__m256i *)(ycp + 16)); // 384, 256
            y3 = _mm256_loadu_si256((__m256i *)(ycp + 32)); // 640, 512

            gather_y_uv_from_yc48(y1, y2, y3);
            y0 = y2;

            _mm256_storeu_si256((__m256i *)(Y + x), y1);

            y1 = _mm256_loadu_si256((__m256i *)(ycpw +  0));
            y2 = _mm256_loadu_si256((__m256i *)(ycpw + 16));
            y3 = _mm256_loadu_si256((__m256i *)(ycpw + 32));

            gather_y_uv_from_yc48(y1, y2, y3);

            _mm256_storeu_si256((__m256i *)(Y + x + width), y1);

            y0 = _mm256_avg_epu16(y0, y2);

            _mm256_storeu_si256((__m256i *)(C + x), y0);
        }
    }
    _mm256_zeroupper();
}

void convert_lw48_to_nv12_i_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y, i;
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_C = (short *)pixel_data->data[1];
    short *ycp, *ycpw;
    short *Y = NULL, *C = NULL;
    __m256i y0, y1, y2, y3;
    for (y = 0; y < height; y += 4) {
        for (i = 0; i < 2; i++) {
            ycp = (short*)pixel + width * (y + i) * 3;
            ycpw= ycp + width*2*3;
            Y   = (short*)dst_Y + width * (y + i);
            C   = (short*)dst_C + width * (y + i*2) / 2;
            for (x = 0; x < width; x += 16, ycp += 48, ycpw += 48) {
                y1 = _mm256_loadu_si256((__m256i *)(ycp +  0)); // 128, 0
                y2 = _mm256_loadu_si256((__m256i *)(ycp + 16)); // 384, 256
                y3 = _mm256_loadu_si256((__m256i *)(ycp + 32)); // 640, 512

                gather_y_uv_from_yc48(y1, y2, y3);
                y0 = y2;

                _mm256_storeu_si256((__m256i *)(Y + x), y1);

                y1 = _mm256_loadu_si256((__m256i *)(ycpw +  0));
                y2 = _mm256_loadu_si256((__m256i *)(ycpw + 16));
                y3 = _mm256_loadu_si256((__m256i *)(ycpw + 32));

                gather_y_uv_from_yc48(y1, y2, y3);

                _mm256_storeu_si256((__m256i *)(Y + x + width*2), y1);

                y1 = _mm256_unpacklo_epi16(y0, y2);
                y0 = _mm256_unpackhi_epi16(y0, y2);
                y1 = _mm256_madd_epi16(y1, yC_INTERLACE_WEIGHT(i));
                y0 = _mm256_madd_epi16(y0, yC_INTERLACE_WEIGHT(i));
                y1 = _mm256_srli_epi32(y1, 2);
                y0 = _mm256_srli_epi32(y0, 2);
                y1 = _mm256_packus_epi32(y1, y0);

                _mm256_storeu_si256((__m256i *)(C + x), y1);
            }
        }
    }
    _mm256_zeroupper();
}
void convert_lw48_to_nv16_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    short *dst_Y = (short *)pixel_data->data[0];
    short *dst_C = (short *)pixel_data->data[1];
    short *ycp = (short *)pixel;
    short * const ycp_fin = ycp + width * height * 3;
    __m256i y1, y2, y3;
    for (; ycp < ycp_fin; ycp += 48, dst_Y += 16, dst_C += 16) {
        y1 = _mm256_loadu_si256((__m256i *)(ycp +  0));
        y2 = _mm256_loadu_si256((__m256i *)(ycp + 16));
        y3 = _mm256_loadu_si256((__m256i *)(ycp + 32));

        gather_y_uv_from_yc48(y1, y2, y3);

        _mm256_storeu_si256((__m256i *)dst_Y, y1);
        _mm256_storeu_si256((__m256i *)dst_C, y2);
    }
    _mm256_zeroupper();
}
void convert_lw48_to_yuv444_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    BYTE *Y = (BYTE *)pixel_data->data[0];
    BYTE *U = (BYTE *)pixel_data->data[1];
    BYTE *V = (BYTE *)pixel_data->data[2];
    short *ycp;
    short *const ycp_fin = (short *)pixel + width * height * 3;
    __m256i y1, y2, y3, yY, yU, yV;
    for (ycp = (short *)pixel; ycp < ycp_fin; ycp += 96, Y += 32, U += 32, V += 32) {
        y1 = _mm256_loadu_si256((__m256i *)(ycp +  0));
        y2 = _mm256_loadu_si256((__m256i *)(ycp + 16));
        y3 = _mm256_loadu_si256((__m256i *)(ycp + 32));

        gather_y_u_v_from_yc48(y1, y2, y3);

        yY = _mm256_srli_epi16(y1, 8);
        yU = _mm256_srli_epi16(y2, 8);
        yV = _mm256_srli_epi16(y3, 8);

        y1 = _mm256_loadu_si256((__m256i *)(ycp + 48));
        y2 = _mm256_loadu_si256((__m256i *)(ycp + 64));
        y3 = _mm256_loadu_si256((__m256i *)(ycp + 80));

        gather_y_u_v_from_yc48(y1, y2, y3);

        y1 = _mm256_srli_epi16(y1, 8);
        y2 = _mm256_srli_epi16(y2, 8);
        y3 = _mm256_srli_epi16(y3, 8);

        yY = _mm256_packus_epi16(yY, y1);
        yU = _mm256_packus_epi16(yU, y2);
        yV = _mm256_packus_epi16(yV, y3);

        yY = _mm256_permute4x64_epi64(yY, _MM_SHUFFLE(3,1,2,0));
        yU = _mm256_permute4x64_epi64(yU, _MM_SHUFFLE(3,1,2,0));
        yV = _mm256_permute4x64_epi64(yV, _MM_SHUFFLE(3,1,2,0));

        _mm256_storeu_si256((__m256i *)Y, yY);
        _mm256_storeu_si256((__m256i *)U, yU);
        _mm256_storeu_si256((__m256i *)V, yV);
    }
    _mm256_zeroupper();
}
void convert_lw48_to_yuv444_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    short *Y = (short *)pixel_data->data[0];
    short *U = (short *)pixel_data->data[1];
    short *V = (short *)pixel_data->data[2];
    short *ycp;
    short *const ycp_fin = (short *)pixel + width * height * 3;
    __m256i y1, y2, y3;
    for (ycp = (short *)pixel; ycp < ycp_fin; ycp += 48, Y += 16, U += 16, V += 16) {
        y1 = _mm256_loadu_si256((__m256i *)(ycp +  0));
        y2 = _mm256_loadu_si256((__m256i *)(ycp + 16));
        y3 = _mm256_loadu_si256((__m256i *)(ycp + 32));

        gather_y_u_v_from_yc48(y1, y2, y3);

        _mm256_storeu_si256((__m256i *)Y, y1);
        _mm256_storeu_si256((__m256i *)U, y2);
        _mm256_storeu_si256((__m256i *)V, y3);
    }
    _mm256_zeroupper();
}


static __forceinline void separate_8bit_packed(__m256i& yA, __m256i& yB, __m256i& yC, const __m256i& y0, const __m256i& y1, const __m256i& y2) {
    alignas(32) static const unsigned char mask_select[] = {
        0xffu, 0x00u, 0x00u, 0xffu, 0x00u, 0x00u, 0xffu, 0x00u, 0x00u, 0xffu, 0x00u, 0x00u, 0xffu, 0x00u, 0x00u, 0xffu,
        0xffu, 0x00u, 0x00u, 0xffu, 0x00u, 0x00u, 0xffu, 0x00u, 0x00u, 0xffu, 0x00u, 0x00u, 0xffu, 0x00u, 0x00u, 0xffu
    };
    alignas(32) static const char mask_shuffle0[] = {
        0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13,
        0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14, 1, 4, 7, 10, 13
    };
    alignas(32) static const char mask_shuffle1[] = {
        1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14,
        1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2, 5, 8, 11, 14
    };
    alignas(32) static const char mask_shuffle2[] = {
        2, 5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15,
        2, 5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15,
    };
    static_assert(sizeof(mask_select) == 32);
    static_assert(sizeof(mask_shuffle0) == 32);
    static_assert(sizeof(mask_shuffle1) == 32);
    static_assert(sizeof(mask_shuffle2) == 32);

    yA = _mm256_blend_epi32(y0, y1, 0xF0);
    yB = _mm256_permute2x128_si256(y0, y2, (2 << 4) | 1);
    yC = _mm256_blend_epi32(y1, y2, 0xF0);

    __m256i mask0_256 = _mm256_load_si256((__m256i*)mask_select);
    __m256i mask1_256 = _mm256_slli_si256(mask0_256, 1);
    __m256i mask2_256 = _mm256_slli_si256(mask0_256, 2);

    __m256i a32_0 = _mm256_and_si256(yA, mask0_256);
    __m256i a32_1 = _mm256_and_si256(yB, mask2_256);
    __m256i a32_2 = _mm256_and_si256(yC, mask1_256);

    __m256i b32_0 = _mm256_and_si256(yA, mask1_256);
    __m256i b32_1 = _mm256_and_si256(yB, mask0_256);
    __m256i b32_2 = _mm256_and_si256(yC, mask2_256);

    __m256i c32_0 = _mm256_and_si256(yA, mask2_256);
    __m256i c32_1 = _mm256_and_si256(yB, mask1_256);
    __m256i c32_2 = _mm256_and_si256(yC, mask0_256);

    __m256i a32_012 = _mm256_or_si256(a32_0, _mm256_or_si256(a32_1, a32_2));
    __m256i b32_012 = _mm256_or_si256(b32_0, _mm256_or_si256(b32_1, b32_2));
    __m256i c32_012 = _mm256_or_si256(c32_0, _mm256_or_si256(c32_1, c32_2));

    yA = _mm256_shuffle_epi8(a32_012, _mm256_load_si256((__m256i*)mask_shuffle0));
    yB = _mm256_shuffle_epi8(b32_012, _mm256_load_si256((__m256i*)mask_shuffle1));
    yC = _mm256_shuffle_epi8(c32_012, _mm256_load_si256((__m256i*)mask_shuffle2));
}

template<int bit_depth>
static __forceinline void convert_rgb2yuv(__m256& y_f1, __m256& u_f1, __m256& v_f1, 
    const __m256& r_f1, const __m256& g_f1, const __m256& b_f1,
    const __m256& coeff_ry, const __m256& coeff_gy, const __m256& coeff_by,
    const __m256& coeff_ru, const __m256& coeff_gu, const __m256& coeff_bu,
    const __m256& coeff_rv, const __m256& coeff_gv, const __m256& coeff_bv) {
    const __m256 offset_y = _mm256_set1_ps(16.0f * (1 << (bit_depth - 8)));
    const __m256 offset_uv = _mm256_set1_ps(128.0f * (1 << (bit_depth - 8)));
    y_f1 = _mm256_fmadd_ps(coeff_ry, r_f1, 
           _mm256_fmadd_ps(coeff_gy, g_f1, 
           _mm256_fmadd_ps(coeff_by, b_f1, offset_y)));
    u_f1 = _mm256_fmadd_ps(coeff_ru, r_f1,
           _mm256_fmadd_ps(coeff_gu, g_f1,
           _mm256_fmadd_ps(coeff_bu, b_f1, offset_uv)));
    v_f1 = _mm256_fmadd_ps(coeff_rv, r_f1,
           _mm256_fmadd_ps(coeff_gv, g_f1,
           _mm256_fmadd_ps(coeff_bv, b_f1, offset_uv)));
}

void convert_rgb_to_yuv444_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    const int out_bit_depth = 8;
    BYTE *ptrY = pixel_data->data[0];
    BYTE *ptrU = pixel_data->data[1];
    BYTE *ptrV = pixel_data->data[2];
    const float *coeff_table = COEFF_RGB2YUV[pixel_data->colormatrix ? 1 : 0];
    int y0 = 0, y1 = height - 1;
    const int srcstep = (width*3 + 3) & ~3;

    // AVX2用の係数をロード
    const __m256 coeff_ry = _mm256_set1_ps(coeff_table[0]);
    const __m256 coeff_gy = _mm256_set1_ps(coeff_table[1]);
    const __m256 coeff_by = _mm256_set1_ps(coeff_table[2]);
    const __m256 coeff_ru = _mm256_set1_ps(coeff_table[3]);
    const __m256 coeff_gu = _mm256_set1_ps(coeff_table[4]);
    const __m256 coeff_bu = _mm256_set1_ps(coeff_table[5]);
    const __m256 coeff_rv = _mm256_set1_ps(coeff_table[6]);
    const __m256 coeff_gv = _mm256_set1_ps(coeff_table[7]);
    const __m256 coeff_bv = _mm256_set1_ps(coeff_table[8]);
    
    const __m256 round_offset = _mm256_set1_ps(0.5f);
    
    for (; y0 < height; y0++, y1--) {
        BYTE *dstY = (BYTE *)(ptrY + y1*width*sizeof(BYTE));
        BYTE *dstU = (BYTE *)(ptrU + y1*width*sizeof(BYTE));
        BYTE *dstV = (BYTE *)(ptrV + y1*width*sizeof(BYTE));
        BYTE *src = (BYTE*)frame + y0*srcstep;
        
        int x = 0;
        // AVX2で32ピクセルずつ処理
        for (; x <= width - 32; x += 32) {
            // 32ピクセル分のBGRデータをロード (96バイト)
            __m256i bgr0 = _mm256_loadu_si256((__m256i*)(src + x*3));      // 32バイト
            __m256i bgr1 = _mm256_loadu_si256((__m256i*)(src + x*3 + 32)); // 32バイト
            __m256i bgr2 = _mm256_loadu_si256((__m256i*)(src + x*3 + 64)); // 32バイト

            __m256i b_8, g_8, r_8;
            separate_8bit_packed(b_8, g_8, r_8, bgr0, bgr1, bgr2);

            __m256i b_16_0 = _mm256_unpacklo_epi8(b_8, _mm256_setzero_si256());
            __m256i b_16_1 = _mm256_unpackhi_epi8(b_8, _mm256_setzero_si256());
            __m256i g_16_0 = _mm256_unpacklo_epi8(g_8, _mm256_setzero_si256());
            __m256i g_16_1 = _mm256_unpackhi_epi8(g_8, _mm256_setzero_si256());
            __m256i r_16_0 = _mm256_unpacklo_epi8(r_8, _mm256_setzero_si256());
            __m256i r_16_1 = _mm256_unpackhi_epi8(r_8, _mm256_setzero_si256());

            __m256i b_32_0 = _mm256_unpacklo_epi16(b_16_0, _mm256_setzero_si256());
            __m256i b_32_1 = _mm256_unpackhi_epi16(b_16_0, _mm256_setzero_si256());
            __m256i b_32_2 = _mm256_unpacklo_epi16(b_16_1, _mm256_setzero_si256());
            __m256i b_32_3 = _mm256_unpackhi_epi16(b_16_1, _mm256_setzero_si256());

            __m256i g_32_0 = _mm256_unpacklo_epi16(g_16_0, _mm256_setzero_si256());
            __m256i g_32_1 = _mm256_unpackhi_epi16(g_16_0, _mm256_setzero_si256());
            __m256i g_32_2 = _mm256_unpacklo_epi16(g_16_1, _mm256_setzero_si256());
            __m256i g_32_3 = _mm256_unpackhi_epi16(g_16_1, _mm256_setzero_si256());
            
            __m256i r_32_0 = _mm256_unpacklo_epi16(r_16_0, _mm256_setzero_si256());
            __m256i r_32_1 = _mm256_unpackhi_epi16(r_16_0, _mm256_setzero_si256());
            __m256i r_32_2 = _mm256_unpacklo_epi16(r_16_1, _mm256_setzero_si256());
            __m256i r_32_3 = _mm256_unpackhi_epi16(r_16_1, _mm256_setzero_si256());
            
            // グループ1: ピクセル0-7 (下位128ビット)
            __m256 b_f0 = _mm256_cvtepi32_ps(b_32_0);
            __m256 g_f0 = _mm256_cvtepi32_ps(g_32_0);
            __m256 r_f0 = _mm256_cvtepi32_ps(r_32_0);

            __m256 y_f0, u_f0, v_f0;
            convert_rgb2yuv<8>(y_f0, u_f0, v_f0, r_f0, g_f0, b_f0, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);
            
            // グループ2: ピクセル8-15 (上位128ビット)
            __m256 b_f1 = _mm256_cvtepi32_ps(b_32_1);
            __m256 g_f1 = _mm256_cvtepi32_ps(g_32_1);
            __m256 r_f1 = _mm256_cvtepi32_ps(r_32_1);

            __m256 y_f1, u_f1, v_f1;
            convert_rgb2yuv<8>(y_f1, u_f1, v_f1, r_f1, g_f1, b_f1, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);
            
            // グループ3: ピクセル16-23（上位128ビットから
            __m256 b_f2 = _mm256_cvtepi32_ps(b_32_2);
            __m256 g_f2 = _mm256_cvtepi32_ps(g_32_2);
            __m256 r_f2 = _mm256_cvtepi32_ps(r_32_2);

            __m256 y_f2, u_f2, v_f2;
            convert_rgb2yuv<8>(y_f2, u_f2, v_f2, r_f2, g_f2, b_f2, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);
            
            // グループ4: ピクセル24-31
            __m256 b_f3 = _mm256_cvtepi32_ps(b_32_3);
            __m256 g_f3 = _mm256_cvtepi32_ps(g_32_3);
            __m256 r_f3 = _mm256_cvtepi32_ps(r_32_3);

            __m256 y_f3, u_f3, v_f3;
            convert_rgb2yuv<8>(y_f3, u_f3, v_f3, r_f3, g_f3, b_f3, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);
            
            // 四捨五入して整数に変換
            __m256i y_i0 = _mm256_cvttps_epi32(_mm256_add_ps(y_f0, round_offset));
            __m256i u_i0 = _mm256_cvttps_epi32(_mm256_add_ps(u_f0, round_offset));
            __m256i v_i0 = _mm256_cvttps_epi32(_mm256_add_ps(v_f0, round_offset));
            __m256i y_i1 = _mm256_cvttps_epi32(_mm256_add_ps(y_f1, round_offset));
            __m256i u_i1 = _mm256_cvttps_epi32(_mm256_add_ps(u_f1, round_offset));
            __m256i v_i1 = _mm256_cvttps_epi32(_mm256_add_ps(v_f1, round_offset));
            __m256i y_i2 = _mm256_cvttps_epi32(_mm256_add_ps(y_f2, round_offset));
            __m256i u_i2 = _mm256_cvttps_epi32(_mm256_add_ps(u_f2, round_offset));
            __m256i v_i2 = _mm256_cvttps_epi32(_mm256_add_ps(v_f2, round_offset));
            __m256i y_i3 = _mm256_cvttps_epi32(_mm256_add_ps(y_f3, round_offset));
            __m256i u_i3 = _mm256_cvttps_epi32(_mm256_add_ps(u_f3, round_offset));
            __m256i v_i3 = _mm256_cvttps_epi32(_mm256_add_ps(v_f3, round_offset));

            // 32bit -> 16bit変換
            __m256i y_16_0 = _mm256_packus_epi32(y_i0, y_i1);  // 0-15
            __m256i y_16_1 = _mm256_packus_epi32(y_i2, y_i3);  // 16-31
            __m256i u_16_0 = _mm256_packus_epi32(u_i0, u_i1);
            __m256i u_16_1 = _mm256_packus_epi32(u_i2, u_i3);
            __m256i v_16_0 = _mm256_packus_epi32(v_i0, v_i1);
            __m256i v_16_1 = _mm256_packus_epi32(v_i2, v_i3);
            
            // 16bit -> 8bit変換
            __m256i y_8 = _mm256_packus_epi16(y_16_0, y_16_1);  // 32ピクセル
            __m256i u_8 = _mm256_packus_epi16(u_16_0, u_16_1);
            __m256i v_8 = _mm256_packus_epi16(v_16_0, v_16_1);
            
            // 結果を256ビットレジスタで格納（32ピクセル = 32バイト）
            _mm256_storeu_si256((__m256i*)(dstY + x), y_8);
            _mm256_storeu_si256((__m256i*)(dstU + x), u_8);
            _mm256_storeu_si256((__m256i*)(dstV + x), v_8);
        }
        
        // 残りのピクセルを従来の方法で処理
        for (; x < width; x++) {
            const float b = (float)src[x*3 + 0];
            const float g = (float)src[x*3 + 1];
            const float r = (float)src[x*3 + 2];
            const float y = (coeff_table[0] * r + coeff_table[1] * g + coeff_table[2] * b +  16.0f) * (1 << (out_bit_depth - 8));
            const float u = (coeff_table[3] * r + coeff_table[4] * g + coeff_table[5] * b + 128.0f) * (1 << (out_bit_depth - 8));
            const float v = (coeff_table[6] * r + coeff_table[7] * g + coeff_table[8] * b + 128.0f) * (1 << (out_bit_depth - 8));
            dstY[x] = clamp((BYTE)(y + 0.5f), 0, (1 << out_bit_depth) - 1);
            dstU[x] = clamp((BYTE)(u + 0.5f), 0, (1 << out_bit_depth) - 1);
            dstV[x] = clamp((BYTE)(v + 0.5f), 0, (1 << out_bit_depth) - 1);
        }
    }
    
    _mm256_zeroupper();
}

void convert_rgb_to_yuv444_16bit_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    const int out_bit_depth = 16;
    BYTE *ptrY = pixel_data->data[0];
    BYTE *ptrU = pixel_data->data[1];
    BYTE *ptrV = pixel_data->data[2];
    const float *coeff_table = COEFF_RGB2YUV[pixel_data->colormatrix ? 1 : 0];
    int y0 = 0, y1 = height - 1;
    const int srcstep = (width*3 + 3) & ~3;
    
    // AVX2用の係数をロード
    const __m256 coeff_ry = _mm256_set1_ps(coeff_table[0]);
    const __m256 coeff_gy = _mm256_set1_ps(coeff_table[1]);
    const __m256 coeff_by = _mm256_set1_ps(coeff_table[2]);
    const __m256 coeff_ru = _mm256_set1_ps(coeff_table[3]);
    const __m256 coeff_gu = _mm256_set1_ps(coeff_table[4]);
    const __m256 coeff_bu = _mm256_set1_ps(coeff_table[5]);
    const __m256 coeff_rv = _mm256_set1_ps(coeff_table[6]);
    const __m256 coeff_gv = _mm256_set1_ps(coeff_table[7]);
    const __m256 coeff_bv = _mm256_set1_ps(coeff_table[8]);
    
    const __m256 offset_y = _mm256_set1_ps(16.0f);
    const __m256 offset_uv = _mm256_set1_ps(128.0f);
    const __m256 round_offset = _mm256_set1_ps(0.5f);
    const __m256 limit_offset = _mm256_set1_ps((float)(1 << (out_bit_depth - 8)));
    
    for (; y0 < height; y0++, y1--) {
        USHORT *dstY = (USHORT *)(ptrY + y1*width*sizeof(USHORT));
        USHORT *dstU = (USHORT *)(ptrU + y1*width*sizeof(USHORT));
        USHORT *dstV = (USHORT *)(ptrV + y1*width*sizeof(USHORT));
        BYTE *src  = (BYTE*)frame + y0*srcstep;
        
        int x = 0;
        // AVX2で32ピクセルずつ処理
        for (; x <= width - 32; x += 32) {
            // 32ピクセル分のBGRデータをロード (96バイト)
            __m256i bgr0 = _mm256_loadu_si256((__m256i*)(src + x*3));      // 32バイト
            __m256i bgr1 = _mm256_loadu_si256((__m256i*)(src + x*3 + 32)); // 32バイト
            __m256i bgr2 = _mm256_loadu_si256((__m256i*)(src + x*3 + 64)); // 32バイト

            __m256i b_8, g_8, r_8;
            separate_8bit_packed(b_8, g_8, r_8, bgr0, bgr1, bgr2);

            __m256i b_16_0 = _mm256_unpacklo_epi8(b_8, _mm256_setzero_si256());
            __m256i b_16_1 = _mm256_unpackhi_epi8(b_8, _mm256_setzero_si256());
            __m256i g_16_0 = _mm256_unpacklo_epi8(g_8, _mm256_setzero_si256());
            __m256i g_16_1 = _mm256_unpackhi_epi8(g_8, _mm256_setzero_si256());
            __m256i r_16_0 = _mm256_unpacklo_epi8(r_8, _mm256_setzero_si256());
            __m256i r_16_1 = _mm256_unpackhi_epi8(r_8, _mm256_setzero_si256());

            __m256i b_32_0 = _mm256_unpacklo_epi16(b_16_0, _mm256_setzero_si256());
            __m256i b_32_1 = _mm256_unpackhi_epi16(b_16_0, _mm256_setzero_si256());
            __m256i b_32_2 = _mm256_unpacklo_epi16(b_16_1, _mm256_setzero_si256());
            __m256i b_32_3 = _mm256_unpackhi_epi16(b_16_1, _mm256_setzero_si256());

            __m256i g_32_0 = _mm256_unpacklo_epi16(g_16_0, _mm256_setzero_si256());
            __m256i g_32_1 = _mm256_unpackhi_epi16(g_16_0, _mm256_setzero_si256());
            __m256i g_32_2 = _mm256_unpacklo_epi16(g_16_1, _mm256_setzero_si256());
            __m256i g_32_3 = _mm256_unpackhi_epi16(g_16_1, _mm256_setzero_si256());
            
            __m256i r_32_0 = _mm256_unpacklo_epi16(r_16_0, _mm256_setzero_si256());
            __m256i r_32_1 = _mm256_unpackhi_epi16(r_16_0, _mm256_setzero_si256());
            __m256i r_32_2 = _mm256_unpacklo_epi16(r_16_1, _mm256_setzero_si256());
            __m256i r_32_3 = _mm256_unpackhi_epi16(r_16_1, _mm256_setzero_si256());
            
            // グループ1: ピクセル0-7 (下位128ビット)
            __m256 b_f0 = _mm256_cvtepi32_ps(b_32_0);
            __m256 g_f0 = _mm256_cvtepi32_ps(g_32_0);
            __m256 r_f0 = _mm256_cvtepi32_ps(r_32_0);

            __m256 y_f0, u_f0, v_f0;
            convert_rgb2yuv<8>(y_f0, u_f0, v_f0, r_f0, g_f0, b_f0, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);
            
            // グループ2: ピクセル8-15 (上位128ビット)
            __m256 b_f1 = _mm256_cvtepi32_ps(b_32_1);
            __m256 g_f1 = _mm256_cvtepi32_ps(g_32_1);
            __m256 r_f1 = _mm256_cvtepi32_ps(r_32_1);

            __m256 y_f1, u_f1, v_f1;
            convert_rgb2yuv<8>(y_f1, u_f1, v_f1, r_f1, g_f1, b_f1, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);
            
            // グループ3: ピクセル16-23（上位128ビットから
            __m256 b_f2 = _mm256_cvtepi32_ps(b_32_2);
            __m256 g_f2 = _mm256_cvtepi32_ps(g_32_2);
            __m256 r_f2 = _mm256_cvtepi32_ps(r_32_2);

            __m256 y_f2, u_f2, v_f2;
            convert_rgb2yuv<8>(y_f2, u_f2, v_f2, r_f2, g_f2, b_f2, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);
            
            // グループ4: ピクセル24-31
            __m256 b_f3 = _mm256_cvtepi32_ps(b_32_3);
            __m256 g_f3 = _mm256_cvtepi32_ps(g_32_3);
            __m256 r_f3 = _mm256_cvtepi32_ps(r_32_3);

            __m256 y_f3, u_f3, v_f3;
            convert_rgb2yuv<8>(y_f3, u_f3, v_f3, r_f3, g_f3, b_f3, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);
            
            // 四捨五入して整数に変換
            __m256i y_i0 = _mm256_cvttps_epi32(_mm256_fmadd_ps(y_f0, limit_offset, round_offset));
            __m256i u_i0 = _mm256_cvttps_epi32(_mm256_fmadd_ps(u_f0, limit_offset, round_offset));
            __m256i v_i0 = _mm256_cvttps_epi32(_mm256_fmadd_ps(v_f0, limit_offset, round_offset));
            __m256i y_i1 = _mm256_cvttps_epi32(_mm256_fmadd_ps(y_f1, limit_offset, round_offset));
            __m256i u_i1 = _mm256_cvttps_epi32(_mm256_fmadd_ps(u_f1, limit_offset, round_offset));
            __m256i v_i1 = _mm256_cvttps_epi32(_mm256_fmadd_ps(v_f1, limit_offset, round_offset));
            __m256i y_i2 = _mm256_cvttps_epi32(_mm256_fmadd_ps(y_f2, limit_offset, round_offset));
            __m256i u_i2 = _mm256_cvttps_epi32(_mm256_fmadd_ps(u_f2, limit_offset, round_offset));
            __m256i v_i2 = _mm256_cvttps_epi32(_mm256_fmadd_ps(v_f2, limit_offset, round_offset));
            __m256i y_i3 = _mm256_cvttps_epi32(_mm256_fmadd_ps(y_f3, limit_offset, round_offset));
            __m256i u_i3 = _mm256_cvttps_epi32(_mm256_fmadd_ps(u_f3, limit_offset, round_offset));
            __m256i v_i3 = _mm256_cvttps_epi32(_mm256_fmadd_ps(v_f3, limit_offset, round_offset));

            // 32bit -> 16bit変換
            __m256i y_16_0 = _mm256_packus_epi32(y_i0, y_i1);  // 0-15
            __m256i y_16_1 = _mm256_packus_epi32(y_i2, y_i3);  // 16-31
            __m256i u_16_0 = _mm256_packus_epi32(u_i0, u_i1);
            __m256i u_16_1 = _mm256_packus_epi32(u_i2, u_i3);
            __m256i v_16_0 = _mm256_packus_epi32(v_i0, v_i1);
            __m256i v_16_1 = _mm256_packus_epi32(v_i2, v_i3);
            
            // 結果を256ビットレジスタで格納（32ピクセル = 32バイト）
            _mm256_storeu_si256((__m256i*)(dstY + x +  0), _mm256_permute2x128_si256(y_16_0, y_16_1, (2 << 4) | 0));
            _mm256_storeu_si256((__m256i*)(dstU + x +  0), _mm256_permute2x128_si256(u_16_0, u_16_1, (2 << 4) | 0));
            _mm256_storeu_si256((__m256i*)(dstV + x +  0), _mm256_permute2x128_si256(v_16_0, v_16_1, (2 << 4) | 0));
            _mm256_storeu_si256((__m256i*)(dstY + x + 16), _mm256_permute2x128_si256(y_16_0, y_16_1, (3 << 4) | 1));
            _mm256_storeu_si256((__m256i*)(dstU + x + 16), _mm256_permute2x128_si256(u_16_0, u_16_1, (3 << 4) | 1));
            _mm256_storeu_si256((__m256i*)(dstV + x + 16), _mm256_permute2x128_si256(v_16_0, v_16_1, (3 << 4) | 1));
        }
        
        // 残りのピクセルを従来の方法で処理
        for (; x < width; x++) {
            const float b = (float)src[x*3 + 0];
            const float g = (float)src[x*3 + 1];
            const float r = (float)src[x*3 + 2];
            const float y = (coeff_table[0] * r + coeff_table[1] * g + coeff_table[2] * b +  16.0f) * (1 << (out_bit_depth - 8));
            const float u = (coeff_table[3] * r + coeff_table[4] * g + coeff_table[5] * b + 128.0f) * (1 << (out_bit_depth - 8));
            const float v = (coeff_table[6] * r + coeff_table[7] * g + coeff_table[8] * b + 128.0f) * (1 << (out_bit_depth - 8));
            dstY[x] = (BYTE)clamp((int)(y + 0.5f), 0, (1 << out_bit_depth) - 1);
            dstU[x] = (BYTE)clamp((int)(u + 0.5f), 0, (1 << out_bit_depth) - 1);
            dstV[x] = (BYTE)clamp((int)(v + 0.5f), 0, (1 << out_bit_depth) - 1);
        }
    }
    
    _mm256_zeroupper();
}

void convert_yuy2_to_yuv422_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    int x, y;
    BYTE *p, *Y, *U, *V;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_U = pixel_data->data[1];
    BYTE *dst_V = pixel_data->data[2];
    __m256i y0, y1, y3;
    for (y = 0; y < height; y++) {
        x  = y * width;
        p  = (BYTE *)frame + (x<<1);
        Y  = (BYTE *)dst_Y +  x;
        U  = (BYTE *)dst_U + (x>>1);
        V  = (BYTE *)dst_V + (x>>1);
        for (x = 0; x <= width-64; x += 64, p += 128) {
            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

            separate_low_up(y0, y1);
            y3 = y1;

            _mm256_storeu_si256((__m256i *)(Y + x), y0);

            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+ 96)), _mm_loadu_si128((__m128i*)(p+64)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+112)), _mm_loadu_si128((__m128i*)(p+80)));

            separate_low_up(y0, y1);

            _mm256_storeu_si256((__m256i *)(Y + x + 32), y0);

            y0 = _mm256_permute2x128_si256(y3, y1, (2 << 4) | 0);
            y1 = _mm256_permute2x128_si256(y3, y1, (3 << 4) | 1);
            separate_low_up(y0, y1);
            _mm256_storeu_si256((__m256i *)(U + (x>>1)), y0);
            _mm256_storeu_si256((__m256i *)(V + (x>>1)), y1);
        }
        for (; x < width; x += 2, p += 4) {
            Y[x +0] = p[0];
            U[x>>1] = p[1];
            Y[x +1] = p[2];
            V[x>>1] = p[3];
        }
    }
    _mm256_zeroupper();
}

void convert_yuy2_to_yuv422_16bit_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    const int out_depth = 16;
    int x, y;
    BYTE *p;
    USHORT *Y, *U, *V;
    BYTE *dst_Y = pixel_data->data[0];
    BYTE *dst_U = pixel_data->data[1];
    BYTE *dst_V = pixel_data->data[2];
    __m256i y0, y1, y3, y0_0, y0_1, y1_0, y1_1;
    for (y = 0; y < height; y++) {
        x  = y * width;
        p  = (BYTE *)frame + (x<<1);
        Y  = (USHORT *)dst_Y +  x;
        U  = (USHORT *)dst_U + (x>>1);
        V  = (USHORT *)dst_V + (x>>1);
        for (x = 0; x <= width-64; x += 64, p += 128) {
            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

            separate_low_up(y0, y1);
            y3 = y1;

            y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
            y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), out_depth - 8);
            y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), out_depth - 8);
            _mm256_storeu_si256((__m256i *)(Y + x +  0), y0_0);
            _mm256_storeu_si256((__m256i *)(Y + x + 16), y0_1);

            y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+ 96)), _mm_loadu_si128((__m128i*)(p+64)));
            y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+112)), _mm_loadu_si128((__m128i*)(p+80)));

            separate_low_up(y0, y1);

            y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
            y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), out_depth - 8);
            y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), out_depth - 8);
            _mm256_storeu_si256((__m256i *)(Y + x + 32), y0_0);
            _mm256_storeu_si256((__m256i *)(Y + x + 48), y0_1);

            y0 = _mm256_permute2x128_si256(y3, y1, (2 << 4) | 0);
            y1 = _mm256_permute2x128_si256(y3, y1, (3 << 4) | 1);
            separate_low_up(y0, y1);
            y0 = _mm256_permute4x64_epi64(y0, _MM_SHUFFLE(3,1,2,0));
            y0_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y0, _mm256_setzero_si256()), out_depth - 8);
            y0_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y0, _mm256_setzero_si256()), out_depth - 8);
            y1 = _mm256_permute4x64_epi64(y1, _MM_SHUFFLE(3,1,2,0));
            y1_0 = _mm256_slli_epi16(_mm256_unpacklo_epi8(y1, _mm256_setzero_si256()), out_depth - 8);
            y1_1 = _mm256_slli_epi16(_mm256_unpackhi_epi8(y1, _mm256_setzero_si256()), out_depth - 8);
            _mm256_storeu_si256((__m256i *)(U + (x>>1) +  0), y0_0);
            _mm256_storeu_si256((__m256i *)(U + (x>>1) + 16), y0_1);
            _mm256_storeu_si256((__m256i *)(V + (x>>1) +  0), y1_0);
            _mm256_storeu_si256((__m256i *)(V + (x>>1) + 16), y1_1);
        }
        for (; x < width; x += 2, p += 4) {
            Y[x +0] = p[0] << (out_depth - 8);
            U[x>>1] = p[1] << (out_depth - 8);
            Y[x +1] = p[2] << (out_depth - 8);
            V[x>>1] = p[3] << (out_depth - 8);
        }
    }
    _mm256_zeroupper();
}

// 22bit精度 rcp
__m256 _mm256_rcpnr_fma_ps(__m256 x) {
	__m256 rcp = _mm256_rcp_ps(x); // 11bit精度
	//rcp*(2-rcp*x)
	return _mm256_mul_ps(rcp, _mm256_fnmadd_ps(x, rcp, _mm256_set1_ps(2.0f)));
}

void unpremultiply_pa64_avx2(__m256& r, __m256& g, __m256& b, const __m256& a) {
    // 乗算付きalphaなrgbをunpremultiplyに変換
    __m256 a_inv = _mm256_mul_ps(_mm256_set1_ps(65535.0f), _mm256_rcpnr_fma_ps(a));
    r = _mm256_mul_ps(r, a_inv);
    g = _mm256_mul_ps(g, a_inv);
    b = _mm256_mul_ps(b, a_inv);
}

void convert_pa64_to_yuv444_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    const int out_bit_depth = 8;
    BYTE *ptrY = pixel_data->data[0];
    BYTE *ptrU = pixel_data->data[1];
    BYTE *ptrV = pixel_data->data[2];
    const float *coeff_table = COEFF_RGB2YUV[pixel_data->colormatrix ? 1 : 0];
    const int srcstep = width * 4;
    
    // AVX2用の係数をロード
    const __m256 coeff_ry = _mm256_set1_ps(coeff_table[0]);
    const __m256 coeff_gy = _mm256_set1_ps(coeff_table[1]);
    const __m256 coeff_by = _mm256_set1_ps(coeff_table[2]);
    const __m256 coeff_ru = _mm256_set1_ps(coeff_table[3]);
    const __m256 coeff_gu = _mm256_set1_ps(coeff_table[4]);
    const __m256 coeff_bu = _mm256_set1_ps(coeff_table[5]);
    const __m256 coeff_rv = _mm256_set1_ps(coeff_table[6]);
    const __m256 coeff_gv = _mm256_set1_ps(coeff_table[7]);
    const __m256 coeff_bv = _mm256_set1_ps(coeff_table[8]);
    
    const __m256 offset_y = _mm256_set1_ps(16.0f);
    const __m256 offset_uv = _mm256_set1_ps(128.0f);
    const __m256 round_offset = _mm256_set1_ps(0.5f);
    const __m256 limit_offset = _mm256_set1_ps(1.0f / (float)(1 << (16 - out_bit_depth)));
    
    for (int y = 0; y < height; y++) {
        BYTE *dstY = (BYTE *)(ptrY + y*width*sizeof(BYTE));
        BYTE *dstU = (BYTE *)(ptrU + y*width*sizeof(BYTE));
        BYTE *dstV = (BYTE *)(ptrV + y*width*sizeof(BYTE));
        USHORT *src  = (USHORT*)frame + y*srcstep;
        
        int x = 0;
        // AVX2で32ピクセルずつ処理
        for (; x <= width - 32; x += 32) {
            __m256i y0 = _mm256_loadu_si256((__m256i *)(src + x*4 +  0));
            __m256i y1 = _mm256_loadu_si256((__m256i *)(src + x*4 + 16));
            __m256i y2 = _mm256_loadu_si256((__m256i *)(src + x*4 + 32));
            __m256i y3 = _mm256_loadu_si256((__m256i *)(src + x*4 + 48));
            __m256i y4 = _mm256_loadu_si256((__m256i *)(src + x*4 + 64));
            __m256i y5 = _mm256_loadu_si256((__m256i *)(src + x*4 + 80));
            __m256i y6 = _mm256_loadu_si256((__m256i *)(src + x*4 + 96));
            __m256i y7 = _mm256_loadu_si256((__m256i *)(src + x*4 + 112));

            __m256i y01_0 = _mm256_permute2x128_si256(y0, y1, (2 << 4) | 0);
            __m256i y01_1 = _mm256_permute2x128_si256(y0, y1, (3 << 4) | 1);
            __m256i y23_0 = _mm256_permute2x128_si256(y2, y3, (2 << 4) | 0);
            __m256i y23_1 = _mm256_permute2x128_si256(y2, y3, (3 << 4) | 1);
            __m256i y45_0 = _mm256_permute2x128_si256(y4, y5, (2 << 4) | 0);
            __m256i y45_1 = _mm256_permute2x128_si256(y4, y5, (3 << 4) | 1);
            __m256i y67_0 = _mm256_permute2x128_si256(y6, y7, (2 << 4) | 0);
            __m256i y67_1 = _mm256_permute2x128_si256(y6, y7, (3 << 4) | 1);

            y0 = _mm256_packus_epi32(_mm256_and_si256(y01_0, _mm256_set1_epi32(0xFFFF)), _mm256_and_si256(y01_1, _mm256_set1_epi32(0xFFFF))); // 3b	3r	2b	2r	1b	1r	0b	0r
            y1 = _mm256_packus_epi32(_mm256_srli_epi32(y01_0, 16), _mm256_srli_epi32(y01_1, 16)); // 3a	3g	2a	2g	1a	1g	0a	0g
            y2 = _mm256_packus_epi32(_mm256_and_si256(y23_0, _mm256_set1_epi32(0xFFFF)), _mm256_and_si256(y23_1, _mm256_set1_epi32(0xFFFF)));
            y3 = _mm256_packus_epi32(_mm256_srli_epi32(y23_0, 16), _mm256_srli_epi32(y23_1, 16));
            y4 = _mm256_packus_epi32(_mm256_and_si256(y45_0, _mm256_set1_epi32(0xFFFF)), _mm256_and_si256(y45_1, _mm256_set1_epi32(0xFFFF))); // 3b	3r	2b	2r	1b	1r	0b	0r
            y5 = _mm256_packus_epi32(_mm256_srli_epi32(y45_0, 16), _mm256_srli_epi32(y45_1, 16)); // 3a	3g	2a	2g	1a	1g	0a	0g
            y6 = _mm256_packus_epi32(_mm256_and_si256(y67_0, _mm256_set1_epi32(0xFFFF)), _mm256_and_si256(y67_1, _mm256_set1_epi32(0xFFFF)));
            y7 = _mm256_packus_epi32(_mm256_srli_epi32(y67_0, 16), _mm256_srli_epi32(y67_1, 16));

            __m256i r_32_0 = _mm256_and_si256(y0, _mm256_set1_epi32(0xFFFF));
            __m256i g_32_0 = _mm256_srli_epi32(y0, 16);
            __m256i r_32_1 = _mm256_and_si256(y2, _mm256_set1_epi32(0xFFFF));
            __m256i g_32_1 = _mm256_srli_epi32(y2, 16);
            __m256i b_32_0 = _mm256_and_si256(y1, _mm256_set1_epi32(0xFFFF));
            __m256i a_32_0 = _mm256_srli_epi32(y1, 16);
            __m256i b_32_1 = _mm256_and_si256(y3, _mm256_set1_epi32(0xFFFF));
            __m256i a_32_1 = _mm256_srli_epi32(y3, 16);
            __m256i r_32_2 = _mm256_and_si256(y4, _mm256_set1_epi32(0xFFFF));
            __m256i g_32_2 = _mm256_srli_epi32(y4, 16);
            __m256i r_32_3 = _mm256_and_si256(y6, _mm256_set1_epi32(0xFFFF));
            __m256i g_32_3 = _mm256_srli_epi32(y6, 16);
            __m256i b_32_2 = _mm256_and_si256(y5, _mm256_set1_epi32(0xFFFF));
            __m256i a_32_2 = _mm256_srli_epi32(y5, 16);
            __m256i b_32_3 = _mm256_and_si256(y7, _mm256_set1_epi32(0xFFFF));
            __m256i a_32_3 = _mm256_srli_epi32(y7, 16);

            // グループ1: ピクセル0-7
            __m256 b_f0 = _mm256_cvtepi32_ps(b_32_0);
            __m256 g_f0 = _mm256_cvtepi32_ps(g_32_0);
            __m256 r_f0 = _mm256_cvtepi32_ps(r_32_0);
            __m256 a_f0 = _mm256_cvtepi32_ps(a_32_0);
            unpremultiply_pa64_avx2(r_f0, g_f0, b_f0, a_f0);

            __m256 y_f0, u_f0, v_f0;
            convert_rgb2yuv<16>(y_f0, u_f0, v_f0, r_f0, g_f0, b_f0, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);
            
            // グループ2: ピクセル8-15
            __m256 b_f1 = _mm256_cvtepi32_ps(b_32_1);
            __m256 g_f1 = _mm256_cvtepi32_ps(g_32_1);
            __m256 r_f1 = _mm256_cvtepi32_ps(r_32_1);
            __m256 a_f1 = _mm256_cvtepi32_ps(a_32_1);
            unpremultiply_pa64_avx2(r_f1, g_f1, b_f1, a_f1);

            __m256 y_f1, u_f1, v_f1;
            convert_rgb2yuv<16>(y_f1, u_f1, v_f1, r_f1, g_f1, b_f1, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);

            // グループ3: ピクセル16-23
            __m256 b_f2 = _mm256_cvtepi32_ps(b_32_2);
            __m256 g_f2 = _mm256_cvtepi32_ps(g_32_2);
            __m256 r_f2 = _mm256_cvtepi32_ps(r_32_2);
            __m256 a_f2 = _mm256_cvtepi32_ps(a_32_2);
            unpremultiply_pa64_avx2(r_f2, g_f2, b_f2, a_f2);

            
            __m256 y_f2, u_f2, v_f2;
            convert_rgb2yuv<16>(y_f2, u_f2, v_f2, r_f2, g_f2, b_f2, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);

            // グループ4: ピクセル24-31
            __m256 b_f3 = _mm256_cvtepi32_ps(b_32_3);
            __m256 g_f3 = _mm256_cvtepi32_ps(g_32_3);
            __m256 r_f3 = _mm256_cvtepi32_ps(r_32_3);
            __m256 a_f3 = _mm256_cvtepi32_ps(a_32_3);
            unpremultiply_pa64_avx2(r_f3, g_f3, b_f3, a_f3);

            __m256 y_f3, u_f3, v_f3;    
            convert_rgb2yuv<16>(y_f3, u_f3, v_f3, r_f3, g_f3, b_f3, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);

            // 四捨五入して整数に変換
            __m256i y_i0 = _mm256_cvttps_epi32(_mm256_fmadd_ps(y_f0, limit_offset, round_offset));
            __m256i u_i0 = _mm256_cvttps_epi32(_mm256_fmadd_ps(u_f0, limit_offset, round_offset));
            __m256i v_i0 = _mm256_cvttps_epi32(_mm256_fmadd_ps(v_f0, limit_offset, round_offset));
            __m256i y_i1 = _mm256_cvttps_epi32(_mm256_fmadd_ps(y_f1, limit_offset, round_offset));
            __m256i u_i1 = _mm256_cvttps_epi32(_mm256_fmadd_ps(u_f1, limit_offset, round_offset));
            __m256i v_i1 = _mm256_cvttps_epi32(_mm256_fmadd_ps(v_f1, limit_offset, round_offset));
            __m256i y_i2 = _mm256_cvttps_epi32(_mm256_fmadd_ps(y_f2, limit_offset, round_offset));
            __m256i u_i2 = _mm256_cvttps_epi32(_mm256_fmadd_ps(u_f2, limit_offset, round_offset));
            __m256i v_i2 = _mm256_cvttps_epi32(_mm256_fmadd_ps(v_f2, limit_offset, round_offset));
            __m256i y_i3 = _mm256_cvttps_epi32(_mm256_fmadd_ps(y_f3, limit_offset, round_offset));
            __m256i u_i3 = _mm256_cvttps_epi32(_mm256_fmadd_ps(u_f3, limit_offset, round_offset));
            __m256i v_i3 = _mm256_cvttps_epi32(_mm256_fmadd_ps(v_f3, limit_offset, round_offset));

            // 32bit -> 16bit変換
            __m256i y_16_0 = _mm256_packus_epi32(y_i0, y_i1);  // 0-15
            __m256i u_16_0 = _mm256_packus_epi32(u_i0, u_i1);
            __m256i v_16_0 = _mm256_packus_epi32(v_i0, v_i1);
            __m256i y_16_1 = _mm256_packus_epi32(y_i2, y_i3);  // 16-31
            __m256i u_16_1 = _mm256_packus_epi32(u_i2, u_i3);
            __m256i v_16_1 = _mm256_packus_epi32(v_i2, v_i3);
            
            // 16bit -> 8bit変換
            __m256i y_8 = _mm256_packus_epi16(y_16_0, y_16_1);  // 32ピクセル
            __m256i u_8 = _mm256_packus_epi16(u_16_0, u_16_1);
            __m256i v_8 = _mm256_packus_epi16(v_16_0, v_16_1);
            
            // 結果を256ビットレジスタで格納（32ピクセル = 32バイト）
            _mm256_storeu_si256((__m256i*)(dstY + x), y_8);
            _mm256_storeu_si256((__m256i*)(dstU + x), u_8);
            _mm256_storeu_si256((__m256i*)(dstV + x), v_8);
        }
        
        // 残りのピクセルを従来の方法で処理
        for (; x < width; x++) {
            float b = (float)src[x*4 + 0];
            float g = (float)src[x*4 + 1];
            float r = (float)src[x*4 + 2];
            float a = (float)src[x*4 + 3];
            float a_inv = 65535.0f / a;
            b *= a_inv, g *= a_inv, r *= a_inv;
            const float py = (coeff_table[0] * r + coeff_table[1] * g + coeff_table[2] * b +  16.0f) * (1 << (out_bit_depth - 8));
            const float pu = (coeff_table[3] * r + coeff_table[4] * g + coeff_table[5] * b + 128.0f) * (1 << (out_bit_depth - 8));
            const float pv = (coeff_table[6] * r + coeff_table[7] * g + coeff_table[8] * b + 128.0f) * (1 << (out_bit_depth - 8));
            dstY[x] = (BYTE)clamp((int)(py + 0.5f), 0, (1 << out_bit_depth) - 1);
            dstU[x] = (BYTE)clamp((int)(pu + 0.5f), 0, (1 << out_bit_depth) - 1);
            dstV[x] = (BYTE)clamp((int)(pv + 0.5f), 0, (1 << out_bit_depth) - 1);
        }
    }
}

void convert_pa64_to_yuv444_16bit_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    const int out_bit_depth = 16;
    USHORT *ptrY = (USHORT *)pixel_data->data[0];
    USHORT *ptrU = (USHORT *)pixel_data->data[1];
    USHORT *ptrV = (USHORT *)pixel_data->data[2];
    const float *coeff_table = COEFF_RGB2YUV[pixel_data->colormatrix ? 1 : 0];
    const int srcstep = width * 4;
    
    // AVX2用の係数をロード
    const __m256 coeff_ry = _mm256_set1_ps(coeff_table[0]);
    const __m256 coeff_gy = _mm256_set1_ps(coeff_table[1]);
    const __m256 coeff_by = _mm256_set1_ps(coeff_table[2]);
    const __m256 coeff_ru = _mm256_set1_ps(coeff_table[3]);
    const __m256 coeff_gu = _mm256_set1_ps(coeff_table[4]);
    const __m256 coeff_bu = _mm256_set1_ps(coeff_table[5]);
    const __m256 coeff_rv = _mm256_set1_ps(coeff_table[6]);
    const __m256 coeff_gv = _mm256_set1_ps(coeff_table[7]);
    const __m256 coeff_bv = _mm256_set1_ps(coeff_table[8]);
    
    const __m256 offset_y = _mm256_set1_ps(16.0f);
    const __m256 offset_uv = _mm256_set1_ps(128.0f);
    const __m256 round_offset = _mm256_set1_ps(0.5f);
    
    for (int y = 0; y < height; y++) {
        USHORT *dstY = (USHORT *)(ptrY + y*width);
        USHORT *dstU = (USHORT *)(ptrU + y*width);
        USHORT *dstV = (USHORT *)(ptrV + y*width);
        USHORT *src  = (USHORT*)frame + y*srcstep;
        
        int x = 0;
        // AVX2で32ピクセルずつ処理
        for (; x <= width - 16; x += 16) {
            __m256i y0 = _mm256_loadu_si256((__m256i *)(src + x*4 +  0));
            __m256i y1 = _mm256_loadu_si256((__m256i *)(src + x*4 + 16));
            __m256i y2 = _mm256_loadu_si256((__m256i *)(src + x*4 + 32));
            __m256i y3 = _mm256_loadu_si256((__m256i *)(src + x*4 + 48));
            
            __m256i y01_0 = _mm256_permute2x128_si256(y0, y1, (2 << 4) | 0);
            __m256i y01_1 = _mm256_permute2x128_si256(y0, y1, (3 << 4) | 1);
            __m256i y23_0 = _mm256_permute2x128_si256(y2, y3, (2 << 4) | 0);
            __m256i y23_1 = _mm256_permute2x128_si256(y2, y3, (3 << 4) | 1);

            y0 = _mm256_packus_epi32(_mm256_and_si256(y01_0, _mm256_set1_epi32(0xFFFF)), _mm256_and_si256(y01_1, _mm256_set1_epi32(0xFFFF))); // 3b	3r	2b	2r	1b	1r	0b	0r
            y1 = _mm256_packus_epi32(_mm256_srli_epi32(y01_0, 16), _mm256_srli_epi32(y01_1, 16)); // 3a	3g	2a	2g	1a	1g	0a	0g
            y2 = _mm256_packus_epi32(_mm256_and_si256(y23_0, _mm256_set1_epi32(0xFFFF)), _mm256_and_si256(y23_1, _mm256_set1_epi32(0xFFFF)));
            y3 = _mm256_packus_epi32(_mm256_srli_epi32(y23_0, 16), _mm256_srli_epi32(y23_1, 16));
            
            __m256i r_32_0 = _mm256_and_si256(y0, _mm256_set1_epi32(0xFFFF));
            __m256i b_32_0 = _mm256_srli_epi32(y0, 16);
            __m256i g_32_0 = _mm256_and_si256(y1, _mm256_set1_epi32(0xFFFF));
            __m256i a_32_0 = _mm256_srli_epi32(y1, 16);
            __m256i r_32_1 = _mm256_and_si256(y2, _mm256_set1_epi32(0xFFFF));
            __m256i b_32_1 = _mm256_srli_epi32(y2, 16);
            __m256i g_32_1 = _mm256_and_si256(y3, _mm256_set1_epi32(0xFFFF));
            __m256i a_32_1 = _mm256_srli_epi32(y3, 16);
            
            // グループ1: ピクセル0-7
            __m256 b_f0 = _mm256_cvtepi32_ps(b_32_0);
            __m256 g_f0 = _mm256_cvtepi32_ps(g_32_0);
            __m256 r_f0 = _mm256_cvtepi32_ps(r_32_0);
            __m256 a_f0 = _mm256_cvtepi32_ps(a_32_0);
            unpremultiply_pa64_avx2(r_f0, g_f0, b_f0, a_f0);

            __m256 y_f0, u_f0, v_f0;
            convert_rgb2yuv<16>(y_f0, u_f0, v_f0, r_f0, g_f0, b_f0, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);
            
            // グループ2: ピクセル8-15
            __m256 b_f1 = _mm256_cvtepi32_ps(b_32_1);
            __m256 g_f1 = _mm256_cvtepi32_ps(g_32_1);
            __m256 r_f1 = _mm256_cvtepi32_ps(r_32_1);
            __m256 a_f1 = _mm256_cvtepi32_ps(a_32_1);
            unpremultiply_pa64_avx2(r_f1, g_f1, b_f1, a_f1);

            __m256 y_f1, u_f1, v_f1;
            convert_rgb2yuv<16>(y_f1, u_f1, v_f1, r_f1, g_f1, b_f1, coeff_ry, coeff_gy, coeff_by, coeff_ru, coeff_gu, coeff_bu, coeff_rv, coeff_gv, coeff_bv);
            
            // 四捨五入して整数に変換
            __m256i y_i0 = _mm256_cvttps_epi32(_mm256_add_ps(y_f0, round_offset));
            __m256i u_i0 = _mm256_cvttps_epi32(_mm256_add_ps(u_f0, round_offset));
            __m256i v_i0 = _mm256_cvttps_epi32(_mm256_add_ps(v_f0, round_offset));
            __m256i y_i1 = _mm256_cvttps_epi32(_mm256_add_ps(y_f1, round_offset));
            __m256i u_i1 = _mm256_cvttps_epi32(_mm256_add_ps(u_f1, round_offset));
            __m256i v_i1 = _mm256_cvttps_epi32(_mm256_add_ps(v_f1, round_offset));

            // 32bit -> 16bit変換
            __m256i y_16_0 = _mm256_packus_epi32(y_i0, y_i1);  // 0-15
            __m256i u_16_0 = _mm256_packus_epi32(u_i0, u_i1);
            __m256i v_16_0 = _mm256_packus_epi32(v_i0, v_i1);
            
            // 結果を256ビットレジスタで格納（32ピクセル = 32バイト）
            _mm256_storeu_si256((__m256i*)(dstY + x +  0), _mm256_permute4x64_epi64(y_16_0, _MM_SHUFFLE(3,1,2,0)));
            _mm256_storeu_si256((__m256i*)(dstU + x +  0), _mm256_permute4x64_epi64(u_16_0, _MM_SHUFFLE(3,1,2,0)));
            _mm256_storeu_si256((__m256i*)(dstV + x +  0), _mm256_permute4x64_epi64(v_16_0, _MM_SHUFFLE(3,1,2,0)));
        }
        
        // 残りのピクセルを従来の方法で処理
        for (; x < width; x++) {
            float b = (float)src[x*4 + 0];
            float g = (float)src[x*4 + 1];
            float r = (float)src[x*4 + 2];
            float a = (float)src[x*4 + 3];
            float a_inv = 65535.0f / a;
            b *= a_inv, g *= a_inv, r *= a_inv;
            const float py = (coeff_table[0] * r + coeff_table[1] * g + coeff_table[2] * b +  16.0f) * (1 << (out_bit_depth - 8));
            const float pu = (coeff_table[3] * r + coeff_table[4] * g + coeff_table[5] * b + 128.0f) * (1 << (out_bit_depth - 8));
            const float pv = (coeff_table[6] * r + coeff_table[7] * g + coeff_table[8] * b + 128.0f) * (1 << (out_bit_depth - 8));
            dstY[x] = (USHORT)clamp((int)(py + 0.5f), 0, (1 << out_bit_depth) - 1);
            dstU[x] = (USHORT)clamp((int)(pu + 0.5f), 0, (1 << out_bit_depth) - 1);
            dstV[x] = (USHORT)clamp((int)(pv + 0.5f), 0, (1 << out_bit_depth) - 1);
        }
    }
}

void convert_pa64_to_rgba_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    const int out_bit_depth = 8;
    BYTE *ptr = pixel_data->data[0];
    const int srcstep = width * 4;
    const float mul = 1.0f /(float)(1 << (16 - out_bit_depth));
    const __m256 round_offset = _mm256_set1_ps(0.5f);
    const __m256 mul256 = _mm256_set1_ps(mul);
    
    for (int y = 0; y < height; y++) {
        BYTE *dst = (BYTE *)(ptr + y*width*4);
        USHORT *src  = (USHORT*)frame + y*srcstep;
        
        int x = 0;
        // AVX2で32ピクセルずつ処理
        for (; x <= width - 32; x += 32) {
            __m256i y0 = _mm256_loadu_si256((__m256i *)(src + x*4 +  0));
            __m256i y1 = _mm256_loadu_si256((__m256i *)(src + x*4 + 16));
            
            __m256i y01_0 = _mm256_permute2x128_si256(y0, y1, (2 << 4) | 0);
            __m256i y01_1 = _mm256_permute2x128_si256(y0, y1, (3 << 4) | 1);

            y0 = _mm256_packus_epi32(_mm256_and_si256(y01_0, _mm256_set1_epi32(0xFFFF)), _mm256_and_si256(y01_1, _mm256_set1_epi32(0xFFFF))); // 3b	3r	2b	2r	1b	1r	0b	0r
            y1 = _mm256_packus_epi32(_mm256_srli_epi32(y01_0, 16), _mm256_srli_epi32(y01_1, 16)); // 3a	3g	2a	2g	1a	1g	0a	0g
            
            __m256i r_32_0 = _mm256_and_si256(y0, _mm256_set1_epi32(0xFFFF));
            __m256i b_32_0 = _mm256_srli_epi32(y0, 16);
            __m256i g_32_0 = _mm256_and_si256(y1, _mm256_set1_epi32(0xFFFF));
            __m256i a_32_0 = _mm256_srli_epi32(y1, 16);
            
            // グループ1: ピクセル0-7
            __m256 b_f0 = _mm256_cvtepi32_ps(b_32_0);
            __m256 g_f0 = _mm256_cvtepi32_ps(g_32_0);
            __m256 r_f0 = _mm256_cvtepi32_ps(r_32_0);
            __m256 a_f0 = _mm256_cvtepi32_ps(a_32_0);
            unpremultiply_pa64_avx2(r_f0, g_f0, b_f0, a_f0);

            // 四捨五入して整数に変換
            __m256i r_i0 = _mm256_cvttps_epi32(_mm256_fmadd_ps(r_f0, mul256, round_offset));
            __m256i g_i0 = _mm256_cvttps_epi32(_mm256_fmadd_ps(g_f0, mul256, round_offset));
            __m256i b_i0 = _mm256_cvttps_epi32(_mm256_fmadd_ps(b_f0, mul256, round_offset));
            __m256i a_i0 = _mm256_cvttps_epi32(_mm256_fmadd_ps(a_f0, mul256, round_offset));

            // 32bit -> 16bit変換
            y0 = _mm256_packus_epi32(r_i0, g_i0);  // 0-15
            y1 = _mm256_packus_epi32(b_i0, a_i0);
            
            // 16bit -> 8bit変換
            __m256i y2 = _mm256_packus_epi16(y0, y1);  // 32ピクセル

            y0 = _mm256_packus_epi16(_mm256_and_si256(y0, _mm256_set1_epi16(0xFF)), _mm256_setzero_si256());
            y1 = _mm256_packus_epi16(_mm256_srli_epi16(y0, 8), _mm256_setzero_si256());
            y0 = _mm256_or_si256(y0, _mm256_shuffle_epi32(y1, _MM_SHUFFLE(1,0,3,2)));
            
            // 結果を256ビットレジスタで格納（32ピクセル = 32バイト）
            _mm256_storeu_si256((__m256i*)(dst + x), y0);
        }
        
        // 残りのピクセルを従来の方法で処理
        for (; x < width; x++) {
            float b = (float)src[x*4 + 0];
            float g = (float)src[x*4 + 1];
            float r = (float)src[x*4 + 2];
            float a = (float)src[x*4 + 3];
            float a_inv = 65535.0f * mul / a;
            b *= a_inv, g *= a_inv, r *= a_inv, a *= mul;
            dst[x*4 + 0] = (BYTE)clamp((int)(r + 0.5f), 0, (1 << out_bit_depth) - 1);
            dst[x*4 + 1] = (BYTE)clamp((int)(g + 0.5f), 0, (1 << out_bit_depth) - 1);
            dst[x*4 + 2] = (BYTE)clamp((int)(b + 0.5f), 0, (1 << out_bit_depth) - 1);
            dst[x*4 + 3] = (BYTE)clamp((int)(a + 0.5f), 0, (1 << out_bit_depth) - 1);
        }
    }
}

void convert_pa64_to_rgba_16bit_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
    const int out_bit_depth = 16;
    USHORT *ptr = (USHORT *)pixel_data->data[0];
    const int srcstep = width * 4;
    const __m256 round_offset = _mm256_set1_ps(0.5f);
    
    for (int y = 0; y < height; y++) {
        USHORT *dst = (USHORT *)(ptr + y*width*4);
        USHORT *src  = (USHORT*)frame + y*srcstep;
        
        int x = 0;
        // AVX2で32ピクセルずつ処理
        for (; x <= width - 32; x += 32) {
            __m256i y0 = _mm256_loadu_si256((__m256i *)(src + x*4 +  0));
            __m256i y1 = _mm256_loadu_si256((__m256i *)(src + x*4 + 16));
            
            __m256i y01_0 = _mm256_permute2x128_si256(y0, y1, (2 << 4) | 0);
            __m256i y01_1 = _mm256_permute2x128_si256(y0, y1, (3 << 4) | 1);

            y0 = _mm256_packus_epi32(_mm256_and_si256(y01_0, _mm256_set1_epi32(0xFFFF)), _mm256_and_si256(y01_1, _mm256_set1_epi32(0xFFFF))); // 3b	3r	2b	2r	1b	1r	0b	0r
            y1 = _mm256_packus_epi32(_mm256_srli_epi32(y01_0, 16), _mm256_srli_epi32(y01_1, 16)); // 3a	3g	2a	2g	1a	1g	0a	0g
            
            __m256i r_32_0 = _mm256_and_si256(y0, _mm256_set1_epi32(0xFFFF));
            __m256i b_32_0 = _mm256_srli_epi32(y0, 16);
            __m256i g_32_0 = _mm256_and_si256(y1, _mm256_set1_epi32(0xFFFF));
            __m256i a_32_0 = _mm256_srli_epi32(y1, 16);
            
            // グループ1: ピクセル0-7
            __m256 b_f0 = _mm256_cvtepi32_ps(b_32_0);
            __m256 g_f0 = _mm256_cvtepi32_ps(g_32_0);
            __m256 r_f0 = _mm256_cvtepi32_ps(r_32_0);
            __m256 a_f0 = _mm256_cvtepi32_ps(a_32_0);
            unpremultiply_pa64_avx2(r_f0, g_f0, b_f0, a_f0);

            // 四捨五入して整数に変換
            __m256i r_i0 = _mm256_cvttps_epi32(_mm256_add_ps(r_f0, round_offset));
            __m256i g_i0 = _mm256_cvttps_epi32(_mm256_add_ps(g_f0, round_offset));
            __m256i b_i0 = _mm256_cvttps_epi32(_mm256_add_ps(b_f0, round_offset));
            __m256i a_i0 = _mm256_cvttps_epi32(_mm256_add_ps(a_f0, round_offset));

            // 32bit -> 16bit変換
            y0 = _mm256_packus_epi32(r_i0, g_i0);  // 0-15
            y1 = _mm256_packus_epi32(b_i0, a_i0);

            __m256i y0_0 = _mm256_packus_epi32(_mm256_and_si256(y0, _mm256_set1_epi32(0xFFFF)), _mm256_setzero_si256());
            __m256i y0_1 = _mm256_packus_epi16(_mm256_srli_epi32(y0, 16), _mm256_setzero_si256());
            __m256i y1_0 = _mm256_packus_epi32(_mm256_and_si256(y1, _mm256_set1_epi32(0xFFFF)), _mm256_setzero_si256());
            __m256i y1_1 = _mm256_packus_epi16(_mm256_srli_epi32(y1, 16), _mm256_setzero_si256());

            y0 = _mm256_or_si256(y0_0, _mm256_shuffle_epi32(y0_1, _MM_SHUFFLE(1,0,3,2)));
            y1 = _mm256_or_si256(y1_0, _mm256_shuffle_epi32(y1_1, _MM_SHUFFLE(1,0,3,2)));
            
            // 結果を256ビットレジスタで格納（32ピクセル = 32バイト）
            _mm256_storeu_si256((__m256i*)(dst + x +  0), y0);
            _mm256_storeu_si256((__m256i*)(dst + x + 16), y1);
        }
        
        // 残りのピクセルを従来の方法で処理
        for (; x < width; x++) {
            float b = (float)src[x*4 + 0];
            float g = (float)src[x*4 + 1];
            float r = (float)src[x*4 + 2];
            float a = (float)src[x*4 + 3];
            float a_inv = 65535.0f / a;
            b *= a_inv, g *= a_inv, r *= a_inv;
            dst[x*4 + 0] = (USHORT)clamp((int)(r + 0.5f), 0, (1 << out_bit_depth) - 1);
            dst[x*4 + 1] = (USHORT)clamp((int)(g + 0.5f), 0, (1 << out_bit_depth) - 1);
            dst[x*4 + 2] = (USHORT)clamp((int)(b + 0.5f), 0, (1 << out_bit_depth) - 1);
            dst[x*4 + 3] = (USHORT)clamp((int)(a + 0.5f), 0, (1 << out_bit_depth) - 1);
        }
    }
}
