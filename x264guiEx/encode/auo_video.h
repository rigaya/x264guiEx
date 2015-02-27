//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx/2.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _AUO_VIDEO_H_
#define _AUO_VIDEO_H_

#include "output.h"
#include "auo_conf.h"
#include "auo_system.h"

typedef struct {
	DWORD FOURCC;   //FOURCC
	DWORD size;  //1ピクセルあたりバイト数
} COLORFORMAT_DATA;

enum {
	CF_YUY2 = 0,
	CF_YC48 = 1,
	CF_RGB  = 2,
	CF_LW48 = 3,
};
static const char * const CF_NAME[] = { "YUY2", "YC48", "RGB", "LW48" };
static const COLORFORMAT_DATA COLORFORMATS[] = {
	{ MAKEFOURCC('Y', 'U', 'Y', '2'), 2 }, //YUY2
	{ MAKEFOURCC('Y', 'C', '4', '8'), 6 }, //YC48
	{ NULL,                           3 }, //RGB
	{ MAKEFOURCC('L', 'W', '4', '8'), 6 }  //LW48
};

BOOL check_x264_mp4_output(const char *exe_path, const char *temp_filename);

BOOL setup_afsvideo(const OUTPUT_INFO *oip, const SYSTEM_DATA *sys_dat, CONF_GUIEX *conf, PRM_ENC *pe);
void close_afsvideo(PRM_ENC *pe);

AUO_RESULT video_output(CONF_GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat);

#endif //_AUO_VIDEO_H_