//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _AUO_ENCODE_H_
#define _AUO_ENCODE_H_

#include <Windows.h>
#include <stdio.h>

#include "auo.h"
#include "output.h"
#include "auo_conf.h"
#include "auo_settings.h"
#include "auo_system.h"
#include "auo_frm.h"

static const char * const PIPE_FN = "-";

typedef AUO_RESULT (*encode_task) (CONF_X264GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat);

void get_aud_filename(char *audfile, size_t nSize, const PRM_ENC *pe); //音声一時ファイル名を作成
void get_muxout_filename(char *filename, size_t nSize, const SYSTEM_DATA *sys_dat, const PRM_ENC *pe); //mux出力ファイル名を作成
void set_chap_filename(char *chap_file, size_t cf_nSize, char *chap_apple, size_t ca_nSize, const char *chap_base, 
					   const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const CONF_X264GUIEX *conf, const char *savfile); //チャプターファイルのパスを生成
void cmd_replace(char *cmd, size_t nSize, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const CONF_X264GUIEX *conf, const char *savefile); //コマンドラインの共通置換を実行
AUO_RESULT move_temporary_files(const CONF_X264GUIEX *conf, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const char *savefile, DWORD ret); //一時ファイルの最終的な移動・削除を実行
DWORD GetExePriority(DWORD set, HANDLE h_aviutl); //実行ファイルに指定すべき優先度を取得

AUO_RESULT getLogFilePath(char *log_file_path, size_t nSize, const PRM_ENC *pe, const char *savefile, const SYSTEM_DATA *sys_dat, const CONF_X264GUIEX *conf);

int check_video_ouput(const CONF_X264GUIEX *conf, const OUTPUT_INFO *oip);
int check_muxer_to_be_used(const CONF_X264GUIEX *conf, int video_output_type, BOOL audio_output);

double get_duration(const CONF_X264GUIEX *conf, const SYSTEM_DATA *sys_dat, const PRM_ENC *pe, const OUTPUT_INFO *oip);

double get_amp_margin_bitrate(double base_bitrate, double margin_multi);

AUO_RESULT amp_check_file(CONF_X264GUIEX *conf, const SYSTEM_DATA *sys_dat, PRM_ENC *pe, const OUTPUT_INFO *oip);

#endif //_AUO_ENCODE_H_