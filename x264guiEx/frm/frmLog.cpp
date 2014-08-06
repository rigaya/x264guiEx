﻿//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include "frmLog.h"
#include "string.h"

#include "auo_version.h"

using namespace x264guiEx;

//すべてfrmLog::Instanceを通じてアクセス

//ログウィンドウを表示させる
void show_log_window() {
	System::Windows::Forms::Application::EnableVisualStyles();
	frmLog::Instance::get()->Show();
	frmLog::Instance::get()->SetWindowTitle(AUO_FULL_NAME, PROGRESSBAR_DISABLED);
}
//ログウィンドウのタイトルを設定
void set_window_title(const char *chr, int progress_mode) {
	frmLog::Instance::get()->SetWindowTitle(chr, progress_mode);
}
//x264からのメッセージをログウィンドウのタイトルに
void set_window_title_x264_mes(const char *chr, int total_drop, int frame_n) {
	frmLog::Instance::get()->SetWindowTitleX264Mes(chr, total_drop, frame_n);
}
//x264guiExからのメッセージとして、ログウィンドウに表示
void write_log_auo_line(int log_type_index, const char *chr) {
	frmLog::Instance::get()->WriteLogAuoLine(chr, log_type_index);
}
void set_task_name(const char *chr) {
	frmLog::Instance::get()->SetTaskName(chr);
}
//進捗情報の表示
void set_log_progress(double progress) {
	frmLog::Instance::get()->SetProgress(progress);
}
//メッセージを直接ログウィンドウに表示
void write_log_line(int log_type_index, const char *chr) {
	frmLog::Instance::get()->WriteLogLine(chr, log_type_index);
}
//ログウィンドウからのx264制御を有効化
void enable_x264_control(DWORD *priority, BOOL *enc_pause, BOOL afs, BOOL add_progress, DWORD *start_time, int _total_frame) {
	frmLog::Instance::get()->Enablex264Control(priority, enc_pause, afs, add_progress, start_time, _total_frame);
}
//ログウィンドウからのx264制御を無効化
void disable_x264_control() {
	frmLog::Instance::get()->Disablex264Control();
}
//ログウィンドウを閉じられるかどうかを設定
void set_prevent_log_close(BOOL prevent, const char *savefile) {
	frmLog::Instance::get()->SetPreventLogWindowClosing(prevent, savefile);
}
//ログウィンドウにイベントを実行させる
void log_process_events() {
	System::Windows::Forms::Application::DoEvents();
}
//現在のログの長さを返す
int get_current_log_len(int current_pass) {
	return frmLog::Instance::get()->GetLogStringLen(current_pass);
}