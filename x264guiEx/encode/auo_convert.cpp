//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "auo.h"
#include "auo_util.h"
#include "auo_video.h"
#include "auo_frm.h"
#include "convert.h"

//音声の16bit->8bit変換の選択
func_audio_16to8 get_audio_16to8_func() {
	return (check_sse2()) ? convert_audio_16to8_sse2 : convert_audio_16to8;
}

enum eInterlace {
	A = -1, //区別の必要なし
	P = 0,  //プログレッシブ用
	I = 1   //インターレース用
};

typedef struct {
	int        input_from_aviutl; //Aviutlからの入力に使用する
	int        output_csp;        //出力色空間
	int        bt709conv;         //BT.709へのcolormatrix変換を行うかどうか
	BOOL       for_10bit;         //10bit用関数であるかどうか
	eInterlace for_interlaced;    //インタレース用関数であるかどうか
	int        fullrange;         //fullrange用関数であるかどうか
	DWORD      mod;               //幅(横解像)に制限(割り切れるかどうか)
	DWORD      SIMD;              //対応するSIMD
	func_convert_frame func;      //関数へのポインタ
} COVERT_FUNC_INFO;

static const DWORD NONE  = 0x00;
static const DWORD SSE2  = 0x01;
static const DWORD SSE3  = 0x02; //使用していない
static const DWORD SSSE3 = 0x04;
static const DWORD SSE41 = 0x08;
static const DWORD SSE42 = 0x10; //使用していない
static const DWORD AVX   = 0x20; //使用していない

static const BOOL BIT_8 = 0;  //8bit用
static const BOOL BIT10 = 1;  //10bit用

static const int _ALL__ = -1; //colormatrix変換を必要としない
static const int BT601_ = 0;  //colormatrix変換なし
static const int BT709S = 1;  //YC48 BT.601 -> YC48 BT.709 -> nv12/nv16/yuv444
static const int BT709F = 2;  //統合されたYC48 BT.601 -> nv12/nv16/yuv444 BT.709 変換

static const int _ALL = -1;   //区別を必要としない
static const int LIMI = 0;    //圧縮レンジ用
static const int FULL = 1;    //fullrange用

//変換関数のテーブル
//上からチェックするので、より厳しい条件で速い関数を上に書くこと
static const COVERT_FUNC_INFO FUNC_TABLE[] = {
	//YUY2 -> nv16(8bit)
	{ CF_YUY2, OUT_CSP_YUV422, _ALL__, BIT_8, A, _ALL,  1,  SSE2,             convert_yuy2_to_nv16_sse2 },
	{ CF_YUY2, OUT_CSP_YUV422, _ALL__, BIT_8, A, _ALL,  1,  NONE,             convert_yuy2_to_nv16 },
	//YC48 -> nv16(10bit)
#ifdef ENABLE_BT709_CONV
	{ CF_YC48, OUT_CSP_YUV422, BT709F, BIT10, A, LIMI,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_bt709_nv16_10bit_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV422, BT709F, BIT10, A, LIMI,  1,  SSE2,             convert_yc48_to_bt709_nv16_10bit_sse2 },
	{ CF_YC48, OUT_CSP_YUV422, BT709S, BIT10, A, LIMI,  1,  NONE,             convert_yc48_to_bt709_nv16_10bit },
	{ CF_YC48, OUT_CSP_YUV422, BT709F, BIT10, A, FULL,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_bt709_nv16_10bit_full_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV422, BT709F, BIT10, A, FULL,  1,  SSE2,             convert_yc48_to_bt709_nv16_10bit_full_sse2 },
	{ CF_YC48, OUT_CSP_YUV422, BT709S, BIT10, A, FULL,  1,  NONE,             convert_yc48_to_bt709_nv16_10bit_full },
#endif //ENABLE_BT709_CONV
	{ CF_YC48, OUT_CSP_YUV422, BT601_, BIT10, A, LIMI,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_nv16_10bit_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV422, BT601_, BIT10, A, LIMI,  1,  SSSE3|SSE2,       convert_yc48_to_nv16_10bit_ssse3 },
	{ CF_YC48, OUT_CSP_YUV422, BT601_, BIT10, A, LIMI,  1,  SSE2,             convert_yc48_to_nv16_10bit_sse2 },
	{ CF_YC48, OUT_CSP_YUV422, BT601_, BIT10, A, LIMI,  1,  NONE,             convert_yc48_to_nv16_10bit },
	{ CF_YC48, OUT_CSP_YUV422, BT601_, BIT10, A, FULL,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_nv16_10bit_full_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV422, BT601_, BIT10, A, FULL,  1,  SSSE3|SSE2,       convert_yc48_to_nv16_10bit_full_ssse3 },
	{ CF_YC48, OUT_CSP_YUV422, BT601_, BIT10, A, FULL,  1,  SSE2,             convert_yc48_to_nv16_10bit_full_sse2 },
	{ CF_YC48, OUT_CSP_YUV422, BT601_, BIT10, A, FULL,  1,  NONE,             convert_yc48_to_nv16_10bit_full },

	//YC48 -> yuv444(8bit)
#ifdef ENABLE_BT709_CONV
	{ CF_YC48, OUT_CSP_YUV444, BT709S, BIT_8, A, _ALL,  1,  SSE2,             convert_yc48_to_bt709_yuv444_sse2 },
	{ CF_YC48, OUT_CSP_YUV444, BT709S, BIT_8, A, _ALL,  1,  NONE,             convert_yc48_to_bt709_yuv444 },
#endif //ENABLE_BT709_CONV
	{ CF_YC48, OUT_CSP_YUV444, BT601_, BIT_8, A, _ALL,  1,  SSE2,             convert_yc48_to_yuv444_sse2 },
	{ CF_YC48, OUT_CSP_YUV444, BT601_, BIT_8, A, _ALL,  1,  NONE,             convert_yc48_to_yuv444 },
	//YC48 -> yuv444(10bit)
#ifdef ENABLE_BT709_CONV
	{ CF_YC48, OUT_CSP_YUV444, BT709F, BIT10, A, LIMI,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_bt709_yuv444_10bit_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV444, BT709F, BIT10, A, LIMI,  1,  SSE2,             convert_yc48_to_bt709_yuv444_10bit_sse2 },
	{ CF_YC48, OUT_CSP_YUV444, BT709S, BIT10, A, LIMI,  1,  NONE,             convert_yc48_to_bt709_yuv444_10bit },
	{ CF_YC48, OUT_CSP_YUV444, BT709F, BIT10, A, FULL,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_bt709_yuv444_10bit_full_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV444, BT709F, BIT10, A, FULL,  1,  SSE2,             convert_yc48_to_bt709_yuv444_10bit_full_sse2 },
	{ CF_YC48, OUT_CSP_YUV444, BT709S, BIT10, A, FULL,  1,  NONE,             convert_yc48_to_bt709_yuv444_10bit_full },
#endif //ENABLE_BT709_CONV
	{ CF_YC48, OUT_CSP_YUV444, BT601_, BIT10, A, LIMI,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_yuv444_10bit_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV444, BT601_, BIT10, A, LIMI,  1,  SSE2,             convert_yc48_to_yuv444_10bit_sse2 },
	{ CF_YC48, OUT_CSP_YUV444, BT601_, BIT10, A, LIMI,  1,  NONE,             convert_yc48_to_yuv444_10bit },
	{ CF_YC48, OUT_CSP_YUV444, BT601_, BIT10, A, FULL,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_yuv444_10bit_full_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV444, BT601_, BIT10, A, FULL,  1,  SSE2,             convert_yc48_to_yuv444_10bit_full_sse2 },
	{ CF_YC48, OUT_CSP_YUV444, BT601_, BIT10, A, FULL,  1,  NONE,             convert_yc48_to_yuv444_10bit_full },

	//RGB Sorting
	{ CF_RGB,  OUT_CSP_RGB,    _ALL__, BIT_8, A, _ALL,  1,  SSSE3,            sort_to_rgb_ssse3 },
	{ CF_RGB,  OUT_CSP_RGB,    _ALL__, BIT_8, A, _ALL,  1,  SSE2,             sort_to_rgb_sse2 },
	{ CF_RGB,  OUT_CSP_RGB,    _ALL__, BIT_8, A, _ALL,  1,  NONE,             sort_to_rgb },

	//YUY2 -> nv12(8bit)
	{ CF_YUY2, OUT_CSP_YUV420, _ALL__, BIT_8, P, _ALL, 16,  SSE2,             convert_yuy2_to_nv12_sse2_mod16 },
	{ CF_YUY2, OUT_CSP_YUV420, _ALL__, BIT_8, P, _ALL,  1,  SSE2,             convert_yuy2_to_nv12_sse2 },
	{ CF_YUY2, OUT_CSP_YUV420, _ALL__, BIT_8, P, _ALL,  1,  NONE,             convert_yuy2_to_nv12 },
	{ CF_YUY2, OUT_CSP_YUV420, _ALL__, BIT_8, I, _ALL, 16,  SSE2,             convert_yuy2_to_nv12_i_sse2_mod16 },
	{ CF_YUY2, OUT_CSP_YUV420, _ALL__, BIT_8, I, _ALL,  1,  SSE2,             convert_yuy2_to_nv12_i_sse2 },
	{ CF_YUY2, OUT_CSP_YUV420, _ALL__, BIT_8, I, _ALL,  1,  NONE,             convert_yuy2_to_nv12_i },

	//YC48 -> nv12(10bit)
#ifdef ENABLE_BT709_CONV
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, P, LIMI,  8,  SSE41|SSSE3|SSE2, convert_yc48_to_bt709_nv12_10bit_sse4_1_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, P, LIMI,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_bt709_nv12_10bit_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, P, LIMI,  8,  SSE2,             convert_yc48_to_bt709_nv12_10bit_sse2_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, P, LIMI,  1,  SSE2,             convert_yc48_to_bt709_nv12_10bit_sse2 },
	{ CF_YC48, OUT_CSP_YUV420, BT709S, BIT10, P, LIMI,  1,  NONE,             convert_yc48_to_bt709_nv12_10bit },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, P, FULL,  8,  SSE41|SSSE3|SSE2, convert_yc48_to_bt709_nv12_10bit_full_sse4_1_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, P, FULL,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_bt709_nv12_10bit_full_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, P, FULL,  8,  SSE2,             convert_yc48_to_bt709_nv12_10bit_full_sse2_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, P, FULL,  1,  SSE2,             convert_yc48_to_bt709_nv12_10bit_full_sse2 },
	{ CF_YC48, OUT_CSP_YUV420, BT709S, BIT10, P, FULL,  1,  NONE,             convert_yc48_to_bt709_nv12_10bit_full },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, I, LIMI,  8,  SSE41|SSSE3|SSE2, convert_yc48_to_bt709_nv12_i_10bit_sse4_1_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, I, LIMI,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_bt709_nv12_i_10bit_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, I, LIMI,  8,  SSE2,             convert_yc48_to_bt709_nv12_i_10bit_sse2_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, I, LIMI,  1,  SSE2,             convert_yc48_to_bt709_nv12_i_10bit_sse2 },
	{ CF_YC48, OUT_CSP_YUV420, BT709S, BIT10, I, LIMI,  1,  NONE,             convert_yc48_to_bt709_nv12_i_10bit },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, I, FULL,  8,  SSE41|SSSE3|SSE2, convert_yc48_to_bt709_nv12_i_10bit_full_sse4_1_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, I, FULL,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_bt709_nv12_i_10bit_full_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, I, FULL,  8,  SSE2,             convert_yc48_to_bt709_nv12_i_10bit_full_sse2_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT709F, BIT10, I, FULL,  1,  SSE2,             convert_yc48_to_bt709_nv12_i_10bit_full_sse2 },
	{ CF_YC48, OUT_CSP_YUV420, BT709S, BIT10, I, FULL,  1,  NONE,             convert_yc48_to_bt709_nv12_i_10bit_full },
#endif //ENABLE_BT709_CONV
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, LIMI,  8,  SSE41|SSSE3|SSE2, convert_yc48_to_nv12_10bit_sse4_1_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, LIMI,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_nv12_10bit_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, LIMI,  8,  SSSE3|SSE2,       convert_yc48_to_nv12_10bit_ssse3_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, LIMI,  1,  SSSE3|SSE2,       convert_yc48_to_nv12_10bit_ssse3 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, LIMI,  8,  SSE2,             convert_yc48_to_nv12_10bit_sse2_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, LIMI,  1,  SSE2,             convert_yc48_to_nv12_10bit_sse2 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, LIMI,  1,  NONE,             convert_yc48_to_nv12_10bit },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, FULL,  8,  SSE41|SSSE3|SSE2, convert_yc48_to_nv12_10bit_full_sse4_1_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, FULL,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_nv12_10bit_full_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, FULL,  8,  SSSE3|SSE2,       convert_yc48_to_nv12_10bit_full_ssse3_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, FULL,  1,  SSSE3|SSE2,       convert_yc48_to_nv12_10bit_full_ssse3 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, FULL,  8,  SSE2,             convert_yc48_to_nv12_10bit_full_sse2_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, FULL,  1,  SSE2,             convert_yc48_to_nv12_10bit_full_sse2 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, P, FULL,  1,  NONE,             convert_yc48_to_nv12_10bit_full },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, LIMI,  8,  SSE41|SSSE3|SSE2, convert_yc48_to_nv12_i_10bit_sse4_1_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, LIMI,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_nv12_i_10bit_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, LIMI,  8,  SSSE3|SSE2,       convert_yc48_to_nv12_i_10bit_ssse3_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, LIMI,  1,  SSSE3|SSE2,       convert_yc48_to_nv12_i_10bit_ssse3 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, LIMI,  8,  SSE2,             convert_yc48_to_nv12_i_10bit_sse2_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, LIMI,  1,  SSE2,             convert_yc48_to_nv12_i_10bit_sse2 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, LIMI,  1,  NONE,             convert_yc48_to_nv12_i_10bit },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, FULL,  8,  SSE41|SSSE3|SSE2, convert_yc48_to_nv12_i_10bit_full_sse4_1_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, FULL,  1,  SSE41|SSSE3|SSE2, convert_yc48_to_nv12_i_10bit_full_sse4_1 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, FULL,  8,  SSSE3|SSE2,       convert_yc48_to_nv12_i_10bit_full_ssse3_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, FULL,  1,  SSSE3|SSE2,       convert_yc48_to_nv12_i_10bit_full_ssse3 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, FULL,  8,  SSE2,             convert_yc48_to_nv12_i_10bit_full_sse2_mod8 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, FULL,  1,  SSE2,             convert_yc48_to_nv12_i_10bit_full_sse2 },
	{ CF_YC48, OUT_CSP_YUV420, BT601_, BIT10, I, FULL,  1,  NONE,             convert_yc48_to_nv12_i_10bit_full },

	{ 0, 0, 0, 0, A, 0, 0, 0, NULL }
};

//使用する関数を選択する
//static func_convert_frame get_convert_func_yuv422(BOOL use10bit, BOOL fullrange, BOOL convert_yc48_to_bt709) {
//	if (!use10bit) {
//		return     (check_sse2())     ? convert_yuy2_to_nv16_sse2              : convert_yuy2_to_nv16;
//	} else {
//		if (convert_yc48_to_bt709) {
//			if (!fullrange) {
//				if      (check_sse4_1())
//					return              convert_yc48_to_bt709_nv16_10bit_sse4_1;
//				else if (check_ssse3())
//					return              convert_yc48_to_bt709_nv16_10bit_ssse3;
//				else if (check_sse2())
//					return              convert_yc48_to_bt709_nv16_10bit_sse2;
//				else
//					return              convert_yc48_to_bt709_nv16_10bit;
//			} else {
//				if      (check_sse4_1())
//					return              convert_yc48_to_bt709_nv16_10bit_full_sse4_1;
//				else if (check_ssse3())
//					return              convert_yc48_to_bt709_nv16_10bit_full_ssse3;
//				else if (check_sse2())
//					return              convert_yc48_to_bt709_nv16_10bit_full_sse2;
//				else
//					return              convert_yc48_to_bt709_nv16_10bit_full;
//			}
//		} else {
//			if (!fullrange) {
//				if      (check_sse4_1())
//					return              convert_yc48_to_nv16_10bit_sse4_1;
//				else if (check_ssse3())
//					return              convert_yc48_to_nv16_10bit_ssse3;
//				else if (check_sse2())
//					return              convert_yc48_to_nv16_10bit_sse2;
//				else
//					return              convert_yc48_to_nv16_10bit;
//			} else {
//				if      (check_sse4_1())
//					return              convert_yc48_to_nv16_10bit_full_sse4_1;
//				else if (check_ssse3())
//					return              convert_yc48_to_nv16_10bit_full_ssse3;
//				else if (check_sse2())
//					return              convert_yc48_to_nv16_10bit_full_sse2;
//				else
//					return              convert_yc48_to_nv16_10bit_full;
//			}
//		}
//	}
//}
//
//static func_convert_frame get_convert_func_yuv444(BOOL use10bit, BOOL fullrange, BOOL convert_yc48_to_bt709) {
//	if (!use10bit) {
//		return         (check_sse2()) ? convert_yc48_to_yuv444_sse2            : convert_yc48_to_yuv444;
//	} else {
//		if (convert_yc48_to_bt709) {		
//			if (!fullrange) {
//				if         (check_sse4_1())
//					return                  convert_yc48_to_bt709_yuv444_10bit_sse4_1;
//				else if    (check_sse2())
//					return                  convert_yc48_to_bt709_yuv444_10bit_sse2;
//				else
//					return                  convert_yc48_to_bt709_yuv444_10bit;
//			} else
//				return     (check_sse2()) ? convert_yc48_to_bt709_yuv444_10bit_full_sse2 : convert_yc48_to_bt709_yuv444_10bit_full;
//		} else {
//			if (!fullrange) {
//				if         (check_sse4_1())
//					return                  convert_yc48_to_yuv444_10bit_sse4_1;
//				else if    (check_sse2())
//					return                  convert_yc48_to_yuv444_10bit_sse2;
//				else
//					return                  convert_yc48_to_yuv444_10bit;
//			} else
//				return     (check_sse2()) ? convert_yc48_to_yuv444_10bit_full_sse2 : convert_yc48_to_yuv444_10bit_full;
//		}
//	}
//}
//
//static func_convert_frame get_convert_func_rgb() {
//	if      (check_ssse3())
//		return sort_to_rgb_ssse3;
//	else if (check_sse2())
//		return sort_to_rgb_sse2;
//	else
//		return sort_to_rgb;
//}
//
//static func_convert_frame get_convert_func_yuv420(int width, BOOL use10bit, BOOL interlaced, BOOL fullrange, BOOL convert_yc48_to_bt709) {
//	if (!use10bit) {
//		if (!interlaced)
//			return     (check_sse2()) ? ((width%16==0) ? convert_yuy2_to_nv12_sse2_mod16              : convert_yuy2_to_nv12_sse2)              : convert_yuy2_to_nv12;
//		else
//			return     (check_sse2()) ? ((width%16==0) ? convert_yuy2_to_nv12_i_sse2_mod16            : convert_yuy2_to_nv12_i_sse2)            : convert_yuy2_to_nv12_i;
//	} else {
//		if (convert_yc48_to_bt709) {
//			if (!interlaced) {
//				if (!fullrange) {
//					if      (check_sse4_1())
//						return               (width%8==0)  ? convert_yc48_to_bt709_nv12_10bit_sse4_1_mod8       : convert_yc48_to_bt709_nv12_10bit_sse4_1;
//					else if (check_ssse3())
//						return               (width%8==0)  ? convert_yc48_to_bt709_nv12_10bit_ssse3_mod8        : convert_yc48_to_bt709_nv12_10bit_ssse3;
//					else if (check_sse2())
//						return               (width%8==0)  ? convert_yc48_to_bt709_nv12_10bit_sse2_mod8         : convert_yc48_to_bt709_nv12_10bit_sse2;
//					else
//						return                               convert_yc48_to_bt709_nv12_10bit;
//				} else {
//					if      (check_sse4_1())
//						return               (width%8==0)  ? convert_yc48_to_bt709_nv12_10bit_full_sse4_1_mod8  : convert_yc48_to_bt709_nv12_10bit_full_sse4_1;
//					else if (check_ssse3())
//						return               (width%8==0)  ? convert_yc48_to_bt709_nv12_10bit_full_ssse3_mod8   : convert_yc48_to_bt709_nv12_10bit_full_ssse3;
//					else if (check_sse2())
//						return               (width%8==0)  ? convert_yc48_to_bt709_nv12_10bit_full_sse2_mod8    : convert_yc48_to_bt709_nv12_10bit_full_sse2;
//					else
//						return                               convert_yc48_to_bt709_nv12_10bit_full;
//				}
//			} else {
//				if (!fullrange) {
//					if      (check_sse4_1())
//						return               (width%8==0)  ? convert_yc48_to_bt709_nv12_i_10bit_sse4_1_mod8      : convert_yc48_to_bt709_nv12_i_10bit_sse4_1;
//					else if (check_ssse3())
//						return               (width%8==0)  ? convert_yc48_to_bt709_nv12_i_10bit_ssse3_mod8       : convert_yc48_to_bt709_nv12_i_10bit_ssse3;
//					else if (check_sse2())
//						return               (width%8==0)  ? convert_yc48_to_bt709_nv12_i_10bit_sse2_mod8        : convert_yc48_to_bt709_nv12_i_10bit_sse2;
//					else
//						return                               convert_yc48_to_bt709_nv12_i_10bit;
//				} else {
//					if      (check_sse4_1())
//						return               (width%8==0)  ? convert_yc48_to_bt709_nv12_i_10bit_full_sse4_1_mod8 : convert_yc48_to_bt709_nv12_i_10bit_full_sse4_1;
//					else if (check_ssse3())
//						return               (width%8==0)  ? convert_yc48_to_bt709_nv12_i_10bit_full_ssse3_mod8  : convert_yc48_to_bt709_nv12_i_10bit_full_ssse3;
//					else if (check_sse2())
//						return               (width%8==0)  ? convert_yc48_to_bt709_nv12_i_10bit_full_sse2_mod8   : convert_yc48_to_bt709_nv12_i_10bit_full_sse2;
//					else
//						return                               convert_yc48_to_bt709_nv12_i_10bit_full;
//				}
//			}
//		} else {
//			if (!interlaced) {
//				if (!fullrange) {
//					if      (check_sse4_1())
//						return               (width%8==0)  ? convert_yc48_to_nv12_10bit_sse4_1_mod8       : convert_yc48_to_nv12_10bit_sse4_1;
//					else if (check_ssse3())
//						return               (width%8==0)  ? convert_yc48_to_nv12_10bit_ssse3_mod8        : convert_yc48_to_nv12_10bit_ssse3;
//					else if (check_sse2())
//						return               (width%8==0)  ? convert_yc48_to_nv12_10bit_sse2_mod8         : convert_yc48_to_nv12_10bit_sse2;
//					else
//						return                               convert_yc48_to_nv12_10bit;
//				} else {
//					if      (check_sse4_1())
//						return               (width%8==0)  ? convert_yc48_to_nv12_10bit_full_sse4_1_mod8  : convert_yc48_to_nv12_10bit_full_sse4_1;
//					else if (check_ssse3())
//						return               (width%8==0)  ? convert_yc48_to_nv12_10bit_full_ssse3_mod8   : convert_yc48_to_nv12_10bit_full_ssse3;
//					else if (check_sse2())
//						return               (width%8==0)  ? convert_yc48_to_nv12_10bit_full_sse2_mod8    : convert_yc48_to_nv12_10bit_full_sse2;
//					else
//						return                               convert_yc48_to_nv12_10bit_full;
//				}
//			} else {
//				if (!fullrange) {
//					if      (check_sse4_1())
//						return               (width%8==0)  ? convert_yc48_to_nv12_i_10bit_sse4_1_mod8      : convert_yc48_to_nv12_i_10bit_sse4_1;
//					else if (check_ssse3())
//						return               (width%8==0)  ? convert_yc48_to_nv12_i_10bit_ssse3_mod8       : convert_yc48_to_nv12_i_10bit_ssse3;
//					else if (check_sse2())
//						return               (width%8==0)  ? convert_yc48_to_nv12_i_10bit_sse2_mod8        : convert_yc48_to_nv12_i_10bit_sse2;
//					else
//						return                               convert_yc48_to_nv12_i_10bit;
//				} else {
//					if      (check_sse4_1())
//						return               (width%8==0)  ? convert_yc48_to_nv12_i_10bit_full_sse4_1_mod8 : convert_yc48_to_nv12_i_10bit_full_sse4_1;
//					else if (check_ssse3())
//						return               (width%8==0)  ? convert_yc48_to_nv12_i_10bit_full_ssse3_mod8  : convert_yc48_to_nv12_i_10bit_full_ssse3;
//					else if (check_sse2())
//						return               (width%8==0)  ? convert_yc48_to_nv12_i_10bit_full_sse2_mod8   : convert_yc48_to_nv12_i_10bit_full_sse2;
//					else
//						return                               convert_yc48_to_nv12_i_10bit_full;
//				}
//			}
//		}
//	}
//}

static DWORD get_availableSIMD() {
	DWORD simd = NONE;
	     if (check_sse4_1()) simd = (SSE2|SSE3|SSSE3|SSE41);
	else if (check_ssse3())  simd = (SSE2|SSE3|SSSE3);
	else if (check_sse3())   simd = (SSE2|SSE3);
	else if (check_sse2())   simd = (SSE2);
	return simd;
}

static void build_simd_info(DWORD simd, char *buf, DWORD nSize) {
	ZeroMemory(buf, nSize);
	if (simd) {
		strcpy_s(buf, nSize, ", using");
		if (simd & SSE2)  strcat_s(buf, nSize, " SSE2");
		if (simd & SSE3)  strcat_s(buf, nSize, " SSE3");
		if (simd & SSSE3) strcat_s(buf, nSize, " SSSE3");
		if (simd & SSE41) strcat_s(buf, nSize, " SSE4.1");
	}
}

static void auo_write_func_info(const COVERT_FUNC_INFO *func_info) {
	if (func_info->bt709conv == BT709S)
		write_log_auo_line_fmt(LOG_INFO, "converting YC48 BT.601 -> BT.709");
	char simd_buf[128];
	build_simd_info(func_info->SIMD, simd_buf, sizeof(simd_buf));

	if (func_info->output_csp == OUT_CSP_RGB) {
		write_log_auo_line_fmt(LOG_INFO, "Sorting RGB%s", simd_buf);
		return;
	}

	char *interlaced = NULL;
	switch (func_info->for_interlaced) {
		case P: interlaced = "p"; break;
		case I: interlaced = "i"; break;
		case A: 
		default:interlaced = ""; break;
	}
	char *colmat_from = (func_info->bt709conv == BT709F) ? "(BT.601)" : "";
	char out_append[256] = { 0 };
	if (func_info->for_10bit == BIT10)  strcpy_s(out_append, sizeof(out_append), " 10bit,");
	if (func_info->bt709conv == BT709F) strcat_s(out_append, sizeof(out_append), " BT.709,");
	if (func_info->fullrange == FULL)   strcat_s(out_append, sizeof(out_append), " fullrange,");
	if (char_has_length(out_append)) {
		out_append[0] = '('; out_append[strlen(out_append)-1] = ')';
	}

	write_log_auo_line_fmt(LOG_INFO, "converting %s%s -> %s%s%s%s",
		CF_NAME[func_info->input_from_aviutl],
		colmat_from,
		specify_csp[func_info->output_csp],
		interlaced,
		out_append,
		simd_buf);
};

//C4189 : ローカル変数が初期化されましたが、参照されていません。
#pragma warning( push )
#pragma warning( disable: 4189 )
//使用する関数を選択する
func_convert_frame get_convert_func(int width, int height, BOOL use10bit, BOOL interlaced, int output_csp, BOOL fullrange, int yc48_colmat_conv) {
	BOOL convert_yc48_to_bt709 = (yc48_colmat_conv == YC48_COLMAT_CONV_BT709 ||
		                         (yc48_colmat_conv == YC48_COLMAT_CONV_AUTO  && height >= COLOR_MATRIX_THRESHOLD));

	DWORD availableSIMD = get_availableSIMD();

	const COVERT_FUNC_INFO *func_info = NULL;
	for (int i = 0; FUNC_TABLE[i].func; i++) {
		if (FUNC_TABLE[i].output_csp != output_csp)
			continue;
		if (FUNC_TABLE[i].for_10bit != use10bit)
			continue;
		if (FUNC_TABLE[i].for_interlaced != A &&
			FUNC_TABLE[i].for_interlaced != (eInterlace)interlaced)
			continue;
#ifdef ENABLE_BT709_CONV
		if (FUNC_TABLE[i].bt709conv != _ALL__ &&
			(FUNC_TABLE[i].bt709conv > 0) != convert_yc48_to_bt709)
			continue;
#endif //ENABLE_BT709_CONV
		if (FUNC_TABLE[i].fullrange != _ALL &&
			FUNC_TABLE[i].fullrange != fullrange)
			continue;
		if ((width % FUNC_TABLE[i].mod) != 0)
			continue;
		if ((FUNC_TABLE[i].SIMD & availableSIMD) != FUNC_TABLE[i].SIMD)
			continue;

		func_info = &FUNC_TABLE[i];
		break;
	}

	if (func_info == NULL)
		return NULL;

	auo_write_func_info(func_info);
	return func_info->func;
}
#pragma warning( pop )

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