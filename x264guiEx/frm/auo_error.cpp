//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include "auo.h"
#include "auo_frm.h" 
#include "auo_pipe.h"

void warning_failed_getting_temp_path() {
	write_log_auo_line(LOG_WARNING, "一時フォルダ名取得に失敗しました。一時フォルダ指定を解除しました。");
}

void warning_no_temp_root(const char *dir) {
	write_log_auo_line_fmt(LOG_WARNING, 
		"指定された一時フォルダ \"%s\" が存在しません。一時フォルダ指定を解除しました。",
		dir);
}

void warning_no_aud_temp_root(const char *dir) {
	write_log_auo_line_fmt(LOG_WARNING,
		"指定された音声用一時フォルダ \"%s\" が存在しません。一時フォルダ指定を解除しました。",
		dir);
}

void error_filename_too_long() {
	write_log_auo_line(LOG_ERROR, "出力ファイル名が長すぎます。もっと短くしてください。");
}

void error_nothing_to_output() {
	write_log_auo_line(LOG_ERROR, "出力すべきものがありません。");
}

void error_invalid_resolution(BOOL width, int mul, int w, int h) {
	write_log_auo_line_fmt(LOG_ERROR, "%s入力解像度が %d で割りきれません。エンコードできません。入力解像度:%dx%d",
		(width) ? "横" : "縦", mul, w, h);
}

void error_no_exe_file(const char *name, const char *path) {
	if (strlen(path))
		write_log_auo_line_fmt(LOG_ERROR, "指定された %s が %s にありません。", name, path);
	else
		write_log_auo_line_fmt(LOG_ERROR, "%s の場所が指定されていません。", name);
	write_log_auo_line_fmt(LOG_ERROR, "%s を用意し、その場所を設定画面かから正しく指定してください。", name);
}

void error_afs_setup(BOOL afs) {
	if (afs) {
		char mes[1024];
		sprintf_s(mes, sizeof(mes), "%s\n%s\n%s\n%s\n%s", 
			"auo [error]: Aviutlからの映像入力の初期化に失敗しました。以下のような原因が考えられます。",
			"             ・自動フィールドシフト(afs)をAviutlで使用していないにもかかわらず、",
			"               x264guiExの設定画面で自動フィールドシフトにチェックを入れていたり、",
			"               自動フィールドシフト非対応の動画(60fps読み込み等)を入力したりしている。",
			"             ・メモリ不足による、メモリ確保の失敗。"
			);
		write_log_line(LOG_ERROR, mes);
	} else
		write_log_auo_line(LOG_ERROR, "Aviutlからの映像入力の初期化に失敗しました。メモリを確保できませんでした。");
}

void error_open_pipe() {
	write_log_auo_line(LOG_ERROR, "パイプの作成に失敗しました。");
}

void error_get_pipe_handle() {
	write_log_auo_line(LOG_ERROR, "パイプハンドルの取得に失敗しました。");
}

void error_run_process(const char *exe_name, int rp_ret) {
	switch (rp_ret) {
		case RP_ERROR_OPEN_PIPE:
			write_log_auo_line(LOG_ERROR, "パイプの作成に失敗しました。");
			break;
		case RP_ERROR_GET_STDIN_FILE_HANDLE:
			write_log_auo_line(LOG_ERROR, "パイプハンドルの取得に失敗しました。");
			break;
		case RP_ERROR_CREATE_PROCESS:
		default:
			write_log_auo_line_fmt(LOG_ERROR, "%s の実行に失敗しました。", exe_name);
			break;
	}
}

void warning_auto_qpfile_failed() {
	write_log_auo_line(LOG_WARNING, "Aviutlのキーフレーム検出用 qpfileの自動作成に失敗しました。");
}

void warning_auo_tcfile_failed() {
	write_log_auo_line(LOG_WARNING, "タイムコードファイル作成に失敗しました。");
}

void error_malloc_pixel_data() {
	write_log_auo_line(LOG_ERROR, "映像バッファ用メモリ確保に失敗しました。");
}

void error_malloc_tc() {
	write_log_auo_line(LOG_ERROR, "タイムコード用メモリ確保に失敗しました。");
}

void error_malloc_8bit() {
	write_log_auo_line(LOG_ERROR, "音声16bit→8bit変換用メモリ確保に失敗しました。");
}

void error_x264_dead() {
	write_log_auo_line(LOG_ERROR, "x264が予期せず途中終了しました。x264に不正なパラメータ(オプション)が渡された可能性があります。");
}

void error_afs_get_frame() {
	write_log_auo_line(LOG_ERROR, "Aviutlからのフレーム読み込みに失敗しました。");
}

void error_open_wavfile() {
	write_log_auo_line(LOG_ERROR, "wavファイルのオープンに失敗しました。");
}

void error_no_wavefile() {
	write_log_auo_line(LOG_ERROR, "wavファイルがみつかりません。音声エンコードに失敗しました。");
}

void error_batfile_original_open() {
	write_log_auo_line(LOG_ERROR, "オリジナルbatファイルのオープンに失敗しました。");
}

void error_batfile_temp_open() {
	write_log_auo_line(LOG_ERROR, "一時batファイルのオープンに失敗しました。");
}

void error_audenc_failed(const char *name, const char *args) {
	write_log_auo_line_fmt(LOG_ERROR, "出力音声ファイルがみつかりません。%s での音声のエンコードに失敗しました。", name);
	write_log_auo_line(    LOG_ERROR, "音声エンコードのコマンドラインは…");
	write_log_auo_line(    LOG_ERROR, args);
}

void error_mux_failed(const char *name, const char *args) {
	write_log_auo_line_fmt(LOG_ERROR, "%s でのmuxに失敗しました。", name);
	write_log_auo_line(    LOG_ERROR, "muxのコマンドラインは…");
	write_log_auo_line(    LOG_ERROR, args);
}

void warning_no_mux_tmp_root(const char *dir) {
	write_log_auo_line_fmt(LOG_WARNING,
		"指定されたmux用一時フォルダ \"%s\" が存在しません。一時フォルダ指定を解除しました。",
		dir);
}

void warning_failed_mux_tmp_drive_space() {
	write_log_auo_line(LOG_WARNING, "指定されたmux用一時フォルダのあるドライブの空き容量取得に失敗しました。mux用一時フォルダ指定を解除しました。");
}

void warning_failed_get_aud_size() {
	write_log_auo_line(LOG_WARNING, "音声一時ファイルのサイズ取得に失敗しました。muxを正常に行えるか確認できません。");
}

void warning_failed_get_vid_size() {
	write_log_auo_line(LOG_WARNING, "映像一時ファイルのサイズ取得に失敗しました。muxを正常に行えるか確認できません。");
}

void warning_mux_tmp_not_enough_space() {
	write_log_auo_line(LOG_WARNING, "mux一時フォルダのドライブに十分な空きがありません。mux用一時フォルダ指定を解除しました。");
}

void info_encoding_aborted() {
	write_log_auo_line(LOG_INFO, "エンコードを中断しました。");
}