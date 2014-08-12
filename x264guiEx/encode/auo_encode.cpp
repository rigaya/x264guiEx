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
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "auo.h"
#include "auo_version.h"
#include "auo_util.h"
#include "auo_conf.h"
#include "auo_settings.h"
#include "auo_system.h"
#include "auo_pipe.h"

#include "auo_encode.h"
#include "auo_error.h"

void get_aud_filename(char *audfile, size_t nSize, const PRM_ENC *pe) {
	PathCombineLong(audfile, nSize, pe->aud_temp_dir, PathFindFileName(pe->temp_filename));
	apply_appendix(audfile, nSize, audfile, pe->append.aud);
}

static void get_muxout_appendix(char *muxout_appendix, size_t nSize, const char *tmp_filename) {
	static const char * const MUXOUT_APPENDIX = "_out";
	strcpy_s(muxout_appendix, nSize, MUXOUT_APPENDIX);
	strcat_s(muxout_appendix, nSize, PathFindExtension(tmp_filename));
}

void get_muxout_filename(char *filename, size_t nSize, const char *tmp_filename) {
	char muxout_appendix[MAX_APPENDIX_LEN];
	get_muxout_appendix(muxout_appendix, sizeof(muxout_appendix), tmp_filename);
	apply_appendix(filename, nSize, tmp_filename, muxout_appendix);
}

//チャプターファイル名とapple形式のチャプターファイル名を同時に作成する
void set_chap_filename(char *chap_file, size_t cf_nSize, char *chap_apple, size_t ca_nSize, const char *chap_base, 
					   const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const CONF_X264GUIEX *conf, const char *savfile) {
	strcpy_s(chap_file, cf_nSize, chap_base);
	cmd_replace(chap_file, cf_nSize, pe, sys_dat, conf, savfile);
	apply_appendix(chap_apple, ca_nSize, chap_file, pe->append.chap_apple);
}

void cmd_replace(char *cmd, size_t nSize, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const CONF_X264GUIEX *conf, const char *savefile) {
	char tmp[MAX_PATH_LEN] = { 0 };
	//置換操作の実行
	//%{vidpath}
	replace(cmd, nSize, "%{vidpath}", pe->temp_filename);
	//%{audpath}
	if (pe->append.aud && str_has_char(pe->append.aud)) {
		get_aud_filename(tmp, _countof(tmp), pe);
		replace(cmd, nSize, "%{audpath}", tmp);
	}
	//%{tmpdir}
	strcpy_s(tmp, _countof(tmp), pe->temp_filename);
	PathRemoveFileSpecFixed(tmp);
	PathForceRemoveBackSlash(tmp);
	replace(cmd, nSize, "%{tmpdir}", tmp);
	//%{tmpfile}
	strcpy_s(tmp, _countof(tmp), pe->temp_filename);
	PathRemoveExtension(tmp);
	replace(cmd, nSize, "%{tmpfile}", tmp);
	//%{tmpname}
	strcpy_s(tmp, _countof(tmp), PathFindFileName(pe->temp_filename));
	PathRemoveExtension(tmp);
	replace(cmd, nSize, "%{tmpname}", tmp);
	//%{savpath}
	replace(cmd, nSize, "%{savpath}", savefile);
	//%{savfile}
	strcpy_s(tmp, _countof(tmp), savefile);
	PathRemoveExtension(tmp);
	replace(cmd, nSize, "%{savfile}", tmp);
	//%{savname}
	strcpy_s(tmp, _countof(tmp), PathFindFileName(savefile));
	PathRemoveExtension(tmp);
	replace(cmd, nSize, "%{savname}", tmp);
	//%{savdir}
	strcpy_s(tmp, _countof(tmp), savefile);
	PathRemoveFileSpecFixed(tmp);
	PathForceRemoveBackSlash(tmp);
	replace(cmd, nSize, "%{savdir}", tmp);
	//%{aviutldir}
	strcpy_s(tmp, _countof(tmp), sys_dat->aviutl_dir);
	PathForceRemoveBackSlash(tmp);
	replace(cmd, nSize, "%{aviutldir}", tmp);
	//%{chpath}
	apply_appendix(tmp, _countof(tmp), pe->temp_filename, pe->append.chap);
	replace(cmd, nSize, "%{chpath}", tmp);
	//%{tcpath}
	apply_appendix(tmp, _countof(tmp), pe->temp_filename, pe->append.tc);
	replace(cmd, nSize, "%{tcpath}", tmp);
	//%{muxout}
	get_muxout_filename(tmp, _countof(tmp), pe->temp_filename);
	replace(cmd, nSize, "%{muxout}", tmp);

	char fullpath[MAX_PATH_LEN];
	replace(cmd, nSize, "%{x264path}",     GetFullPath(sys_dat->exstg->s_x264.fullpath,                   fullpath, _countof(fullpath)));
	replace(cmd, nSize, "%{x264_10path}",  GetFullPath(sys_dat->exstg->s_x264.fullpath_10bit,             fullpath, _countof(fullpath)));
	replace(cmd, nSize, "%{audencpath}",   GetFullPath(sys_dat->exstg->s_aud[conf->aud.encoder].fullpath, fullpath, _countof(fullpath)));
	replace(cmd, nSize, "%{mp4muxerpath}", GetFullPath(sys_dat->exstg->s_mux[MUXER_MP4].fullpath,         fullpath, _countof(fullpath)));
	replace(cmd, nSize, "%{mkvmuxerpath}", GetFullPath(sys_dat->exstg->s_mux[MUXER_MKV].fullpath,         fullpath, _countof(fullpath)));
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
		apply_appendix(move_from, _countof(move_from), temp_filename, appendix);
	else
		strcpy_s(move_from, _countof(move_from), temp_filename);

	if (!PathFileExists(move_from)) {
		if (must_exist)
			write_log_auo_line_fmt(LOG_WARNING, "%sファイルが見つかりませんでした。", name);
		return (must_exist) ? FALSE : TRUE;
	}
	if (ret == AUO_RESULT_SUCCESS && erase) {
		remove(move_from);
		return TRUE;
	}
	if (savefile == NULL || appendix == NULL)
		return TRUE;
	char move_to[MAX_PATH_LEN] = { 0 };
	apply_appendix(move_to, _countof(move_to), savefile, appendix);
	if (_stricmp(move_from, move_to) != NULL) {
		if (PathFileExists(move_to))
			remove(move_to);
		if (rename(move_from, move_to))
			write_log_auo_line_fmt(LOG_WARNING, "%sファイルの移動に失敗しました。", name);
	}
	return TRUE;
}

AUO_RESULT move_temporary_files(const CONF_X264GUIEX *conf, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const char *savefile, DWORD ret) {
	//動画ファイル
	if (!conf->oth.out_audio_only)
		if (!move_temp_file(PathFindExtension(savefile), pe->temp_filename, savefile, ret, FALSE, "出力", !ret))
			ret |= AUO_RESULT_ERROR;
	//mux後ファイル
	if (pe->muxer_to_be_used >= 0) {
		char muxout_appendix[MAX_APPENDIX_LEN];
		get_muxout_appendix(muxout_appendix, _countof(muxout_appendix), pe->temp_filename);
		move_temp_file(muxout_appendix, pe->temp_filename, savefile, ret, FALSE, "mux後ファイル", FALSE);
	}
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
		set_chap_filename(chap_file, _countof(chap_file), chap_apple, _countof(chap_apple), muxer_mode->chap_file, pe, sys_dat, conf, savefile);
		move_temp_file(NULL, chap_file,  NULL, ret, TRUE, "チャプター",        FALSE);
		move_temp_file(NULL, chap_apple, NULL, ret, TRUE, "チャプター(Apple)", FALSE);
	}
	//ステータスファイル
	if (conf->x264.use_auto_npass && sys_dat->exstg->s_local.auto_del_stats) {
		char stats[MAX_PATH_LEN];
		strcpy_s(stats, sizeof(stats), conf->vid.stats);
		cmd_replace(stats, sizeof(stats), pe, sys_dat, conf, savefile);
		move_temp_file(NULL, stats, NULL, ret, TRUE, "ステータス", FALSE);
		strcat_s(stats, sizeof(stats), ".mbtree");
		move_temp_file(NULL, stats, NULL, ret, TRUE, "mbtree ステータス", FALSE);
	}
	//音声ファイル(wav)
	if (strcmp(pe->append.aud, pe->append.wav)) //「wav出力」ならここでは処理せず下のエンコード後ファイルとして扱う
		move_temp_file(pe->append.wav,  pe->temp_filename, savefile, ret, TRUE, "wav", FALSE);
	//音声ファイル(エンコード後ファイル)
	char aud_tempfile[MAX_PATH_LEN];
	PathCombineLong(aud_tempfile, _countof(aud_tempfile), pe->aud_temp_dir, PathFindFileName(pe->temp_filename));
	if (!move_temp_file(pe->append.aud, aud_tempfile, savefile, ret, !conf->oth.out_audio_only && pe->muxer_to_be_used != MUXER_DISABLED, "音声", conf->oth.out_audio_only))
		ret |= AUO_RESULT_ERROR;
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

AUO_RESULT getLogFilePath(char *log_file_path, size_t nSize, const PRM_ENC *pe, const char *savefile, const SYSTEM_DATA *sys_dat, const CONF_X264GUIEX *conf) {
	AUO_RESULT ret = AUO_RESULT_SUCCESS;
	guiEx_settings stg(TRUE); //ログウィンドウの保存先設定は最新のものを使用する
	stg.load_log_win();
	switch (stg.s_log.auto_save_log_mode) {
		case AUTO_SAVE_LOG_CUSTOM:
			char log_file_dir[MAX_PATH_LEN];
			strcpy_s(log_file_path, nSize, stg.s_log.auto_save_log_path);
			cmd_replace(log_file_path, nSize, pe, sys_dat, conf, savefile);
			PathGetDirectory(log_file_dir, _countof(log_file_dir), log_file_path);
			if (DirectoryExistsOrCreate(log_file_dir))
				break;
			ret = AUO_RESULT_WARNING;
			//下へフォールスルー
		case AUTO_SAVE_LOG_OUTPUT_DIR:
		default:
			apply_appendix(log_file_path, nSize, savefile, "_log.txt"); 
			break;
	}
	return ret;
}

//tc_filenameのタイムコードを分析して動画の長さを得て、
//duration(秒)にセットする
//fpsにはAviutlからの値を与える(参考として使う)
static AUO_RESULT get_duration_from_timecode(double *duration, const char *tc_filename, double fps) {
	AUO_RESULT ret = AUO_RESULT_SUCCESS;
	FILE *fp = NULL;
	*duration = 0.0;
	if (!(NULL == fopen_s(&fp, tc_filename, "r") && fp)) {
		//ファイルオープンエラー
		ret |= AUO_RESULT_ERROR;
	} else {
		const int avg_frames = 5; //平均をとるフレーム数
		char buf[256];
		double timecode[avg_frames];
		//ファイルからタイムコードを読み出し
		int frame = 0;
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			if (buf[0] == '#')
				continue;
			if (1 != sscanf_s(buf, "%lf", &timecode[frame%avg_frames])) {
				ret |= AUO_RESULT_ERROR; break;
			}
			frame++;
		}
		fclose(fp);
		frame--; //最後のフレームに合わせる
		switch (frame) {
			case -1: //1フレーム分も読めなかった
				ret |= AUO_RESULT_ERROR; break;
			case 0: //1フレームのみ
				*duration = 1.0 / fps; break;
			default: //フレーム時間を求める((avg_frames-1)フレーム分から平均をとる)
				int div = 0, n = min(frame, avg_frames);
				double sum = 0.0;
				for (int i = 0; i < n; i++) {
					sum += timecode[i];
					div += i;
				}
				double frame_time = -1.0 * (sum - timecode[frame%avg_frames] * n) / (double)div;
				*duration = (timecode[frame%avg_frames] + frame_time) / 1000.0;
				break;
		}
	}
	return ret;
}

double get_duration(const CONF_X264GUIEX *conf, const SYSTEM_DATA *sys_dat, const PRM_ENC *pe, const OUTPUT_INFO *oip) {
	char buffer[MAX_PATH_LEN];
	//Aviutlから再生時間情報を取得
	double duration = (((double)oip->n * (double)oip->scale) / (double)oip->rate);
	//tcfile-inなら、動画の長さはタイムコードから取得する
	if (conf->x264.use_tcfilein || 0 == get_option_value(conf->vid.cmdex, "--tcfile-in", buffer, sizeof(buffer))) {
		double duration_tmp = 0.0;
		if (conf->x264.use_tcfilein)
			strcpy_s(buffer, sizeof(buffer), conf->vid.tcfile_in);
		cmd_replace(buffer, sizeof(buffer), pe, sys_dat, conf, oip->savefile);
		if (AUO_RESULT_SUCCESS == get_duration_from_timecode(&duration_tmp, buffer, oip->rate / (double)oip->scale))
			duration = duration_tmp;
		else
			warning_failed_to_get_duration_from_timecode();
	}
	return duration;
}

double get_amp_margin_bitrate(double base_bitrate, double margin_multi) {
	return base_bitrate * clamp(1.0 - margin_multi / sqrt(base_bitrate / 100.0), 0.8, 1.0);
}

static AUO_RESULT amp_move_old_file(const char *muxout, const char *savefile) {
	if (!PathFileExists(muxout))
		return AUO_RESULT_ERROR;
	char filename[MAX_PATH_LEN];
	char appendix[MAX_APPENDIX_LEN];
	for (int i = 0; i && PathFileExists(filename); i++) {
		sprintf_s(appendix, _countof(appendix), "_try%d%s", PathFindExtension(savefile));
		apply_appendix(filename, _countof(filename), savefile, appendix);
	}
	return (rename(muxout, filename) == 0) ? AUO_RESULT_SUCCESS : AUO_RESULT_ERROR;
}

//戻り値
//AUO_RESULT_SUCCESS  … チェック完了
//AUO_RESULT_ERROR    … チェックできない
//AUO_REESULT_WARNING … 再エンコの必要あり
AUO_RESULT amp_check_file(CONF_X264GUIEX *conf, const SYSTEM_DATA *sys_dat, PRM_ENC *pe, const OUTPUT_INFO *oip) {
	if (!conf->x264.use_auto_npass || !conf->vid.amp_check)
		return AUO_RESULT_SUCCESS;
	//チェックするファイル名を取得
	char muxout[MAX_PATH_LEN];
	if (PathFileExists(pe->temp_filename)) {
		strcpy_s(muxout, _countof(muxout), pe->temp_filename);
	} else {
		//tempfileがない場合、mux後ファイルをチェックする
		get_muxout_filename(muxout, _countof(muxout), pe->temp_filename);
		if (pe->muxer_to_be_used < 0 || !PathFileExists(muxout)) {
			error_check_muxout_exist(); warning_amp_failed();
			return AUO_RESULT_ERROR;
		}
	}
	//ファイルサイズを取得し、ビットレートを計算する
	UINT64 filesize = 0;
	if (!GetFileSizeUInt64(muxout, &filesize)) {
		warning_failed_check_muxout_filesize(); warning_amp_failed();
		return AUO_RESULT_ERROR;
	}
	double file_bitrate = (filesize * 8.0) / 1000.0 / get_duration(conf, sys_dat, pe, oip);
	DWORD status = NULL;
	//ファイルサイズのチェックを行う
	if ((conf->vid.amp_check & AMPLIMIT_FILE_SIZE) && filesize > conf->vid.amp_limit_file_size * 1024*1024)
		status |= AMPLIMIT_FILE_SIZE;
	//ビットレートのチェックを行う
	if ((conf->vid.amp_check & AMPLIMIT_BITRATE) && file_bitrate > conf->vid.amp_limit_bitrate)
		status |= AMPLIMIT_BITRATE;

	BOOL retry = (status && pe->current_x264_pass < pe->amp_x264_pass_limit);
	//再エンコードを行う
	if (retry) {
		pe->total_x264_pass++;
		//再エンコ時は現在の目標ビットレートより少し下げたレートでエンコーダを行う
		//3通りの方法で計算してみる
		double margin_bitrate = get_amp_margin_bitrate(conf->x264.bitrate, sys_dat->exstg->s_local.amp_bitrate_margin_multi * 0.5);
		double bitrate_limit  = (conf->vid.amp_check & AMPLIMIT_BITRATE)   ? conf->x264.bitrate - 0.5 * (file_bitrate - conf->vid.amp_limit_bitrate) : conf->x264.bitrate;
		double filesize_limit = (conf->vid.amp_check & AMPLIMIT_FILE_SIZE) ? conf->x264.bitrate - 0.5 * ((filesize - conf->vid.amp_limit_file_size*1024*1024))* 8.0/1000.0 / get_duration(conf, sys_dat, pe, oip) : conf->x264.bitrate;
		conf->x264.bitrate = (int)(0.5 + min(margin_bitrate, min(bitrate_limit, filesize_limit)));
		//必要なら、今回作成した動画を待避
		if (sys_dat->exstg->s_local.amp_keep_old_file)
			amp_move_old_file(muxout, oip->savefile);
	}
	info_amp_result(status, retry, filesize, file_bitrate, conf->vid.amp_limit_file_size, conf->vid.amp_limit_bitrate, pe->current_x264_pass - conf->x264.auto_npass, conf->x264.bitrate);

	return (retry) ? AUO_RESULT_WARNING : AUO_RESULT_SUCCESS;
}
