//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx/2.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include "convert.h"
#include "convert_const.h"

//AVX2はVC++2012から
#if (_MSC_VER >= 1700)
//AVX2用コード
#include <immintrin.h> //イントリンシック命令 AVX / AVX2


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
		ySA = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(sh + 32)), _mm_loadu_si128((__m128i*)(sh +  0)));
		ySB = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(sh + 48)), _mm_loadu_si128((__m128i*)(sh + 16)));
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

void convert_yuy2_to_nv12_avx2_mod32(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	BYTE *p, *pw, *Y, *C;
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	__m256i y0, y1, y3, y4, y5;
	__m256i yMaskLowByte = _mm256_srli_epi16(_mm256_cmpeq_epi8(_mm256_setzero_si256(), _mm256_setzero_si256()), 8);
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

			y4 = _mm256_srli_epi16(y0, 8);
			y5 = _mm256_srli_epi16(y1, 8);

			y0 = _mm256_and_si256(y0, yMaskLowByte);
			y1 = _mm256_and_si256(y1, yMaskLowByte);

			y0 = _mm256_packus_epi16(y0, y1);
			y3 = _mm256_packus_epi16(y4, y5);

			_mm256_store_si256((__m256i *)(Y + x), y0);
			//-----------1行目終了---------------

			//-----------2行目---------------
			y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+32)), _mm_loadu_si128((__m128i*)(pw+ 0)));
			y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+48)), _mm_loadu_si128((__m128i*)(pw+16)));

			y4 = _mm256_srli_epi16(y0, 8);
			y5 = _mm256_srli_epi16(y1, 8);

			y0 = _mm256_and_si256(y0, yMaskLowByte);
			y1 = _mm256_and_si256(y1, yMaskLowByte);
			
			y0 = _mm256_packus_epi16(y0, y1);
			y1 = _mm256_packus_epi16(y4, y5);

			_mm256_store_si256((__m256i *)(Y + width + x), y0);
			//-----------2行目終了---------------

			y1 = _mm256_avg_epu8(y1, y3);  //VUVUVUVUVUVUVUVU
			_mm256_store_si256((__m256i *)(C + x), y1);
		}
	}
	_mm256_zeroupper();
}

void convert_yuy2_to_nv12_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	BYTE *p, *pw, *Y, *C;
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	__m256i y0, y1, y3, y4, y5;
	__m256i yMaskLowByte = _mm256_srli_epi16(_mm256_cmpeq_epi8(_mm256_setzero_si256(), _mm256_setzero_si256()), 8);
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

			y4 = _mm256_srli_epi16(y0, 8);
			y5 = _mm256_srli_epi16(y1, 8);

			y0 = _mm256_and_si256(y0, yMaskLowByte);
			y1 = _mm256_and_si256(y1, yMaskLowByte);

			y0 = _mm256_packus_epi16(y0, y1);
			y3 = _mm256_packus_epi16(y4, y5);

			_mm256_storeu_si256((__m256i *)(Y + x), y0);
			//-----------1行目終了---------------

			//-----------2行目---------------
			y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+32)), _mm_loadu_si128((__m128i*)(pw+ 0)));
			y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+48)), _mm_loadu_si128((__m128i*)(pw+16)));

			y4 = _mm256_srli_epi16(y0, 8);
			y5 = _mm256_srli_epi16(y1, 8);

			y0 = _mm256_and_si256(y0, yMaskLowByte);
			y1 = _mm256_and_si256(y1, yMaskLowByte);
			
			y0 = _mm256_packus_epi16(y0, y1);
			y1 = _mm256_packus_epi16(y4, y5);

			_mm256_storeu_si256((__m256i *)(Y + width + x), y0);
			//-----------2行目終了---------------

			y1 = _mm256_avg_epu8(y1, y3);  //VUVUVUVUVUVUVUVU
			_mm256_storeu_si256((__m256i *)(C + x), y1);
		}
	}
	_mm256_zeroupper();
}

void convert_yuy2_to_nv12_i_avx2_mod32(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	BYTE *p, *pw, *Y, *C;
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	__m256i y0, y1, y3, y4, y5;
	__m256i yMaskLowByte = _mm256_srli_epi16(_mm256_cmpeq_epi8(_mm256_setzero_si256(), _mm256_setzero_si256()), 8);
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

				y4 = _mm256_srli_epi16(y0, 8);
				y5 = _mm256_srli_epi16(y1, 8);

				y0 = _mm256_and_si256(y0, yMaskLowByte);
				y1 = _mm256_and_si256(y1, yMaskLowByte);

				y0 = _mm256_packus_epi16(y0, y1);
				y3 = _mm256_packus_epi16(y4, y5);

				_mm256_store_si256((__m256i *)(Y + x), y0);
				//-----------1+i行目終了---------------

				//-----------3+i行目---------------
				y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+32)), _mm_loadu_si128((__m128i*)(pw+ 0)));
				y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+48)), _mm_loadu_si128((__m128i*)(pw+16)));

				y4 = _mm256_srli_epi16(y0, 8);
				y5 = _mm256_srli_epi16(y1, 8);

				y0 = _mm256_and_si256(y0, yMaskLowByte);
				y1 = _mm256_and_si256(y1, yMaskLowByte);
			
				y0 = _mm256_packus_epi16(y0, y1);
				y1 = _mm256_packus_epi16(y4, y5);

				_mm256_store_si256((__m256i *)(Y + (width<<1) + x), y0);
				//-----------3+i行目終了---------------

				y0 = _mm256_unpacklo_epi8(y1, y3);
				y1 = _mm256_unpackhi_epi8(y1, y3);
				y0 = _mm256_maddubs_epi16(y0, yC_INTERLACE_WEIGHT(i));
				y1 = _mm256_maddubs_epi16(y1, yC_INTERLACE_WEIGHT(i));
				y0 = _mm256_add_epi16(y0, _mm256_set1_epi16(2));
				y1 = _mm256_add_epi16(y1, _mm256_set1_epi16(2));
				y0 = _mm256_srai_epi16(y0, 2);
				y1 = _mm256_srai_epi16(y1, 2);
				y0 = _mm256_packus_epi16(y0, y1); //VUVUVUVUVUVUVUVU
				_mm256_store_si256((__m256i *)(C + x), y0);
			}
		}
	}
	_mm256_zeroupper();
}

void convert_yuy2_to_nv12_i_avx2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	BYTE *p, *pw, *Y, *C;
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	__m256i y0, y1, y3, y4, y5;
	__m256i yMaskLowByte = _mm256_srli_epi16(_mm256_cmpeq_epi8(_mm256_setzero_si256(), _mm256_setzero_si256()), 8);
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			x  = (y + i) * width;
			p  = (BYTE *)frame + (x<<1);
			pw  = p   + (width<<2);
			Y  = (BYTE *)dst_Y +  x;
			C  = (BYTE *)dst_C + ((x+width*i)>>1);
			for (x = 0; x < width; x += 32, p += 64, pw += 64) {
				//-----------    1行目   ---------------
				y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
				y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

				y4 = _mm256_srli_epi16(y0, 8);
				y5 = _mm256_srli_epi16(y1, 8);

				y0 = _mm256_and_si256(y0, yMaskLowByte);
				y1 = _mm256_and_si256(y1, yMaskLowByte);

				y0 = _mm256_packus_epi16(y0, y1);
				y3 = _mm256_packus_epi16(y4, y5);

				_mm256_storeu_si256((__m256i *)(Y + x), y0);
				//-----------1行目終了---------------

				//-----------3行目---------------
				y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+32)), _mm_loadu_si128((__m128i*)(pw+ 0)));
				y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(pw+48)), _mm_loadu_si128((__m128i*)(pw+16)));

				y4 = _mm256_srli_epi16(y0, 8);
				y5 = _mm256_srli_epi16(y1, 8);

				y0 = _mm256_and_si256(y0, yMaskLowByte);
				y1 = _mm256_and_si256(y1, yMaskLowByte);
			
				y0 = _mm256_packus_epi16(y0, y1);
				y1 = _mm256_packus_epi16(y4, y5);

				_mm256_storeu_si256((__m256i *)(Y + (width<<1) + x), y0);
				//-----------3行目終了---------------

				y0 = _mm256_unpacklo_epi8(y1, y3);
				y1 = _mm256_unpackhi_epi8(y1, y3);
				y0 = _mm256_maddubs_epi16(y0, yC_INTERLACE_WEIGHT(i));
				y1 = _mm256_maddubs_epi16(y1, yC_INTERLACE_WEIGHT(i));
				y0 = _mm256_add_epi16(y0, _mm256_set1_epi16(2));
				y1 = _mm256_add_epi16(y1, _mm256_set1_epi16(2));
				y0 = _mm256_srai_epi16(y0, 2);
				y1 = _mm256_srai_epi16(y1, 2);
				y0 = _mm256_packus_epi16(y0, y1); //VUVUVUVUVUVUVUVU
				_mm256_storeu_si256((__m256i *)(C + x), y0);
			}
		}
	}
	_mm256_zeroupper();
}

void convert_yc48_to_nv12_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m256i y0, y1, y2, y3, y6, y7;
	for (y = 0; y < height; y += 2) {
		ycp = (short*)pixel + width * y * 3;
		ycpw= ycp + width*3;
		Y   = (short*)dst_Y + width * y;
		C   = (short*)dst_C + width * y / 2;
		for (x = 0; x < width; x += 16, ycp += 48, ycpw += 48) {
			y7 = _mm256_loadu_si256((__m256i *)(ycp +  0)); // 128, 0
			y3 = _mm256_loadu_si256((__m256i *)(ycp + 16)); // 384, 256
			y0 = _mm256_loadu_si256((__m256i *)(ycp + 32)); // 640, 512

			_mm_prefetch((const char *)ycpw, _MM_HINT_T1);
			
			y1 = _mm256_blend_epi32(y7, y3, 0xf0);                    // 384, 0
			y2 = _mm256_permute2x128_si256(y7, y0, (0x02<<4) + 0x01); // 512, 128
			y3 = _mm256_blend_epi32(y3, y0, 0xf0);                    // 640, 256

			y0 = _mm256_blend_epi16(y1, y2, MASK_INT_Y);
			y0 = _mm256_blend_epi16(y0, y3, MASK_INT_Y>>2);
			y0 = _mm256_shuffle_epi8(y0, yC_SUFFLE_YCP_Y);

			y6 = _mm256_blend_epi16(y1, y2, MASK_INT_UV);
			y6 = _mm256_blend_epi16(y6, y3, MASK_INT_UV>>2);
			y6 = _mm256_alignr_epi8(y6, y6, 2);
			y6 = _mm256_shuffle_epi32(y6, _MM_SHUFFLE(1, 2, 3, 0));//UV1行目

				//Y 1行目計算
				y1 = _mm256_cmpeq_epi8(y1, y1);
				y1 = _mm256_srli_epi16(y1, RSFT_ONE);
				y7 = _mm256_unpackhi_epi16(y0, y1);
				y0 = _mm256_unpacklo_epi16(y0, y1);

				y0 = _mm256_madd_epi16(y0, yC_Y_L_MA_16);
				y7 = _mm256_madd_epi16(y7, yC_Y_L_MA_16);
				y0 = _mm256_srai_epi32(y0, Y_L_RSH_16);
				y7 = _mm256_srai_epi32(y7, Y_L_RSH_16);
				y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
				y0 = _mm256_add_epi32(y0, y1);
				y7 = _mm256_add_epi32(y7, y1);

				y0 = _mm256_packus_epi32(y0, y7);

			_mm256_storeu_si256((__m256i *)(Y + x), y0);

			y0 = _mm256_loadu_si256((__m256i *)(ycpw +  0));
			y7 = _mm256_loadu_si256((__m256i *)(ycpw + 16));
			y3 = _mm256_loadu_si256((__m256i *)(ycpw + 32));
			y1 = _mm256_blend_epi32(y0, y7, 0xf0);                    // 384, 0
			y2 = _mm256_permute2x128_si256(y0, y3, (0x02<<4) + 0x01); // 512, 128
			y3 = _mm256_blend_epi32(y7, y3, 0xf0);                    // 640, 256

			y0 = _mm256_blend_epi16(y1, y2, MASK_INT_Y);
			y0 = _mm256_blend_epi16(y0, y3, MASK_INT_Y>>2);
			y0 = _mm256_shuffle_epi8(y0, yC_SUFFLE_YCP_Y);

			y1 = _mm256_blend_epi16(y1, y2, MASK_INT_UV);
			y1 = _mm256_blend_epi16(y1, y3, MASK_INT_UV>>2); 
			y1 = _mm256_alignr_epi8(y1, y1, 2);
			y2 = _mm256_shuffle_epi32(y1, _MM_SHUFFLE(1, 2, 3, 0));//UV2行目

			//UV 1行目 + 2行目
			y6 = _mm256_add_epi16(y2, y6);
				
				//Y 2行目計算
				y1 = _mm256_cmpeq_epi8(y1, y1);
				y1 = _mm256_srli_epi16(y1, RSFT_ONE);
				y7 = _mm256_unpackhi_epi16(y0, y1);
				y0 = _mm256_unpacklo_epi16(y0, y1);

				y0 = _mm256_madd_epi16(y0, yC_Y_L_MA_16);
				y7 = _mm256_madd_epi16(y7, yC_Y_L_MA_16);
				y0 = _mm256_srai_epi32(y0, Y_L_RSH_16);
				y7 = _mm256_srai_epi32(y7, Y_L_RSH_16);
				y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
				y0 = _mm256_add_epi32(y0, y1);
				y7 = _mm256_add_epi32(y7, y1);

				y0 = _mm256_packus_epi32(y0, y7);

			_mm256_storeu_si256((__m256i *)(Y + x + width), y0);

			y0 = _mm256_add_epi16(y6, _mm256_set1_epi16(UV_OFFSET_x2));

			y1 = _mm256_cmpeq_epi8(y1, y1);
			y1 = _mm256_srli_epi16(y1, RSFT_ONE);
			y7 = _mm256_unpackhi_epi16(y0, y1);
			y0 = _mm256_unpacklo_epi16(y0, y1);

			y0 = _mm256_madd_epi16(y0, yC_UV_L_MA_16_420P);
			y7 = _mm256_madd_epi16(y7, yC_UV_L_MA_16_420P);
			y0 = _mm256_srai_epi32(y0, UV_L_RSH_16_420P);
			y7 = _mm256_srai_epi32(y7, UV_L_RSH_16_420P);
			y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
			y0 = _mm256_add_epi32(y0, y1);
			y7 = _mm256_add_epi32(y7, y1);

			y0 = _mm256_packus_epi32(y0, y7);

			_mm256_storeu_si256((__m256i *)(C + x), y0);
		}
	}
	_mm256_zeroupper();
}

void convert_yc48_to_nv12_i_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m256i y0, y1, y2, y3, y6, y7;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			ycp = (short*)pixel + width * (y + i) * 3;
			ycpw= ycp + width*2*3;
			Y   = (short*)dst_Y + width * (y + i);
			C   = (short*)dst_C + width * (y + i*2) / 2;
			for (x = 0; x < width; x += 16, ycp += 48, ycpw += 48) {
				y7 = _mm256_loadu_si256((__m256i *)(ycp +  0)); // 128, 0
				y3 = _mm256_loadu_si256((__m256i *)(ycp + 16)); // 384, 256
				y0 = _mm256_loadu_si256((__m256i *)(ycp + 32)); // 640, 512

				_mm_prefetch((const char *)ycpw, _MM_HINT_T1);
				
				y1 = _mm256_blend_epi32(y7, y3, 0xf0);                    // 384, 0
				y2 = _mm256_permute2x128_si256(y7, y0, (0x02<<4) + 0x01); // 512, 128
				y3 = _mm256_blend_epi32(y3, y0, 0xf0);                    // 640, 256

				y0 = _mm256_blend_epi16(y1, y2, MASK_INT_Y);
				y0 = _mm256_blend_epi16(y0, y3, MASK_INT_Y>>2);
				y0 = _mm256_shuffle_epi8(y0, yC_SUFFLE_YCP_Y);

				y6 = _mm256_blend_epi16(y1, y2, MASK_INT_UV);
				y6 = _mm256_blend_epi16(y6, y3, MASK_INT_UV>>2);
				y6 = _mm256_alignr_epi8(y6, y6, 2);
				y6 = _mm256_shuffle_epi32(y6, _MM_SHUFFLE(1, 2, 3, 0));//UV1行目

					//Y 1行目計算
					y1 = _mm256_cmpeq_epi8(y1, y1);
					y1 = _mm256_srli_epi16(y1, RSFT_ONE);
					y7 = _mm256_unpackhi_epi16(y0, y1);
					y0 = _mm256_unpacklo_epi16(y0, y1);

					y0 = _mm256_madd_epi16(y0, yC_Y_L_MA_16);
					y7 = _mm256_madd_epi16(y7, yC_Y_L_MA_16);
					y0 = _mm256_srai_epi32(y0, Y_L_RSH_16);
					y7 = _mm256_srai_epi32(y7, Y_L_RSH_16);
					y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
					y0 = _mm256_add_epi32(y0, y1);
					y7 = _mm256_add_epi32(y7, y1);

					y0 = _mm256_packus_epi32(y0, y7);

					_mm256_storeu_si256((__m256i *)(Y + x), y0);

				y0 = _mm256_loadu_si256((__m256i *)(ycpw +  0));
				y7 = _mm256_loadu_si256((__m256i *)(ycpw + 16));
				y3 = _mm256_loadu_si256((__m256i *)(ycpw + 32));
				y1 = _mm256_blend_epi32(y0, y7, 0xf0);                    // 384, 0
				y2 = _mm256_permute2x128_si256(y0, y3, (0x02<<4) + 0x01); // 512, 128
				y3 = _mm256_blend_epi32(y7, y3, 0xf0);                    // 640, 256

				y0 = _mm256_blend_epi16(y1, y2, MASK_INT_Y);
				y0 = _mm256_blend_epi16(y0, y3, MASK_INT_Y>>2);
				y0 = _mm256_shuffle_epi8(y0, yC_SUFFLE_YCP_Y);

				y1 = _mm256_blend_epi16(y1, y2, MASK_INT_UV);
				y1 = _mm256_blend_epi16(y1, y3, MASK_INT_UV>>2);
				y1 = _mm256_alignr_epi8(y1, y1, 2);
				y2 = _mm256_shuffle_epi32(y1, _MM_SHUFFLE(1, 2, 3, 0));//UV3+i行目

					//Y 3行目計算
					y1 = _mm256_cmpeq_epi8(y1, y1);
					y1 = _mm256_srli_epi16(y1, RSFT_ONE);
					y7 = _mm256_unpackhi_epi16(y0, y1);
					y0 = _mm256_unpacklo_epi16(y0, y1);

					y0 = _mm256_madd_epi16(y0, yC_Y_L_MA_16);
					y7 = _mm256_madd_epi16(y7, yC_Y_L_MA_16);
					y0 = _mm256_srai_epi32(y0, Y_L_RSH_16);
					y7 = _mm256_srai_epi32(y7, Y_L_RSH_16);
					y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
					y0 = _mm256_add_epi32(y0, y1);
					y7 = _mm256_add_epi32(y7, y1);

					y0 = _mm256_packus_epi32(y0, y7);

					_mm256_storeu_si256((__m256i *)(Y + x + width*2), y0);

				y6 = _mm256_add_epi16(y6, _mm256_set1_epi16(UV_OFFSET_x1));
				y2 = _mm256_add_epi16(y2, _mm256_set1_epi16(UV_OFFSET_x1));

				y1 = _mm256_cmpeq_epi8(y1, y1);
				y1 = _mm256_srli_epi16(y1, RSFT_ONE);

				y7 = _mm256_unpackhi_epi16(y6, y1);
				y6 = _mm256_unpacklo_epi16(y6, y1);
				y3 = _mm256_unpackhi_epi16(y2, y1);
				y2 = _mm256_unpacklo_epi16(y2, y1);

				y6 = _mm256_madd_epi16(y6, yC_UV_L_MA_16_420I(i));
				y7 = _mm256_madd_epi16(y7, yC_UV_L_MA_16_420I(i));
				y2 = _mm256_madd_epi16(y2, yC_UV_L_MA_16_420I((i+1)&0x01));
				y3 = _mm256_madd_epi16(y3, yC_UV_L_MA_16_420I((i+1)&0x01));
				y0 = _mm256_add_epi32(y6, y2);
				y7 = _mm256_add_epi32(y7, y3);
				y0 = _mm256_srai_epi32(y0, UV_L_RSH_16_420I);
				y7 = _mm256_srai_epi32(y7, UV_L_RSH_16_420I);
				y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
				y0 = _mm256_add_epi32(y0, y1);
				y7 = _mm256_add_epi32(y7, y1);

				y0 = _mm256_packus_epi32(y0, y7);

				_mm256_storeu_si256((__m256i *)(C + x), y0);
			}
		}
	}
	_mm256_zeroupper();
}

void convert_yc48_to_nv12_16bit_avx2_mod16(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m256i y0, y1, y2, y3, y6, y7;
	for (y = 0; y < height; y += 2) {
		ycp = (short*)pixel + width * y * 3;
		ycpw= ycp + width*3;
		Y   = (short*)dst_Y + width * y;
		C   = (short*)dst_C + width * y / 2;
		for (x = 0; x < width; x += 16, ycp += 48, ycpw += 48) {
			y7 = _mm256_loadu_si256((__m256i *)(ycp +  0)); // 128, 0
			y3 = _mm256_loadu_si256((__m256i *)(ycp + 16)); // 384, 256
			y0 = _mm256_loadu_si256((__m256i *)(ycp + 32)); // 640, 512

			_mm_prefetch((const char *)ycpw, _MM_HINT_T1);
				
			y1 = _mm256_blend_epi32(y7, y3, 0xf0);                    // 384, 0
			y2 = _mm256_permute2x128_si256(y7, y0, (0x02<<4) + 0x01); // 512, 128
			y3 = _mm256_blend_epi32(y3, y0, 0xf0);                    // 640, 256

			y0 = _mm256_blend_epi16(y1, y2, MASK_INT_Y);
			y0 = _mm256_blend_epi16(y0, y3, MASK_INT_Y>>2);
			y0 = _mm256_shuffle_epi8(y0, yC_SUFFLE_YCP_Y);

			y6 = _mm256_blend_epi16(y1, y2, MASK_INT_UV);
			y6 = _mm256_blend_epi16(y6, y3, MASK_INT_UV>>2);
			y6 = _mm256_alignr_epi8(y6, y6, 2);
			y6 = _mm256_shuffle_epi32(y6, _MM_SHUFFLE(1, 2, 3, 0));//UV1行目

				//Y 1行目計算
				y1 = _mm256_cmpeq_epi8(y1, y1);
				y1 = _mm256_srli_epi16(y1, RSFT_ONE);
				y7 = _mm256_unpackhi_epi16(y0, y1);
				y0 = _mm256_unpacklo_epi16(y0, y1);

				y0 = _mm256_madd_epi16(y0, yC_Y_L_MA_16);
				y7 = _mm256_madd_epi16(y7, yC_Y_L_MA_16);
				y0 = _mm256_srai_epi32(y0, Y_L_RSH_16);
				y7 = _mm256_srai_epi32(y7, Y_L_RSH_16);
				y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
				y0 = _mm256_add_epi32(y0, y1);
				y7 = _mm256_add_epi32(y7, y1);

				y0 = _mm256_packus_epi32(y0, y7);

			_mm256_stream_si256((__m256i *)(Y + x), y0);

			y0 = _mm256_loadu_si256((__m256i *)(ycpw +  0));
			y7 = _mm256_loadu_si256((__m256i *)(ycpw + 16));
			y3 = _mm256_loadu_si256((__m256i *)(ycpw + 32));
			y1 = _mm256_blend_epi32(y0, y7, 0xf0);                    // 384, 0
			y2 = _mm256_permute2x128_si256(y0, y3, (0x02<<4) + 0x01); // 512, 128
			y3 = _mm256_blend_epi32(y7, y3, 0xf0);                    // 640, 256

			y0 = _mm256_blend_epi16(y1, y2, MASK_INT_Y);
			y0 = _mm256_blend_epi16(y0, y3, MASK_INT_Y>>2);
			y0 = _mm256_shuffle_epi8(y0, yC_SUFFLE_YCP_Y);

			y1 = _mm256_blend_epi16(y1, y2, MASK_INT_UV);
			y1 = _mm256_blend_epi16(y1, y3, MASK_INT_UV>>2); 
			y1 = _mm256_alignr_epi8(y1, y1, 2);
			y2 = _mm256_shuffle_epi32(y1, _MM_SHUFFLE(1, 2, 3, 0));//UV2行目

			//UV 1行目 + 2行目
			y6 = _mm256_add_epi16(y2, y6);
				
				//Y 2行目計算
				y1 = _mm256_cmpeq_epi8(y1, y1);
				y1 = _mm256_srli_epi16(y1, RSFT_ONE);
				y7 = _mm256_unpackhi_epi16(y0, y1);
				y0 = _mm256_unpacklo_epi16(y0, y1);

				y0 = _mm256_madd_epi16(y0, yC_Y_L_MA_16);
				y7 = _mm256_madd_epi16(y7, yC_Y_L_MA_16);
				y0 = _mm256_srai_epi32(y0, Y_L_RSH_16);
				y7 = _mm256_srai_epi32(y7, Y_L_RSH_16);
				y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
				y0 = _mm256_add_epi32(y0, y1);
				y7 = _mm256_add_epi32(y7, y1);

				y0 = _mm256_packus_epi32(y0, y7);

			_mm256_stream_si256((__m256i *)(Y + x + width), y0);

			y0 = _mm256_add_epi16(y6, _mm256_set1_epi16(UV_OFFSET_x2));

			y1 = _mm256_cmpeq_epi8(y1, y1);
			y1 = _mm256_srli_epi16(y1, RSFT_ONE);
			y7 = _mm256_unpackhi_epi16(y0, y1);
			y0 = _mm256_unpacklo_epi16(y0, y1);

			y0 = _mm256_madd_epi16(y0, yC_UV_L_MA_16_420P);
			y7 = _mm256_madd_epi16(y7, yC_UV_L_MA_16_420P);
			y0 = _mm256_srai_epi32(y0, UV_L_RSH_16_420P);
			y7 = _mm256_srai_epi32(y7, UV_L_RSH_16_420P);
			y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
			y0 = _mm256_add_epi32(y0, y1);
			y7 = _mm256_add_epi32(y7, y1);

			y0 = _mm256_packus_epi32(y0, y7);

			_mm256_stream_si256((__m256i *)(C + x), y0);
		}
	}
	_mm256_zeroupper();
}

void convert_yc48_to_nv12_i_16bit_avx2_mod16(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	int x, y, i;
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp, *ycpw;
	short *Y = NULL, *C = NULL;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m256i y0, y1, y2, y3, y6, y7;
	for (y = 0; y < height; y += 4) {
		for (i = 0; i < 2; i++) {
			ycp = (short*)pixel + width * (y + i) * 3;
			ycpw= ycp + width*2*3;
			Y   = (short*)dst_Y + width * (y + i);
			C   = (short*)dst_C + width * (y + i*2) / 2;
			for (x = 0; x < width; x += 16, ycp += 48, ycpw += 48) {
				y7 = _mm256_loadu_si256((__m256i *)(ycp +  0)); // 128, 0
				y3 = _mm256_loadu_si256((__m256i *)(ycp + 16)); // 384, 256
				y0 = _mm256_loadu_si256((__m256i *)(ycp + 32)); // 640, 512

				_mm_prefetch((const char *)ycpw, _MM_HINT_T1);
				
				y1 = _mm256_blend_epi32(y7, y3, 0xf0);                    // 384, 0
				y2 = _mm256_permute2x128_si256(y7, y0, (0x02<<4) + 0x01); // 512, 128
				y3 = _mm256_blend_epi32(y3, y0, 0xf0);                    // 640, 256

				y0 = _mm256_blend_epi16(y1, y2, MASK_INT_Y);
				y0 = _mm256_blend_epi16(y0, y3, MASK_INT_Y>>2);
				y0 = _mm256_shuffle_epi8(y0, yC_SUFFLE_YCP_Y);

				y6 = _mm256_blend_epi16(y1, y2, MASK_INT_UV);
				y6 = _mm256_blend_epi16(y6, y3, MASK_INT_UV>>2);
				y6 = _mm256_alignr_epi8(y6, y6, 2);
				y6 = _mm256_shuffle_epi32(y6, _MM_SHUFFLE(1, 2, 3, 0));//UV1行目

					//Y 1行目計算
					y1 = _mm256_cmpeq_epi8(y1, y1);
					y1 = _mm256_srli_epi16(y1, RSFT_ONE);
					y7 = _mm256_unpackhi_epi16(y0, y1);
					y0 = _mm256_unpacklo_epi16(y0, y1);

					y0 = _mm256_madd_epi16(y0, yC_Y_L_MA_16);
					y7 = _mm256_madd_epi16(y7, yC_Y_L_MA_16);
					y0 = _mm256_srai_epi32(y0, Y_L_RSH_16);
					y7 = _mm256_srai_epi32(y7, Y_L_RSH_16);
					y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
					y0 = _mm256_add_epi32(y0, y1);
					y7 = _mm256_add_epi32(y7, y1);

					y0 = _mm256_packus_epi32(y0, y7);

					_mm256_stream_si256((__m256i *)(Y + x), y0);

				y0 = _mm256_loadu_si256((__m256i *)(ycpw +  0));
				y7 = _mm256_loadu_si256((__m256i *)(ycpw + 16));
				y3 = _mm256_loadu_si256((__m256i *)(ycpw + 32));
				y1 = _mm256_blend_epi32(y0, y7, 0xf0);                    // 384, 0
				y2 = _mm256_permute2x128_si256(y0, y3, (0x02<<4) + 0x01); // 512, 128
				y3 = _mm256_blend_epi32(y7, y3, 0xf0);                    // 640, 256

				y0 = _mm256_blend_epi16(y1, y2, MASK_INT_Y);
				y0 = _mm256_blend_epi16(y0, y3, MASK_INT_Y>>2);
				y0 = _mm256_shuffle_epi8(y0, yC_SUFFLE_YCP_Y);

				y1 = _mm256_blend_epi16(y1, y2, MASK_INT_UV);
				y1 = _mm256_blend_epi16(y1, y3, MASK_INT_UV>>2);
				y1 = _mm256_alignr_epi8(y1, y1, 2);
				y2 = _mm256_shuffle_epi32(y1, _MM_SHUFFLE(1, 2, 3, 0));//UV3+i行目

					//Y 3行目計算
					y1 = _mm256_cmpeq_epi8(y1, y1);
					y1 = _mm256_srli_epi16(y1, RSFT_ONE);
					y7 = _mm256_unpackhi_epi16(y0, y1);
					y0 = _mm256_unpacklo_epi16(y0, y1);

					y0 = _mm256_madd_epi16(y0, yC_Y_L_MA_16);
					y7 = _mm256_madd_epi16(y7, yC_Y_L_MA_16);
					y0 = _mm256_srai_epi32(y0, Y_L_RSH_16);
					y7 = _mm256_srai_epi32(y7, Y_L_RSH_16);
					y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
					y0 = _mm256_add_epi32(y0, y1);
					y7 = _mm256_add_epi32(y7, y1);

					y0 = _mm256_packus_epi32(y0, y7);

					_mm256_stream_si256((__m256i *)(Y + x + width*2), y0);

				y6 = _mm256_add_epi16(y6, _mm256_set1_epi16(UV_OFFSET_x1));
				y2 = _mm256_add_epi16(y2, _mm256_set1_epi16(UV_OFFSET_x1));

				y1 = _mm256_cmpeq_epi8(y1, y1);
				y1 = _mm256_srli_epi16(y1, RSFT_ONE);

				y7 = _mm256_unpackhi_epi16(y6, y1);
				y6 = _mm256_unpacklo_epi16(y6, y1);
				y3 = _mm256_unpackhi_epi16(y2, y1);
				y2 = _mm256_unpacklo_epi16(y2, y1);

				y6 = _mm256_madd_epi16(y6, yC_UV_L_MA_16_420I(i));
				y7 = _mm256_madd_epi16(y7, yC_UV_L_MA_16_420I(i));
				y2 = _mm256_madd_epi16(y2, yC_UV_L_MA_16_420I((i+1)&0x01));
				y3 = _mm256_madd_epi16(y3, yC_UV_L_MA_16_420I((i+1)&0x01));
				y0 = _mm256_add_epi32(y6, y2);
				y7 = _mm256_add_epi32(y7, y3);
				y0 = _mm256_srai_epi32(y0, UV_L_RSH_16_420I);
				y7 = _mm256_srai_epi32(y7, UV_L_RSH_16_420I);
				y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
				y0 = _mm256_add_epi32(y0, y1);
				y7 = _mm256_add_epi32(y7, y1);

				y0 = _mm256_packus_epi32(y0, y7);

				_mm256_stream_si256((__m256i *)(C + x), y0);
			}
		}
	}
	_mm256_zeroupper();
}

void convert_yc48_to_yuv444_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *Y = (short *)pixel_data->data[0];
	short *U = (short *)pixel_data->data[1];
	short *V = (short *)pixel_data->data[2];
	short *ycp;
	short *const ycp_fin = (short *)pixel + width * height * 3;
	const int MASK_INT = 0x40 + 0x08 + 0x01;
	__m256i y0, y1, y2, y3, y6, y7;
	for (ycp = (short *)pixel; ycp < ycp_fin; ycp += 48, Y += 16, U += 16, V += 16) {
		y7 = _mm256_loadu_si256((__m256i *)(ycp +  0));
		y3 = _mm256_loadu_si256((__m256i *)(ycp + 16));
		y0 = _mm256_loadu_si256((__m256i *)(ycp + 32));

		y1 = _mm256_blend_epi32(y7, y3, 0xf0);                    // 384, 0
		y2 = _mm256_permute2x128_si256(y7, y0, (0x02<<4) + 0x01); // 512, 128
		y3 = _mm256_blend_epi32(y3, y0, 0xf0);                    // 640, 256

		y0 = _mm256_blend_epi16(y3, y1, MASK_INT);
		y6 = _mm256_blend_epi16(y2, y3, MASK_INT);
		y7 = _mm256_blend_epi16(y1, y2, MASK_INT);

		y0 = _mm256_blend_epi16(y0, y2, MASK_INT<<1);
		y6 = _mm256_blend_epi16(y6, y1, MASK_INT<<1);
		y7 = _mm256_blend_epi16(y7, y3, MASK_INT<<1);

		y1 = yC_SUFFLE_YCP_Y;
		y0 = _mm256_shuffle_epi8(y0, y1);
		y1 = _mm256_alignr_epi8(y1, y1, 6);
		y6 = _mm256_shuffle_epi8(y6, y1);
		y1 = _mm256_alignr_epi8(y1, y1, 6);
		y3 = _mm256_shuffle_epi8(y7, y1);

			//Y計算
			y1 = _mm256_cmpeq_epi8(y1, y1);
			y1 = _mm256_srli_epi16(y1, RSFT_ONE);
			y7 = _mm256_unpackhi_epi16(y0, y1);
			y0 = _mm256_unpacklo_epi16(y0, y1);

			y0 = _mm256_madd_epi16(y0, yC_Y_L_MA_16);
			y7 = _mm256_madd_epi16(y7, yC_Y_L_MA_16);
			y0 = _mm256_srai_epi32(y0, Y_L_RSH_16);
			y7 = _mm256_srai_epi32(y7, Y_L_RSH_16);
			y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
			y0 = _mm256_add_epi32(y0, y1);
			y7 = _mm256_add_epi32(y7, y1);

			y0 = _mm256_packus_epi32(y0, y7);

		_mm256_stream_si256((__m256i *)Y, y0);

			//U計算
			y0 = _mm256_add_epi16(y6, _mm256_set1_epi16(UV_OFFSET_x1));

			y1 = _mm256_cmpeq_epi8(y1, y1);
			y1 = _mm256_srli_epi16(y1, RSFT_ONE);
			y7 = _mm256_unpackhi_epi16(y0, y1);
			y0 = _mm256_unpacklo_epi16(y0, y1);

			y0 = _mm256_madd_epi16(y0, yC_UV_L_MA_16_444);
			y7 = _mm256_madd_epi16(y7, yC_UV_L_MA_16_444);
			y0 = _mm256_srai_epi32(y0, UV_L_RSH_16_444);
			y7 = _mm256_srai_epi32(y7, UV_L_RSH_16_444);
			y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
			y0 = _mm256_add_epi32(y0, y1);
			y7 = _mm256_add_epi32(y7, y1);

			y0 = _mm256_packus_epi32(y0, y7);

		_mm256_stream_si256((__m256i *)U, y0);

			//V計算
			y0 = _mm256_add_epi16(y3, _mm256_set1_epi16(UV_OFFSET_x1));

			y1 = _mm256_cmpeq_epi8(y1, y1);
			y1 = _mm256_srli_epi16(y1, RSFT_ONE);
			y7 = _mm256_unpackhi_epi16(y0, y1);
			y0 = _mm256_unpacklo_epi16(y0, y1);

			y0 = _mm256_madd_epi16(y0, yC_UV_L_MA_16_444);
			y7 = _mm256_madd_epi16(y7, yC_UV_L_MA_16_444);
			y0 = _mm256_srai_epi32(y0, UV_L_RSH_16_444);
			y7 = _mm256_srai_epi32(y7, UV_L_RSH_16_444);
			y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
			y0 = _mm256_add_epi32(y0, y1);
			y7 = _mm256_add_epi32(y7, y1);

			y0 = _mm256_packus_epi32(y0, y7);

		_mm256_stream_si256((__m256i *)V, y0);
	}
	_mm256_zeroupper();
}

void convert_yuy2_to_nv16_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	BYTE *p = (BYTE *)pixel;
	BYTE * const p_fin = p + width * height * 2;
	BYTE *dst_Y = pixel_data->data[0];
	BYTE *dst_C = pixel_data->data[1];
	__m256i y0, y1, y4, y5;
	__m256i yMaskLowByte = _mm256_srli_epi16(_mm256_cmpeq_epi8(_mm256_setzero_si256(), _mm256_setzero_si256()), 8);
	for (; p < p_fin; p += 64, dst_Y += 32, dst_C += 32) {
		y0 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+32)), _mm_loadu_si128((__m128i*)(p+ 0)));
		y1 = _mm256_set_m128i(_mm_loadu_si128((__m128i*)(p+48)), _mm_loadu_si128((__m128i*)(p+16)));

		y4 = _mm256_srli_epi16(y0, 8);
		y5 = _mm256_srli_epi16(y1, 8);

		y0 = _mm256_and_si256(y0, yMaskLowByte);
		y1 = _mm256_and_si256(y1, yMaskLowByte);

		y0 = _mm256_packus_epi16(y0, y1);
		y1 = _mm256_packus_epi16(y4, y5);

		_mm256_stream_si256((__m256i *)dst_Y, y0);
		_mm256_stream_si256((__m256i *)dst_C, y1);
	}
	_mm256_zeroupper();
}

void convert_yc48_to_nv16_16bit_avx2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height) {
	short *dst_Y = (short *)pixel_data->data[0];
	short *dst_C = (short *)pixel_data->data[1];
	short *ycp = (short *)pixel;
	short * const ycp_fin = ycp + width * height * 3;
	const int MASK_INT_Y  = 0x80 + 0x10 + 0x02;
	const int MASK_INT_UV = 0x40 + 0x20 + 0x01;
	__m256i y0, y1, y2, y3;
	for (; ycp < ycp_fin; ycp += 48, dst_Y += 16, dst_C += 16) {
		y2 = _mm256_loadu_si256((__m256i *)(ycp +  0));
		y3 = _mm256_loadu_si256((__m256i *)(ycp + 16));
		y0 = _mm256_loadu_si256((__m256i *)(ycp + 32));

		y1 = _mm256_blend_epi32(y2, y3, 0xf0);                    // 384, 0
		y2 = _mm256_permute2x128_si256(y2, y0, (0x02<<4) + 0x01); // 512, 128
		y3 = _mm256_blend_epi32(y3, y0, 0xf0);                    // 640, 256

		y0 = _mm256_blend_epi16(y1, y2, MASK_INT_Y);
		y0 = _mm256_blend_epi16(y0, y3, MASK_INT_Y>>2);
		y0 = _mm256_shuffle_epi8(y0, yC_SUFFLE_YCP_Y);

		y1 = _mm256_blend_epi16(y1, y2, MASK_INT_UV);
		y1 = _mm256_blend_epi16(y1, y3, MASK_INT_UV>>2);
		y1 = _mm256_alignr_epi8(y1, y1, 2);
		y3 = _mm256_shuffle_epi32(y1, _MM_SHUFFLE(1, 2, 3, 0));//UV行目

			//Y 計算
			y1 = _mm256_cmpeq_epi8(y1, y1);
			y1 = _mm256_srli_epi16(y1, RSFT_ONE);
			y2 = _mm256_unpackhi_epi16(y0, y1);
			y0 = _mm256_unpacklo_epi16(y0, y1);

			y0 = _mm256_madd_epi16(y0, yC_Y_L_MA_16);
			y2 = _mm256_madd_epi16(y2, yC_Y_L_MA_16);
			y0 = _mm256_srai_epi32(y0, Y_L_RSH_16);
			y2 = _mm256_srai_epi32(y2, Y_L_RSH_16);
			y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
			y0 = _mm256_add_epi32(y0, y1);
			y2 = _mm256_add_epi32(y2, y1);

			y0 = _mm256_packus_epi32(y0, y2);

		_mm256_stream_si256((__m256i *)dst_Y, y0);

		y0 = _mm256_add_epi16(y3, _mm256_set1_epi16(UV_OFFSET_x1));

		y1 = _mm256_cmpeq_epi8(y1, y1);
		y1 = _mm256_srli_epi16(y1, RSFT_ONE);
		y2 = _mm256_unpackhi_epi16(y0, y1);
		y0 = _mm256_unpacklo_epi16(y0, y1);

		y0 = _mm256_madd_epi16(y0, yC_UV_L_MA_16_444);
		y2 = _mm256_madd_epi16(y2, yC_UV_L_MA_16_444);
		y0 = _mm256_srai_epi32(y0, UV_L_RSH_16_444);
		y2 = _mm256_srai_epi32(y2, UV_L_RSH_16_444);
		y1 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(_mm256_slli_epi16(y1, LSFT_YCC_16))); //32bit YCC生成
		y0 = _mm256_add_epi32(y0, y1);
		y2 = _mm256_add_epi32(y2, y1);

		y0 = _mm256_packus_epi32(y0, y2);

		_mm256_stream_si256((__m256i *)dst_C, y0);
	}
	_mm256_zeroupper();
}
#endif
