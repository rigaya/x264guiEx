//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "auo.h"
#include "auo_util.h"
#include "auo_conf.h"
#include "auo_settings.h"
#include "auo_system.h"

#include "auo_frm.h"
#include "auo_encode.h"

void get_aud_filename(char *audfile, size_t nSize, const PRM_ENC *pe) {
	PathCombineLong(audfile, nSize, pe->aud_temp_dir, PathFindFileName(pe->temp_filename));
	apply_appendix(audfile, nSize, audfile, pe->append.aud);
}

void get_muxout_filename(char *filename, size_t nSize, const char *tmp_filename) {
	static const char * const MUXOUT_APPENDIX = "_out";
	char ext[MAX_APPENDIX_LEN];
	strcpy_s(ext, sizeof(ext), PathFindExtension(tmp_filename));
	strcpy_s(filename, nSize, tmp_filename);
	strcpy_s(PathFindExtension(filename), nSize - (PathFindExtension(filename) - filename), MUXOUT_APPENDIX);
	strcat_s(filename, nSize, ext);
}

//チャプターファイル名とapple形式のチャプターファイル名を同時に作成する
void set_chap_filename(char *chap_file, size_t cf_nSize, char *chap_apple, size_t ca_nSize, const char *chap_base, 
					   const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const char *savfile) {
	strcpy_s(chap_file, cf_nSize, chap_base);
	cmd_replace(chap_file, cf_nSize, pe, sys_dat, savfile);
	apply_appendix(chap_apple, ca_nSize, chap_file, pe->append.chap_apple);
}

void cmd_replace(char *cmd, size_t nSize, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const char *savefile) {
	char tmp[MAX_PATH_LEN] = { 0 };
	//置換操作の実行
	//%{vidpath}
	replace(cmd, nSize, "%{vidpath}", pe->temp_filename);
	//%{audpath}
	if (pe->append.aud && strlen(pe->append.aud)) {
		get_aud_filename(tmp, sizeof(tmp), pe);
		replace(cmd, nSize, "%{audpath}", tmp);
	}
	//%{tmpdir}
	strcpy_s(tmp, sizeof(tmp), pe->temp_filename);
	PathRemoveFileSpecFixed(tmp);
	PathForceRemoveBackSlash(tmp);
	replace(cmd, nSize, "%{tmpdir}", tmp);
	//%{tmpfile}
	strcpy_s(tmp, sizeof(tmp), pe->temp_filename);
	PathRemoveExtension(tmp);
	replace(cmd, nSize, "%{tmpfile}", tmp);
	//%{tmpname}
	strcpy_s(tmp, sizeof(tmp), PathFindFileName(pe->temp_filename));
	PathRemoveExtension(tmp);
	replace(cmd, nSize, "%{tmpname}", tmp);
	//%{savpath}
	replace(cmd, nSize, "%{savpath}", savefile);
	//%{savfile}
	strcpy_s(tmp, sizeof(tmp), savefile);
	PathRemoveExtension(tmp);
	replace(cmd, nSize, "%{savfile}", tmp);
	//%{savname}
	strcpy_s(tmp, sizeof(tmp), PathFindFileName(savefile));
	PathRemoveExtension(tmp);
	replace(cmd, nSize, "%{savname}", tmp);
	//%{savdir}
	strcpy_s(tmp, sizeof(tmp), savefile);
	PathRemoveFileSpecFixed(tmp);
	PathForceRemoveBackSlash(tmp);
	replace(cmd, nSize, "%{savdir}", tmp);
	//%{aviutldir}
	strcpy_s(tmp, sizeof(tmp), sys_dat->aviutl_dir);
	PathForceRemoveBackSlash(tmp);
	replace(cmd, nSize, "%{aviutldir}", tmp);
	//%{chpath}
	apply_appendix(tmp, sizeof(tmp), pe->temp_filename, pe->append.chap);
	replace(cmd, nSize, "%{chpath}", tmp);
	//%{tcpath}
	apply_appendix(tmp, sizeof(tmp), pe->temp_filename, pe->append.tc);
	replace(cmd, nSize, "%{tcpath}", tmp);
	//%{muxout}
	get_muxout_filename(tmp, sizeof(tmp), pe->temp_filename);
	replace(cmd, nSize, "%{muxout}", tmp);
}

//一時ファイルの移動・削除を行う 
// move_from -> move_to
// temp_filename … 動画ファイルの一時ファイル名。これにappendixをつけてmove_from を作る。
//                  appndixがNULLのときはこれをそのままmove_fromとみなす。
// appendix      … ファイルの後修飾子。NULLも可。
// savefile      … 保存動画ファイル名。これにappendixをつけてmove_to を作る。NULLだと move_to に移動できない。
// ret, erase    … これまでのエラーと一時ファイルを削除するかどうか。エラーがない場合にのみ削除できる
// name          … 一時ファイルの種類の名前
// must_exist    … trueのとき、移動するべきファイルが存在しないとエラーを返し、ファイルが存在しないことを伝える
static BOOL move_temp_file(const char *appendix, const char *temp_filename, const char *savefile, DWORD ret, BOOL erase, const char *name, BOOL must_exist) {
	char move_from[MAX_PATH_LEN] = { 0 };
	if (appendix)
		apply_appendix(move_from, sizeof(move_from), temp_filename, appendix);
	else
		strcpy_s(move_from, sizeof(move_from), temp_filename);

	if (!PathFileExists(move_from)) {
		if (must_exist)
			write_log_auo_line_fmt(LOG_WARNING, "%sファイルが見つかりませんでした。", name);
		return (must_exist) ? FALSE : TRUE;
	}
	if (ret == OUT_RESULT_SUCCESS && erase) {
		remove(move_from);
		return TRUE;
	}
	if (savefile == NULL || appendix == NULL)
		return TRUE;
	char move_to[MAX_PATH_LEN] = { 0 };
	apply_appendix(move_to, sizeof(move_to), savefile, appendix);
	if (_stricmp(move_from, move_to) != NULL) {
		if (PathFileExists(move_to))
			remove(move_to);
		if (rename(move_from, move_to))
			write_log_auo_line_fmt(LOG_WARNING, "%sファイルの移動に失敗しました。", name);
	}
	return TRUE;
}

DWORD move_temporary_files(const CONF_X264GUIEX *conf, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const char *savefile, DWORD ret) {
	//動画ファイル
	if (!conf->oth.out_audio_only)
		if (!move_temp_file(PathFindExtension(savefile), pe->temp_filename, savefile, ret, FALSE, "出力", !ret))
			ret |= OUT_RESULT_ERROR;
	//qpファイル
	move_temp_file(pe->append.qp,   pe->temp_filename, savefile, ret, TRUE, "qp", FALSE);
	//tcファイル
	BOOL erase_tc = conf->vid.afs && !conf->vid.auo_tcfile_out && pe->muxer_to_be_used != MUXER_DISABLED;
	move_temp_file(pe->append.tc,   pe->temp_filename, savefile, ret, erase_tc, "タイムコード", FALSE);
	//チャプターファイル
	if (pe->muxer_to_be_used >= 0 && sys_dat->exstg->s_local.auto_del_chap) {
		char chap_file[MAX_PATH_LEN];
		char chap_apple[MAX_PATH_LEN];
		const MUXER_CMD_EX *muxer_mode = &sys_dat->exstg->s_mux[pe->muxer_to_be_used].ex_cmd[(pe->muxer_to_be_used == MUXER_MKV) ? conf->mux.mkv_mode : conf->mux.mp4_mode];
		set_chap_filename(chap_file, sizeof(chap_file), chap_apple, sizeof(chap_apple), muxer_mode->chap_file, pe, sys_dat, savefile);
		move_temp_file(NULL, chap_file,  NULL, ret, TRUE, "チャプター",        FALSE);
		move_temp_file(NULL, chap_apple, NULL, ret, TRUE, "チャプター(Apple)", FALSE);
	}
	//ステータスファイル
	if (conf->x264.use_auto_npass && sys_dat->exstg->s_local.auto_del_stats) {
		char stats[MAX_PATH_LEN];
		strcpy_s(stats, sizeof(stats), conf->vid.stats);
		cmd_replace(stats, sizeof(stats), pe, sys_dat, savefile);
		move_temp_file(NULL, stats, NULL, ret, TRUE, "ステータス", FALSE);
		strcat_s(stats, sizeof(stats), ".mbtree");
		move_temp_file(NULL, stats, NULL, ret, TRUE, "mbtree ステータス", FALSE);
	}
	//音声ファイル(wav)
	move_temp_file(pe->append.wav,  pe->temp_filename, savefile, ret, TRUE, "wav", FALSE);
	//音声ファイル(エンコード後ファイル)
	char aud_tempfile[MAX_PATH_LEN];
	PathCombineLong(aud_tempfile, sizeof(aud_tempfile), pe->aud_temp_dir, PathFindFileName(pe->temp_filename));
	if (!move_temp_file(pe->append.aud, aud_tempfile, savefile, ret, !conf->oth.out_audio_only && pe->muxer_to_be_used != MUXER_DISABLED, "音声", conf->oth.out_audio_only))
		ret |= OUT_RESULT_ERROR;
	return ret;
}

DWORD GetExePriority(DWORD set, HANDLE h_aviutl) {
	if (set == AVIUTLSYNC_PRIORITY_CLASS)
		return (h_aviutl) ? GetPriorityClass(h_aviutl) : NORMAL_PRIORITY_CLASS;
	else
		return priority_table[set].value;
}

int check_video_ouput(const CONF_X264GUIEX *conf, const OUTPUT_INFO *oip) {
	if ((oip->flag & OUTPUT_INFO_FLAG_VIDEO) && !conf->oth.out_audio_only)
		return (check_ext(oip->savefile, ".mp4")) ? VIDEO_OUTPUT_MP4 : ((check_ext(oip->savefile, ".mkv")) ? VIDEO_OUTPUT_MKV : VIDEO_OUTPUT_RAW);
	return VIDEO_OUTPUT_DISABLED;
}

int check_muxer_to_be_used(const CONF_X264GUIEX *conf, int video_output_type, BOOL audio_output) {
	//if (conf.vid.afs)
	//	conf.mux.disable_mp4ext = conf.mux.disable_mkvext = FALSE; //afsなら外部muxerを強制する

	//音声なし、afsなしならmuxしない
	if (!audio_output && !conf->vid.afs)
		return MUXER_DISABLED;

	if (video_output_type == VIDEO_OUTPUT_MP4 && !conf->mux.disable_mp4ext)
		return (conf->vid.afs) ? MUXER_TC2MP4 : MUXER_MP4;
	else if (video_output_type == VIDEO_OUTPUT_MKV && !conf->mux.disable_mkvext)
		return MUXER_MKV;
	else
		return MUXER_DISABLED;
}