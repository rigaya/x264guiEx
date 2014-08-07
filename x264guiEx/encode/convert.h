//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _CONVERT_H_
#define _CONVERT_H_

#include <Windows.h>

typedef	struct {
	short	y;					//	画素(輝度    )データ (     0 ～ 4096 )
	short	cb;					//	画素(色差(青))データ ( -2048 ～ 2048 )
	short	cr;					//	画素(色差(赤))データ ( -2048 ～ 2048 )
								//	画素データは範囲外に出ていることがあります
								//	また範囲内に収めなくてもかまいません
} PIXEL_YC;

typedef struct {
	int   count;       //planarの枚数。packedなら1
	BYTE *data[3];     //planarの先頭へのポインタ
	int   size[3];     //planarのサイズ
	int   total_size;  //全planarのサイズの総和
} CONVERT_CF_DATA;

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
///   (YUV444)U,V = clamp(((cb,cr + 2048) * 7 + 66)>>7) + 16, 0, 255)
///   (YUV420)U,V = clamp(((cb0,cr0 + cb1,cr1 + 4096) * 7 + 66*2)>>8) + 16, 0, 255)
///
///   YC48 -> YUV(10bit) YC圧縮
///
///           Y   = clamp(((y * 219 + (383>>2))>>10) + 64, 0, 1023)
///   (YUV444)U,V = clamp(((cb,cr + 2048) * 7 + (66>>2))>>5) + 64, 0, 1023)
///   (YUV420)U,V = clamp(((cb0,cr0 + cb1,cr1 + 4096) * 7 + (66>>2)*2)>>6) + 64, 0, 1023)
///
///   YC48 -> YUV(10bit) fullrange
///
///           Y   = clamp(((y * 3517 + 3867)>>14) + 64, 0, 1023)
///   (YUV444)U,V = clamp(((cb,cr + 2048) * 899 +  0)>>12) + 64, 0, 1023)
///   (YUV420)U,V = clamp(((cb0,cr0 + cb1,cr1 + 4096) * 899 + 0*2)>>13) + 64, 0, 1023)
///

static const int UV_OFFSET     = (1<<12);
static const int UV_OFFSET_444 = (1<<11);

static const int RSFT_ONE      = 15;
static const int RSFT_LIMIT_10 = 6;
static const int LSFT_YCC_10   = 6;

//各ビットの最大値
static const int LIMIT_8    = (1<< 8) - 1;
static const int LIMIT_10   = (1<<10) - 1;

//YC48 bt601 -> bt709
static const int COLMAT_MUL_YY   = 16384;
static const int COLMAT_MUL_Yb   = -1934;
static const int COLMAT_MUL_Yr   = -3485;
static const int COLMAT_MUL_bb   = 16690;
static const int COLMAT_MUL_br   =  1886;
static const int COLMAT_MUL_rb   =  1220;
static const int COLMAT_MUL_rr   = 16796;
static const int COLMAT_MUL_RSHT =    14;

//YC圧縮レンジ用定数
static const int Y_L_MUL    = 219;
static const int Y_L_ADD_8  = 383;
static const int Y_L_ADD_10 = Y_L_ADD_8>>2;
static const int Y_L_RSH_8  = 12;
static const int Y_L_RSH_10 = 10;
static const int Y_L_YCC_8  = 16;
static const int Y_L_YCC_10 = 64;

static const int UV_L_MUL        = 7;
static const int UV_L_ADD_8_444  = 66;
static const int UV_L_ADD_8      = UV_L_ADD_8_444<<1;
static const int UV_L_ADD_10_444 = UV_L_ADD_8_444>>2;
static const int UV_L_ADD_10     = UV_L_ADD_8_444>>1;
static const int UV_L_RSH_8      = 8;
static const int UV_L_RSH_10     = 6;
static const int UV_L_RSH_8_444  = 7;
static const int UV_L_RSH_10_444 = 5;
static const int UV_L_YCC_8      = 16;
static const int UV_L_YCC_10     = 64;

//フルレンジ用定数(10bitのみ)
static const int Y_F_MUL         = 3517;
static const int Y_F_ADD_10      = 3867;
static const int Y_F_RSH_10      = 14;
static const int Y_F_YCC_10      = 64;
static const int UV_F_MUL        = 899;
static const int UV_F_ADD_10_444 = 0;
static const int UV_F_ADD_10     = (UV_F_ADD_10_444<<1);
static const int UV_F_RSH_10     = 13;
static const int UV_F_RSH_10_444 = 12;
static const int UV_F_YCC_10     = 64;

#define ALIGN16_CONST_ARRAY static const _declspec(align(16))

ALIGN16_CONST_ARRAY short Array_Y_L_MA_8[]       = { Y_L_MUL,  Y_L_ADD_8,       Y_L_MUL,  Y_L_ADD_8,       Y_L_MUL,  Y_L_ADD_8,       Y_L_MUL,  Y_L_ADD_8     };
ALIGN16_CONST_ARRAY short Array_UV_L_MA_8[]      = {UV_L_MUL, UV_L_ADD_8,      UV_L_MUL, UV_L_ADD_8,      UV_L_MUL, UV_L_ADD_8,      UV_L_MUL, UV_L_ADD_8     };
ALIGN16_CONST_ARRAY short Array_UV_L_MA_8_444[]  = {UV_L_MUL, UV_L_ADD_8_444,  UV_L_MUL, UV_L_ADD_8_444,  UV_L_MUL, UV_L_ADD_8_444,  UV_L_MUL, UV_L_ADD_8_444 };
ALIGN16_CONST_ARRAY short Array_Y_L_MA_10[]      = { Y_L_MUL,  Y_L_ADD_10,      Y_L_MUL,  Y_L_ADD_10,      Y_L_MUL,  Y_L_ADD_10,      Y_L_MUL,  Y_L_ADD_10    };
ALIGN16_CONST_ARRAY short Array_UV_L_MA_10[]     = {UV_L_MUL, UV_L_ADD_10,     UV_L_MUL, UV_L_ADD_10,     UV_L_MUL, UV_L_ADD_10,     UV_L_MUL, UV_L_ADD_10    };
ALIGN16_CONST_ARRAY short Array_UV_L_MA_10_444[] = {UV_L_MUL, UV_L_ADD_10_444, UV_L_MUL, UV_L_ADD_10_444, UV_L_MUL, UV_L_ADD_10_444, UV_L_MUL, UV_L_ADD_10_444};
ALIGN16_CONST_ARRAY short Array_Y_F_MA_10[]      = { Y_F_MUL,  Y_F_ADD_10,      Y_F_MUL,  Y_F_ADD_10,      Y_F_MUL,  Y_F_ADD_10,      Y_F_MUL,  Y_F_ADD_10    };
ALIGN16_CONST_ARRAY short Array_UV_F_MA_10[]     = {UV_F_MUL, UV_F_ADD_10,     UV_F_MUL, UV_F_ADD_10,     UV_F_MUL, UV_F_ADD_10,     UV_F_MUL, UV_F_ADD_10    };
ALIGN16_CONST_ARRAY short Array_UV_F_MA_10_444[] = {UV_F_MUL, UV_F_ADD_10_444, UV_F_MUL, UV_F_ADD_10_444, UV_F_MUL, UV_F_ADD_10_444, UV_F_MUL, UV_F_ADD_10_444};

ALIGN16_CONST_ARRAY short Array_COLMAT_Y[]       = {COLMAT_MUL_Yb, COLMAT_MUL_Yr, COLMAT_MUL_Yb, COLMAT_MUL_Yr, COLMAT_MUL_Yb, COLMAT_MUL_Yr, COLMAT_MUL_Yb, COLMAT_MUL_Yr};
ALIGN16_CONST_ARRAY short Array_COLMAT_UV[]      = {COLMAT_MUL_rb, COLMAT_MUL_rr, COLMAT_MUL_rb, COLMAT_MUL_br, COLMAT_MUL_bb, COLMAT_MUL_rr, COLMAT_MUL_rb, COLMAT_MUL_br};

ALIGN16_CONST_ARRAY short Array_MASK_YCP2Y[]     = {0, -1, 0, 0, -1, 0, 0, -1};
ALIGN16_CONST_ARRAY short Array_MASK_YCP2UV[]    = {-1, 0, 0, 0, 0, -1, -1, 0};
ALIGN16_CONST_ARRAY BYTE  Array_SUFFLE_YCP_Y[]   = {0, 1, 6, 7, 12, 13, 2, 3, 8, 9, 14, 15, 4, 5, 10, 11};

#define xC_ZERO           _mm_setzero_si128()
#define xC_ONE            _mm_set1_epi16(1)
#define xC_YCC_8          _mm_set1_epi16(Y_L_YCC_8)
#define xC_YCC_10         _mm_set1_epi16(Y_L_YCC_10)
#define xC_UV_OFFSET      _mm_set1_epi16(UV_OFFSET)
#define xC_UV_OFFSET_444  _mm_set1_epi16(UV_OFFSET_444)
#define  xC_Y_L_MA_8      _mm_load_si128((__m128i*) Array_Y_L_MA_8)
#define xC_UV_L_MA_8      _mm_load_si128((__m128i*)Array_UV_L_MA_8)
#define xC_UV_L_MA_8_444  _mm_load_si128((__m128i*)Array_UV_L_MA_8_444)
#define  xC_Y_L_MA_10     _mm_load_si128((__m128i*) Array_Y_L_MA_10)
#define xC_UV_L_MA_10     _mm_load_si128((__m128i*)Array_UV_L_MA_10)
#define xC_UV_L_MA_10_444 _mm_load_si128((__m128i*)Array_UV_L_MA_10_444)
#define  xC_Y_F_MA_10     _mm_load_si128((__m128i*) Array_Y_F_MA_10)
#define xC_UV_F_MA_10     _mm_load_si128((__m128i*)Array_UV_F_MA_10)
#define xC_UV_F_MA_10_444 _mm_load_si128((__m128i*)Array_UV_F_MA_10_444)
#define xC_LIMITMAX       _mm_set1_epi16(LIMIT_10)

#define xC_COLMAT_Y      _mm_load_si128((__m128i*)Array_COLMAT_Y)
#define xC_COLMAT_UV     _mm_load_si128((__m128i*)Array_COLMAT_UV)

#define MASK_YCP2Y       _mm_load_si128((__m128i*)Array_MASK_YCP2Y)
#define MASK_YCP2UV      _mm_load_si128((__m128i*)Array_MASK_YCP2UV)
#define SUFFLE_YCP_Y     _mm_load_si128((__m128i*)Array_SUFFLE_YCP_Y)
//#define SUFFLE_YCP_UV = _mm_set_epi8( 9,  8,  7,  6, 13, 12, 11, 10,  1,  0, 15, 14,  5,  4,  3 ,  2);

//音声16bit->8bit変換
typedef void (*func_audio_16to8) (BYTE *dst, short *src, int n);

void convert_audio_16to8(BYTE *dst, short *src, int n);
void convert_audio_16to8_sse2(BYTE *dst, short *src, int n);


//動画変換
typedef void (*func_convert_frame) (void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void sort_to_rgb(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void sort_to_rgb_sse2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void sort_to_rgb_ssse3(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yuy2_to_nv12(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yuy2_to_nv12_i(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yuy2_to_nv12_sse2_mod16(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yuy2_to_nv12_sse2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yuy2_to_nv12_i_sse2_mod16(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yuy2_to_nv12_i_sse2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);

#if (_MSC_VER >= 1600) //AVX のサポートはVC++2010(VS10)から
void convert_yuy2_to_nv12_avx_mod16(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yuy2_to_nv12_avx(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yuy2_to_nv12_i_avx_mod16(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yuy2_to_nv12_i_avx(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
#endif //_MSC_VER >= 1600

void convert_yc48_to_nv12_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

#if (_MSC_VER >= 1600)
void convert_yc48_to_nv12_10bit_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_full_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_full_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_avx_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_avx_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_full_avx_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_full_avx_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
#endif //_MSC_VER >= 1600

void convert_yc48_to_nv12_10bit_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_full_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_full_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_sse4_1_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_sse4_1_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_full_sse4_1_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_full_sse4_1_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_nv12_10bit_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_full_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_full_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_ssse3_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_ssse3_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_full_ssse3_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_full_ssse3_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_nv12_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_sse2_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_sse2_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_full_sse2_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_full_sse2_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yuy2_to_nv16(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yuy2_to_nv16_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_nv16_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv16_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_nv16_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv16_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv16_10bit_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv16_10bit_full_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv16_10bit_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv16_10bit_full_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
#if (_MSC_VER >= 1600)
void convert_yc48_to_nv16_10bit_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv16_10bit_full_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
#endif

void convert_yc48_to_yuv444(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_yuv444_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_yuv444_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_yuv444_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_yuv444_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_yuv444_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_yuv444_10bit_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_yuv444_10bit_full_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
#if (_MSC_VER >= 1600)
void convert_yc48_to_yuv444_10bit_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_yuv444_10bit_full_avx(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
#endif

void convert_yc48_to_bt709_nv12_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_i_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_i_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_bt709_nv12_10bit_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_i_10bit_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_10bit_full_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_i_10bit_full_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_10bit_sse4_1_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_i_10bit_sse4_1_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_10bit_full_sse4_1_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_i_10bit_full_sse4_1_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_i_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_i_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_10bit_sse2_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_i_10bit_sse2_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_10bit_full_sse2_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv12_i_10bit_full_sse2_mod8(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_bt709_nv16_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv16_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_bt709_nv16_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv16_10bit_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv16_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_nv16_10bit_full_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_bt709_yuv444(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_yuv444_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_yuv444_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_bt709_yuv444_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_yuv444_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_yuv444_10bit_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_yuv444_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_bt709_yuv444_10bit_full_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

#endif //_CONVERT_H_
