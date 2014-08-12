//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "output.h"
#include "auo.h"
#include "auo_version.h"
#include "auo_util.h"
#include "auo_conf.h"
#include "auo_settings.h"
#include "auo_system.h"
#include "auo_frm.h"
#include "auo_encode.h"
#include "auo_faw2aac.h"

typedef OUTPUT_PLUGIN_TABLE* (*func_get_auo_table)(void);

BOOL check_if_faw2aac_exists() {
	char aviutl_dir[MAX_PATH_LEN];
	get_aviutl_dir(aviutl_dir, _countof(aviutl_dir));

	for (int i = 0; i < _countof(FAW2AAC_NAME); i++) {
		char faw2aac_path[MAX_PATH_LEN];
		PathCombineLong(faw2aac_path, _countof(faw2aac_path), aviutl_dir, FAW2AAC_NAME[i]);
		if (PathFileExists(faw2aac_path))
			return TRUE;
	}
	return FALSE;
}


// 進捗表示用
static const OUTPUT_INFO *g_oip = NULL;
static BOOL auo_rest_time_disp(int now, int total) {
	if (g_oip)
		g_oip->func_rest_time_disp(now, total);
	//進捗表示
	static DWORD tm_last = timeGetTime();
	DWORD tm;
	if ((tm = timeGetTime()) - tm_last > LOG_UPDATE_INTERVAL * 5) {
		set_log_progress(now / (double)total);
		tm_last = tm;
	}
	return TRUE;
};


AUO_RESULT audio_faw2aac(CONF_X264GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
	AUO_RESULT ret = AUO_RESULT_SUCCESS;
	HMODULE hModule = NULL;
	func_get_auo_table getFAW2AACTable = NULL;
	OUTPUT_PLUGIN_TABLE *opt = NULL;
	char aviutl_dir[MAX_PATH_LEN];
	get_aviutl_dir(aviutl_dir, _countof(aviutl_dir));

	for (int i = 0; i < _countof(FAW2AAC_NAME); i++) {
		char faw2aac_path[MAX_PATH_LEN];
		PathCombineLong(faw2aac_path, _countof(faw2aac_path), aviutl_dir, FAW2AAC_NAME[i]);
		if (PathFileExists(faw2aac_path)) {
			hModule = LoadLibrary(faw2aac_path);
			break;
		}
	}

	if (hModule == NULL) {
		ret = AUO_RESULT_ERROR; write_log_auo_line(LOG_INFO, "faw2aac.auoが見つかりませんでした。");
	} else if (
		   NULL == (getFAW2AACTable = (func_get_auo_table)GetProcAddress(hModule, "GetOutputPluginTable")) 
		|| NULL == (opt = getFAW2AACTable()) 
		|| NULL ==  opt->func_output) {
		ret = AUO_RESULT_ERROR; write_log_auo_line(LOG_WARNING, "faw2aac.auoのロードに失敗しました。");
	} else {
		OUTPUT_INFO oip_faw2aac = *oip;
		//audfile名作成
		char audfile[MAX_PATH_LEN];
		const AUDIO_SETTINGS *aud_stg = &sys_dat->exstg->s_aud[conf->aud.encoder];
		strcpy_s(pe->append.aud, _countof(pe->append.aud), aud_stg->aud_appendix); //pe一時パラメータにコピーしておく
		get_aud_filename(audfile, _countof(audfile), pe);
		oip_faw2aac.savefile = audfile;
		//進捗表示の取り込み
		g_oip = oip;
		oip_faw2aac.func_rest_time_disp = auo_rest_time_disp;

		//開始
		if (opt->func_init && !opt->func_init()) {
			ret = AUO_RESULT_ERROR; write_log_auo_line(LOG_WARNING, "faw2aac.auoの初期化に失敗しました。");
		} else {
			set_window_title("faw2aac", PROGRESSBAR_CONTINUOUS);
			write_log_auo_line(LOG_INFO, "faw2aac で音声エンコードを行います。");
			if (FALSE == opt->func_output(&oip_faw2aac)) {
				ret = AUO_RESULT_ERROR; write_log_auo_line(LOG_WARNING, "faw2aac.auoの実行に失敗しました。");
			}
			if (opt->func_exit)
				opt->func_exit();
		}
	}

	if (hModule)
		FreeLibrary(hModule);

	set_window_title(AUO_FULL_NAME, PROGRESSBAR_DISABLED);

	return ret;
}
