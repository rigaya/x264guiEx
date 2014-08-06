//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <Math.h>
#include <stdlib.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "output.h"
#include "auo.h"
#include "auo_version.h"
#include "auo_frm.h"
#include "auo_pipe.h"
#include "auo_encode.h"
#include "auo_error.h"
#include "auo_conf.h"
#include "auo_util.h"
#include "auo_chapter.h"
#include "auo_system.h"
#include "auo_encode.h"

static void show_mux_info(const char *mux_stg_name, BOOL audmux, BOOL tcmux, const char *muxer_mode_name) {
	char mes[1024];
	static const char * const ON_OFF_INFO[] = { "off", "on" };

	sprintf_s(mes, sizeof(mes), "%s でmuxを行います。音声mux:%s, tcmux:%s, 拡張モード:%s", 
		mux_stg_name,
		ON_OFF_INFO[audmux != 0],
		ON_OFF_INFO[tcmux != 0],
		muxer_mode_name);
	write_log_auo_line_fmt(LOG_INFO, mes);

	sprintf_s(mes, sizeof(mes), "%s で mux中...", mux_stg_name);
	set_window_title(mes, PROGRESSBAR_MARQUEE);
}

static BOOL check_mux_tmpdir(const char *mux_tmpdir, const CONF_X264GUIEX *conf, const PRM_ENC *pe, __int64 expected_filesize) {
	//正常にサイズを取得できているか
	if (expected_filesize < 0)
		return FALSE;
	//一時フォルダ指定を使用するかどうか
	if (!conf->mux.mp4_temp_dir)
		return FALSE;
	//mp4系かどうか
	if (!(pe->muxer_to_be_used == MUXER_MP4 || pe->muxer_to_be_used == MUXER_TC2MP4))
		return FALSE;
	//指定されたドライブが存在するかどうか
	char temp_root[MAX_PATH_LEN];
	if (!PathGetRoot(mux_tmpdir, temp_root, sizeof(temp_root)) || !PathIsDirectory(temp_root)) {
		warning_no_mux_tmp_root(temp_root);
		return FALSE;
	}
	//ドライブの空き容量取得
	ULARGE_INTEGER drive_avail_space = { 0 };
	if (!GetDiskFreeSpaceEx(temp_root, &drive_avail_space, NULL, NULL)) {
		warning_failed_mux_tmp_drive_space();
		return FALSE;
	}
	//出力先が同じドライブかどうか
	__int64 required_space = (__int64)(expected_filesize * 1.01); //ちょい多め
	char vid_root[MAX_PATH_LEN];
	strcpy_s(vid_root, sizeof(vid_root), pe->temp_filename);
	PathStripToRoot(vid_root);
	if (_stricmp(vid_root, temp_root) == NULL)
		required_space *= 2;

	//判定
	if ((__int64)drive_avail_space.QuadPart < required_space) {
		warning_mux_tmp_not_enough_space();
		return FALSE;
	}
	return TRUE;
}

static BOOL get_expected_filesize(const PRM_ENC *pe, BOOL enable_aud_mux, __int64 *_expected_filesize) {
	*_expected_filesize = 0;
	//動画ファイルのサイズ
	__int64 vid_size = 0;
	if (!GetFileSizeInt64(pe->temp_filename, &vid_size)) {
		warning_failed_get_vid_size();
		return FALSE;
	}
	*_expected_filesize += vid_size;
	//音声ファイルのサイズ
	if (enable_aud_mux) {
		__int64 aud_size = 0;
		char audfile[MAX_PATH_LEN] = { 0 };
		get_aud_filename(audfile, sizeof(audfile), pe);
		if (!GetFileSizeInt64(audfile, &aud_size)) {
			warning_failed_get_aud_size();
			return FALSE;
		}
		*_expected_filesize += aud_size;
	}
	return TRUE;
}

static BOOL check_muxout_filesize(const char *muxout, __int64 expected_filesize) {
	__int64 muxout_filesize = 0;
	if (!GetFileSizeInt64(muxout, &muxout_filesize))
		return FALSE;
	if (muxout_filesize <= (__int64)(expected_filesize * 0.99 * (1.0 - exp(-1.0 * expected_filesize / (128.0 * 1024.0)))))
		return FALSE;
	return TRUE;
}

//%{par_x}と%{par_y}の置換を行う
static void replace_par(char *cmd, size_t nSize, const CONF_X264GUIEX *conf, const OUTPUT_INFO *oip) {	
	char buf[64];
	CONF_X264 conf_tmp;
	memcpy(&conf_tmp, &conf->x264, sizeof(CONF_X264));
	apply_guiEx_auto_settings(&conf_tmp, oip->w, oip->h, oip->rate, oip->scale);
	if (conf_tmp.sar.x <= 0 || conf_tmp.sar.y <= 0) {
		conf_tmp.sar.x = 1;
		conf_tmp.sar.y = 1;
	}

	sprintf_s(buf, sizeof(buf), "%d", conf_tmp.sar.x);
	replace(cmd, nSize, "%{par_x}", buf);
	sprintf_s(buf, sizeof(buf), "%d", conf_tmp.sar.y);
	replace(cmd, nSize, "%{par_y}", buf);
}
static DWORD build_mux_cmd(char *cmd, size_t nSize, const CONF_X264GUIEX *conf, const OUTPUT_INFO *oip, const PRM_ENC *pe, 
						  const SYSTEM_DATA *sys_dat, const MUXER_SETTINGS *mux_stg, __int64 expected_filesize) {
	strcpy_s(cmd, nSize, mux_stg->base_cmd);
	BOOL enable_aud_mux = (oip->flag & OUTPUT_INFO_FLAG_AUDIO) != 0;
	BOOL enable_tc_mux = (conf->vid.afs) != 0;
	const MUXER_CMD_EX *muxer_mode = &mux_stg->ex_cmd[(pe->muxer_to_be_used == MUXER_MKV) ? conf->mux.mkv_mode : conf->mux.mp4_mode];
	char *audstr = (enable_aud_mux) ? mux_stg->aud_cmd : "";
	char *tcstr  = (enable_tc_mux) ? mux_stg->tc_cmd : "";
	char *exstr  = (conf->mux.apple_mode && strlen(muxer_mode->cmd_apple)) ? muxer_mode->cmd_apple : muxer_mode->cmd;
	//音声用コマンド
	replace(cmd, nSize, "%{au_cmd}",  audstr);
	//タイムコード用
	replace(cmd, nSize, "%{tc_cmd}",  tcstr);
	//一時ファイル用(指定できなければ無視)
	if (check_mux_tmpdir(sys_dat->exstg->s_local.custom_mp4box_tmp_dir, conf, pe, expected_filesize)) {
		if (!DirectoryExistsOrCreate(sys_dat->exstg->s_local.custom_mp4box_tmp_dir)) {
			replace(cmd, nSize, "%{tmp_cmd}", "");
			warning_no_mux_tmp_root(sys_dat->exstg->s_local.custom_mp4box_tmp_dir);
		} else {
			replace(cmd, nSize, "%{tmp_cmd}", mux_stg->tmp_cmd);
			char m_tmp_dir[MAX_PATH_LEN];
			strcpy_s(m_tmp_dir, sizeof(m_tmp_dir), sys_dat->exstg->s_local.custom_mp4box_tmp_dir);
			PathForceRemoveBackSlash(m_tmp_dir);
			replace(cmd, nSize, "%{m_tmpdir}", m_tmp_dir);
		}
	} else {
		replace(cmd, nSize, "%{tmp_cmd}", "");
	}
	//拡張オプションとチャプター処理
	//とりあえず必要なくてもチャプターファイル名を作る
	char chap_file[MAX_PATH_LEN];
	char chap_apple[MAX_PATH_LEN];
	set_chap_filename(chap_file, sizeof(chap_file), chap_apple, sizeof(chap_apple), 
		muxer_mode->chap_file, pe, sys_dat, oip->savefile);
	replace(cmd, nSize, "%{ex_cmd}", exstr);
	//もし、チャプターファイル名への置換があるなら、チャプターファイルの存在をチェックする
	if ((strstr(cmd, "%{chapter}") || strstr(cmd, "%{chap_apple}")) && !PathFileExists(chap_file)) {
		//チャプターファイルが存在しない
		warning_mux_no_chapter_file();
		replace(cmd, nSize, "%{chapter}",    "");
		replace(cmd, nSize, "%{chap_apple}", "");
	} else {
		replace(cmd, nSize, "%{chapter}", chap_file);
		//mp4系ならapple形式チャプター追加も考慮する
		if (pe->muxer_to_be_used != MUXER_MKV) {
			//apple形式チャプターファイルへの置換が行われたら、apple形式チャプターファイルを作成する
			if (strstr(cmd, "%{chap_apple}")) {
				DWORD duration_ms = (DWORD)(((__int64)oip->n * (__int64)oip->scale * 1000) / (__int64)oip->rate);
				AuoChapStatus sts = convert_chapter(chap_apple, chap_file, CODE_PAGE_UNSET, duration_ms);
				if (sts != AUO_CHAP_ERR_NONE) {
					warning_mux_chapter(sts);
					replace(cmd, nSize, "%{chap_apple}", "");
				} else {
					replace(cmd, nSize, "%{chap_apple}", chap_apple);
				}
			}
		}
	}
	//tc2mp4ならmp4boxの場所を指定
	if (pe->muxer_to_be_used == MUXER_TC2MP4)
		sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " -L \"%s\"", sys_dat->exstg->s_mux[MUXER_MP4].fullpath);
	//アスペクト比
	replace_par(cmd, nSize, conf, oip);
	//その他の置換を実行
	cmd_replace(cmd, nSize, pe, sys_dat, oip->savefile);
	//情報表示
	show_mux_info(mux_stg->dispname, enable_aud_mux, enable_tc_mux, muxer_mode->name);
	return AUO_RESULT_SUCCESS;
}

DWORD mux(const CONF_X264GUIEX *conf, const OUTPUT_INFO *oip, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
	DWORD ret = AUO_RESULT_SUCCESS;
	//muxの必要がなければ終了
	if (pe->muxer_to_be_used == MUXER_DISABLED)
		return ret;

	MUXER_SETTINGS *mux_stg = &sys_dat->exstg->s_mux[pe->muxer_to_be_used];

	if (!PathFileExists(mux_stg->fullpath)) {
		ret |= AUO_RESULT_ERROR; error_no_exe_file(mux_stg->dispname, mux_stg->fullpath);
		return ret;
	}
	if (pe->muxer_to_be_used == MUXER_TC2MP4 && !PathFileExists(sys_dat->exstg->s_mux[MUXER_MP4].fullpath)) {		
		ret |= AUO_RESULT_ERROR; error_no_exe_file(sys_dat->exstg->s_mux[MUXER_MP4].dispname, sys_dat->exstg->s_mux[MUXER_MP4].fullpath);
		return ret;
	}
	__int64 expected_filesize = 0;
	char muxcmd[MAX_CMD_LEN]  = { 0 };
	char muxargs[MAX_CMD_LEN] = { 0 };
	char muxdir[MAX_PATH_LEN] = { 0 };
	char muxout[MAX_PATH_LEN] = { 0 };
	DWORD mux_priority = GetExePriority(conf->mux.priority, pe->h_p_aviutl);
	get_muxout_filename(muxout, sizeof(muxout), pe->temp_filename);

	PROCESS_INFORMATION pi_mux;
	int rp_ret;

	PathGetDirectory(muxdir, sizeof(muxdir), mux_stg->fullpath);

	//mux終了後の予想サイズを取得
	if (AUO_RESULT_SUCCESS != get_expected_filesize(pe, (oip->flag & OUTPUT_INFO_FLAG_AUDIO) != 0, &expected_filesize))
		return AUO_RESULT_ERROR;
	
	//コマンドライン生成・情報表示
	if (AUO_RESULT_SUCCESS != build_mux_cmd(muxcmd, sizeof(muxcmd), conf, oip, pe, sys_dat, mux_stg, expected_filesize))
		return AUO_RESULT_ERROR; //エラーメッセージはbuild_mux_cmd関数内で吐かれる
	sprintf_s(muxargs, sizeof(muxargs), "\"%s\" %s", mux_stg->fullpath, muxcmd);

	if ((rp_ret = RunProcess(muxargs, muxdir, &pi_mux, NULL, mux_priority, FALSE, conf->mux.minimized)) != RP_SUCCESS) {
		//エラー
		ret |= AUO_RESULT_ERROR; error_run_process(mux_stg->dispname, rp_ret);
	} else {
		while (WaitForSingleObject(pi_mux.hProcess, LOG_UPDATE_INTERVAL) == WAIT_TIMEOUT)
			log_process_events();

		if (check_muxout_filesize(muxout, expected_filesize)) {
			remove(pe->temp_filename);
			rename(muxout, pe->temp_filename);
		} else {
			ret |= AUO_RESULT_ERROR; error_mux_failed(mux_stg->dispname, muxargs);
			if (PathFileExists(muxout))
				remove(muxout);
		}
		CloseHandle(pi_mux.hProcess);
		CloseHandle(pi_mux.hThread);
	}

	set_window_title(AUO_FULL_NAME, PROGRESSBAR_DISABLED);

	return ret;
}