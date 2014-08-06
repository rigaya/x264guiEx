//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _AUO_H_
#define _AUO_H_

#include <Windows.h>

const int   MAX_PATH_LEN          = 1024; //NTFSでは32768文字らしいが...いらんやろ
const int   MAX_APPENDIX_LEN      = 63; //適当

const int   MAX_CMD_LEN           = 8192; //コマンドラインの最大長はよくわからん

const DWORD AUDIO_BUFFER_DEFAULT  = 48000;
const DWORD AUDIO_BUFFER_MAX      = AUDIO_BUFFER_DEFAULT * 30;

const int   VIDEO_OUTPUT_DISABLED = -2;
const int   VIDEO_OUTPUT_RAW      = -1;
const int   VIDEO_OUTPUT_MP4      = 0;
const int   VIDEO_OUTPUT_MKV      = 1;

const int   MUXER_DISABLED = VIDEO_OUTPUT_DISABLED;
const int   MUXER_MP4      = VIDEO_OUTPUT_MP4;
const int   MUXER_MKV      = VIDEO_OUTPUT_MKV;
const int   MUXER_TC2MP4   = VIDEO_OUTPUT_MP4 + 2;

const DWORD OUT_RESULT_SUCCESS    = 0x0000;
const DWORD OUT_RESULT_ERROR      = 0x0001;
const DWORD OUT_RESULT_ABORT      = 0x0002;

const int   OUT_CSP_YUV420 = 0;
const int   OUT_CSP_YUV444 = 1;
const int   OUT_CSP_RGB    = 2;

typedef struct {
	WCHAR *text;
	DWORD value;
} PRIORITY_CLASS;

const DWORD AVIUTLSYNC_PRIORITY_CLASS = 0;

const PRIORITY_CLASS priority_table[] = {
	{L"AviutlSync",       AVIUTLSYNC_PRIORITY_CLASS   },
	{L"higher",           HIGH_PRIORITY_CLASS         },
	{L"high",             ABOVE_NORMAL_PRIORITY_CLASS },
	{L"normal",           NORMAL_PRIORITY_CLASS       },
	{L"low",              BELOW_NORMAL_PRIORITY_CLASS },
	{L"lower",            IDLE_PRIORITY_CLASS         },
	{L"",                 NORMAL_PRIORITY_CLASS       },
	{L"realtime(非推奨)", REALTIME_PRIORITY_CLASS     },
	{NULL,                0                           }
};

void open_log_window(const char *savefile, int current_pass, int total_pass);

void write_log_auo_line_fmt(int log_type_index, const char *format, ... );
void write_log_auo_enc_time(const char *mes, DWORD time);

#endif //_AUO_H_