//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _AUO_FRM_H_
#define _AUO_FRM_H_

#include <Windows.h>
#include "auo_conf.h"
#include "auo_system.h"

const int LOG_UPDATE_INTERVAL = 50;

const int LOG_INFO    = 0;
const int LOG_WARNING = 1;
const int LOG_ERROR   = 2;

///ログ表示で使う色              R    G    B
const int LOG_COLOR[3][3] = { { 198, 253, 226 },   //LOG_INFO
                              { 245, 218,  90 },   //LOG_WARNING
							  { 253,  83, 121 } }; //LOG_ERROR

const float LOG_TRANSPARENT_RATIO[2] = { 1.000f, 0.725f };

const int PROGRESSBAR_DISABLED   = 0;
const int PROGRESSBAR_CONTINUOUS = 1;
const int PROGRESSBAR_MARQUEE    = 2;

//設定ウィンドウ
void ShowfrmConfig(CONF_X264GUIEX *conf, const SYSTEM_DATA *sys_dat);

//ログウィンドウ制御
void show_log_window();
void set_window_title(const char *chr, int progress_mode);
void set_window_title_x264_mes(const char *chr, int total_drop, int frame_n);
void set_task_name(const char *chr);
void set_log_progress(double progress);
void write_log_auo_line(int log_type_index, const char *chr);
void write_log_line(int log_type_index, const char *chr);
void enable_x264_control(DWORD *priority, BOOL *enc_pause, BOOL afs, BOOL add_progress, DWORD *start_time, int _total_frame);
void disable_x264_control();
void set_prevent_log_close(BOOL prevent, const char *savefile);
void log_process_events();
int get_current_log_len(int current_pass);

void write_log_x264_mes(char * const mes, DWORD *log_len, int total_drop, int current_frames);
void write_args(const char *args);

#endif //_AUO_FRM_H_