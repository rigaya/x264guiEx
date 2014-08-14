//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx/2.xx by rigaya
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

/// -- AVX命令を生成するためのファイル ------------------------------------------------------
///
///    コンパイルオプションで /arch:AVX を使用する
///      関数自体はコピペしてきて関数名をavxにするだけの簡単なお仕事
///
/// -----------------------------------------------------------------------------------------

#include "convert.h"
#include "convert_const.h"

//AVXはVC++2010から
#if (_MSC_VER >= 1600)
void convert_yuy2_to_nv12_avx_mod16(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
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

void convert_yuy2_to_nv12_avx(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
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

void convert_yuy2_to_nv12_i_avx_mod16(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	BYTE *p, *pw, *Y, *C;
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	__m128i x0, x1, x2, x3;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			x  = (y + i) * width;
			p  = (BYTE *)frame + (x<<1);
			pw = p + (width<<2);
			Y  = (BYTE *)dst_Y +  x;
			C  = (BYTE *)dst_C + ((x+width*i)>>1);
			for (x = 0; x < width; x += 16, p += 32, pw += 32) {
				//-----------    1+i行目   ---------------
				x0 = _mm_loadu_si128((__m128i *)(p+ 0));    // VYUYVYUYVYUYVYUY
				x1 = _mm_loadu_si128((__m128i *)(p+16));    // VYUYVYUYVYUYVYUY

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
				x0 = _mm_loadu_si128((__m128i *)(pw+ 0));    // VYUYVYUYVYUYVYUY
				x1 = _mm_loadu_si128((__m128i *)(pw+16));    // VYUYVYUYVYUYVYUY

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

				x0 = _mm_unpacklo_epi8(x1, x3);
				x1 = _mm_unpackhi_epi8(x1, x3);
				x0 = _mm_maddubs_epi16(x0, xC_INTERLACE_WEIGHT(i));
				x1 = _mm_maddubs_epi16(x1, xC_INTERLACE_WEIGHT(i));
				x0 = _mm_add_epi16(x0, _mm_set1_epi16(2));
				x1 = _mm_add_epi16(x1, _mm_set1_epi16(2));
				x0 = _mm_srai_epi16(x0, 2);
				x1 = _mm_srai_epi16(x1, 2);
				x0 = _mm_packus_epi16(x0, x1); //VUVUVUVUVUVUVUVU
				_mm_stream_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yuy2_to_nv12_i_avx(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
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
				x0 = _mm_loadu_si128((__m128i *)(p+ 0));    // VYUYVYUYVYUYVYUY
				x1 = _mm_loadu_si128((__m128i *)(p+16));    // VYUYVYUYVYUYVYUY

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
				x0 = _mm_loadu_si128((__m128i *)(pw+ 0));    // VYUYVYUYVYUYVYUY
				x1 = _mm_loadu_si128((__m128i *)(pw+16));    // VYUYVYUYVYUYVYUY

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

				x0 = _mm_unpacklo_epi8(x1, x3);
				x1 = _mm_unpackhi_epi8(x1, x3);
				x0 = _mm_maddubs_epi16(x0, xC_INTERLACE_WEIGHT(i));
				x1 = _mm_maddubs_epi16(x1, xC_INTERLACE_WEIGHT(i));
				x0 = _mm_add_epi16(x0, _mm_set1_epi16(2));
				x1 = _mm_add_epi16(x1, _mm_set1_epi16(2));
				x0 = _mm_srai_epi16(x0, 2);
				x1 = _mm_srai_epi16(x1, 2);
				x0 = _mm_packus_epi16(x0, x1); //VUVUVUVUVUVUVUVU
				_mm_storeu_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_16bit_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
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
			x1 = _mm_loadu_si128((__m128i *)(ycp +  0));
			x2 = _mm_loadu_si128((__m128i *)(ycp +  8));
			x3 = _mm_loadu_si128((__m128i *)(ycp + 16));

			_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

			x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
			x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
			x0 = _mm_shuffle_epi8(x0, xC_SUFFLE_YCP_Y);

			x6 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
			x6 = _mm_blend_epi16(x6, x3, MASK_INT_UV>>2);
			x6 = _mm_alignr_epi8(x6, x6, 2);
			x6 = _mm_shuffle_epi32(x6, _MM_SHUFFLE(1, 2, 3, 0));//UV1行目

				//Y 1行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA_16);
				x7 = _mm_madd_epi16(x7, xC_Y_L_MA_16);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_16);
				x7 = _mm_srai_epi32(x7, Y_L_RSH_16);
				x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
				x0 = _mm_add_epi32(x0, x1);
				x7 = _mm_add_epi32(x7, x1);

				x0 = _mm_packus_epi32(x0, x7);

			_mm_storeu_si128((__m128i *)(Y + x), x0);

			x1 = _mm_loadu_si128((__m128i *)(ycpw +  0));
			x2 = _mm_loadu_si128((__m128i *)(ycpw +  8));
			x3 = _mm_loadu_si128((__m128i *)(ycpw + 16));

			x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
			x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
			x0 = _mm_shuffle_epi8(x0, xC_SUFFLE_YCP_Y);

			x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
			x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
			x1 = _mm_alignr_epi8(x1, x1, 2);
			x2 = _mm_shuffle_epi32(x1, _MM_SHUFFLE(1, 2, 3, 0));//UV2行目

			//UV 1行目 + 2行目
			x6 = _mm_add_epi16(x2, x6);
				
				//Y 2行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA_16);
				x7 = _mm_madd_epi16(x7, xC_Y_L_MA_16);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_16);
				x7 = _mm_srai_epi32(x7, Y_L_RSH_16);
				x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
				x0 = _mm_add_epi32(x0, x1);
				x7 = _mm_add_epi32(x7, x1);

				x0 = _mm_packus_epi32(x0, x7);

			_mm_storeu_si128((__m128i *)(Y + x + width), x0);

			x0 = _mm_add_epi16(x6, _mm_set1_epi16(UV_OFFSET_x2));

			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_UV_L_MA_16_420P);
			x7 = _mm_madd_epi16(x7, xC_UV_L_MA_16_420P);
			x0 = _mm_srai_epi32(x0, UV_L_RSH_16_420P);
			x7 = _mm_srai_epi32(x7, UV_L_RSH_16_420P);
			x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
			x0 = _mm_add_epi32(x0, x1);
			x7 = _mm_add_epi32(x7, x1);

			x0 = _mm_packus_epi32(x0, x7);

			_mm_storeu_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_16bit_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
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
				x1 = _mm_loadu_si128((__m128i *)(ycp +  0));
				x2 = _mm_loadu_si128((__m128i *)(ycp +  8));
				x3 = _mm_loadu_si128((__m128i *)(ycp + 16));

				_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

				x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
				x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
				x0 = _mm_shuffle_epi8(x0, xC_SUFFLE_YCP_Y);

				x6 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
				x6 = _mm_blend_epi16(x6, x3, MASK_INT_UV>>2);
				x6 = _mm_alignr_epi8(x6, x6, 2);
				x6 = _mm_shuffle_epi32(x6, _MM_SHUFFLE(1, 2, 3, 0));//UV1行目

					//Y 1行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA_16);
					x7 = _mm_madd_epi16(x7, xC_Y_L_MA_16);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_16);
					x7 = _mm_srai_epi32(x7, Y_L_RSH_16);
					x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
					x0 = _mm_add_epi32(x0, x1);
					x7 = _mm_add_epi32(x7, x1);

					x0 = _mm_packus_epi32(x0, x7);

					_mm_storeu_si128((__m128i *)(Y + x), x0);

				x1 = _mm_loadu_si128((__m128i *)(ycpw +  0));
				x2 = _mm_loadu_si128((__m128i *)(ycpw +  8));
				x3 = _mm_loadu_si128((__m128i *)(ycpw + 16));

				x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
				x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
				x0 = _mm_shuffle_epi8(x0, xC_SUFFLE_YCP_Y);

				x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
				x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2);
				x1 = _mm_alignr_epi8(x1, x1, 2);
				x2 = _mm_shuffle_epi32(x1, _MM_SHUFFLE(1, 2, 3, 0));//UV3+i行目

					//Y 3行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA_16);
					x7 = _mm_madd_epi16(x7, xC_Y_L_MA_16);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_16);
					x7 = _mm_srai_epi32(x7, Y_L_RSH_16);
					x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
					x0 = _mm_add_epi32(x0, x1);
					x7 = _mm_add_epi32(x7, x1);

					x0 = _mm_packus_epi32(x0, x7);

					_mm_storeu_si128((__m128i *)(Y + x + width*2), x0);

				x6 = _mm_add_epi16(x6, _mm_set1_epi16(UV_OFFSET_x1));
				x2 = _mm_add_epi16(x2, _mm_set1_epi16(UV_OFFSET_x1));

				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);

				x7 = _mm_unpackhi_epi16(x6, x1);
				x6 = _mm_unpacklo_epi16(x6, x1);
				x3 = _mm_unpackhi_epi16(x2, x1);
				x2 = _mm_unpacklo_epi16(x2, x1);

				x6 = _mm_madd_epi16(x6, xC_UV_L_MA_16_420I(i));
				x7 = _mm_madd_epi16(x7, xC_UV_L_MA_16_420I(i));
				x2 = _mm_madd_epi16(x2, xC_UV_L_MA_16_420I((i+1)&0x01));
				x3 = _mm_madd_epi16(x3, xC_UV_L_MA_16_420I((i+1)&0x01));
				x0 = _mm_add_epi32(x6, x2);
				x7 = _mm_add_epi32(x7, x3);
				x0 = _mm_srai_epi32(x0, UV_L_RSH_16_420I);
				x7 = _mm_srai_epi32(x7, UV_L_RSH_16_420I);
				x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
				x0 = _mm_add_epi32(x0, x1);
				x7 = _mm_add_epi32(x7, x1);

				x0 = _mm_packus_epi32(x0, x7);

				_mm_storeu_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_nv12_16bit_avx_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
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
			x1 = _mm_loadu_si128((__m128i *)(ycp +  0));
			x2 = _mm_loadu_si128((__m128i *)(ycp +  8));
			x3 = _mm_loadu_si128((__m128i *)(ycp + 16));

			_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

			x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
			x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
			x0 = _mm_shuffle_epi8(x0, xC_SUFFLE_YCP_Y);

			x6 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
			x6 = _mm_blend_epi16(x6, x3, MASK_INT_UV>>2); 
			x6 = _mm_alignr_epi8(x6, x6, 2);
			x6 = _mm_shuffle_epi32(x6, _MM_SHUFFLE(1, 2, 3, 0));//UV1行目

				//Y 1行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA_16);
				x7 = _mm_madd_epi16(x7, xC_Y_L_MA_16);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_16);
				x7 = _mm_srai_epi32(x7, Y_L_RSH_16);
				x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
				x0 = _mm_add_epi32(x0, x1);
				x7 = _mm_add_epi32(x7, x1);

				x0 = _mm_packus_epi32(x0, x7);

			_mm_stream_si128((__m128i *)(Y + x), x0);

			x1 = _mm_loadu_si128((__m128i *)(ycpw +  0));
			x2 = _mm_loadu_si128((__m128i *)(ycpw +  8));
			x3 = _mm_loadu_si128((__m128i *)(ycpw + 16));

			x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
			x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
			x0 = _mm_shuffle_epi8(x0, xC_SUFFLE_YCP_Y);

			x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
			x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
			x1 = _mm_alignr_epi8(x1, x1, 2);
			x2 = _mm_shuffle_epi32(x1, _MM_SHUFFLE(1, 2, 3, 0));//UV3+i行目

			//UV 1行目 + 2行目
			x6 = _mm_add_epi16(x2, x6);
				
				//Y 2行目計算
				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);
				x7 = _mm_unpackhi_epi16(x0, x1);
				x0 = _mm_unpacklo_epi16(x0, x1);

				x0 = _mm_madd_epi16(x0, xC_Y_L_MA_16);
				x7 = _mm_madd_epi16(x7, xC_Y_L_MA_16);
				x0 = _mm_srai_epi32(x0, Y_L_RSH_16);
				x7 = _mm_srai_epi32(x7, Y_L_RSH_16);
				x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
				x0 = _mm_add_epi32(x0, x1);
				x7 = _mm_add_epi32(x7, x1);

				x0 = _mm_packus_epi32(x0, x7);

			_mm_stream_si128((__m128i *)(Y + x + width), x0);

			x0 = _mm_add_epi16(x6, _mm_set1_epi16(UV_OFFSET_x2));

			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_UV_L_MA_16_420P);
			x7 = _mm_madd_epi16(x7, xC_UV_L_MA_16_420P);
			x0 = _mm_srai_epi32(x0, UV_L_RSH_16_420P);
			x7 = _mm_srai_epi32(x7, UV_L_RSH_16_420P);
			x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
			x0 = _mm_add_epi32(x0, x1);
			x7 = _mm_add_epi32(x7, x1);

			x0 = _mm_packus_epi32(x0, x7);

			_mm_stream_si128((__m128i *)(C + x), x0);
		}
	}
}

void convert_yc48_to_nv12_i_16bit_avx_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
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
				x1 = _mm_loadu_si128((__m128i *)(ycp +  0));
				x2 = _mm_loadu_si128((__m128i *)(ycp +  8));
				x3 = _mm_loadu_si128((__m128i *)(ycp + 16));

				_mm_prefetch((const char *)ycpw, _MM_HINT_T1);

				x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
				x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
				x0 = _mm_shuffle_epi8(x0, xC_SUFFLE_YCP_Y);

				x6 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
				x6 = _mm_blend_epi16(x6, x3, MASK_INT_UV>>2);
				x6 = _mm_alignr_epi8(x6, x6, 2);
				x6 = _mm_shuffle_epi32(x6, _MM_SHUFFLE(1, 2, 3, 0));//UV1行目

					//Y 1行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA_16);
					x7 = _mm_madd_epi16(x7, xC_Y_L_MA_16);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_16);
					x7 = _mm_srai_epi32(x7, Y_L_RSH_16);
					x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
					x0 = _mm_add_epi32(x0, x1);
					x7 = _mm_add_epi32(x7, x1);

					x0 = _mm_packus_epi32(x0, x7);

					_mm_stream_si128((__m128i *)(Y + x), x0);

				x1 = _mm_loadu_si128((__m128i *)(ycpw +  0));
				x2 = _mm_loadu_si128((__m128i *)(ycpw +  8));
				x3 = _mm_loadu_si128((__m128i *)(ycpw + 16));

				x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
				x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
				x0 = _mm_shuffle_epi8(x0, xC_SUFFLE_YCP_Y);

				x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
				x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2); 
				x1 = _mm_alignr_epi8(x1, x1, 2);
				x2 = _mm_shuffle_epi32(x1, _MM_SHUFFLE(1, 2, 3, 0));//UV3+i行目

					//Y 3行目計算
					x1 = _mm_cmpeq_epi8(x1, x1);
					x1 = _mm_srli_epi16(x1, RSFT_ONE);
					x7 = _mm_unpackhi_epi16(x0, x1);
					x0 = _mm_unpacklo_epi16(x0, x1);

					x0 = _mm_madd_epi16(x0, xC_Y_L_MA_16);
					x7 = _mm_madd_epi16(x7, xC_Y_L_MA_16);
					x0 = _mm_srai_epi32(x0, Y_L_RSH_16);
					x7 = _mm_srai_epi32(x7, Y_L_RSH_16);
					x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
					x0 = _mm_add_epi32(x0, x1);
					x7 = _mm_add_epi32(x7, x1);

					x0 = _mm_packus_epi32(x0, x7);

					_mm_stream_si128((__m128i *)(Y + x + width*2), x0);

				x6 = _mm_add_epi16(x6, _mm_set1_epi16(UV_OFFSET_x1));
				x2 = _mm_add_epi16(x2, _mm_set1_epi16(UV_OFFSET_x1));

				x1 = _mm_cmpeq_epi8(x1, x1);
				x1 = _mm_srli_epi16(x1, RSFT_ONE);

				x7 = _mm_unpackhi_epi16(x6, x1);
				x6 = _mm_unpacklo_epi16(x6, x1);
				x3 = _mm_unpackhi_epi16(x2, x1);
				x2 = _mm_unpacklo_epi16(x2, x1);

				x6 = _mm_madd_epi16(x6, xC_UV_L_MA_16_420I(i));
				x7 = _mm_madd_epi16(x7, xC_UV_L_MA_16_420I(i));
				x2 = _mm_madd_epi16(x2, xC_UV_L_MA_16_420I((i+1)&0x01));
				x3 = _mm_madd_epi16(x3, xC_UV_L_MA_16_420I((i+1)&0x01));
				x0 = _mm_add_epi32(x6, x2);
				x7 = _mm_add_epi32(x7, x3);
				x0 = _mm_srai_epi32(x0, UV_L_RSH_16_420I);
				x7 = _mm_srai_epi32(x7, UV_L_RSH_16_420I);
				x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
				x0 = _mm_add_epi32(x0, x1);
				x7 = _mm_add_epi32(x7, x1);

				x0 = _mm_packus_epi32(x0, x7);

				_mm_stream_si128((__m128i *)(C + x), x0);
			}
		}
	}
}

void convert_yc48_to_yuv444_16bit_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *Y = (short *)pixel_data->data[0];
	short *U = (short *)pixel_data->data[1];
	short *V = (short *)pixel_data->data[2];
	short *ycp;
	short *const ycp_fin = (short *)pixel + width * height * 3;
	const int MASK_INT = 0x40 + 0x08 + 0x01;
	__m128i x0, x1, x2, x3, x6, x7;
	for (ycp = (short *)pixel; ycp < ycp_fin; ycp += 24, Y += 8, U += 8, V += 8) {
		x1 = _mm_loadu_si128((__m128i *)(ycp +  0));
		x2 = _mm_loadu_si128((__m128i *)(ycp +  8));
		x3 = _mm_loadu_si128((__m128i *)(ycp + 16));

		x0 = _mm_blend_epi16(x3, x1, MASK_INT);
		x6 = _mm_blend_epi16(x2, x3, MASK_INT);
		x7 = _mm_blend_epi16(x1, x2, MASK_INT);

		x0 = _mm_blend_epi16(x0, x2, MASK_INT<<1);
		x6 = _mm_blend_epi16(x6, x1, MASK_INT<<1);
		x7 = _mm_blend_epi16(x7, x3, MASK_INT<<1);

		x1 = xC_SUFFLE_YCP_Y;
		x0 = _mm_shuffle_epi8(x0, x1);
		x1 = _mm_alignr_epi8(x1, x1, 6);
		x6 = _mm_shuffle_epi8(x6, x1);
		x1 = _mm_alignr_epi8(x1, x1, 6);
		x3 = _mm_shuffle_epi8(x7, x1);

			//Y計算
			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_Y_L_MA_16);
			x7 = _mm_madd_epi16(x7, xC_Y_L_MA_16);
			x0 = _mm_srai_epi32(x0, Y_L_RSH_16);
			x7 = _mm_srai_epi32(x7, Y_L_RSH_16);
			x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
			x0 = _mm_add_epi32(x0, x1);
			x7 = _mm_add_epi32(x7, x1);

			x0 = _mm_packus_epi32(x0, x7);

		_mm_stream_si128((__m128i *)Y, x0);

			//U計算
			x0 = _mm_add_epi16(x6, _mm_set1_epi16(UV_OFFSET_x1));

			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_UV_L_MA_16_444);
			x7 = _mm_madd_epi16(x7, xC_UV_L_MA_16_444);
			x0 = _mm_srai_epi32(x0, UV_L_RSH_16_444);
			x7 = _mm_srai_epi32(x7, UV_L_RSH_16_444);
			x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
			x0 = _mm_add_epi32(x0, x1);
			x7 = _mm_add_epi32(x7, x1);

			x0 = _mm_packus_epi32(x0, x7);

		_mm_stream_si128((__m128i *)U, x0);

			//V計算
			x0 = _mm_add_epi16(x3, _mm_set1_epi16(UV_OFFSET_x1));

			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x7 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_UV_L_MA_16_444);
			x7 = _mm_madd_epi16(x7, xC_UV_L_MA_16_444);
			x0 = _mm_srai_epi32(x0, UV_L_RSH_16_444);
			x7 = _mm_srai_epi32(x7, UV_L_RSH_16_444);
			x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
			x0 = _mm_add_epi32(x0, x1);
			x7 = _mm_add_epi32(x7, x1);

			x0 = _mm_packus_epi32(x0, x7);

		_mm_stream_si128((__m128i *)V, x0);
	}
}

void convert_yuy2_to_nv16_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	BYTE *p = (BYTE *)pixel;
	BYTE * const p_fin = p + width * height * 2;
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	__m128i x0, x1, x2;
	for (; p < p_fin; p += 32, dst_Y += 16, dst_C += 16) {
		x0 = _mm_loadu_si128((__m128i *)(p+ 0));    // VYUYVYUYVYUYVYUY
		x1 = _mm_loadu_si128((__m128i *)(p+16));    // VYUYVYUYVYUYVYUY

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

void convert_yc48_to_nv16_16bit_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp = (short *)pixel;
	short * const ycp_fin = ycp + width * height * 3;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m128i x0, x1, x2, x3;
	for (; ycp < ycp_fin; ycp += 24, dst_Y += 8, dst_C += 8) {
		x1 = _mm_loadu_si128((__m128i *)(ycp +  0));
		x2 = _mm_loadu_si128((__m128i *)(ycp +  8));
		x3 = _mm_loadu_si128((__m128i *)(ycp + 16));

		x0 = _mm_blend_epi16(x1, x2, MASK_INT_Y);
		x0 = _mm_blend_epi16(x0, x3, MASK_INT_Y>>2);
		x0 = _mm_shuffle_epi8(x0, xC_SUFFLE_YCP_Y);

		x1 = _mm_blend_epi16(x1, x2, MASK_INT_UV);
		x1 = _mm_blend_epi16(x1, x3, MASK_INT_UV>>2);
		x1 = _mm_alignr_epi8(x1, x1, 2);
		x3 = _mm_shuffle_epi32(x1, _MM_SHUFFLE(1, 2, 3, 0));//UV行目

			//Y 計算
			x1 = _mm_cmpeq_epi8(x1, x1);
			x1 = _mm_srli_epi16(x1, RSFT_ONE);
			x2 = _mm_unpackhi_epi16(x0, x1);
			x0 = _mm_unpacklo_epi16(x0, x1);

			x0 = _mm_madd_epi16(x0, xC_Y_L_MA_16);
			x2 = _mm_madd_epi16(x2, xC_Y_L_MA_16);
			x0 = _mm_srai_epi32(x0, Y_L_RSH_16);
			x2 = _mm_srai_epi32(x2, Y_L_RSH_16);
			x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
			x0 = _mm_add_epi32(x0, x1);
			x2 = _mm_add_epi32(x2, x1);

			x0 = _mm_packus_epi32(x0, x2);

		_mm_stream_si128((__m128i *)dst_Y, x0);

		x0 = _mm_add_epi16(x3, _mm_set1_epi16(UV_OFFSET_x1));

		x1 = _mm_cmpeq_epi8(x1, x1);
		x1 = _mm_srli_epi16(x1, RSFT_ONE);
		x2 = _mm_unpackhi_epi16(x0, x1);
		x0 = _mm_unpacklo_epi16(x0, x1);

		x0 = _mm_madd_epi16(x0, xC_UV_L_MA_16_444);
		x2 = _mm_madd_epi16(x2, xC_UV_L_MA_16_444);
		x0 = _mm_srai_epi32(x0, UV_L_RSH_16_444);
		x2 = _mm_srai_epi32(x2, UV_L_RSH_16_444);
		x1 = _mm_unpacklo_epi16(_mm_slli_epi16(x1, LSFT_YCC_16), _mm_setzero_si128()); //32bit YCC生成
		x0 = _mm_add_epi32(x0, x1);
		x2 = _mm_add_epi32(x2, x1);

		x0 = _mm_packus_epi32(x0, x2);

		_mm_stream_si128((__m128i *)dst_C, x0);
	}
}
#endif //(_MSC_VER >= 1600)

