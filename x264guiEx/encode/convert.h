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

//音声16bit->8bit変換
typedef void (*func_audio_16to8) (BYTE *dst, short *src, int n);

func_audio_16to8 get_audio_16to8_func(); //使用する音声16bit->8bit関数の選択

void convert_audio_16to8(BYTE *dst, short *src, int n);
void convert_audio_16to8_sse2(BYTE *dst, short *src, int n);


//動画変換
typedef void (*func_convert_frame) (void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);

func_convert_frame get_convert_func(int width, BOOL use10bit, BOOL interlaced, int output_csp, BOOL fullrange); //使用する関数の選択

BOOL malloc_pixel_data(CONVERT_CF_DATA * const pixel_data, int width, int height, int output_csp, BOOL use10bit); //映像バッファ用メモリ確保
void free_pixel_data(CONVERT_CF_DATA *pixel_data); //映像バッファ用メモリ開放

void sort_to_rgb(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void sort_to_rgb_sse2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void sort_to_rgb_ssse3(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yuy2_to_nv12(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yuy2_to_nv12_i(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yuy2_to_nv12_sse2_mod16(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yuy2_to_nv12_sse2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yuy2_to_nv12_i_sse2_mod16(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yuy2_to_nv12_i_sse2(void *frame, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_nv12_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv12_i_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

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
void convert_yc48_to_nv16_10bit_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv16_10bit_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv16_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv16_10bit_full_ssse3(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_nv16_10bit_full_sse4_1(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_yuv444(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_yuv444_10bit(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_yuv444_10bit_full(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

void convert_yc48_to_yuv444_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_yuv444_10bit_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);
void convert_yc48_to_yuv444_10bit_full_sse2(void *pixel, CONVERT_CF_DATA *pixel_data, const int width, const int height);

#endif //_CONVERT_H_
