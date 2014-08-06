﻿//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _AUO_CONF_H_
#define _AUO_CONF_H_

#include <Windows.h>
#include "auo.h"
#include "auo_options.h"

const int CONF_INITIALIZED = 1;

const int TMP_DIR_OUTPUT = 0;
const int TMP_DIR_SYSTEM = 1;
const int TMP_DIR_CUSTOM = 2;

static const char *CONF_NAME          = "x264guiEx ConfigFile";
const int CONF_NAME_BLOCK_LEN         = 32;
const int CONF_BLOCK_MAX              = 32;
const int CONF_BLOCK_COUNT            = 5; //最大 CONF_BLOCK_MAXまで
const int CONF_HEAD_SIZE              = (3 + CONF_BLOCK_MAX) * sizeof(int) + CONF_BLOCK_MAX * sizeof(size_t) + CONF_NAME_BLOCK_LEN;
const int CONF_ERROR_FILE_OPEN        = 1;
const int CONF_ERROR_BLOCK_SIZE       = 2;
const int CONF_ERROR_INVALID_FILENAME = 3;

const int CMDEX_MAX_LEN = 2048;    //追加コマンドラインの最大長

typedef struct {
	BOOL afs;                      //自動フィールドシフトの使用
	BOOL afs_bitrate_correction;   //afs & 2pass時、ドロップ数に応じてビットレートを補正
	BOOL auo_tcfile_out;           //auo側でタイムコードを出力する
	BOOL check_keyframe;           //キーフレームチェックを行う
	int  priority;                 //x264のCPU優先度(インデックス)
	char stats[MAX_PATH_LEN];      //x264用ステータスファイルの場所
	char tcfile_in[MAX_PATH_LEN];  //x264 tcfile-in用タイムコードファイルの場所
	char cqmfile[MAX_PATH_LEN];    //x264 cqmfileの場所
	char cmdex[CMDEX_MAX_LEN];     //追加コマンドライン
} CONF_VIDEO; //動画用設定(x264以外)

typedef struct {
	int  encoder;            //使用する音声エンコーダ
	int  enc_mode;           //使用する音声エンコーダの設定
	int  bitrate;            //ビットレート指定モード
	BOOL use_2pass;          //音声2passエンコードを行う
	BOOL use_wav;            //パイプを使用せず、wavを出力してエンコードを行う
	BOOL faw_check;          //FAWCheckを行う
	int  priority;           //音声エンコーダのCPU優先度(インデックス)
	BOOL minimized;          //音声エンコーダを最小化で実行
	int aud_temp_dir;        //音声専用一時フォルダ
	BOOL audio_encode_first; //音声を先にエンコード
} CONF_AUDIO; //音声用設定

typedef struct {
	BOOL disable_mp4ext;  //mp4出力時、外部muxerを使用する
	BOOL disable_mkvext;  //mkv出力時、外部muxerを使用する
	int  mp4_mode;        //mp4 外部muxer用追加コマンドの設定
	int  mkv_mode;        //mkv 外部muxer用追加コマンドの設定
	BOOL minimized;       //muxを最小化で実行
	int  priority;        //mux優先度(インデックス)
	int  mp4_temp_dir;    //mp4box用一時ディレクトリ
} CONF_MUX; //muxer用設定

typedef struct {
	BOOL disable_guicmd; //GUIによるコマンドライン生成を停止(CLIモード)
	int  temp_dir;       //一時ディレクトリ
	BOOL out_audio_only; //音声のみ出力
} CONF_OTHER;

typedef struct {
	char        conf_name[CONF_NAME_BLOCK_LEN];  //保存時に使用
	int         size_all;                        //保存時: CONF_X264GUIEXの全サイズ / 設定中、エンコ中: CONF_INITIALIZED
	int         head_size;                       //ヘッダ部分の全サイズ
	int         block_count;                     //ヘッダ部を除いた設定のブロック数
	int         block_size[CONF_BLOCK_MAX];      //各ブロックのサイズ
	size_t      block_head_p[CONF_BLOCK_MAX];    //各ブロックのポインタ位置
	CONF_X264   x264;                            //x264についての設定
	CONF_VIDEO  vid;                             //その他動画についての設定
	CONF_AUDIO  aud;                             //音声についての設定
	CONF_MUX    mux;                             //muxについての設定
	CONF_OTHER  oth;                             //その他の設定
} CONF_X264GUIEX;

class guiEx_config {
private:
	static const size_t conf_block_pointer[CONF_BLOCK_COUNT];
	static const int conf_block_data[CONF_BLOCK_COUNT];
public:
	guiEx_config();
	int  load_x264guiEx_conf(CONF_X264GUIEX *conf, const char *stg_file);       //設定をstgファイルから読み込み
	int  save_x264guiEx_conf(const CONF_X264GUIEX *conf, const char *stg_file); //設定をstgファイルとして保存
};

//定義はx264guiEx.cpp
void init_CONF_X264GUIEX(CONF_X264GUIEX *conf, BOOL use_10bit); //初期化し、x264設定のデフォルトを設定 

#endif //_AUO_CONF_H_