//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <mmintrin.h>  //イントリンシック命令 SSE
#include <emmintrin.h> //イントリンシック命令 SSE2
#include <tmmintrin.h> //イントリンシック命令 SSSE3
#include <smmintrin.h> //イントリンシック命令 SSE4.1

#include "convert.h"
#include "auo_options.h"
#include "auo_util.h"

///---------------------------
///
///    定数の名前
///      Y: 輝度,       UV: 色差
///      L: 圧縮レンジ, F:  フルレンジ
///      8: 8bit,       10: 10bit
///      MUL: 積        ADD: 加算
///      MA : 積和
///      RSH: 右シフト
///      YCC: 圧縮レンジのゲタ
///      444: YUV444時の計算に使用(UVで平均を取らない)
///
///-------------------------

///
///   計算式
///   ※clamp(x, low, high) = (((x) <= (high)) ? (((x) >= (low)) ? (x) : (low)) : (high))
///
///   YC48(y,cb,cr) -> YUV(8bit)
///
///           Y   = clamp((y * 219 + 383)>>12) + 16, 0, 255)
///   (YUV444)U,V = clamp(((cb,cr + 2048) * 1793 + 0)>>16) + 16, 0, 255)
///   (YUV420)U,V = clamp(((cb0,cr0 + cb1,cr1 + 4096) * 1793 + 0)>>15) + 16, 0, 255)
///
///   YC48 -> YUV(10bit) YC圧縮
///
///           Y   = clamp(((y * 219 + 383)>>10) + 64, 0, 1023)
///   (YUV444)U,V = clamp(((cb,cr + 2048) * 1793 +  0)>>14) + 64, 0, 1023)
///   (YUV420)U,V = clamp(((cb0,cr0 + cb1,cr1 + 4096) * 1793 + 66*2)>>13) + 64, 0, 1023)
///
///   YC48 -> YUV(10bit) fullrange
///
///           Y   = clamp(((y * 3517 + 3867)>>14) + 64, 0, 1023)
///   (YUV444)U,V = clamp(((cb,cr + 2048) * 899 +  1991)>>13) + 64, 0, 1023)
///   (YUV420)U,V = clamp(((cb0,cr0 + cb1,cr1 + 4096) * 899 + 1991*2)>>12) + 64, 0, 1023)
///

static const int UV_OFFSET     = (1<<12);
static const int UV_OFFSET_444 = (1<<11);

static const int RSFT_ONE      = 15;
static const int RSFT_LIMIT_10 = 6;
static const int LSFT_YCC_10   = 6;

//各ビットの最大値
static const int LIMIT_8    = (1<< 8) - 1;
static const int LIMIT_10   = (1<<10) - 1;

//YC圧縮レンジ用定数
static const int Y_L_MUL    = 219;
static const int Y_L_ADD    = 383;
static const int Y_L_RSH_8  = 12;
static const int Y_L_RSH_10 = 10;
static const int Y_L_YCC_8  = 16;
static const int Y_L_YCC_10 = 64;

static const int UV_L_MUL        = 1793;
static const int UV_L_ADD_444    = 0;
static const int UV_L_ADD        = (UV_L_ADD_444<<1);
static const int UV_L_RSH_8      = 16;
static const int UV_L_RSH_10     = 14;
static const int UV_L_RSH_8_444  = 15;
static const int UV_L_RSH_10_444 = 13;
static const int UV_L_YCC_8      = 16;
static const int UV_L_YCC_10     = 64;

//フルレンジ用定数(10bitのみ)
static const int Y_F_MUL         = 3517;
static const int Y_F_ADD         = 3867;
static const int Y_F_RSH_10      = 14;
static const int Y_F_YCC_10      = 64;
static const int UV_F_MUL        = 899;
static const int UV_F_ADD_444    = 2048;
static const int UV_F_ADD        = (UV_F_ADD_444<<1);
static const int UV_F_RSH_10     = 13;
static const int UV_F_RSH_10_444 = 12;
static const int UV_F_YCC_10     = 64;

static const __m128i xC_ZERO          = _mm_setzero_si128();
static const __m128i xC_ONE           = _mm_set1_epi16(1);
static const __m128i xC_YCC_8         = _mm_set1_epi16(Y_L_YCC_8);
static const __m128i xC_YCC_10        = _mm_set1_epi16(Y_L_YCC_10); 
static const __m128i xC_UV_OFFSET     = _mm_set1_epi16(UV_OFFSET);
static const __m128i xC_UV_OFFSET_444 = _mm_set1_epi16(UV_OFFSET_444);
static const __m128i xC_Y_L_MA        = _mm_set_epi16( Y_L_ADD,      Y_L_MUL,  Y_L_ADD,      Y_L_MUL,  Y_L_ADD,      Y_L_MUL,  Y_L_ADD,      Y_L_MUL);
static const __m128i xC_Y_F_MA        = _mm_set_epi16( Y_F_ADD,      Y_F_MUL,  Y_F_ADD,      Y_F_MUL,  Y_F_ADD,      Y_F_MUL,  Y_F_ADD,      Y_F_MUL);
static const __m128i xC_UV_L_MA       = _mm_set_epi16(UV_L_ADD,     UV_L_MUL, UV_L_ADD,     UV_L_MUL, UV_L_ADD,     UV_L_MUL, UV_L_ADD,     UV_L_MUL);
static const __m128i xC_UV_F_MA       = _mm_set_epi16(UV_F_ADD,     UV_F_MUL, UV_F_ADD,     UV_F_MUL, UV_F_ADD,     UV_F_MUL, UV_F_ADD,     UV_F_MUL);
static const __m128i xC_UV_L_MA_444   = _mm_set_epi16(UV_L_ADD_444, UV_L_MUL, UV_L_ADD_444, UV_L_MUL, UV_L_ADD_444, UV_L_MUL, UV_L_ADD_444, UV_L_MUL);
static const __m128i xC_UV_F_MA_444   = _mm_set_epi16(UV_F_ADD_444, UV_F_MUL, UV_F_ADD_444, UV_F_MUL, UV_F_ADD_444, UV_F_MUL, UV_F_ADD_444, UV_F_MUL);
static const __m128i xC_LIMITMAX      = _mm_set1_epi16(LIMIT_10);

static const __m128i MASK_YCP2Y    = _mm_set_epi16(-1, 0, 0, -1, 0, 0, -1, 0);
static const __m128i MASK_YCP2UV   = _mm_set_epi16(0, -1, -1, 0, 0, 0, 0, -1);
static const __m128i SUFFLE_YCP_Y  = _mm_set_epi8(11, 10,  5,  4, 15, 14,  9,  8,  3,  2, 13, 12,  7,  6,  1 ,  0);
static const __m128i SUFFLE_YCP_UV = _mm_set_epi8( 9,  8,  7,  6, 13, 12, 11, 10,  1,  0, 15, 14,  5,  4,  3 ,  2);

//SSE4.1の_mm_blendv_epi8(__m128i a, __m128i b, __m128i mask) のSSE2版のようなもの
static inline __m128i select_by_mask(__m128i a, __m128i b, __m128i mask) {
	return _mm_or_si128( _mm_andnot_si128(mask,a), _mm_and_si128(b,mask) );
}

func_audio_16to8 get_audio_16to8_func() {
	return (check_sse2()) ? convert_audio_16to8_sse2 : convert_audio_16to8;
}

//直前の16byteアライメント
static inline void * get_aligned_next(void *p) {
	return (void *)(((size_t)p + 15) & ~15);
}
//直後の16byteアライメント
static inline void * get_aligned_prev(void *p) {
	return (void *)(((size_t)p) & ~15);
}
//16bit音声 -> 8bit音声
void convert_audio_16to8(BYTE *dst, short *src, int n) {
	BYTE *byte = dst;
	const BYTE *fin = byte + n;
	short *sh = src;
	while (byte < fin) {
		*byte = (*sh >> 8) + 128;
		byte++;
		sh++;
	}
}
//上のSSE2版
void convert_audio_16to8_sse2(BYTE *dst, short *src, int n) {
	BYTE *byte = dst;
	short *sh = src;
	BYTE * const loop_start = (BYTE *)get_aligned_next(dst);
	BYTE * const loop_fin   = (BYTE *)get_aligned_prev(dst + n);
	BYTE * const fin = dst + n;
	__m128i xSA, xSB;
	static const __m128i xConst = _mm_set1_epi16(128);
	//アライメント調整
	while (byte < loop_start) {
		*byte = (*sh >> 8) + 128;
		byte++;
		sh++;
	}
	//メインループ
	while (byte < loop_fin) {
		xSA = _mm_loadu_si128((const __m128i *)sh);
		sh += 8;
		xSA = _mm_srai_epi16(xSA, 8);
		xSA = _mm_add_epi16(xSA, xConst);
		xSB = _mm_loadu_si128((const __m128i *)sh);
		sh += 8;
		xSB = _mm_srai_epi16(xSB, 8);
		xSB = _mm_add_epi16(xSB, xConst);
		xSA = _mm_packus_epi16(xSA, xSB);
		_mm_stream_si128((__m128i *)byte, xSA);
		byte += 16;
	}
	//残り
	while (byte < fin) {
		*byte = (*sh >> 8) + 128;
		byte++;
		sh++;
	}
}
//使用する関数を選択する
static func_convert_frame get_convert_func_yuv422(BOOL use10bit, BOOL fullrange) {
	if (!use10bit) {
		return     (check_sse2())     ? convert_yuy2_to_nv16_sse2              : convert_yuy2_to_nv16;
	} else {
		if (!fullrange) {
			if      (check_sse4_1())
					return              convert_yc48_to_nv16_10bit_sse4_1;
				else if (check_ssse3())
					return              convert_yc48_to_nv16_10bit_ssse3;
				else if (check_sse2())
					return              convert_yc48_to_nv16_10bit_sse2;
				else
					return              convert_yc48_to_nv16_10bit;
		} else {
			if      (check_sse4_1())
					return              convert_yc48_to_nv16_10bit_full_sse4_1;
				else if (check_ssse3())
					return              convert_yc48_to_nv16_10bit_full_ssse3;
				else if (check_sse2())
					return              convert_yc48_to_nv16_10bit_full_sse2;
				else
					return              convert_yc48_to_nv16_10bit_full;
		}
	}
}

static func_convert_frame get_convert_func_yuv444(BOOL use10bit, BOOL fullrange) {
	if (!use10bit) {
		return         (check_sse2()) ? convert_yc48_to_yuv444_sse2            : convert_yc48_to_yuv444;
	} else {
		if (!fullrange)
			return     (check_sse2()) ? convert_yc48_to_yuv444_10bit_sse2      : convert_yc48_to_yuv444_10bit;
		else
			return     (check_sse2()) ? convert_yc48_to_yuv444_10bit_full_sse2 : convert_yc48_to_yuv444_10bit_full;
	}
}

static func_convert_frame get_convert_func_rgb() {
	if      (check_ssse3())
		return sort_to_rgb_ssse3;
	else if (check_sse2())
		return sort_to_rgb_sse2;
	else
		return sort_to_rgb;
}

static func_convert_frame get_convert_func_yuv420(int width, BOOL use10bit, BOOL interlaced, BOOL fullrange) {
	if (!use10bit) {
		if (!interlaced)
			return     (check_sse2()) ? ((width%16==0) ? convert_yuy2_to_nv12_sse2_mod16              : convert_yuy2_to_nv12_sse2)              : convert_yuy2_to_nv12;
		else
			return     (check_sse2()) ? ((width%16==0) ? convert_yuy2_to_nv12_i_sse2_mod16            : convert_yuy2_to_nv12_i_sse2)            : convert_yuy2_to_nv12_i;
	} else {
		if (!interlaced) {
			if (!fullrange) {
				if      (check_sse4_1())
					return               (width%8==0)  ? convert_yc48_to_nv12_10bit_sse4_1_mod8       : convert_yc48_to_nv12_10bit_sse4_1;
				else if (check_ssse3())
					return               (width%8==0)  ? convert_yc48_to_nv12_10bit_ssse3_mod8        : convert_yc48_to_nv12_10bit_ssse3;
				else if (check_sse2())
					return               (width%8==0)  ? convert_yc48_to_nv12_10bit_sse2              : convert_yc48_to_nv12_10bit_sse2;
				else
					return                               convert_yc48_to_nv12_10bit;
			} else {
				if      (check_sse4_1())
					return               (width%8==0)  ? convert_yc48_to_nv12_10bit_full_sse4_1_mod8  : convert_yc48_to_nv12_10bit_full_sse4_1;
				else if (check_ssse3())
					return               (width%8==0)  ? convert_yc48_to_nv12_10bit_full_ssse3_mod8   : convert_yc48_to_nv12_10bit_full_ssse3;
				else if (check_sse2())
					return               (width%8==0)  ? convert_yc48_to_nv12_10bit_full_sse2_mod8    : convert_yc48_to_nv12_10bit_full_sse2;
				else
					return                               convert_yc48_to_nv12_10bit_full;
			}
		} else {
			if (!fullrange) {
				if      (check_sse4_1())
					return               (width%8==0)  ? convert_yc48_to_nv12_i_10bit_sse4_1_mod8      : convert_yc48_to_nv12_i_10bit_sse4_1;
				else if (check_ssse3())
					return               (width%8==0)  ? convert_yc48_to_nv12_i_10bit_ssse3_mod8       : convert_yc48_to_nv12_i_10bit_ssse3;
				else if (check_sse2())
					return               (width%8==0)  ? convert_yc48_to_nv12_i_10bit_sse2_mod8        : convert_yc48_to_nv12_i_10bit_sse2;
				else
					return                               convert_yc48_to_nv12_i_10bit;
			} else {
				if      (check_sse4_1())
					return               (width%8==0)  ? convert_yc48_to_nv12_i_10bit_full_sse4_1_mod8 : convert_yc48_to_nv12_i_10bit_full_sse4_1;
				else if (check_ssse3())
					return               (width%8==0)  ? convert_yc48_to_nv12_i_10bit_full_ssse3_mod8  : convert_yc48_to_nv12_i_10bit_full_ssse3;
				else if (check_sse2())
					return               (width%8==0)  ? convert_yc48_to_nv12_i_10bit_full_sse2_mod8   : convert_yc48_to_nv12_i_10bit_full_sse2;
				else
					return                               convert_yc48_to_nv12_i_10bit_full;
			}
		}
	}
}

//使用する関数を選択する
func_convert_frame get_convert_func(int width, BOOL use10bit, BOOL interlaced, int output_csp, BOOL fullrange) {
	switch (output_csp) {
		case OUT_CSP_YUV422:
			return get_convert_func_yuv422(use10bit, fullrange);
		case OUT_CSP_YUV444:
			return get_convert_func_yuv444(use10bit, fullrange);
		case OUT_CSP_RGB:
			return get_convert_func_rgb();
		case OUT_CSP_YUV420:
		default:
			return get_convert_func_yuv420(width, use10bit, interlaced, fullrange);
	}
}

BOOL malloc_pixel_data(CONVERT_CF_DATA * const pixel_data, int width, int height, int output_csp, BOOL use10bit) {
	BOOL ret = TRUE;
	int pixel_size = (use10bit) ? sizeof(short) : sizeof(BYTE);
	switch (output_csp) {
		case OUT_CSP_RGB:
			if (check_ssse3())
				width = ceil_div_int(width, 5) * 5;
			break;
		case OUT_CSP_YUV420:
		case OUT_CSP_YUV422:
		case OUT_CSP_YUV444:
		default:
			if (check_sse2())
				width = (width + 15) & ~15;
			break;
	}
	int frame_size = width * height * pixel_size;
	ZeroMemory(pixel_data->data, sizeof(pixel_data->data));
	switch (output_csp) {
		case OUT_CSP_YUV422:
			if ((pixel_data->data[0] = (BYTE *)_mm_malloc(frame_size * 2, 16)) == NULL)
				ret = FALSE;
			pixel_data->data[1] = pixel_data->data[0] + frame_size;
			break;
		case OUT_CSP_YUV444:
			if ((pixel_data->data[0] = (BYTE *)_mm_malloc(frame_size * 3, 16)) == NULL)
				ret = FALSE;
			pixel_data->data[1] = pixel_data->data[0] + frame_size;
			pixel_data->data[2] = pixel_data->data[1] + frame_size;
			break;
		case OUT_CSP_RGB:
			if ((pixel_data->data[0] = (BYTE *)_mm_malloc(frame_size * 3, 16)) == NULL)
				ret = FALSE;
			break;
		case OUT_CSP_YUV420:
		default:
			//YUV一気に確保
			if ((pixel_data->data[0] = (BYTE *)_mm_malloc(frame_size * 3 / 2, 16)) == NULL)
				ret = FALSE;
			//割り振り
			pixel_data->data[1] = pixel_data->data[0] + frame_size;
			break;
	}
	return ret;
}

void free_pixel_data(CONVERT_CF_DATA *pixel_data) {
	if (pixel_data->data[0])
		_mm_free(pixel_data->data[0]);
	ZeroMemory(pixel_data, sizeof(CONVERT_CF_DATA));
}
//AviutlのRGBをx264用に並び替える
void sort_to_rgb(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	BYTE *ptr = pixel_data->data[0];
	BYTE *dst, *src;
	int y0 = 0, y1 = height - 1;
	const int step = (width*3 + 3) & ~3;
	for (; y0 < height; y0++, y1--) {
		dst = ptr          + y1*width*3;
		src = (BYTE*)frame + y0*step;
		for (int x = 0; x < width; x++) {
			dst[x*3 + 0] = src[x*3 + 0];
			dst[x*3 + 2] = src[x*3 + 1];
			dst[x*3 + 1] = src[x*3 + 2];
		}
	}
}
void sort_to_rgb_sse2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	BYTE *ptr = pixel_data->data[0];
	BYTE *dst, *src, *src_fin;
	__m128i x0, x1;
	__m128i xMask = _mm_set_epi8( 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0);
	int y0 = 0, y1 = height - 1;
	const int step = (width*3 + 3) & ~3;
	for (; y0 < height; y0++, y1--) {
		dst = ptr          + y0*width*3;
		src = (BYTE*)frame + y1*step;
		src_fin = src + width*3;
		for (; src < src_fin; src += 15, dst += 15) {
			x0    = _mm_loadu_si128((const __m128i *)src);
			x1    = _mm_srli_si128(x0, 1);
			x0    = select_by_mask(x0, x1, xMask);
			x1    = _mm_slli_si128(x1, 2);
			xMask = _mm_slli_si128(xMask, 1);
			x0    = select_by_mask(x0, x1, xMask);
			xMask = _mm_srli_si128(xMask, 1);
			_mm_storeu_si128((__m128i *)dst, x0);
		}
	}
}
void sort_to_rgb_ssse3(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	static const __m128i xC_SHUF = _mm_set_epi8(16, 13, 14, 12, 10, 11,  9,  7,  8,  6,  4,  5,  3,  1,  2,  0);
	BYTE *ptr = pixel_data->data[0];
	BYTE *dst, *src, *src_fin;
	__m128i x0;
	int y0 = 0, y1 = height - 1;
	const int step = (width*3 + 3) & ~3;
	for (; y0 < height; y0++, y1--) {
		dst = ptr          + y0*width*3;
		src = (BYTE*)frame + y1*step;
		src_fin = src + width*3;
		for (; src < src_fin; src += 15, dst += 15) {
			x0 = _mm_loadu_si128((const __m128i *)src);
			x0 = _mm_shuffle_epi8(x0, xC_SHUF);
			_mm_storeu_si128((__m128i *)dst, x0);
		}
	}
}
//適当。
void convert_yuy2_to_nv12(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	BYTE *Y = pixel_data->data[0];
	BYTE *C = pixel_data->data[1];
	BYTE *p = (BYTE *)frame;
	for (y = 0; y < height; y += 2) {
		for (x = 0; x < width; x += 2) {
			Y[ y   *width + x    ] = p[( y   *width + x)*2    ];
			Y[ y   *width + x + 1] = p[( y   *width + x)*2 + 2];
			Y[(y+1)*width + x    ] = p[((y+1)*width + x)*2    ];
			Y[(y+1)*width + x + 1] = p[((y+1)*width + x)*2 + 2];
			C[y*width/2   + x    ] =(p[( y   *width + x)*2 + 1] + p[((y+1)*width + x)*2 + 1] + 1)/2;
			C[y*width/2   + x + 1] =(p[( y   *width + x)*2 + 3] + p[((y+1)*width + x)*2 + 3] + 1)/2;
		}
	}
}
//これも適当。
void convert_yuy2_to_nv12_i(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	BYTE *Y = pixel_data->data[0];
	BYTE *C = pixel_data->data[1];
	BYTE *p = (BYTE *)frame;
	for (y = 0; y < height; y += 2) {
		for (x = 0; x < width; x += 2) {
			Y[ y   *width + x    ] = p[( y   *width + x)*2    ];
			Y[ y   *width + x + 1] = p[( y   *width + x)*2 + 2];
			Y[(y+1)*width + x    ] = p[((y+1)*width + x)*2    ];
			Y[(y+1)*width + x + 1] = p[((y+1)*width + x)*2 + 2];
			Y[(y+2)*width + x    ] = p[((y+2)*width + x)*2    ];
			Y[(y+2)*width + x + 1] = p[((y+2)*width + x)*2 + 2];
			Y[(y+3)*width + x    ] = p[((y+3)*width + x)*2    ];
			Y[(y+3)*width + x + 1] = p[((y+3)*width + x)*2 + 2];
			C[y*width/2   + x    ] =(p[( y   *width + x)*2 + 1] + p[((y+2)*width + x)*2 + 1] + 1)/2;
			C[y*width/2   + x + 1] =(p[( y   *width + x)*2 + 3] + p[((y+2)*width + x)*2 + 3] + 1)/2;
			C[(y+1)*width/2 + x  ] =(p[((y+1)*width + x)*2 + 1] + p[((y+3)*width + x)*2 + 1] + 1)/2;
			C[(y+1)*width/2 + x+1] =(p[((y+1)*width + x)*2 + 3] + p[((y+3)*width + x)*2 + 3] + 1)/2;
		}
	}
}
//YUY2->NV12 SSE2版
void convert_yuy2_to_nv12_sse2_mod16(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	BYTE *p, *pw, *Y, *C;
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	__m128i x0, x1, x2, x3;
	for (y = 0; y < height; y += 2) {
		x  = y * width;
		p  = (BYTE *)frame + (x<<1);
		pw = p + (width<<1);
		Y  = (BYTE *)dst_Y +  x;
		C  = (BYTE *)dst_C + (x>>1);
		for (x = 0; x < width; x += 16, p += 32, pw += 32) {
			//-----------1行目---------------
			x0 = _mm_loadu_si128((const __m128i *)(p+ 0));    // VYUYVYUYVYUYVYUY
			x1 = _mm_loadu_si128((const __m128i *)(p+16));    // VYUYVYUYVYUYVYUY

			_mm_prefetch((const char *)pw, _MM_HINT_T1);

			x2 = _mm_unpacklo_epi8(x0, x1); //VVYYUUYYVVYYUUYY
			x1 = _mm_unpackhi_epi8(x0, x1); //VVYYUUYYVVYYUUYY

			x0 = _mm_unpacklo_epi8(x2, x1); //VVVVYYYYUUUUYYYY
			x1 = _mm_unpackhi_epi8(x2, x1); //VVVVYYYYUUUUYYYY

			x2 = _mm_unpacklo_epi8(x0, x1); //UUUUUUUUYYYYYYYY
			x1 = _mm_unpackhi_epi8(x0, x1); //VVVVVVVVYYYYYYYY

			x0 = _mm_unpacklo_epi8(x2, x1); //YYYYYYYYYYYYYYYY
			x3 = _mm_unpackhi_epi8(x2, x1); //VUVUVUVUVUVUVUVU

			_mm_stream_si128((__m128i *)(Y + x), x0);
			//-----------1行目終了---------------

			//-----------2行目---------------
			x0 = _mm_loadu_si128((const __m128i *)(pw+ 0));    // VYUYVYUYVYUYVYUY
			x1 = _mm_loadu_si128((const __m128i *)(pw+16));    // VYUYVYUYVYUYVYUY

			x2 = _mm_unpacklo_epi8(x0, x1); //VVYYUUYYVVYYUUYY
			x1 = _mm_unpackhi_epi8(x0, x1); //VVYYUUYYVVYYUUYY

			x0 = _mm_unpacklo_epi8(x2, x1); //VVVVYYYYUUUUYYYY
			x1 = _mm_unpackhi_epi8(x2, x1); //VVVVYYYYUUUUYYYY

			x2 = _mm_unpacklo_epi8(x0, x1); //UUUUUUUUYYYYYYYY
			x1 = _mm_unpackhi_epi8(x0, x1); //VVVVVVVVYYYYYYYY

			x0 = _mm_unpacklo_epi8(x2, x1); //YYYYYYYYYYYYYYYY
			x1 = _mm_unpackhi_epi8(x2, x1); //VUVUVUVUVUVUVUVU

			_mm_stream_si128((__m128i *)(Y + width + x), x0);
			//-----------2行目終了---------------

			x1 = _mm_avg_epu8(x1, x3);  //VUVUVUVUVUVUVUVU
			_mm_stream_si128((__m128i *)(C + x), x1);
		}
	}
}

void convert_yuy2_to_nv12_sse2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	BYTE *p, *pw, *Y, *C;
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	__m128i x0, x1, x2, x3;
	for (y = 0; y < height; y += 2) {
		x  = y * width;
		p  = (BYTE *)frame + (x<<1);
		pw = p + (width<<1);
		Y  = (BYTE *)dst_Y +  x;
		C  = (BYTE *)dst_C + (x>>1);
		for (x = 0; x < width; x += 16, p += 32, pw += 32) {
			//-----------1行目---------------
			x0 = _mm_loadu_si128((const __m128i *)(p+ 0));    // VYUYVYUYVYUYVYUY
			x1 = _mm_loadu_si128((const __m128i *)(p+16));    // VYUYVYUYVYUYVYUY

			_mm_prefetch((const char *)pw, _MM_HINT_T1);

			x2 = _mm_unpacklo_epi8(x0, x1); //VVYYUUYYVVYYUUYY
			x1 = _mm_unpackhi_epi8(x0, x1); //VVYYUUYYVVYYUUYY

			x0 = _mm_unpacklo_epi8(x2, x1); //VVVVYYYYUUUUYYYY
			x1 = _mm_unpackhi_epi8(x2, x1); //VVVVYYYYUUUUYYYY

			x2 = _mm_unpacklo_epi8(x0, x1); //UUUUUUUUYYYYYYYY
			x1 = _mm_unpackhi_epi8(x0, x1); //VVVVVVVVYYYYYYYY

			x0 = _mm_unpacklo_epi8(x2, x1); //YYYYYYYYYYYYYYYY
			x3 = _mm_unpackhi_epi8(x2, x1); //VUVUVUVUVUVUVUVU

			_mm_storeu_si128((__m128i *)(Y + x), x0);
			//-----------1行目終了---------------

			//-----------2行目---------------
			x0 = _mm_loadu_si128((const __m128i *)(pw+ 0));    // VYUYVYUYVYUYVYUY
			x1 = _mm_loadu_si128((const __m128i *)(pw+16));    // VYUYVYUYVYUYVYUY

			x2 = _mm_unpacklo_epi8(x0, x1); //VVYYUUYYVVYYUUYY
			x1 = _mm_unpackhi_epi8(x0, x1); //VVYYUUYYVVYYUUYY

			x0 = _mm_unpacklo_epi8(x2, x1); //VVVVYYYYUUUUYYYY
			x1 = _mm_unpackhi_epi8(x2, x1); //VVVVYYYYUUUUYYYY

			x2 = _mm_unpacklo_epi8(x0, x1); //UUUUUUUUYYYYYYYY
			x1 = _mm_unpackhi_epi8(x0, x1); //VVVVVVVVYYYYYYYY

			x0 = _mm_unpacklo_epi8(x2, x1); //YYYYYYYYYYYYYYYY
			x1 = _mm_unpackhi_epi8(x2, x1); //VUVUVUVUVUVUVUVU

			_mm_storeu_si128((__m128i *)(Y + width + x), x0);
			//-----------2行目終了---------------

			x1 = _mm_avg_epu8(x1, x3);  //VUVUVUVUVUVUVUVU
			_mm_storeu_si128((__m128i *)(C + x), x1);
		}
	}
}

void convert_yuy2_to_nv12_i_sse2_mod16(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	BYTE *p, *pw, *Y, *C;
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	__m128i x0, x1, x2, x3;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			x  = (y + i) * width;
			p  = (BYTE *)frame + (x<<1);
			pw  = p   + (width<<2);
			Y  = (BYTE *)dst_Y +  x;
			C  = (BYTE *)dst_C + ((x+width*i)>>1);
			for (x = 0; x < width; x += 16, p += 32, pw += 32) {
				//-----------    1+i行目   ---------------
				x0 = _mm_loadu_si128((const __m128i *)(p+ 0));    // VYUYVYUYVYUYVYUY
				x1 = _mm_loadu_si128((const __m128i *)(p+16));    // VYUYVYUYVYUYVYUY

				_mm_prefetch((const char *)pw, _MM_HINT_T1);

				x2 = _mm_unpacklo_epi8(x0, x1); //VVYYUUYYVVYYUUYY
				x1 = _mm_unpackhi_epi8(x0, x1); //VVYYUUYYVVYYUUYY

				x0 = _mm_unpacklo_epi8(x2, x1); //VVVVYYYYUUUUYYYY
				x1 = _mm_unpackhi_epi8(x2, x1); //VVVVYYYYUUUUYYYY

				x2 = _mm_unpacklo_epi8(x0, x1); //UUUUUUUUYYYYYYYY
				x1 = _mm_unpackhi_epi8(x0, x1); //VVVVVVVVYYYYYYYY

				x0 = _mm_unpacklo_epi8(x2, x1); //YYYYYYYYYYYYYYYY
				x3 = _mm_unpackhi_epi8(x2, x1); //VUVUVUVUVUVUVUVU

				_mm_stream_si128((__m128i *)(Y + x), x0);
				//-----------1+i行目終了---------------

				//-----------3+i行目---------------
				x0 = _mm_loadu_si128((const __m128i *)(pw+ 0));    // VYUYVYUYVYUYVYUY
				x1 = _mm_loadu_si128((const __m128i *)(pw+16));    // VYUYVYUYVYUYVYUY

				x2 = _mm_unpacklo_epi8(x0, x1); //VVYYUUYYVVYYUUYY
				x1 = _mm_unpackhi_epi8(x0, x1); //VVYYUUYYVVYYUUYY

				x0 = _mm_unpacklo_epi8(x2, x1); //VVVVYYYYUUUUYYYY
				x1 = _mm_unpackhi_epi8(x2, x1); //VVVVYYYYUUUUYYYY

				x2 = _mm_unpacklo_epi8(x0, x1); //UUUUUUUUYYYYYYYY
				x1 = _mm_unpackhi_epi8(x0, x1); //VVVVVVVVYYYYYYYY

				x0 = _mm_unpacklo_epi8(x2, x1); //YYYYYYYYYYYYYYYY
				x1 = _mm_unpackhi_epi8(x2, x1); //VUVUVUVUVUVUVUVU

				_mm_stream_si128((__m128i *)(Y + (width<<1) + x), x0);
				//-----------3+i行目終了---------------

				x1 = _mm_avg_epu8(x1, x3);  //VUVUVUVUVUVUVUVU
				_mm_stream_si128((__m128i *)(C + x), x1);
			}
		}
	}
}

void convert_yuy2_to_nv12_i_sse2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	BYTE *p, *pw, *Y, *C;
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	__m128i x0, x1, x2, x3;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			x  = (y + i) * width;
			p  = (BYTE *)frame + (x<<1);
			pw  = p   + (width<<2);
			Y  = (BYTE *)dst_Y +  x;
			C  = (BYTE *)dst_C + ((x+width*i)>>1);
			for (x = 0; x < width; x += 16, p += 32, pw += 32) {
				//-----------    1行目   ---------------
				x0 = _mm_loadu_si128((const __m128i *)(p+ 0));    // VYUYVYUYVYUYVYUY
				x1 = _mm_loadu_si128((const __m128i *)(p+16));    // VYUYVYUYVYUYVYUY

				_mm_prefetch((const char *)pw, _MM_HINT_T1);

				x2 = _mm_unpacklo_epi8(x0, x1); //VVYYUUYYVVYYUUYY
				x1 = _mm_unpackhi_epi8(x0, x1); //VVYYUUYYVVYYUUYY

				x0 = _mm_unpacklo_epi8(x2, x1); //VVVVYYYYUUUUYYYY
				x1 = _mm_unpackhi_epi8(x2, x1); //VVVVYYYYUUUUYYYY

				x2 = _mm_unpacklo_epi8(x0, x1); //UUUUUUUUYYYYYYYY
				x1 = _mm_unpackhi_epi8(x0, x1); //VVVVVVVVYYYYYYYY

				x0 = _mm_unpacklo_epi8(x2, x1); //YYYYYYYYYYYYYYYY
				x3 = _mm_unpackhi_epi8(x2, x1); //VUVUVUVUVUVUVUVU

				_mm_storeu_si128((__m128i *)(Y + x), x0);
				//-----------1行目終了---------------

				//-----------3行目---------------
				x0 = _mm_loadu_si128((const __m128i *)(pw+ 0));    // VYUYVYUYVYUYVYUY
				x1 = _mm_loadu_si128((const __m128i *)(pw+16));    // VYUYVYUYVYUYVYUY

				x2 = _mm_unpacklo_epi8(x0, x1); //VVYYUUYYVVYYUUYY
				x1 = _mm_unpackhi_epi8(x0, x1); //VVYYUUYYVVYYUUYY

				x0 = _mm_unpacklo_epi8(x2, x1); //VVVVYYYYUUUUYYYY
				x1 = _mm_unpackhi_epi8(x2, x1); //VVVVYYYYUUUUYYYY

				x2 = _mm_unpacklo_epi8(x0, x1); //UUUUUUUUYYYYYYYY
				x1 = _mm_unpackhi_epi8(x0, x1); //VVVVVVVVYYYYYYYY

				x0 = _mm_unpacklo_epi8(x2, x1); //YYYYYYYYYYYYYYYY
				x1 = _mm_unpackhi_epi8(x2, x1); //VUVUVUVUVUVUVUVU

				_mm_storeu_si128((__m128i *)(Y + (width<<1) + x), x0);
				//-----------3行目終了---------------

				x1 = _mm_avg_epu8(x1, x3);  //VUVUVUVUVUVUVUVU
				_mm_storeu_si128((__m128i *)(C + x), x1);
			}
		}
	}
}

static int inline pixel_YC48_to_YUV(int y, int mul, int add, int rshift, int ycc, int min, int max) {
	return clamp(((y * mul + add) >> rshift) + ycc, min, max);
}
void convert_yc48_to_nv12_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x = 0, y = 0, i = 0;
	PIXEL_YC *ycp;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *Y = NULL, *C = (short *)dst_C;
	for (y = 0; y < height; y += 2) {
		i = width * y;
		ycp = (PIXEL_YC *)pixel + i;
		Y = (short *)dst_Y + i;
		for (x = 0; x < width; x += 2) {
			Y[x        ] = (short)pixel_YC48_to_YUV(ycp[x        ].y, Y_L_MUL, Y_L_ADD, Y_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			Y[x+1      ] = (short)pixel_YC48_to_YUV(ycp[x+1      ].y, Y_L_MUL, Y_L_ADD, Y_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			Y[x  +width] = (short)pixel_YC48_to_YUV(ycp[x  +width].y, Y_L_MUL, Y_L_ADD, Y_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			Y[x+1+width] = (short)pixel_YC48_to_YUV(ycp[x+1+width].y, Y_L_MUL, Y_L_ADD, Y_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			*C = (short)pixel_YC48_to_YUV(((int)ycp[x].cb + (int)ycp[x+width].cb) + UV_OFFSET, UV_L_MUL, UV_L_ADD, UV_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			C++;
			*C = (short)pixel_YC48_to_YUV(((int)ycp[x].cr + (int)ycp[x+width].cr) + UV_OFFSET, UV_L_MUL, UV_L_ADD, UV_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			C++;
		}
	}
}
//YC48から輝度をロードする
static inline void _mm_set_ycp_y(__m128i& x0, short *ycp) {
	x0 = _mm_insert_epi16(x0, *(ycp     ), 0);
	x0 = _mm_insert_epi16(x0, *(ycp +  3), 1);
	x0 = _mm_insert_epi16(x0, *(ycp +  6), 2);
	x0 = _mm_insert_epi16(x0, *(ycp +  9), 3);
	x0 = _mm_insert_epi16(x0, *(ycp + 12), 4);
	x0 = _mm_insert_epi16(x0, *(ycp + 15), 5);
	x0 = _mm_insert_epi16(x0, *(ycp + 18), 6);
	x0 = _mm_insert_epi16(x0, *(ycp + 21), 7);
}
//YC48から色差をロードする(左側のみ)
static inline void _mm_set_ycp_c(__m128i& x0, short *ycp) {
	x0 = _mm_insert_epi16(x0, *(ycp +  1), 0);
	x0 = _mm_insert_epi16(x0, *(ycp +  2), 1);
	x0 = _mm_insert_epi16(x0, *(ycp +  7), 2);
	x0 = _mm_insert_epi16(x0, *(ycp +  8), 3);
	x0 = _mm_insert_epi16(x0, *(ycp + 13), 4);
	x0 = _mm_insert_epi16(x0, *(ycp + 14), 5);
	x0 = _mm_insert_epi16(x0, *(ycp + 19), 6);
	x0 = _mm_insert_epi16(x0, *(ycp + 20), 7);
}
//YC48から色差をロードする(左側のみ)SSE4.1版
static inline void _mm_set_ycp_c_sse4(__m128i& x0, short *ycp) {
	int *p = (int*)(ycp+1);
	x0 = _mm_insert_epi32(x0, *p, 0);
	p += 3;
	x0 = _mm_insert_epi32(x0, *p, 1);
	p += 3;
	x0 = _mm_insert_epi32(x0, *p, 2);
	p += 3;
	x0 = _mm_insert_epi32(x0, *p, 3);
}
void convert_yc48_to_nv12_i_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x = 0, y = 0, i = 0;
	PIXEL_YC *ycp = NULL;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *Y = NULL, *C = NULL;
	for (y = 0; y < height; y += 4) {
		i = width * y;
		ycp = (PIXEL_YC *)pixel + i;
		Y = (short *)dst_Y + i;
		C = (short *)dst_C + (i>>1);
		for (x = 0; x < width; x += 2) {
			Y[x          ] = (BYTE)pixel_YC48_to_YUV(ycp[x          ].y, Y_L_MUL, Y_L_ADD, Y_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			Y[x+1        ] = (BYTE)pixel_YC48_to_YUV(ycp[x+1        ].y, Y_L_MUL, Y_L_ADD, Y_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			Y[x  +width  ] = (BYTE)pixel_YC48_to_YUV(ycp[x  +width  ].y, Y_L_MUL, Y_L_ADD, Y_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			Y[x+1+width  ] = (BYTE)pixel_YC48_to_YUV(ycp[x+1+width  ].y, Y_L_MUL, Y_L_ADD, Y_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			Y[x  +width*2] = (BYTE)pixel_YC48_to_YUV(ycp[x  +width*2].y, Y_L_MUL, Y_L_ADD, Y_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			Y[x+1+width*2] = (BYTE)pixel_YC48_to_YUV(ycp[x+1+width*2].y, Y_L_MUL, Y_L_ADD, Y_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			Y[x  +width*3] = (BYTE)pixel_YC48_to_YUV(ycp[x  +width*3].y, Y_L_MUL, Y_L_ADD, Y_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			Y[x+1+width*3] = (BYTE)pixel_YC48_to_YUV(ycp[x+1+width*3].y, Y_L_MUL, Y_L_ADD, Y_L_RSH_10, Y_L_YCC_10, 0, LIMIT_10);
			*C              = (BYTE)pixel_YC48_to_YUV(((int)ycp[x      ].cb + (int)ycp[x+width*2].cb) + UV_OFFSET, UV_L_MUL, UV_L_ADD, UV_L_RSH_10, UV_L_YCC_10, 0, LIMIT_10);
			*(C+(width>>1)) = (BYTE)pixel_YC48_to_YUV(((int)ycp[x+width].cb + (int)ycp[x+width*3].cb) + UV_OFFSET, UV_L_MUL, UV_L_ADD, UV_L_RSH_10, UV_L_YCC_10, 0, LIMIT_10);
			C++;
			*C              = (BYTE)pixel_YC48_to_YUV(((int)ycp[x      ].cr + (int)ycp[x+width*2].cr) + UV_OFFSET, UV_L_MUL, UV_L_ADD, UV_L_RSH_10, UV_L_YCC_10, 0, LIMIT_10);
			*(C+(width>>1)) = (BYTE)pixel_YC48_to_YUV(((int)ycp[x+width].cr + (int)ycp[x+width*3].cr) + UV_OFFSET, UV_L_MUL, UV_L_ADD, UV_L_RSH_10, UV_L_YCC_10, 0, LIMIT_10);
			C++;
		}
	}
}
void convert_yc48_to_nv12_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x = 0, y = 0, i = 0;
	PIXEL_YC *ycp;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *Y = NULL, *C = (short *)dst_C;
	for (y = 0; y < height; y += 2) {
		i = width * y;
		ycp = (PIXEL_YC *)pixel + i;
		Y = (short *)dst_Y + i;
		for (x = 0; x < width; x += 2) {
			Y[x        ] = (short)pixel_YC48_to_YUV(ycp[x        ].y, Y_F_MUL, Y_F_ADD, Y_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			Y[x+1      ] = (short)pixel_YC48_to_YUV(ycp[x+1      ].y, Y_F_MUL, Y_F_ADD, Y_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			Y[x  +width] = (short)pixel_YC48_to_YUV(ycp[x  +width].y, Y_F_MUL, Y_F_ADD, Y_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			Y[x+1+width] = (short)pixel_YC48_to_YUV(ycp[x+1+width].y, Y_F_MUL, Y_F_ADD, Y_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			*C = (short)pixel_YC48_to_YUV(((int)ycp[x].cb + (int)ycp[x+width].cb) + UV_OFFSET, UV_F_MUL, UV_F_ADD, UV_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			C++;
			*C = (short)pixel_YC48_to_YUV(((int)ycp[x].cr + (int)ycp[x+width].cr) + UV_OFFSET, UV_F_MUL, UV_F_ADD, UV_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			C++;
		}
	}
}

void convert_yc48_to_nv12_i_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x = 0, y = 0, i = 0;
	PIXEL_YC *ycp = NULL;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *Y = NULL, *C = NULL;
	for (y = 0; y < height; y += 4) {
		i = width * y;
		ycp = (PIXEL_YC *)pixel + i;
		Y = (short *)dst_Y + i;
		C = (short *)dst_C + (i>>1);
		for (x = 0; x < width; x += 2) {
			Y[x          ] = (BYTE)pixel_YC48_to_YUV(ycp[x          ].y, Y_F_MUL, Y_F_ADD, Y_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			Y[x+1        ] = (BYTE)pixel_YC48_to_YUV(ycp[x+1        ].y, Y_F_MUL, Y_F_ADD, Y_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			Y[x  +width  ] = (BYTE)pixel_YC48_to_YUV(ycp[x  +width  ].y, Y_F_MUL, Y_F_ADD, Y_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			Y[x+1+width  ] = (BYTE)pixel_YC48_to_YUV(ycp[x+1+width  ].y, Y_F_MUL, Y_F_ADD, Y_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			Y[x  +width*2] = (BYTE)pixel_YC48_to_YUV(ycp[x  +width*2].y, Y_F_MUL, Y_F_ADD, Y_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			Y[x+1+width*2] = (BYTE)pixel_YC48_to_YUV(ycp[x+1+width*2].y, Y_F_MUL, Y_F_ADD, Y_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			Y[x  +width*3] = (BYTE)pixel_YC48_to_YUV(ycp[x  +width*3].y, Y_F_MUL, Y_F_ADD, Y_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			Y[x+1+width*3] = (BYTE)pixel_YC48_to_YUV(ycp[x+1+width*3].y, Y_F_MUL, Y_F_ADD, Y_F_RSH_10, Y_F_YCC_10, 0, LIMIT_10);
			*C              = (BYTE)pixel_YC48_to_YUV(((int)ycp[x      ].cb + (int)ycp[x+width*2].cb) + UV_OFFSET, UV_F_MUL, UV_F_ADD, UV_F_RSH_10, UV_F_YCC_10, 0, LIMIT_10);
			*(C+(width>>1)) = (BYTE)pixel_YC48_to_YUV(((int)ycp[x+width].cb + (int)ycp[x+width*3].cb) + UV_OFFSET, UV_F_MUL, UV_F_ADD, UV_F_RSH_10, UV_F_YCC_10, 0, LIMIT_10);
			C++;
			*C              = (BYTE)pixel_YC48_to_YUV(((int)ycp[x      ].cr + (int)ycp[x+width*2].cr) + UV_OFFSET, UV_F_MUL, UV_F_ADD, UV_F_RSH_10, UV_F_YCC_10, 0, LIMIT_10);
			*(C+(width>>1)) = (BYTE)pixel_YC48_to_YUV(((int)ycp[x+width].cr + (int)ycp[x+width*3].cr) + UV_OFFSET, UV_F_MUL, UV_F_ADD, UV_F_RSH_10, UV_F_YCC_10, 0, LIMIT_10);
			C++;
		}
	}
}

void convert_yc48_to_nv12_10bit_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1, x2, x3, x4, x5, x6, x7;
	for (y = 0; y < height; y += 2) {
		ycp = (short*)pixel + width * y * 3;
		ycpw= ycp + width*3;
		Y   = (short*)dst_Y + width * y;
		C   = (short*)dst_C + width * y / 2;
		for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
			x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

			_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

			x4 = MASK_YCP2Y;
			x0 = select_by_mask(x1, x2, x4);
			x7 = _mm_srli_si128(x4, 4);
			x0 = select_by_mask(x0, x3, x7);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x5 = MASK_YCP2UV;
			x6 = select_by_mask(x1, x2, x5);
			x7 = _mm_srli_si128(x5, 4);
			x6 = select_by_mask(x6, x3, x7); 
			x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

				//Y 1行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
				x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_storeu_si128((__m128i *)(Y + x), x0);

			x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

			x0 = select_by_mask(x1, x2, x4);
			x7 = _mm_srli_si128(x4, 4);
			x0 = select_by_mask(x0, x3, x7);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x1 = select_by_mask(x1, x2, x5);
			x7 = _mm_srli_si128(x5, 4);
			x1 = select_by_mask(x1, x3, x7);
			x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);

			//UV 1行目 + 2行目
			x6 = _mm_add_epi16(x1, x6);
				
				//Y 2行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
				x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_storeu_si128((__m128i *)(Y + x + width), x0);

			x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_UV_L_MA);
			x0 = _mm_srai_epi32(x0, UV_L_RSH_10);

			x7 = _mm_madd_epi16(x7, xC_UV_L_MA);
			x7 = _mm_srai_epi32(x7, UV_L_RSH_10);

			x0 = _mm_packs_epi32(x0, x7);

			x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
			x0 = _mm_add_epi16(x0, x1);
			x7 = _mm_cmpeq_epi8(x7, x7);
			x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
			x0 = _mm_min_epi16(x0, x7);
			x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_storeu_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_10bit_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1, x2, x3, x4, x5, x6, x7;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			ycp = (short*)pixel + width * (y + i) * 3;
			ycpw= ycp + width*2*3;
			Y   = (short*)dst_Y + width * (y + i);
			C   = (short*)dst_C + width * (y + i*2) / 2;
			for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
				x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

				_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

				x4 = MASK_YCP2Y;
				x0 = select_by_mask(x1, x2, x4);
				x7 = _mm_srli_si128(x4, 4);
				x0 = select_by_mask(x0, x3, x7);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x5 = MASK_YCP2UV;
				x6 = select_by_mask(x1, x2, x5);
				x7 = _mm_srli_si128(x5, 4);
				x6 = select_by_mask(x6, x3, x7); 
				x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

					//Y 1行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
					x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_storeu_si128((__m128i *)(Y + x), x0);

				x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

				x0 = select_by_mask(x1, x2, x4);
				x7 = _mm_srli_si128(x4, 4);
				x0 = select_by_mask(x0, x3, x7);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x1 = select_by_mask(x1, x2, x5);
				x7 = _mm_srli_si128(x5, 4);
				x1 = select_by_mask(x1, x3, x7);
				x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);

				//UV 1行目 + 3行目
				x6 = _mm_add_epi16(x1, x6);

					//Y 3行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
					x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_storeu_si128((__m128i *)(Y + x + width*2), x0);

				x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_UV_L_MA);
				x0 = _mm_srai_epi32(x0, UV_L_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_UV_L_MA);
				x7 = _mm_srai_epi32(x7, UV_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

				_mm_storeu_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_10bit_ssse3_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1, x2, x3, x4, x5, x6, x7;
	for (y = 0; y < height; y += 2) {
		ycp = (short*)pixel + width * y * 3;
		ycpw= ycp + width*3;
		Y   = (short*)dst_Y + width * y;
		C   = (short*)dst_C + width * y / 2;
		for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
			x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

			_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

			x4 = MASK_YCP2Y;
			x0 = select_by_mask(x1, x2, x4);
			x7 = _mm_srli_si128(x4, 4);
			x0 = select_by_mask(x0, x3, x7);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x5 = MASK_YCP2UV;
			x6 = select_by_mask(x1, x2, x5);
			x7 = _mm_srli_si128(x5, 4);
			x6 = select_by_mask(x6, x3, x7); 
			x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

				//Y 1行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
				x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_stream_si128((__m128i *)(Y + x), x0);

			x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

			x0 = select_by_mask(x1, x2, x4);
			x7 = _mm_srli_si128(x4, 4);
			x0 = select_by_mask(x0, x3, x7);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x1 = select_by_mask(x1, x2, x5);
			x7 = _mm_srli_si128(x5, 4);
			x1 = select_by_mask(x1, x3, x7);
			x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);

			//UV 1行目 + 2行目
			x6 = _mm_add_epi16(x1, x6);
				
				//Y 2行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
				x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_stream_si128((__m128i *)(Y + x + width), x0);

			x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_UV_L_MA);
			x0 = _mm_srai_epi32(x0, UV_L_RSH_10);

			x7 = _mm_madd_epi16(x7, xC_UV_L_MA);
			x7 = _mm_srai_epi32(x7, UV_L_RSH_10);

			x0 = _mm_packs_epi32(x0, x7);

			x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
			x0 = _mm_add_epi16(x0, x1);
			x7 = _mm_cmpeq_epi8(x7, x7);
			x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
			x0 = _mm_min_epi16(x0, x7);
			x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_stream_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_10bit_ssse3_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1, x2, x3, x4, x5, x6, x7;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			ycp = (short*)pixel + width * (y + i) * 3;
			ycpw= ycp + width*2*3;
			Y   = (short*)dst_Y + width * (y + i);
			C   = (short*)dst_C + width * (y + i*2) / 2;
			for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
				x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

				_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

				x4 = MASK_YCP2Y;
				x0 = select_by_mask(x1, x2, x4);
				x7 = _mm_srli_si128(x4, 4);
				x0 = select_by_mask(x0, x3, x7);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x5 = MASK_YCP2UV;
				x6 = select_by_mask(x1, x2, x5);
				x7 = _mm_srli_si128(x5, 4);
				x6 = select_by_mask(x6, x3, x7); 
				x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

					//Y 1行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
					x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_stream_si128((__m128i *)(Y + x), x0);

				x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

				x0 = select_by_mask(x1, x2, x4);
				x7 = _mm_srli_si128(x4, 4);
				x0 = select_by_mask(x0, x3, x7);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x1 = select_by_mask(x1, x2, x5);
				x7 = _mm_srli_si128(x5, 4);
				x1 = select_by_mask(x1, x3, x7);
				x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);

				//UV 1行目 + 3行目
				x6 = _mm_add_epi16(x1, x6);

					//Y 3行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
					x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_stream_si128((__m128i *)(Y + x + width*2), x0);

				x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_UV_L_MA);
				x0 = _mm_srai_epi32(x0, UV_L_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_UV_L_MA);
				x7 = _mm_srai_epi32(x7, UV_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

				_mm_stream_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_10bit_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m128i x0, x1, x2, x3, x6, x7;
	for (y = 0; y < height; y += 2) {
		ycp = (short*)pixel + width * y * 3;
		ycpw= ycp + width*3;
		Y   = (short*)dst_Y + width * y;
		C   = (short*)dst_C + width * y / 2;
		for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
			x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

			_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

			x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
			x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x6 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
			x6 = _mm_blend_epi16(x6, x3, MASK_INT_UV>>2); 
			x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

				//Y 1行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
				x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_storeu_si128((__m128i *)(Y + x), x0);

			x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

			x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
			x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
			x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
			x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);//UV2行目

			//UV 1行目 + 2行目
			x6 = _mm_add_epi16(x1, x6);
				
				//Y 2行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
				x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_storeu_si128((__m128i *)(Y + x + width), x0);

			x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_UV_L_MA);
			x0 = _mm_srai_epi32(x0, UV_L_RSH_10);

			x7 = _mm_madd_epi16(x7, xC_UV_L_MA);
			x7 = _mm_srai_epi32(x7, UV_L_RSH_10);

			x0 = _mm_packs_epi32(x0, x7);

			x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
			x0 = _mm_add_epi16(x0, x1);
			x7 = _mm_cmpeq_epi8(x7, x7);
			x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
			x0 = _mm_min_epi16(x0, x7);
			x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_storeu_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_10bit_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m128i x0, x1, x2, x3, x6, x7;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			ycp = (short*)pixel + width * (y + i) * 3;
			ycpw= ycp + width*2*3;
			Y   = (short*)dst_Y + width * (y + i);
			C   = (short*)dst_C + width * (y + i*2) / 2;
			for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
				x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

				_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

				x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
				x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x6 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
				x6 = _mm_blend_epi16(x6, x3, MASK_INT_UV>>2); 
				x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

					//Y 1行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
					x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_storeu_si128((__m128i *)(Y + x), x0);

				x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

				x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
				x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
				x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
				x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);//UV3行目

				//UV 1行目 + 3行目
				x6 = _mm_add_epi16(x1, x6);

					//Y 3行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
					x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_storeu_si128((__m128i *)(Y + x + width*2), x0);

				x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_UV_L_MA);
				x0 = _mm_srai_epi32(x0, UV_L_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_UV_L_MA);
				x7 = _mm_srai_epi32(x7, UV_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

				_mm_storeu_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_10bit_sse4_1_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m128i x0, x1, x2, x3, x6, x7;
	for (y = 0; y < height; y += 2) {
		ycp = (short*)pixel + width * y * 3;
		ycpw= ycp + width*3;
		Y   = (short*)dst_Y + width * y;
		C   = (short*)dst_C + width * y / 2;
		for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
			x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

			_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

			x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
			x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x6 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
			x6 = _mm_blend_epi16(x6, x3, MASK_INT_UV>>2); 
			x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

				//Y 1行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
				x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_stream_si128((__m128i *)(Y + x), x0);

			x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

			x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
			x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
			x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
			x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);//UV2行目

			//UV 1行目 + 2行目
			x6 = _mm_add_epi16(x1, x6);
				
				//Y 2行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
				x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_stream_si128((__m128i *)(Y + x + width), x0);

			x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_UV_L_MA);
			x0 = _mm_srai_epi32(x0, UV_L_RSH_10);

			x7 = _mm_madd_epi16(x7, xC_UV_L_MA);
			x7 = _mm_srai_epi32(x7, UV_L_RSH_10);

			x0 = _mm_packs_epi32(x0, x7);

			x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
			x0 = _mm_add_epi16(x0, x1);
			x7 = _mm_cmpeq_epi8(x7, x7);
			x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
			x0 = _mm_min_epi16(x0, x7);
			x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_stream_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_10bit_sse4_1_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m128i x0, x1, x2, x3, x6, x7;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			ycp = (short*)pixel + width * (y + i) * 3;
			ycpw= ycp + width*2*3;
			Y   = (short*)dst_Y + width * (y + i);
			C   = (short*)dst_C + width * (y + i*2) / 2;
			for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
				x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

				_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

				x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
				x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x6 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
				x6 = _mm_blend_epi16(x6, x3, MASK_INT_UV>>2); 
				x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

					//Y 1行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
					x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_stream_si128((__m128i *)(Y + x), x0);

				x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

				x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
				x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
				x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
				x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);//UV3行目

				//UV 1行目 + 3行目
				x6 = _mm_add_epi16(x1, x6);

					//Y 3行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_L_MA);
					x7 = _mm_srai_epi32(x7, Y_L_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_stream_si128((__m128i *)(Y + x + width*2), x0);

				x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_UV_L_MA);
				x0 = _mm_srai_epi32(x0, UV_L_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_UV_L_MA);
				x7 = _mm_srai_epi32(x7, UV_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

				_mm_stream_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_10bit_full_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1, x2, x3, x4, x5, x6, x7;
	for (y = 0; y < height; y += 2) {
		ycp = (short*)pixel + width * y * 3;
		ycpw= ycp + width*3;
		Y   = (short*)dst_Y + width * y;
		C   = (short*)dst_C + width * y / 2;
		for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
			x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

			_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

			x4 = MASK_YCP2Y;
			x0 = select_by_mask(x1, x2, x4);
			x7 = _mm_srli_si128(x4, 4);
			x0 = select_by_mask(x0, x3, x7);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x5 = MASK_YCP2UV;
			x6 = select_by_mask(x1, x2, x5);
			x7 = _mm_srli_si128(x5, 4);
			x6 = select_by_mask(x6, x3, x7); 
			x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

				//Y 1行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
				x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
				x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_storeu_si128((__m128i *)(Y + x), x0);

			x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

			x0 = select_by_mask(x1, x2, x4);
			x7 = _mm_srli_si128(x4, 4);
			x0 = select_by_mask(x0, x3, x7);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x1 = select_by_mask(x1, x2, x5);
			x7 = _mm_srli_si128(x5, 4);
			x1 = select_by_mask(x1, x3, x7);
			x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);

			//UV 1行目 + 2行目
			x6 = _mm_add_epi16(x1, x6);
				
				//Y 2行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
				x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
				x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_storeu_si128((__m128i *)(Y + x + width), x0);

			x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_UV_F_MA);
			x0 = _mm_srai_epi32(x0, UV_F_RSH_10);

			x7 = _mm_madd_epi16(x7, xC_UV_F_MA);
			x7 = _mm_srai_epi32(x7, UV_F_RSH_10);

			x0 = _mm_packs_epi32(x0, x7);

			x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
			x0 = _mm_add_epi16(x0, x1);
			x7 = _mm_cmpeq_epi8(x7, x7);
			x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
			x0 = _mm_min_epi16(x0, x7);
			x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_storeu_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_10bit_full_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1, x2, x3, x4, x5, x6, x7;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			ycp = (short*)pixel + width * (y + i) * 3;
			ycpw= ycp + width*2*3;
			Y   = (short*)dst_Y + width * (y + i);
			C   = (short*)dst_C + width * (y + i*2) / 2;
			for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
				x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

				_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

				x4 = MASK_YCP2Y;
				x0 = select_by_mask(x1, x2, x4);
				x7 = _mm_srli_si128(x4, 4);
				x0 = select_by_mask(x0, x3, x7);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x5 = MASK_YCP2UV;
				x6 = select_by_mask(x1, x2, x5);
				x7 = _mm_srli_si128(x5, 4);
				x6 = select_by_mask(x6, x3, x7); 
				x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

					//Y 1行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
					x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
					x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_storeu_si128((__m128i *)(Y + x), x0);

				x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

				x0 = select_by_mask(x1, x2, x4);
				x7 = _mm_srli_si128(x4, 4);
				x0 = select_by_mask(x0, x3, x7);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x1 = select_by_mask(x1, x2, x5);
				x7 = _mm_srli_si128(x5, 4);
				x1 = select_by_mask(x1, x3, x7);
				x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);

				//UV 1行目 + 3行目
				x6 = _mm_add_epi16(x1, x6);

					//Y 3行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
					x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
					x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_storeu_si128((__m128i *)(Y + x + width*2), x0);

				x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_UV_F_MA);
				x0 = _mm_srai_epi32(x0, UV_F_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_UV_F_MA);
				x7 = _mm_srai_epi32(x7, UV_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

				_mm_storeu_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_10bit_full_ssse3_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1, x2, x3, x4, x5, x6, x7;
	for (y = 0; y < height; y += 2) {
		ycp = (short*)pixel + width * y * 3;
		ycpw= ycp + width*3;
		Y   = (short*)dst_Y + width * y;
		C   = (short*)dst_C + width * y / 2;
		for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
			x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

			_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

			x4 = MASK_YCP2Y;
			x0 = select_by_mask(x1, x2, x4);
			x7 = _mm_srli_si128(x4, 4);
			x0 = select_by_mask(x0, x3, x7);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x5 = MASK_YCP2UV;
			x6 = select_by_mask(x1, x2, x5);
			x7 = _mm_srli_si128(x5, 4);
			x6 = select_by_mask(x6, x3, x7); 
			x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

				//Y 1行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
				x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
				x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_stream_si128((__m128i *)(Y + x), x0);

			x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

			x0 = select_by_mask(x1, x2, x4);
			x7 = _mm_srli_si128(x4, 4);
			x0 = select_by_mask(x0, x3, x7);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x1 = select_by_mask(x1, x2, x5);
			x7 = _mm_srli_si128(x5, 4);
			x1 = select_by_mask(x1, x3, x7);
			x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);

			//UV 1行目 + 2行目
			x6 = _mm_add_epi16(x1, x6);
				
				//Y 2行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
				x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
				x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_stream_si128((__m128i *)(Y + x + width), x0);

			x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_UV_F_MA);
			x0 = _mm_srai_epi32(x0, UV_F_RSH_10);

			x7 = _mm_madd_epi16(x7, xC_UV_F_MA);
			x7 = _mm_srai_epi32(x7, UV_F_RSH_10);

			x0 = _mm_packs_epi32(x0, x7);

			x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
			x0 = _mm_add_epi16(x0, x1);
			x7 = _mm_cmpeq_epi8(x7, x7);
			x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
			x0 = _mm_min_epi16(x0, x7);
			x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_stream_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_10bit_full_ssse3_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1, x2, x3, x4, x5, x6, x7;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			ycp = (short*)pixel + width * (y + i) * 3;
			ycpw= ycp + width*2*3;
			Y   = (short*)dst_Y + width * (y + i);
			C   = (short*)dst_C + width * (y + i*2) / 2;
			for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
				x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

				_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

				x4 = MASK_YCP2Y;
				x0 = select_by_mask(x1, x2, x4);
				x7 = _mm_srli_si128(x4, 4);
				x0 = select_by_mask(x0, x3, x7);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x5 = MASK_YCP2UV;
				x6 = select_by_mask(x1, x2, x5);
				x7 = _mm_srli_si128(x5, 4);
				x6 = select_by_mask(x6, x3, x7); 
				x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

					//Y 1行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
					x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
					x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_stream_si128((__m128i *)(Y + x), x0);

				x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

				x0 = select_by_mask(x1, x2, x4);
				x7 = _mm_srli_si128(x4, 4);
				x0 = select_by_mask(x0, x3, x7);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x1 = select_by_mask(x1, x2, x5);
				x7 = _mm_srli_si128(x5, 4);
				x1 = select_by_mask(x1, x3, x7);
				x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);

				//UV 1行目 + 3行目
				x6 = _mm_add_epi16(x1, x6);

					//Y 3行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
					x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
					x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_stream_si128((__m128i *)(Y + x + width*2), x0);

				x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_UV_F_MA);
				x0 = _mm_srai_epi32(x0, UV_F_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_UV_F_MA);
				x7 = _mm_srai_epi32(x7, UV_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

				_mm_stream_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_10bit_full_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m128i x0, x1, x2, x3, x6, x7;
	for (y = 0; y < height; y += 2) {
		ycp = (short*)pixel + width * y * 3;
		ycpw= ycp + width*3;
		Y   = (short*)dst_Y + width * y;
		C   = (short*)dst_C + width * y / 2;
		for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
			x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

			_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

			x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
			x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x6 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
			x6 = _mm_blend_epi16(x6, x3, MASK_INT_UV>>2); 
			x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

				//Y 1行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
				x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
				x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_storeu_si128((__m128i *)(Y + x), x0);

			x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

			x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
			x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
			x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
			x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);//UV2行目

			//UV 1行目 + 2行目
			x6 = _mm_add_epi16(x1, x6);
				
				//Y 2行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
				x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
				x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_storeu_si128((__m128i *)(Y + x + width), x0);

			x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_UV_F_MA);
			x0 = _mm_srai_epi32(x0, UV_F_RSH_10);

			x7 = _mm_madd_epi16(x7, xC_UV_F_MA);
			x7 = _mm_srai_epi32(x7, UV_F_RSH_10);

			x0 = _mm_packs_epi32(x0, x7);

			x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
			x0 = _mm_add_epi16(x0, x1);
			x7 = _mm_cmpeq_epi8(x7, x7);
			x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
			x0 = _mm_min_epi16(x0, x7);
			x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_storeu_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_10bit_full_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m128i x0, x1, x2, x3, x6, x7;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			ycp = (short*)pixel + width * (y + i) * 3;
			ycpw= ycp + width*2*3;
			Y   = (short*)dst_Y + width * (y + i);
			C   = (short*)dst_C + width * (y + i*2) / 2;
			for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
				x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

				_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

				x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
				x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x6 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
				x6 = _mm_blend_epi16(x6, x3, MASK_INT_UV>>2); 
				x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

					//Y 1行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
					x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
					x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_storeu_si128((__m128i *)(Y + x), x0);

				x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

				x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
				x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
				x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
				x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);//UV3行目

				//UV 1行目 + 3行目
				x6 = _mm_add_epi16(x1, x6);

					//Y 3行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
					x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
					x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_storeu_si128((__m128i *)(Y + x + width*2), x0);

				x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_UV_F_MA);
				x0 = _mm_srai_epi32(x0, UV_F_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_UV_F_MA);
				x7 = _mm_srai_epi32(x7, UV_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

				_mm_storeu_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_10bit_full_sse4_1_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m128i x0, x1, x2, x3, x6, x7;
	for (y = 0; y < height; y += 2) {
		ycp = (short*)pixel + width * y * 3;
		ycpw= ycp + width*3;
		Y   = (short*)dst_Y + width * y;
		C   = (short*)dst_C + width * y / 2;
		for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
			x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

			_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

			x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
			x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x6 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
			x6 = _mm_blend_epi16(x6, x3, MASK_INT_UV>>2); 
			x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

				//Y 1行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
				x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
				x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_stream_si128((__m128i *)(Y + x), x0);

			x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
			x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
			x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

			x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
			x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
			x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

			x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
			x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
			x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);//UV2行目

			//UV 1行目 + 2行目
			x6 = _mm_add_epi16(x1, x6);
				
				//Y 2行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
				x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
				x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_stream_si128((__m128i *)(Y + x + width), x0);

			x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_UV_F_MA);
			x0 = _mm_srai_epi32(x0, UV_F_RSH_10);

			x7 = _mm_madd_epi16(x7, xC_UV_F_MA);
			x7 = _mm_srai_epi32(x7, UV_F_RSH_10);

			x0 = _mm_packs_epi32(x0, x7);

			x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
			x0 = _mm_add_epi16(x0, x1);
			x7 = _mm_cmpeq_epi8(x7, x7);
			x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
			x0 = _mm_min_epi16(x0, x7);
			x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

			_mm_stream_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_10bit_full_sse4_1_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m128i x0, x1, x2, x3, x6, x7;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			ycp = (short*)pixel + width * (y + i) * 3;
			ycpw= ycp + width*2*3;
			Y   = (short*)dst_Y + width * (y + i);
			C   = (short*)dst_C + width * (y + i*2) / 2;
			for (x = 0; x < width; x += 8, ycp += 24, ycpw += 24) {
				x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

				_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

				x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
				x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x6 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
				x6 = _mm_blend_epi16(x6, x3, MASK_INT_UV>>2); 
				x6 = _mm_shuffle_epi8(x6, SUFFLE_YCP_UV);//UV1行目

					//Y 1行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
					x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
					x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_stream_si128((__m128i *)(Y + x), x0);

				x1 = _mm_loadu_si128((const __m128i *)(ycpw +  0));
				x2 = _mm_loadu_si128((const __m128i *)(ycpw +  8));
				x3 = _mm_loadu_si128((const __m128i *)(ycpw + 16));

				x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
				x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
				x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

				x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
				x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
				x1 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);//UV3行目

				//UV 1行目 + 3行目
				x6 = _mm_add_epi16(x1, x6);

					//Y 3行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
					x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

					x7 = _mm_madd_epi16(x7, xC_Y_F_MA);
					x7 = _mm_srai_epi32(x7, Y_F_RSH_10);

					x0 = _mm_packs_epi32(x0, x7);

					x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
					x0 = _mm_add_epi16(x0, x1);
					x7 = _mm_cmpeq_epi8(x7, x7);
					x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
					x0 = _mm_min_epi16(x0, x7);
					x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

					_mm_stream_si128((__m128i *)(Y + x + width*2), x0);

				x0 = _mm_add_epi16(x6, xC_UV_OFFSET);

				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_UV_F_MA);
				x0 = _mm_srai_epi32(x0, UV_F_RSH_10);

				x7 = _mm_madd_epi16(x7, xC_UV_F_MA);
				x7 = _mm_srai_epi32(x7, UV_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x7);

				x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
				x0 = _mm_add_epi16(x0, x1);
				x7 = _mm_cmpeq_epi8(x7, x7);
				x7 = _mm_srli_epi16(x7, RSFT_LIMIT_10);
				x0 = _mm_min_epi16(x0, x7);
				x0 = _mm_max_epi16(x0, _mm_xor_si128(x7, x7));

				_mm_stream_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i, k;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1;
	for (y = 0; y < height; y += 2) {
		k = width * y;
		for (i = 0; i < 2; i++) {
			ycp = (short *)pixel + (k + width*i)*3;
			Y   = (short *)dst_Y + (k + width*i);
			for (x = 0; x < width; x += 8, ycp += 24) {
				_mm_set_ycp_y(x0, ycp);

				x1 = _mm_unpackhi_epi16(x0, xC_ONE);
				x0 = _mm_unpacklo_epi16(x0, xC_ONE);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

				x1 = _mm_madd_epi16(x1, xC_Y_L_MA);
				x1 = _mm_srai_epi32(x1, Y_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x1);

				x0 = _mm_add_epi16(x0, xC_YCC_10);
				x0 = _mm_min_epi16(x0, xC_LIMITMAX);
				x0 = _mm_max_epi16(x0, xC_ZERO);

				_mm_storeu_si128((__m128i *)(Y + x), x0);
			}
		}

		ycp = (short*)pixel + k*3;
		ycpw = ycp + width*3;
		
		C = (short*)dst_C + (k>>1);

		for (x = 0; x < width; x += 8, ycpw += 24, ycp += 24) {
			_mm_set_ycp_c(x0, ycp );
			_mm_set_ycp_c(x1, ycpw);

			x0 = _mm_add_epi16(x0, x1);

			x0 = _mm_add_epi16(x0, xC_UV_OFFSET);

			x1 = _mm_unpackhi_epi16(x0, xC_ONE);
			x0 = _mm_unpacklo_epi16(x0, xC_ONE);

			x0 = _mm_madd_epi16(x0, xC_UV_L_MA);
			x0 = _mm_srai_epi32(x0, UV_L_RSH_10);

			x1 = _mm_madd_epi16(x1, xC_UV_L_MA);
			x1 = _mm_srai_epi32(x1, UV_L_RSH_10);

			x0 = _mm_packs_epi32(x0, x1);

			x0 = _mm_add_epi16(x0, xC_YCC_10);
			x0 = _mm_min_epi16(x0, xC_LIMITMAX);
			x0 = _mm_max_epi16(x0, xC_ZERO);

			_mm_storeu_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x = 0, y = 0, i = 0, j = 0, k = 0;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1;
	for (y = 0; y < height; y += 4) {
		k = width * y;
		for (j = 0; j < 2; j++) {
			//1+j行目と3+j行目を処理
			for (i = 0; i < 2; i++) {
				ycp = (short *)pixel + (k + width*(2*i+j))*3;
				Y   = (short *)dst_Y +  k + width*(2*i+j);
				for (x = 0; x < width; x += 8, ycp += 24) {
					_mm_set_ycp_y(x0, ycp);

					x1 = _mm_unpackhi_epi16(x0, xC_ONE);
					x0 = _mm_unpacklo_epi16(x0, xC_ONE);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

					x1 = _mm_madd_epi16(x1, xC_Y_L_MA);
					x1 = _mm_srai_epi32(x1, Y_L_RSH_10);

					x0 = _mm_packs_epi32(x0, x1);

					x0 = _mm_add_epi16(x0, xC_YCC_10);
					x0 = _mm_min_epi16(x0, xC_LIMITMAX);
					x0 = _mm_max_epi16(x0, xC_ZERO);

					_mm_storeu_si128((__m128i *)(Y + x), x0);
				}
			}

			ycp = (short*)pixel + (k + width*j)*3;
			ycpw = ycp + width*3*2;

			C = (short*)dst_C + ((k + width*j*2)>>1);

			for (x = 0; x < width; x += 8, ycpw += 24, ycp += 24) {
				_mm_set_ycp_c(x0, ycp );
				_mm_set_ycp_c(x1, ycpw);

				x0 = _mm_add_epi16(x0, x1);

				x0 = _mm_add_epi16(x0, xC_UV_OFFSET);

				x1 = _mm_unpackhi_epi16(x0, xC_ONE);
				x0 = _mm_unpacklo_epi16(x0, xC_ONE);

				x0 = _mm_madd_epi16(x0, xC_UV_L_MA);
				x0 = _mm_srai_epi32(x0, UV_L_RSH_10);

				x1 = _mm_madd_epi16(x1, xC_UV_L_MA);
				x1 = _mm_srai_epi32(x1, UV_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x1);

				x0 = _mm_add_epi16(x0, xC_YCC_10);
				x0 = _mm_min_epi16(x0, xC_LIMITMAX);
				x0 = _mm_max_epi16(x0, xC_ZERO);

				_mm_storeu_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i, k;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1;
	for (y = 0; y < height; y += 2) {
		k = width * y;
		for (i = 0; i < 2; i++) {
			ycp = (short *)pixel + (k + width*i)*3;
			Y   = (short *)dst_Y + (k + width*i);
			for (x = 0; x < width; x += 8, ycp += 24) {
				_mm_set_ycp_y(x0, ycp);

				x1 = _mm_unpackhi_epi16(x0, xC_ONE);
				x0 = _mm_unpacklo_epi16(x0, xC_ONE);

				x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
				x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

				x1 = _mm_madd_epi16(x1, xC_Y_F_MA);
				x1 = _mm_srai_epi32(x1, Y_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x1);

				x0 = _mm_add_epi16(x0, xC_YCC_10);
				x0 = _mm_min_epi16(x0, xC_LIMITMAX);
				x0 = _mm_max_epi16(x0, xC_ZERO);

				_mm_storeu_si128((__m128i *)(Y + x), x0);
			}
		}

		ycp = (short*)pixel + k*3;
		ycpw = ycp + width*3;
		
		C = (short*)dst_C + (k>>1);

		for (x = 0; x < width; x += 8, ycpw += 24, ycp += 24) {
			_mm_set_ycp_c(x0, ycp );
			_mm_set_ycp_c(x1, ycpw);

			x0 = _mm_add_epi16(x0, x1);

			x0 = _mm_add_epi16(x0, xC_UV_OFFSET);

			x1 = _mm_unpackhi_epi16(x0, xC_ONE);
			x0 = _mm_unpacklo_epi16(x0, xC_ONE);

			x0 = _mm_madd_epi16(x0, xC_UV_F_MA);
			x0 = _mm_srai_epi32(x0, UV_F_RSH_10);

			x1 = _mm_madd_epi16(x1, xC_UV_F_MA);
			x1 = _mm_srai_epi32(x1, UV_F_RSH_10);

			x0 = _mm_packs_epi32(x0, x1);

			x0 = _mm_add_epi16(x0, xC_YCC_10);
			x0 = _mm_min_epi16(x0, xC_LIMITMAX);
			x0 = _mm_max_epi16(x0, xC_ZERO);

			_mm_storeu_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x = 0, y = 0, i = 0, j = 0, k = 0;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1;
	for (y = 0; y < height; y += 4) {
		k = width * y;
		for (j = 0; j < 2; j++) {
			for (i = 0; i < 2; i++) {
				ycp = (short *)pixel + (k + width*(2*i+j))*3;
				Y   = (short *)dst_Y +  k + width*(2*i+j);
				for (x = 0; x < width; x += 8, ycp += 24) {
					_mm_set_ycp_y(x0, ycp);

					x1 = _mm_unpackhi_epi16(x0, xC_ONE);
					x0 = _mm_unpacklo_epi16(x0, xC_ONE);

					x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
					x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

					x1 = _mm_madd_epi16(x1, xC_Y_F_MA);
					x1 = _mm_srai_epi32(x1, Y_F_RSH_10);

					x0 = _mm_packs_epi32(x0, x1);

					x0 = _mm_add_epi16(x0, xC_YCC_10);
					x0 = _mm_min_epi16(x0, xC_LIMITMAX);
					x0 = _mm_max_epi16(x0, xC_ZERO);

					_mm_storeu_si128((__m128i *)(Y + x), x0);
				}
			}

			ycp = (short*)pixel + (k + width*j)*3;
			ycpw = ycp + width*3*2;

			C = (short*)dst_C + ((k + width*j*2)>>1);

			for (x = 0; x < width; x += 8, ycpw += 24, ycp += 24) {
				_mm_set_ycp_c(x0, ycp );
				_mm_set_ycp_c(x1, ycpw);

				x0 = _mm_add_epi16(x0, x1);

				x0 = _mm_add_epi16(x0, xC_UV_OFFSET);

				x1 = _mm_unpackhi_epi16(x0, xC_ONE);
				x0 = _mm_unpacklo_epi16(x0, xC_ONE);

				x0 = _mm_madd_epi16(x0, xC_UV_F_MA);
				x0 = _mm_srai_epi32(x0, UV_F_RSH_10);

				x1 = _mm_madd_epi16(x1, xC_UV_F_MA);
				x1 = _mm_srai_epi32(x1, UV_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x1);

				x0 = _mm_add_epi16(x0, xC_YCC_10);
				x0 = _mm_min_epi16(x0, xC_LIMITMAX);
				x0 = _mm_max_epi16(x0, xC_ZERO);

				_mm_storeu_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_10bit_sse2_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i, k;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1;
	for (y = 0; y < height; y += 2) {
		k = width * y;
		for (i = 0; i < 2; i++) {
			ycp = (short *)pixel + (k + width*i)*3;
			Y   = (short *)dst_Y + (k + width*i);
			for (x = 0; x < width; x += 8, ycp += 24) {
				_mm_set_ycp_y(x0, ycp);

				x1 = _mm_unpackhi_epi16(x0, xC_ONE);
				x0 = _mm_unpacklo_epi16(x0, xC_ONE);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

				x1 = _mm_madd_epi16(x1, xC_Y_L_MA);
				x1 = _mm_srai_epi32(x1, Y_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x1);

				x0 = _mm_add_epi16(x0, xC_YCC_10);
				x0 = _mm_min_epi16(x0, xC_LIMITMAX);
				x0 = _mm_max_epi16(x0, xC_ZERO);

				_mm_stream_si128((__m128i *)(Y + x), x0);
			}
		}

		ycp = (short*)pixel + k*3;
		ycpw = ycp + width*3;
		
		C = (short*)dst_C + (k>>1);

		for (x = 0; x < width; x += 8, ycpw += 24, ycp += 24) {
			_mm_set_ycp_c(x0, ycp );
			_mm_set_ycp_c(x1, ycpw);

			x0 = _mm_add_epi16(x0, x1);

			x0 = _mm_add_epi16(x0, xC_UV_OFFSET);

			x1 = _mm_unpackhi_epi16(x0, xC_ONE);
			x0 = _mm_unpacklo_epi16(x0, xC_ONE);

			x0 = _mm_madd_epi16(x0, xC_UV_L_MA);
			x0 = _mm_srai_epi32(x0, UV_L_RSH_10);

			x1 = _mm_madd_epi16(x1, xC_UV_L_MA);
			x1 = _mm_srai_epi32(x1, UV_L_RSH_10);

			x0 = _mm_packs_epi32(x0, x1);

			x0 = _mm_add_epi16(x0, xC_YCC_10);
			x0 = _mm_min_epi16(x0, xC_LIMITMAX);
			x0 = _mm_max_epi16(x0, xC_ZERO);

			_mm_stream_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_10bit_sse2_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x = 0, y = 0, i = 0, j = 0, k = 0;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1;
	for (y = 0; y < height; y += 4) {
		k = width * y;
		for (j = 0; j < 2; j++) {
			for (i = 0; i < 2; i++) {
				ycp = (short *)pixel + (k + width*(2*i+j))*3;
				Y   = (short *)dst_Y +  k + width*(2*i+j);
				for (x = 0; x < width; x += 8, ycp += 24) {
					_mm_set_ycp_y(x0, ycp);

					x1 = _mm_unpackhi_epi16(x0, xC_ONE);
					x0 = _mm_unpacklo_epi16(x0, xC_ONE);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

					x1 = _mm_madd_epi16(x1, xC_Y_L_MA);
					x1 = _mm_srai_epi32(x1, Y_L_RSH_10);

					x0 = _mm_packs_epi32(x0, x1);

					x0 = _mm_add_epi16(x0, xC_YCC_10);
					x0 = _mm_min_epi16(x0, xC_LIMITMAX);
					x0 = _mm_max_epi16(x0, xC_ZERO);

					_mm_stream_si128((__m128i *)(Y + x), x0);
				}
			}

			ycp = (short*)pixel + (k + width*j)*3;
			ycpw = ycp + width*3*2;

			C = (short*)dst_C + ((k + width*j*2)>>1);

			for (x = 0; x < width; x += 8, ycpw += 24, ycp += 24) {
				_mm_set_ycp_c(x0, ycp );
				_mm_set_ycp_c(x1, ycpw);

				x0 = _mm_add_epi16(x0, x1);

				x0 = _mm_add_epi16(x0, xC_UV_OFFSET);

				x1 = _mm_unpackhi_epi16(x0, xC_ONE);
				x0 = _mm_unpacklo_epi16(x0, xC_ONE);

				x0 = _mm_madd_epi16(x0, xC_UV_L_MA);
				x0 = _mm_srai_epi32(x0, UV_L_RSH_10);

				x1 = _mm_madd_epi16(x1, xC_UV_L_MA);
				x1 = _mm_srai_epi32(x1, UV_L_RSH_10);

				x0 = _mm_packs_epi32(x0, x1);

				x0 = _mm_add_epi16(x0, xC_YCC_10);
				x0 = _mm_min_epi16(x0, xC_LIMITMAX);
				x0 = _mm_max_epi16(x0, xC_ZERO);

				_mm_stream_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_10bit_full_sse2_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i, k;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1;
	for (y = 0; y < height; y += 2) {
		k = width * y;
		for (i = 0; i < 2; i++) {
			ycp = (short *)pixel + (k + width*i)*3;
			Y   = (short *)dst_Y + (k + width*i);
			for (x = 0; x < width; x += 8, ycp += 24) {
				_mm_set_ycp_y(x0, ycp);

				x1 = _mm_unpackhi_epi16(x0, xC_ONE);
				x0 = _mm_unpacklo_epi16(x0, xC_ONE);

				x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
				x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

				x1 = _mm_madd_epi16(x1, xC_Y_F_MA);
				x1 = _mm_srai_epi32(x1, Y_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x1);

				x0 = _mm_add_epi16(x0, xC_YCC_10);
				x0 = _mm_min_epi16(x0, xC_LIMITMAX);
				x0 = _mm_max_epi16(x0, xC_ZERO);

				_mm_stream_si128((__m128i *)(Y + x), x0);
			}
		}

		ycp = (short*)pixel + k*3;
		ycpw = ycp + width*3;
		
		C = (short*)dst_C + (k>>1);

		for (x = 0; x < width; x += 8, ycpw += 24, ycp += 24) {
			_mm_set_ycp_c(x0, ycp );
			_mm_set_ycp_c(x1, ycpw);

			x0 = _mm_add_epi16(x0, x1);

			x0 = _mm_add_epi16(x0, xC_UV_OFFSET);

			x1 = _mm_unpackhi_epi16(x0, xC_ONE);
			x0 = _mm_unpacklo_epi16(x0, xC_ONE);

			x0 = _mm_madd_epi16(x0, xC_UV_F_MA);
			x0 = _mm_srai_epi32(x0, UV_F_RSH_10);

			x1 = _mm_madd_epi16(x1, xC_UV_F_MA);
			x1 = _mm_srai_epi32(x1, UV_F_RSH_10);

			x0 = _mm_packs_epi32(x0, x1);

			x0 = _mm_add_epi16(x0, xC_YCC_10);
			x0 = _mm_min_epi16(x0, xC_LIMITMAX);
			x0 = _mm_max_epi16(x0, xC_ZERO);

			_mm_stream_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_10bit_full_sse2_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x = 0, y = 0, i = 0, j = 0, k = 0;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	__m128i x0, x1;
	for (y = 0; y < height; y += 4) {
		k = width * y;
		for (j = 0; j < 2; j++) {
			for (i = 0; i < 2; i++) {
				ycp = (short *)pixel + (k + width*(2*i+j))*3;
				Y   = (short *)dst_Y +  k + width*(2*i+j);
				for (x = 0; x < width; x += 8, ycp += 24) {
					_mm_set_ycp_y(x0, ycp);

					x1 = _mm_unpackhi_epi16(x0, xC_ONE);
					x0 = _mm_unpacklo_epi16(x0, xC_ONE);

					x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
					x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

					x1 = _mm_madd_epi16(x1, xC_Y_F_MA);
					x1 = _mm_srai_epi32(x1, Y_F_RSH_10);

					x0 = _mm_packs_epi32(x0, x1);

					x0 = _mm_add_epi16(x0, xC_YCC_10);
					x0 = _mm_min_epi16(x0, xC_LIMITMAX);
					x0 = _mm_max_epi16(x0, xC_ZERO);

					_mm_stream_si128((__m128i *)(Y + x), x0);
				}
			}

			ycp = (short*)pixel + (k + width*j)*3;
			ycpw = ycp + width*3*2;

			C = (short*)dst_C + ((k + width*j*2)>>1);

			for (x = 0; x < width; x += 8, ycpw += 24, ycp += 24) {
				_mm_set_ycp_c(x0, ycp );
				_mm_set_ycp_c(x1, ycpw);

				x0 = _mm_add_epi16(x0, x1);

				x0 = _mm_add_epi16(x0, xC_UV_OFFSET);

				x1 = _mm_unpackhi_epi16(x0, xC_ONE);
				x0 = _mm_unpacklo_epi16(x0, xC_ONE);

				x0 = _mm_madd_epi16(x0, xC_UV_F_MA);
				x0 = _mm_srai_epi32(x0, UV_F_RSH_10);

				x1 = _mm_madd_epi16(x1, xC_UV_F_MA);
				x1 = _mm_srai_epi32(x1, UV_F_RSH_10);

				x0 = _mm_packs_epi32(x0, x1);

				x0 = _mm_add_epi16(x0, xC_YCC_10);
				x0 = _mm_min_epi16(x0, xC_LIMITMAX);
				x0 = _mm_max_epi16(x0, xC_ZERO);

				_mm_stream_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_yuv444(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	PIXEL_YC *ycp_fin = (PIXEL_YC *)pixel + width * height;
	BYTE *Y = pixel_data->data[0];
	BYTE *U = pixel_data->data[1];
	BYTE *V = pixel_data->data[2];
	for (PIXEL_YC *ycp = (PIXEL_YC *)pixel; ycp < ycp_fin; ycp++, Y++, U++, V++) {
		*Y = (BYTE)pixel_YC48_to_YUV(ycp->y,                   Y_L_MUL,  Y_L_ADD,      Y_L_RSH_8,      Y_L_YCC_8, 0, LIMIT_8);
		*U = (BYTE)pixel_YC48_to_YUV(ycp->cb + UV_OFFSET_444, UV_L_MUL, UV_L_ADD_444, UV_L_RSH_8_444, UV_L_YCC_8, 0, LIMIT_8);
		*V = (BYTE)pixel_YC48_to_YUV(ycp->cr + UV_OFFSET_444, UV_L_MUL, UV_L_ADD_444, UV_L_RSH_8_444, UV_L_YCC_8, 0, LIMIT_8);
	}
}

void convert_yc48_to_yuv444_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	PIXEL_YC *ycp_fin = (PIXEL_YC *)pixel + width * height;
	short *Y = (short *)pixel_data->data[0];
	short *U = (short *)pixel_data->data[1];
	short *V = (short *)pixel_data->data[2];
	for (PIXEL_YC *ycp = (PIXEL_YC *)pixel; ycp < ycp_fin; ycp++, Y++, U++, V++) {
		*Y = (short)pixel_YC48_to_YUV(ycp->y,                   Y_L_MUL,  Y_L_ADD,      Y_L_RSH_10,      Y_L_YCC_10, 0, LIMIT_10);
		*U = (short)pixel_YC48_to_YUV(ycp->cb + UV_OFFSET_444, UV_L_MUL, UV_L_ADD_444, UV_L_RSH_10_444, UV_L_YCC_10, 0, LIMIT_10);
		*V = (short)pixel_YC48_to_YUV(ycp->cr + UV_OFFSET_444, UV_L_MUL, UV_L_ADD_444, UV_L_RSH_10_444, UV_L_YCC_10, 0, LIMIT_10);
	}
}

void convert_yc48_to_yuv444_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	PIXEL_YC *ycp_fin = (PIXEL_YC *)pixel + width * height;
	short *Y = (short *)pixel_data->data[0];
	short *U = (short *)pixel_data->data[1];
	short *V = (short *)pixel_data->data[2];
	for (PIXEL_YC *ycp = (PIXEL_YC *)pixel; ycp < ycp_fin; ycp++, Y++, U++, V++) {
		*Y = (short)pixel_YC48_to_YUV(ycp->y,                   Y_F_MUL,  Y_F_ADD,      Y_F_RSH_10,      Y_F_YCC_10, 0, LIMIT_10);
		*U = (short)pixel_YC48_to_YUV(ycp->cb + UV_OFFSET_444, UV_F_MUL, UV_F_ADD_444, UV_F_RSH_10_444, UV_F_YCC_10, 0, LIMIT_10);
		*V = (short)pixel_YC48_to_YUV(ycp->cr + UV_OFFSET_444, UV_F_MUL, UV_F_ADD_444, UV_F_RSH_10_444, UV_F_YCC_10, 0, LIMIT_10);
	}
}

void convert_yc48_to_yuv444_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	BYTE *Y = pixel_data->data[0];
	BYTE *C[2] = { pixel_data->data[1], pixel_data->data[2] };
	short *ycp;
	short *const ycp_fin = (short *)pixel + width * height * 3;
	int i;
	__m128i x0, x1, x2;
	for (ycp = (short*)pixel; ycp < ycp_fin; ycp += 48, Y += 16, C[0] += 16, C[1] += 16) {
		//Y
		_mm_set_ycp_y(x0, ycp);

		x1 = _mm_unpackhi_epi16(x0, xC_ONE);
		x0 = _mm_unpacklo_epi16(x0, xC_ONE);

		x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
		x0 = _mm_srai_epi32(x0, Y_L_RSH_8);

		x1 = _mm_madd_epi16(x1, xC_Y_L_MA);
		x1 = _mm_srai_epi32(x1, Y_L_RSH_8);

		x0 = _mm_packs_epi32(x0, x1);

		x0 = _mm_add_epi16(x0, xC_YCC_8);

		_mm_set_ycp_y(x2, ycp + 24);

		x1 = _mm_unpackhi_epi16(x2, xC_ONE);
		x2 = _mm_unpacklo_epi16(x2, xC_ONE);

		x2 = _mm_madd_epi16(x2, xC_Y_L_MA);
		x2 = _mm_srai_epi32(x2, Y_L_RSH_8);

		x1 = _mm_madd_epi16(x1, xC_Y_L_MA);
		x1 = _mm_srai_epi32(x1, Y_L_RSH_8);

		x2 = _mm_packs_epi32(x2, x1);

		x2 = _mm_add_epi16(x2, xC_YCC_8);

		x0 = _mm_packus_epi16(x0, x2);

		_mm_stream_si128((__m128i *)Y, x0);

		//UV
		for (i = 0; i < 2; i++) {
			_mm_set_ycp_y(x0, ycp+i+1);
			x0 = _mm_add_epi16(x0, xC_UV_OFFSET_444);

			x1 = _mm_unpackhi_epi16(x0, xC_ONE);
			x0 = _mm_unpacklo_epi16(x0, xC_ONE);

			x0 = _mm_madd_epi16(x0, xC_UV_L_MA_444);
			x0 = _mm_srai_epi32(x0, UV_L_RSH_8_444);

			x1 = _mm_madd_epi16(x1, xC_UV_L_MA_444);
			x1 = _mm_srai_epi32(x1, UV_L_RSH_8_444);

			x0 = _mm_packs_epi32(x0, x1);

			x0 = _mm_add_epi16(x0, xC_YCC_8);

			_mm_set_ycp_y(x2, ycp+i+25);			
			x2 = _mm_add_epi16(x2, xC_UV_OFFSET_444);

			x1 = _mm_unpackhi_epi16(x2, xC_ONE);
			x2 = _mm_unpacklo_epi16(x2, xC_ONE);

			x2 = _mm_madd_epi16(x2, xC_UV_L_MA_444);
			x2 = _mm_srai_epi32(x2, UV_L_RSH_8_444);

			x1 = _mm_madd_epi16(x1, xC_UV_L_MA_444);
			x1 = _mm_srai_epi32(x1, UV_L_RSH_8_444);

			x2 = _mm_packs_epi32(x2, x1);

			x2 = _mm_add_epi16(x2, xC_YCC_8);		

			x0 = _mm_packus_epi16(x0, x2);

			_mm_stream_si128((__m128i *)C[i], x0);
		}
	}
}

void convert_yc48_to_yuv444_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *Y      = (short *)pixel_data->data[0];
	short *C[2] = { (short *)pixel_data->data[1], (short *)pixel_data->data[2] };
	short *ycp;
	short *const ycp_fin = (short *)pixel + width * height * 3;
	int i;
	__m128i x0, x1;
	for (ycp = (short *)pixel; ycp < ycp_fin; ycp += 24, Y += 8, C[0] += 8, C[1] += 8) {
		//Y
		_mm_set_ycp_y(x0, ycp);

		x1 = _mm_unpackhi_epi16(x0, xC_ONE);
		x0 = _mm_unpacklo_epi16(x0, xC_ONE);

		x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
		x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

		x1 = _mm_madd_epi16(x1, xC_Y_L_MA);
		x1 = _mm_srai_epi32(x1, Y_L_RSH_10);

		x0 = _mm_packs_epi32(x0, x1);

		x0 = _mm_add_epi16(x0, xC_YCC_10);
		x0 = _mm_min_epi16(x0, xC_LIMITMAX);
		x0 = _mm_max_epi16(x0, xC_ZERO);

		_mm_stream_si128((__m128i *)Y, x0);

		//UV
		for (i = 0; i < 2; i++) {
			_mm_set_ycp_y(x0, ycp+i+1);
			x0 = _mm_add_epi16(x0, xC_UV_OFFSET_444);

			x1 = _mm_unpackhi_epi16(x0, xC_ONE);
			x0 = _mm_unpacklo_epi16(x0, xC_ONE);

			x0 = _mm_madd_epi16(x0, xC_UV_L_MA_444);
			x0 = _mm_srai_epi32(x0, UV_L_RSH_10_444);

			x1 = _mm_madd_epi16(x1, xC_UV_L_MA_444);
			x1 = _mm_srai_epi32(x1, UV_L_RSH_10_444);

			x0 = _mm_packs_epi32(x0, x1);

			x0 = _mm_add_epi16(x0, xC_YCC_10);
			x0 = _mm_min_epi16(x0, xC_LIMITMAX);
			x0 = _mm_max_epi16(x0, xC_ZERO);

			_mm_stream_si128((__m128i *)C[i], x0);
		}
	}
}

void convert_yc48_to_yuv444_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *Y      = (short *)pixel_data->data[0];
	short *C[2] = { (short *)pixel_data->data[1], (short *)pixel_data->data[2] };
	short *ycp;
	short *const ycp_fin = (short *)pixel + width * height * 3;
	int i;
	__m128i x0, x1;
	for (ycp = (short *)pixel; ycp < ycp_fin; ycp += 24, Y += 8, C[0] += 8, C[1] += 8) {
		//Y
		_mm_set_ycp_y(x0, ycp);

		x1 = _mm_unpackhi_epi16(x0, xC_ONE);
		x0 = _mm_unpacklo_epi16(x0, xC_ONE);

		x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
		x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

		x1 = _mm_madd_epi16(x1, xC_Y_F_MA);
		x1 = _mm_srai_epi32(x1, Y_F_RSH_10);

		x0 = _mm_packs_epi32(x0, x1);

		x0 = _mm_add_epi16(x0, xC_YCC_10);
		x0 = _mm_min_epi16(x0, xC_LIMITMAX);
		x0 = _mm_max_epi16(x0, xC_ZERO);

		_mm_stream_si128((__m128i *)Y, x0);

		//UV
		for (i = 0; i < 2; i++) {
			_mm_set_ycp_y(x0, ycp+i+1);
			x0 = _mm_add_epi16(x0, xC_UV_OFFSET_444);

			x1 = _mm_unpackhi_epi16(x0, xC_ONE);
			x0 = _mm_unpacklo_epi16(x0, xC_ONE);

			x0 = _mm_madd_epi16(x0, xC_UV_F_MA_444);
			x0 = _mm_srai_epi32(x0, UV_F_RSH_10_444);

			x1 = _mm_madd_epi16(x1, xC_UV_F_MA_444);
			x1 = _mm_srai_epi32(x1, UV_F_RSH_10_444);

			x0 = _mm_packs_epi32(x0, x1);

			x0 = _mm_add_epi16(x0, xC_YCC_10);
			x0 = _mm_min_epi16(x0, xC_LIMITMAX);
			x0 = _mm_max_epi16(x0, xC_ZERO);

			_mm_stream_si128((__m128i *)C[i], x0);
		}
	}
}

void convert_yuy2_to_nv16(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	BYTE *p = (BYTE *)pixel;
	const int n = width * height;
	for (int i = 0; i < n; i += 2) {
		dst_Y[i]   = p[i*2 + 0];
		dst_C[i]   = p[i*2 + 1];
		dst_Y[i+1] = p[i*2 + 2];
		dst_C[i+1] = p[i*2 + 3];
	}
}

void convert_yuy2_to_nv16_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	BYTE *p = (BYTE *)pixel;
	BYTE * const p_fin = p + width * height * 2;
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	__m128i x0, x1, x2;
	for (; p < p_fin; p += 32, dst_Y += 16, dst_C += 16) {
		x0 = _mm_loadu_si128((const __m128i *)(p+ 0));    // VYUYVYUYVYUYVYUY
		x1 = _mm_loadu_si128((const __m128i *)(p+16));    // VYUYVYUYVYUYVYUY

		x2 = _mm_unpacklo_epi8(x0, x1); //VVYYUUYYVVYYUUYY
		x1 = _mm_unpackhi_epi8(x0, x1); //VVYYUUYYVVYYUUYY

		x0 = _mm_unpacklo_epi8(x2, x1); //VVVVYYYYUUUUYYYY
		x1 = _mm_unpackhi_epi8(x2, x1); //VVVVYYYYUUUUYYYY

		x2 = _mm_unpacklo_epi8(x0, x1); //UUUUUUUUYYYYYYYY
		x1 = _mm_unpackhi_epi8(x0, x1); //VVVVVVVVYYYYYYYY

		x0 = _mm_unpacklo_epi8(x2, x1); //YYYYYYYYYYYYYYYY
		x1 = _mm_unpackhi_epi8(x2, x1); //VUVUVUVUVUVUVUVU

		_mm_stream_si128((__m128i *)dst_Y, x0);
		_mm_stream_si128((__m128i *)dst_C, x1);
	}
}

void convert_yc48_to_nv16_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	PIXEL_YC *ycp = (PIXEL_YC *)pixel;
	const int n = width * height;
	for (int i = 0; i < n; i += 2) {
		dst_Y[i+0] = (short)pixel_YC48_to_YUV(ycp[i+0].y,                   Y_L_MUL,  Y_L_ADD,      Y_L_RSH_10,      Y_L_YCC_10, 0, LIMIT_10);
		dst_C[i+0] = (short)pixel_YC48_to_YUV(ycp[i+0].cb + UV_OFFSET_444, UV_L_MUL, UV_L_ADD_444, UV_L_RSH_10_444, UV_L_YCC_10, 0, LIMIT_10);
		dst_C[i+1] = (short)pixel_YC48_to_YUV(ycp[i+0].cr + UV_OFFSET_444, UV_L_MUL, UV_L_ADD_444, UV_L_RSH_10_444, UV_L_YCC_10, 0, LIMIT_10);
		dst_Y[i+1] = (short)pixel_YC48_to_YUV(ycp[i+1].y,                   Y_L_MUL,  Y_L_ADD,      Y_L_RSH_10,      Y_L_YCC_10, 0, LIMIT_10);
	}
}

void convert_yc48_to_nv16_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	PIXEL_YC *ycp = (PIXEL_YC *)pixel;
	const int n = width * height;
	for (int i = 0; i < n; i += 2) {
		dst_Y[i+0] = (short)pixel_YC48_to_YUV(ycp[i+0].y,                   Y_F_MUL,  Y_F_ADD,      Y_F_RSH_10,      Y_F_YCC_10, 0, LIMIT_10);
		dst_C[i+0] = (short)pixel_YC48_to_YUV(ycp[i+0].cb + UV_OFFSET_444, UV_F_MUL, UV_F_ADD_444, UV_F_RSH_10_444, UV_F_YCC_10, 0, LIMIT_10);
		dst_C[i+1] = (short)pixel_YC48_to_YUV(ycp[i+0].cr + UV_OFFSET_444, UV_F_MUL, UV_F_ADD_444, UV_F_RSH_10_444, UV_F_YCC_10, 0, LIMIT_10);
		dst_Y[i+1] = (short)pixel_YC48_to_YUV(ycp[i+1].y,                   Y_F_MUL,  Y_F_ADD,      Y_F_RSH_10,      Y_F_YCC_10, 0, LIMIT_10);
	}
}

void convert_yc48_to_nv16_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp = (short *)pixel;
	short * const ycp_fin = ycp + width * height * 3;
	__m128i x0, x2;
	for (; ycp < ycp_fin; ycp += 24, dst_Y += 8, dst_C += 8) {
		_mm_set_ycp_y(x0, ycp);

			//Y 計算
			x2 = _mm_unpackhi_epi16(x0, xC_ONE);
			x0 = _mm_unpacklo_epi16(x0, xC_ONE);

			x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
			x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

			x2 = _mm_madd_epi16(x2, xC_Y_L_MA);
			x2 = _mm_srai_epi32(x2, Y_L_RSH_10);

			x0 = _mm_packs_epi32(x0, x2);

			x0 = _mm_add_epi16(x0, xC_YCC_10);
			x0 = _mm_min_epi16(x0, xC_LIMITMAX);
			x0 = _mm_max_epi16(x0, xC_ZERO);

		_mm_stream_si128((__m128i *)dst_Y, x0);

		_mm_set_ycp_c(x2, ycp);

		x0 = _mm_add_epi16(x2, xC_UV_OFFSET_444);

		x2 = _mm_unpackhi_epi16(x0, xC_ONE);
		x0 = _mm_unpacklo_epi16(x0, xC_ONE);

		x0 = _mm_madd_epi16(x0, xC_UV_L_MA_444);
		x0 = _mm_srai_epi32(x0, UV_L_RSH_10_444);

		x2 = _mm_madd_epi16(x2, xC_UV_L_MA_444);
		x2 = _mm_srai_epi32(x2, UV_L_RSH_10_444);

		x0 = _mm_packs_epi32(x0, x2);

		x0 = _mm_add_epi16(x0, xC_YCC_10);
		x0 = _mm_min_epi16(x0, xC_LIMITMAX);
		x0 = _mm_max_epi16(x0, xC_ZERO);

		_mm_stream_si128((__m128i *)dst_C, x0);
	}
}

void convert_yc48_to_nv16_10bit_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp = (short *)pixel;
	short * const ycp_fin = ycp + width * height * 3;
	__m128i x0, x1, x2, x3, x4;
	for (; ycp < ycp_fin; ycp += 24, dst_Y += 8, dst_C += 8) {
		x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
		x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
		x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

		x4 = MASK_YCP2Y;
		x0 = select_by_mask(x1, x2, x4);
		x4 = _mm_srli_si128(x4, 4);
		x0 = select_by_mask(x0, x3, x4);
		x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

		x4 = MASK_YCP2UV;
		x1 = select_by_mask(x1, x2, x4);
		x4 = _mm_srli_si128(x4, 4);
		x1 = select_by_mask(x1, x3, x4); 
		x3 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);//UV

			//Y 計算
			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x2 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
			x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

			x2 = _mm_madd_epi16(x2, xC_Y_L_MA);
			x2 = _mm_srai_epi32(x2, Y_L_RSH_10);

			x0 = _mm_packs_epi32(x0, x2);

			x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
			x0 = _mm_add_epi16(x0, x1);
			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_LIMIT_10);
			x0 = _mm_min_epi16(x0, x1);
			x0 = _mm_max_epi16(x0, _mm_xor_si128(x1, x1));

		_mm_stream_si128((__m128i *)dst_Y, x0);

		x0 = _mm_add_epi16(x3, xC_UV_OFFSET_444);

		x1 = _mm_cmpeq_epi8(x1, x1);
		x1 = _mm_srli_epi16(x1, RSFT_ONE);
		x2 = _mm_unpackhi_epi16(x0, x1);
		x0 = _mm_unpacklo_epi16(x0, x1);

		x0 = _mm_madd_epi16(x0, xC_UV_L_MA_444);
		x0 = _mm_srai_epi32(x0, UV_L_RSH_10_444);

		x2 = _mm_madd_epi16(x2, xC_UV_L_MA_444);
		x2 = _mm_srai_epi32(x2, UV_L_RSH_10_444);

		x0 = _mm_packs_epi32(x0, x2);

		x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
		x0 = _mm_add_epi16(x0, x1);
		x1 = _mm_cmpeq_epi8(x1, x1);
		x1 = _mm_srli_epi16(x1, RSFT_LIMIT_10);
		x0 = _mm_min_epi16(x0, x1);
		x0 = _mm_max_epi16(x0, _mm_xor_si128(x1, x1));

		_mm_stream_si128((__m128i *)dst_C, x0);
	}
}

void convert_yc48_to_nv16_10bit_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp = (short *)pixel;
	short * const ycp_fin = ycp + width * height * 3;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m128i x0, x1, x2, x3;
	for (; ycp < ycp_fin; ycp += 24, dst_Y += 8, dst_C += 8) {
		x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
		x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
		x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

		x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
		x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
		x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

		x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
		x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
		x3 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);//UV行目

			//Y 計算
			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x2 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_Y_L_MA);
			x0 = _mm_srai_epi32(x0, Y_L_RSH_10);

			x2 = _mm_madd_epi16(x2, xC_Y_L_MA);
			x2 = _mm_srai_epi32(x2, Y_L_RSH_10);

			x0 = _mm_packs_epi32(x0, x2);

			x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
			x0 = _mm_add_epi16(x0, x1);
			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_LIMIT_10);
			x0 = _mm_min_epi16(x0, x1);
			x0 = _mm_max_epi16(x0, _mm_xor_si128(x1, x1));

		_mm_stream_si128((__m128i *)dst_Y, x0);

		x0 = _mm_add_epi16(x3, xC_UV_OFFSET_444);

		x1 = _mm_cmpeq_epi8(x1, x1);
		x1 = _mm_srli_epi16(x1, RSFT_ONE);
		x2 = _mm_unpackhi_epi16(x0, x1);
		x0 = _mm_unpacklo_epi16(x0, x1);

		x0 = _mm_madd_epi16(x0, xC_UV_L_MA_444);
		x0 = _mm_srai_epi32(x0, UV_L_RSH_10_444);

		x2 = _mm_madd_epi16(x2, xC_UV_L_MA_444);
		x2 = _mm_srai_epi32(x2, UV_L_RSH_10_444);

		x0 = _mm_packs_epi32(x0, x2);

		x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
		x0 = _mm_add_epi16(x0, x1);
		x1 = _mm_cmpeq_epi8(x1, x1);
		x1 = _mm_srli_epi16(x1, RSFT_LIMIT_10);
		x0 = _mm_min_epi16(x0, x1);
		x0 = _mm_max_epi16(x0, _mm_xor_si128(x1, x1));

		_mm_stream_si128((__m128i *)dst_C, x0);
	}
}

void convert_yc48_to_nv16_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp = (short *)pixel;
	short * const ycp_fin = ycp + width * height * 3;
	__m128i x0, x2;
	for (; ycp < ycp_fin; ycp += 24, dst_Y += 8, dst_C += 8) {
		_mm_set_ycp_y(x0, ycp);

			//Y 計算
			x2 = _mm_unpackhi_epi16(x0, xC_ONE);
			x0 = _mm_unpacklo_epi16(x0, xC_ONE);

			x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
			x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

			x2 = _mm_madd_epi16(x2, xC_Y_F_MA);
			x2 = _mm_srai_epi32(x2, Y_F_RSH_10);

			x0 = _mm_packs_epi32(x0, x2);

			x0 = _mm_add_epi16(x0, xC_YCC_10);
			x0 = _mm_min_epi16(x0, xC_LIMITMAX);
			x0 = _mm_max_epi16(x0, xC_ZERO);

		_mm_stream_si128((__m128i *)dst_Y, x0);

		_mm_set_ycp_c(x2, ycp);

		x0 = _mm_add_epi16(x2, xC_UV_OFFSET_444);

		x2 = _mm_unpackhi_epi16(x0, xC_ONE);
		x0 = _mm_unpacklo_epi16(x0, xC_ONE);

		x0 = _mm_madd_epi16(x0, xC_UV_F_MA_444);
		x0 = _mm_srai_epi32(x0, UV_F_RSH_10_444);

		x2 = _mm_madd_epi16(x2, xC_UV_F_MA_444);
		x2 = _mm_srai_epi32(x2, UV_F_RSH_10_444);

		x0 = _mm_packs_epi32(x0, x2);

		x0 = _mm_add_epi16(x0, xC_YCC_10);
		x0 = _mm_min_epi16(x0, xC_LIMITMAX);
		x0 = _mm_max_epi16(x0, xC_ZERO);

		_mm_stream_si128((__m128i *)dst_C, x0);
	}
}

void convert_yc48_to_nv16_10bit_full_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp = (short *)pixel;
	short * const ycp_fin = ycp + width * height * 3;
	__m128i x0, x1, x2, x3, x4;
	for (; ycp < ycp_fin; ycp += 24, dst_Y += 8, dst_C += 8) {
		x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
		x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
		x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

		x4 = MASK_YCP2Y;
		x0 = select_by_mask(x1, x2, x4);
		x4 = _mm_srli_si128(x4, 4);
		x0 = select_by_mask(x0, x3, x4);
		x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

		x4 = MASK_YCP2UV;
		x1 = select_by_mask(x1, x2, x4);
		x4 = _mm_srli_si128(x4, 4);
		x1 = select_by_mask(x1, x3, x4); 
		x3 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);//UV

			//Y 計算
			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x2 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
			x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

			x2 = _mm_madd_epi16(x2, xC_Y_F_MA);
			x2 = _mm_srai_epi32(x2, Y_F_RSH_10);

			x0 = _mm_packs_epi32(x0, x2);

			x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
			x0 = _mm_add_epi16(x0, x1);
			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_LIMIT_10);
			x0 = _mm_min_epi16(x0, x1);
			x0 = _mm_max_epi16(x0, _mm_xor_si128(x1, x1));

		_mm_stream_si128((__m128i *)dst_Y, x0);

		x0 = _mm_add_epi16(x3, xC_UV_OFFSET_444);

		x2 = _mm_unpackhi_epi16(x0, xC_ONE);
		x0 = _mm_unpacklo_epi16(x0, xC_ONE);

		x0 = _mm_madd_epi16(x0, xC_UV_F_MA_444);
		x0 = _mm_srai_epi32(x0, UV_F_RSH_10_444);

		x2 = _mm_madd_epi16(x2, xC_UV_F_MA_444);
		x2 = _mm_srai_epi32(x2, UV_F_RSH_10_444);

		x0 = _mm_packs_epi32(x0, x2);

		x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
		x0 = _mm_add_epi16(x0, x1);
		x1 = _mm_cmpeq_epi8(x1, x1);
		x1 = _mm_srli_epi16(x1, RSFT_LIMIT_10);
		x0 = _mm_min_epi16(x0, x1);
		x0 = _mm_max_epi16(x0, _mm_xor_si128(x1, x1));

		_mm_stream_si128((__m128i *)dst_C, x0);
	}
}

void convert_yc48_to_nv16_10bit_full_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp = (short *)pixel;
	short * const ycp_fin = ycp + width * height * 3;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m128i x0, x1, x2, x3;
	for (; ycp < ycp_fin; ycp += 24, dst_Y += 8, dst_C += 8) {
		x1 = _mm_loadu_si128((const __m128i *)(ycp +  0));
		x2 = _mm_loadu_si128((const __m128i *)(ycp +  8));
		x3 = _mm_loadu_si128((const __m128i *)(ycp + 16));

		x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
		x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
		x0 = _mm_shuffle_epi8(x0, SUFFLE_YCP_Y);

		x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
		x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
		x3 = _mm_shuffle_epi8(x1, SUFFLE_YCP_UV);//UV行目

			//Y 計算
			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x2 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_Y_F_MA);
			x0 = _mm_srai_epi32(x0, Y_F_RSH_10);

			x2 = _mm_madd_epi16(x2, xC_Y_F_MA);
			x2 = _mm_srai_epi32(x2, Y_F_RSH_10);

			x0 = _mm_packs_epi32(x0, x2);

			x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
			x0 = _mm_add_epi16(x0, x1);
			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_LIMIT_10);
			x0 = _mm_min_epi16(x0, x1);
			x0 = _mm_max_epi16(x0, _mm_xor_si128(x1, x1));

		_mm_stream_si128((__m128i *)dst_Y, x0);

		x0 = _mm_add_epi16(x3, xC_UV_OFFSET_444);

		x1 = _mm_cmpeq_epi8(x1, x1);
		x1 = _mm_srli_epi16(x1, RSFT_ONE);
		x2 = _mm_unpackhi_epi16(x0, x1);
		x0 = _mm_unpacklo_epi16(x0, x1);

		x0 = _mm_madd_epi16(x0, xC_UV_F_MA_444);
		x0 = _mm_srai_epi32(x0, UV_F_RSH_10_444);

		x2 = _mm_madd_epi16(x2, xC_UV_F_MA_444);
		x2 = _mm_srai_epi32(x2, UV_F_RSH_10_444);

		x0 = _mm_packs_epi32(x0, x2);

		x1 = _mm_slli_epi16(x1, LSFT_YCC_10);
		x0 = _mm_add_epi16(x0, x1);
		x1 = _mm_cmpeq_epi8(x1, x1);
		x1 = _mm_srli_epi16(x1, RSFT_LIMIT_10);
		x0 = _mm_min_epi16(x0, x1);
		x0 = _mm_max_epi16(x0, _mm_xor_si128(x1, x1));

		_mm_stream_si128((__m128i *)dst_C, x0);
	}
}