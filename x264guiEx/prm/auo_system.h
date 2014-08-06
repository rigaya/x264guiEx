//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _AUO_SYSTEM_H_
#define _AUO_SYSTEM_H_

#include <Windows.h>
#include "auo.h"
#include "auo_settings.h"
#include "auo_conf.h"

typedef struct {
	int video_out_type;                 //出力する動画のフォーマット(拡張子により判断)
	int muxer_to_be_used;               //使用するmuxerのインデックス
	int current_x264_pass;              //現在のx264パス数
	int total_x264_pass;                //最大x264パス数
	int drop_count;                     //ドロップ数
	BOOL afs_init;                      //動画入力の準備ができているか
	HANDLE h_p_aviutl;                  //優先度取得用のAviutlのハンドル
	char temp_filename[MAX_PATH_LEN];   //一時ファイル名
	char aud_temp_dir[MAX_PATH_LEN];    //音声一時ディレクトリ
	FILE_APPENDIX append;               //ファイル名に追加する文字列のリスト
} PRM_ENC;

typedef struct {
	BOOL init;
	char auo_path[MAX_PATH_LEN];    //x264guiEx.auoのフルパス
	char aviutl_dir[MAX_PATH_LEN];  //Aviutlのディレクトリ(\無し)
	guiEx_settings *exstg;          //ini設定
} SYSTEM_DATA;

void init_SYSTEM_DATA(SYSTEM_DATA *_sys_dat);
void delete_SYSTEM_DATA(SYSTEM_DATA *_sys_dat);

#endif //_AUO_SYSTEM_H_