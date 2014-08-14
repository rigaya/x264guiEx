//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx/2.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <Math.h>
#include <float.h>
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <vector>
#include <string>

#include "auo.h"
#include "auo_version.h"
#include "auo_util.h"
#include "auo_conf.h"
#include "auo_settings.h"
#include "auo_system.h"
#include "auo_pipe.h"

#include "auo_frm.h"
#include "auo_encode.h"
#include "auo_error.h"

void get_aud_filename(char *audfile, size_t nSize, const PRM_ENC *pe, int i_aud) {
	PathCombineLong(audfile, nSize, pe->aud_temp_dir, PathFindFileName(pe->temp_filename));
	apply_appendix(audfile, nSize, audfile, pe->append.aud[i_aud]);
}

static void get_muxout_appendix(char *muxout_appendix, size_t nSize, const SYSTEM_DATA *sys_dat, const PRM_ENC *pe) {
	static const char * const MUXOUT_APPENDIX = "_muxout";
	strcpy_s(muxout_appendix, nSize, MUXOUT_APPENDIX);
	const char *ext = (pe->muxer_to_be_used >= 0 && str_has_char(sys_dat->exstg->s_mux[pe->muxer_to_be_used].out_ext)) ?
		sys_dat->exstg->s_mux[pe->muxer_to_be_used].out_ext : PathFindExtension(pe->temp_filename);
	strcat_s(muxout_appendix, nSize, ext);
}

void get_muxout_filename(char *filename, size_t nSize, const SYSTEM_DATA *sys_dat, const PRM_ENC *pe) {
	char muxout_appendix[MAX_APPENDIX_LEN];
	get_muxout_appendix(muxout_appendix, sizeof(muxout_appendix), sys_dat, pe);
	apply_appendix(filename, nSize, pe->temp_filename, muxout_appendix);
}

//チャプターファイル名とapple形式のチャプターファイル名を同時に作成する
void set_chap_filename(char *chap_file, size_t cf_nSize, char *chap_apple, size_t ca_nSize, const char *chap_base, 
					   const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const CONF_GUIEX *conf, const OUTPUT_INFO *oip) {
	strcpy_s(chap_file, cf_nSize, chap_base);
	cmd_replace(chap_file, cf_nSize, pe, sys_dat, conf, oip);
	apply_appendix(chap_apple, ca_nSize, chap_file, pe->append.chap_apple);
	sys_dat->exstg->apply_fn_replace(PathFindFileName(chap_apple), ca_nSize - (PathFindFileName(chap_apple) - chap_apple));
}

void insert_num_to_replace_key(char *key, size_t nSize, int num) {
	char tmp[128];
	int key_len = strlen(key);
	sprintf_s(tmp, _countof(tmp), "%d%s", num, &key[key_len-1]);
	key[key_len-1] = '\0';
	strcat_s(key, nSize, tmp);
}

static void replace_aspect_ratio(char *cmd, size_t nSize, const CONF_GUIEX *conf, const OUTPUT_INFO *oip) {
	const int w = oip->w;
	const int h = oip->h;
 
	int sar_x = conf->x264.sar.x;
	int sar_y = conf->x264.sar.y;
	int dar_x = 0;
	int dar_y = 0;
	if (sar_x * sar_y > 0) {
		if (sar_x < 0) {
			dar_x = -1 * sar_x;
			dar_y = -1 * sar_y;
			set_guiEx_auto_sar(&sar_x, &sar_y, w, h);
		} else {
			dar_x = sar_x * w;
			dar_y = sar_y * h;
			const int gcd = get_gcd(dar_x, dar_y);
			dar_x /= gcd;
			dar_y /= gcd;
		}
	}
	if (sar_x * sar_y <= 0)
		sar_x = sar_y = 1;
	if (dar_x * dar_y <= 0)
		dar_x = dar_y = 1;

	char buf[32];
	//%{sar_x} / %{par_x}
	sprintf_s(buf, _countof(buf), "%d", sar_x);
	replace(cmd, nSize, "%{sar_x}", buf);
	replace(cmd, nSize, "%{par_x}", buf);
	//%{sar_x} / %{sar_y}
	sprintf_s(buf, _countof(buf), "%d", sar_y);
	replace(cmd, nSize, "%{sar_y}", buf);
	replace(cmd, nSize, "%{par_y}", buf);
	//%{dar_x}
	sprintf_s(buf, _countof(buf), "%d", dar_x);
	replace(cmd, nSize, "%{dar_x}", buf);
	//%{dar_y}
	sprintf_s(buf, _countof(buf), "%d", dar_y);
	replace(cmd, nSize, "%{dar_y}", buf);
}

void cmd_replace(char *cmd, size_t nSize, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const CONF_GUIEX *conf, const OUTPUT_INFO *oip) {
	char tmp[MAX_PATH_LEN] = { 0 };
	//置換操作の実行
	//%{vidpath}
	replace(cmd, nSize, "%{vidpath}", pe->temp_filename);
	//%{audpath}
	for (int i_aud = 0; i_aud < pe->aud_count; i_aud++) {
		if (str_has_char(pe->append.aud[i_aud])) {
			get_aud_filename(tmp, _countof(tmp), pe, i_aud);
			char aud_key[128] = "%{audpath}";
			if (i_aud)
				insert_num_to_replace_key(aud_key, _countof(aud_key), i_aud);
			replace(cmd, nSize, aud_key, tmp);
		}
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
	replace(cmd, nSize, "%{savpath}", oip->savefile);
	//%{savfile}
	strcpy_s(tmp, _countof(tmp), oip->savefile);
	PathRemoveExtension(tmp);
	replace(cmd, nSize, "%{savfile}", tmp);
	//%{savname}
	strcpy_s(tmp, _countof(tmp), PathFindFileName(oip->savefile));
	PathRemoveExtension(tmp);
	replace(cmd, nSize, "%{savname}", tmp);
	//%{savdir}
	strcpy_s(tmp, _countof(tmp), oip->savefile);
	PathRemoveFileSpecFixed(tmp);
	PathForceRemoveBackSlash(tmp);
	replace(cmd, nSize, "%{savdir}", tmp);
	//%{aviutldir}
	strcpy_s(tmp, _countof(tmp), sys_dat->aviutl_dir);
	PathForceRemoveBackSlash(tmp);
	replace(cmd, nSize, "%{aviutldir}", tmp);
	//%{chpath}
	apply_appendix(tmp, _countof(tmp), oip->savefile, pe->append.chap);
	replace(cmd, nSize, "%{chpath}", tmp);
	//%{tcpath}
	apply_appendix(tmp, _countof(tmp), pe->temp_filename, pe->append.tc);
	replace(cmd, nSize, "%{tcpath}", tmp);
	//%{muxout}
	get_muxout_filename(tmp, _countof(tmp), sys_dat, pe);
	replace(cmd, nSize, "%{muxout}", tmp);
	//%{fps_rate}
	int fps_rate = oip->rate;
	int fps_scale = oip->scale;
#ifdef MSDK_SAMPLE_VERSION
	if (conf->qsv.vpp.nDeinterlace == MFX_DEINTERLACE_IT)
		fps_rate = (fps_rate * 4) / 5;
#endif
	const int fps_gcd = get_gcd(fps_rate, fps_scale);
	fps_rate /= fps_gcd;
	fps_scale /= fps_gcd;
	sprintf_s(tmp, sizeof(tmp), "%d", fps_rate);
	replace(cmd, nSize, "%{fps_rate}", tmp);
	//%{fps_rate_times_4}
	fps_rate *= 4;
	sprintf_s(tmp, sizeof(tmp), "%d", fps_rate);
	replace(cmd, nSize, "%{fps_rate_times_4}", tmp);
	//%{fps_scale}
	sprintf_s(tmp, sizeof(tmp), "%d", fps_scale);
	replace(cmd, nSize, "%{fps_scale}", tmp);
	//アスペクト比
	replace_aspect_ratio(cmd, nSize, conf, oip);

	char fullpath[MAX_PATH_LEN];
	replace(cmd, nSize, "%{x264path}",     GetFullPath(sys_dat->exstg->s_x264.fullpath,                   fullpath, _countof(fullpath)));
	replace(cmd, nSize, "%{x264_10path}",  GetFullPath(sys_dat->exstg->s_x264.fullpath_highbit,           fullpath, _countof(fullpath)));
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

AUO_RESULT move_temporary_files(const CONF_GUIEX *conf, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const OUTPUT_INFO *oip, DWORD ret) {
	//動画ファイル
	if (!conf->oth.out_audio_only)
		if (!move_temp_file(PathFindExtension((pe->muxer_to_be_used >= 0) ? oip->savefile : pe->temp_filename), pe->temp_filename, oip->savefile, ret, FALSE, "出力", !ret))
			ret |= AUO_RESULT_ERROR;
	//動画のみファイル
	if (str_has_char(pe->muxed_vid_filename) && PathFileExists(pe->muxed_vid_filename))
		remove(pe->muxed_vid_filename);
	//mux後ファイル
	if (pe->muxer_to_be_used >= 0) {
		char muxout_appendix[MAX_APPENDIX_LEN];
		get_muxout_appendix(muxout_appendix, _countof(muxout_appendix), sys_dat, pe);
		move_temp_file(muxout_appendix, pe->temp_filename, oip->savefile, ret, FALSE, "mux後ファイル", FALSE);
	}
	//qpファイル
	move_temp_file(pe->append.qp,   pe->temp_filename, oip->savefile, ret, !sys_dat->exstg->s_local.keep_qp_file, "qp", FALSE);
	//tcファイル
	BOOL erase_tc = conf->vid.afs && !conf->vid.auo_tcfile_out && pe->muxer_to_be_used != MUXER_DISABLED;
	move_temp_file(pe->append.tc,   pe->temp_filename, oip->savefile, ret, erase_tc, "タイムコード", FALSE);
	//チャプターファイル
	if (pe->muxer_to_be_used >= 0 && sys_dat->exstg->s_local.auto_del_chap) {
		char chap_file[MAX_PATH_LEN];
		char chap_apple[MAX_PATH_LEN];
		const MUXER_CMD_EX *muxer_mode = &sys_dat->exstg->s_mux[pe->muxer_to_be_used].ex_cmd[(pe->muxer_to_be_used == MUXER_MKV) ? conf->mux.mkv_mode : conf->mux.mp4_mode];
		set_chap_filename(chap_file, _countof(chap_file), chap_apple, _countof(chap_apple), muxer_mode->chap_file, pe, sys_dat, conf, oip);
		move_temp_file(NULL, chap_file,  NULL, ret, TRUE, "チャプター",        FALSE);
		move_temp_file(NULL, chap_apple, NULL, ret, TRUE, "チャプター(Apple)", FALSE);
	}
	//ステータスファイル
	if (conf->x264.use_auto_npass && sys_dat->exstg->s_local.auto_del_stats) {
		char stats[MAX_PATH_LEN];
		strcpy_s(stats, sizeof(stats), conf->vid.stats);
		cmd_replace(stats, sizeof(stats), pe, sys_dat, conf, oip);
		move_temp_file(NULL, stats, NULL, ret, TRUE, "ステータス", FALSE);
		strcat_s(stats, sizeof(stats), ".mbtree");
		move_temp_file(NULL, stats, NULL, ret, TRUE, "mbtree ステータス", FALSE);
	}
	//音声ファイル(wav)
	if (strcmp(pe->append.aud[0], pe->append.wav)) //「wav出力」ならここでは処理せず下のエンコード後ファイルとして扱う
		move_temp_file(pe->append.wav,  pe->temp_filename, oip->savefile, ret, TRUE, "wav", FALSE);
	//音声ファイル(エンコード後ファイル)
	char aud_tempfile[MAX_PATH_LEN];
	PathCombineLong(aud_tempfile, _countof(aud_tempfile), pe->aud_temp_dir, PathFindFileName(pe->temp_filename));
	for (int i_aud = 0; i_aud < pe->aud_count; i_aud++)
		if (!move_temp_file(pe->append.aud[i_aud], aud_tempfile, oip->savefile, ret, !conf->oth.out_audio_only && pe->muxer_to_be_used != MUXER_DISABLED, "音声", conf->oth.out_audio_only))
			ret |= AUO_RESULT_ERROR;
	return ret;
}

DWORD GetExePriority(DWORD set, HANDLE h_aviutl) {
	if (set == AVIUTLSYNC_PRIORITY_CLASS)
		return (h_aviutl) ? GetPriorityClass(h_aviutl) : NORMAL_PRIORITY_CLASS;
	else
		return priority_table[set].value;
}

int check_video_ouput(const CONF_GUIEX *conf, const OUTPUT_INFO *oip) {
	if ((oip->flag & OUTPUT_INFO_FLAG_VIDEO) && !conf->oth.out_audio_only) {
		if (check_ext(oip->savefile, ".mp4"))  return VIDEO_OUTPUT_MP4;
		if (check_ext(oip->savefile, ".mkv"))  return VIDEO_OUTPUT_MKV;
		if (check_ext(oip->savefile, ".mpg"))  return VIDEO_OUTPUT_MPEG2;
		if (check_ext(oip->savefile, ".mpeg")) return VIDEO_OUTPUT_MPEG2;
		return VIDEO_OUTPUT_RAW;
	}
	return VIDEO_OUTPUT_DISABLED;
}

int check_muxer_to_be_used(const CONF_GUIEX *conf, int video_output_type, BOOL audio_output) {
	//if (conf.vid.afs)
	//	conf.mux.disable_mp4ext = conf.mux.disable_mkvext = FALSE; //afsなら外部muxerを強制する

	//音声なし、afsなしならmuxしない
	if (!audio_output && !conf->vid.afs)
		return MUXER_DISABLED;

	if (video_output_type == VIDEO_OUTPUT_MP4 && !conf->mux.disable_mp4ext)
		return (conf->vid.afs) ? MUXER_TC2MP4 : MUXER_MP4;
	else if (video_output_type == VIDEO_OUTPUT_MKV && !conf->mux.disable_mkvext)
		return MUXER_MKV;
	else if (video_output_type == VIDEO_OUTPUT_MPEG2 && !conf->mux.disable_mpgext)
		return MUXER_MPG;
	else
		return MUXER_DISABLED;
}

AUO_RESULT getLogFilePath(char *log_file_path, size_t nSize, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const CONF_GUIEX *conf, const OUTPUT_INFO *oip) {
	AUO_RESULT ret = AUO_RESULT_SUCCESS;
	guiEx_settings stg(TRUE); //ログウィンドウの保存先設定は最新のものを使用する
	stg.load_log_win();
	switch (stg.s_log.auto_save_log_mode) {
		case AUTO_SAVE_LOG_CUSTOM:
			char log_file_dir[MAX_PATH_LEN];
			strcpy_s(log_file_path, nSize, stg.s_log.auto_save_log_path);
			cmd_replace(log_file_path, nSize, pe, sys_dat, conf, oip);
			PathGetDirectory(log_file_dir, _countof(log_file_dir), log_file_path);
			if (DirectoryExistsOrCreate(log_file_dir))
				break;
			ret = AUO_RESULT_WARNING;
			//下へフォールスルー
		case AUTO_SAVE_LOG_OUTPUT_DIR:
		default:
			apply_appendix(log_file_path, nSize, oip->savefile, "_log.txt"); 
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

double get_duration(const CONF_GUIEX *conf, const SYSTEM_DATA *sys_dat, const PRM_ENC *pe, const OUTPUT_INFO *oip) {
	char buffer[MAX_PATH_LEN];
	//Aviutlから再生時間情報を取得
	double duration = (((double)oip->n * (double)oip->scale) / (double)oip->rate);
	//tcfile-inなら、動画の長さはタイムコードから取得する
	if (conf->x264.use_tcfilein || 0 == get_option_value(conf->vid.cmdex, "--tcfile-in", buffer, sizeof(buffer))) {
		double duration_tmp = 0.0;
		if (conf->x264.use_tcfilein)
			strcpy_s(buffer, sizeof(buffer), conf->vid.tcfile_in);
		cmd_replace(buffer, sizeof(buffer), pe, sys_dat, conf, oip);
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
	for (int i = 0; !i || PathFileExists(filename); i++) {
		sprintf_s(appendix, _countof(appendix), "_try%d%s", i, PathFindExtension(savefile));
		apply_appendix(filename, _countof(filename), savefile, appendix);
	}
	return (rename(muxout, filename) == 0) ? AUO_RESULT_SUCCESS : AUO_RESULT_ERROR;
}

//戻り値
//  0 … チェック完了
// -1 … チェックできない(エラー)
//  1 … 動画を再エンコ
//  2 … 音声を再エンコ
int amp_check_file(CONF_GUIEX *conf, const SYSTEM_DATA *sys_dat, PRM_ENC *pe, const OUTPUT_INFO *oip) {
	if (!conf->x264.use_auto_npass || !conf->vid.amp_check || conf->oth.out_audio_only)
		return 0;
	//チェックするファイル名を取得
	char muxout[MAX_PATH_LEN];
	if (PathFileExists(pe->temp_filename)) {
		strcpy_s(muxout, _countof(muxout), pe->temp_filename);
	} else {
		//tempfileがない場合、mux後ファイルをチェックする
		get_muxout_filename(muxout, _countof(muxout), sys_dat, pe);
		if (pe->muxer_to_be_used < 0 || !PathFileExists(muxout)) {
			error_check_muxout_exist(); warning_amp_failed();
			return -1;
		}
	}
	//ファイルサイズを取得し、ビットレートを計算する
	UINT64 filesize = 0;
	if (!GetFileSizeUInt64(muxout, &filesize)) {
		warning_failed_check_muxout_filesize(); warning_amp_failed();
		return -1;
	}
	const double duration = get_duration(conf, sys_dat, pe, oip);
	double file_bitrate = (filesize * 8.0) / 1000.0 / duration;
	DWORD status = NULL;
	//ファイルサイズのチェックを行う
	if ((conf->vid.amp_check & AMPLIMIT_FILE_SIZE) && filesize > conf->vid.amp_limit_file_size * 1024*1024)
		status |= AMPLIMIT_FILE_SIZE;
	//ビットレートのチェックを行う
	if ((conf->vid.amp_check & AMPLIMIT_BITRATE) && file_bitrate > conf->vid.amp_limit_bitrate)
		status |= AMPLIMIT_BITRATE;

	BOOL retry = (status && pe->current_x264_pass < pe->amp_x264_pass_limit);
	BOOL show_header = FALSE;
	int amp_result = 0;
	//再エンコードを行う
	if (retry) {
		//音声がビットレートモードなら音声再エンコによる調整を検討する
		double limit_bitrate = DBL_MAX;
		if (status & AMPLIMIT_FILE_SIZE)
			limit_bitrate = min(limit_bitrate, (conf->vid.amp_limit_file_size * 1024*1024)*8.0/1000 / duration);
		if (status & AMPLIMIT_BITRATE)
			limit_bitrate = min(limit_bitrate, conf->vid.amp_limit_bitrate);
		const double bitrate_delta = file_bitrate - limit_bitrate;
		const AUDIO_SETTINGS *aud_stg = &sys_dat->exstg->s_aud[conf->aud.encoder];
		if ((oip->flag & OUTPUT_INFO_FLAG_AUDIO)
			&& aud_stg->mode[conf->aud.enc_mode].bitrate
			&& bitrate_delta + 1.0 < conf->aud.bitrate * sys_dat->exstg->s_local.amp_reenc_audio_multi
			&& str_has_char(pe->muxed_vid_filename)
			&& PathFileExists(pe->muxed_vid_filename)) {
			//音声の再エンコードで修正
			amp_result = 2;
			conf->aud.bitrate -= (int)(bitrate_delta + 1.5);

			//動画のみファイルをもとの位置へ
			remove(pe->temp_filename);
			char temp_ext[MAX_APPENDIX_LEN];
			strcpy_s(temp_ext, _countof(temp_ext), VID_FILE_APPENDIX);
			strcat_s(temp_ext, _countof(temp_ext), PathFindExtension(pe->temp_filename));
			replace(pe->temp_filename, _countof(pe->temp_filename), temp_ext, temp_ext + strlen(VID_FILE_APPENDIX));
			if (PathFileExists(pe->temp_filename)) remove(pe->temp_filename);
			rename(pe->muxed_vid_filename, pe->temp_filename);

			//音声エンコードではヘッダーが表示されないので、 ここで表示しておく
			show_header = TRUE;
		} else {
			//動画の再エンコードで修正
			amp_result = 1;
			pe->total_x264_pass++;
			if (conf->x264.rc_mode == X264_RC_CRF) {
				//上限確認付 品質基準VBR(可変レート)の場合、自動的に再設定
				pe->amp_x264_pass_limit++;
				pe->current_x264_pass = 1;
				conf->x264.rc_mode = X264_RC_BITRATE;
				conf->x264.slow_first_pass = FALSE;
				conf->x264.nul_out = TRUE;
				//ここでは目標ビットレートには-1を指定しておき、
				//後段で上限設定をもとに修正させる
				conf->x264.bitrate = -1;
				//自動マルチパスの1pass目には本来ヘッダーが表示されないので、 ここで表示しておく
				show_header = TRUE;
			} else {
				//再エンコ時は現在の目標ビットレートより少し下げたレートでエンコーダを行う
				//3通りの方法で計算してみる
				double margin_bitrate = get_amp_margin_bitrate(conf->x264.bitrate, sys_dat->exstg->s_local.amp_bitrate_margin_multi * 0.5);
				double bitrate_limit  = (conf->vid.amp_check & AMPLIMIT_BITRATE)   ? conf->x264.bitrate - 0.5 * (file_bitrate - conf->vid.amp_limit_bitrate) : conf->x264.bitrate;
				double filesize_limit = (conf->vid.amp_check & AMPLIMIT_FILE_SIZE) ? conf->x264.bitrate - 0.5 * ((filesize - conf->vid.amp_limit_file_size*1024*1024))* 8.0/1000.0 / get_duration(conf, sys_dat, pe, oip) : conf->x264.bitrate;
				conf->x264.bitrate = (int)(0.5 + min(margin_bitrate, min(bitrate_limit, filesize_limit)));
			}
			//必要なら、今回作成した動画を待避
			if (sys_dat->exstg->s_local.amp_keep_old_file)
				amp_move_old_file(muxout, oip->savefile);
		}
	}
	info_amp_result(status, amp_result, filesize, file_bitrate, conf->vid.amp_limit_file_size, conf->vid.amp_limit_bitrate, pe->current_x264_pass - conf->x264.auto_npass, (amp_result == 2) ? conf->aud.bitrate : conf->x264.bitrate);

	if (show_header)
		open_log_window(oip->savefile, pe->current_x264_pass, pe->total_x264_pass);

	return amp_result;
}

int ReadLogExe(PIPE_SET *pipes, const char *exename, LOG_CACHE *log_line_cache) {
	DWORD pipe_read = 0;
	if (!PeekNamedPipe(pipes->stdOut.h_read, NULL, 0, NULL, &pipe_read, NULL))
		return -1;
	if (pipe_read) {
		ReadFile(pipes->stdOut.h_read, pipes->read_buf + pipes->buf_len, sizeof(pipes->read_buf) - pipes->buf_len - 1, &pipe_read, NULL);
		pipes->buf_len += pipe_read;
		pipes->read_buf[pipes->buf_len] = '\0';
		write_log_exe_mes(pipes->read_buf, &pipes->buf_len, exename, log_line_cache);
	}
	return (int)pipe_read;
}

void write_cached_lines(int log_level, const char *exename, LOG_CACHE *log_line_cache) {
	static const char *const LOG_LEVEL_STR[] = { "info", "warning", "error" };
	static const char *MESSAGE_FORMAT = "%s [%s]: %s";
	char *buffer = NULL;
	int buffer_len = 0;
	log_level = clamp(log_level, LOG_INFO, LOG_ERROR);
	const int additional_length = strlen(exename) + strlen(LOG_LEVEL_STR[log_level]) + strlen(MESSAGE_FORMAT) - strlen("%s") * 3 + 1;
	for (int i = 0; i < log_line_cache->idx; i++) {
		const int required_buffer_len = strlen(log_line_cache->lines[i]) + additional_length;
		if (buffer_len < required_buffer_len) {
			if (buffer) free(buffer);
			buffer = (char *)malloc(required_buffer_len * sizeof(buffer[0]));
			buffer_len = required_buffer_len;
		}
		if (buffer) {
			sprintf_s(buffer, buffer_len, MESSAGE_FORMAT, exename, LOG_LEVEL_STR[log_level], log_line_cache->lines[i]);
			write_log_line(log_level, buffer);
		}
	}
	if (buffer) free(buffer);
}
