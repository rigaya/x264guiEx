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

	sprintf_s(mes, _countof(mes), "%s でmuxを行います。音声mux:%s, tcmux:%s, 拡張モード:%s", 
		mux_stg_name,
		ON_OFF_INFO[audmux != 0],
		ON_OFF_INFO[tcmux != 0],
		muxer_mode_name);
	write_log_auo_line_fmt(LOG_INFO, mes);

	sprintf_s(mes, _countof(mes), "%s で mux中...", mux_stg_name);
	set_window_title(mes, PROGRESSBAR_MARQUEE);
}

//muxの空き容量などを計算し、行えるかを確認する
static DWORD check_mux_disk_space(const char *muxer_fullpath, const char *mux_tmpdir, const CONF_X264GUIEX *conf, const PRM_ENC *pe, UINT64 expected_filesize) {
	//mp4系かどうか
	if (!(pe->muxer_to_be_used == MUXER_MP4 || pe->muxer_to_be_used == MUXER_TC2MP4))
		return AUO_RESULT_SUCCESS;

	DWORD ret = AUO_RESULT_SUCCESS;
	UINT64 required_space = (UINT64)(expected_filesize * 1.01); //ちょい多め
	//出力先ドライブ
	char vid_root[MAX_PATH_LEN];
	strcpy_s(vid_root, _countof(vid_root), pe->temp_filename);
	PathStripToRoot(vid_root);
	//一時フォルダの指定について検証
	if (conf->mux.mp4_temp_dir) {
		ULARGE_INTEGER temp_drive_avail_space = { 0 };
		BOOL tmp_same_drive_as_out = FALSE;
		//指定されたドライブが存在するかどうか
		char temp_root[MAX_PATH_LEN];
		if (!PathGetRoot(mux_tmpdir, temp_root, _countof(temp_root)) ||
			!PathIsDirectory(temp_root) ||
			!DirectoryExistsOrCreate(mux_tmpdir)) {
			ret = AUO_RESULT_WARNING; warning_no_mux_tmp_root(temp_root);
		//空き容量を取得できていなければ、チェックを終了する
		} else if (expected_filesize <= 0) {
			return AUO_RESULT_SUCCESS;
		//ドライブの空き容量取得
		} else if (!GetDiskFreeSpaceEx(temp_root, &temp_drive_avail_space, NULL, NULL)) {
			ret = AUO_RESULT_WARNING; warning_failed_mux_tmp_drive_space();
		//一時フォルダと出力先が同じフォルダかどうかで、一時フォルダの必要とされる空き領域が変わる
		} else {
			tmp_same_drive_as_out = (_stricmp(vid_root, temp_root) == NULL);
			if ((UINT64)temp_drive_avail_space.QuadPart < required_space * (1 + tmp_same_drive_as_out)) {
				ret = AUO_RESULT_WARNING; warning_mux_tmp_not_enough_space();
			}
		}
		//一時フォルダと出力先が同じフォルダならさらなる検証の必要はない
		if (tmp_same_drive_as_out && ret == AUO_RESULT_SUCCESS)
			return ret;
	}
	//空き容量を取得できていなければ、チェックを終了する
	if (expected_filesize <= 0)
		return AUO_RESULT_SUCCESS;
	//一時ファイルが指定されていないときにはカレントディレクトリのあるドライブ(muxerのあるドライブ)に一時ファイルが作られる
	//その一時フォルダのドライブについて検証
	if (!conf->mux.mp4_temp_dir || ret == AUO_RESULT_WARNING) {
		char muxer_root[MAX_PATH_LEN];
		//ドライブの空き容量取得
		ULARGE_INTEGER muxer_drive_avail_space = { 0 };
		if (!PathGetRoot(mux_stg->fullpath, muxer_root, _countof(muxer_root)) ||
			!GetDiskFreeSpaceEx(muxer_root, &muxer_drive_avail_space, NULL, NULL)) {
			error_failed_muxer_drive_space(); return AUO_RESULT_ERROR;
		}
		//一時フォルダと出力先が同じフォルダかどうかで、一時フォルダの必要とされる空き領域が変わる
		BOOL muxer_same_drive_as_out = (_stricmp(vid_root, muxer_root) == NULL);
		if ((UINT64)muxer_drive_avail_space.QuadPart < required_space * (1 + muxer_same_drive_as_out)) {
			error_muxer_drive_not_enough_space(); return AUO_RESULT_ERROR;
		}
		//一時フォルダと出力先が同じフォルダならさらなる検証の必要はない
		if (muxer_same_drive_as_out && ret == AUO_RESULT_SUCCESS)
			return ret;
	}
	//出力先のドライブの空き容量
	//ドライブの空き容量取得
	ULARGE_INTEGER out_drive_avail_space = { 0 };
	if (!GetDiskFreeSpaceEx(vid_root, &out_drive_avail_space, NULL, NULL)) {
		error_failed_out_drive_space(); return AUO_RESULT_ERROR;
	}
	if ((UINT64)out_drive_avail_space.QuadPart < required_space) {
		error_out_drive_not_enough_space(); return AUO_RESULT_ERROR;
	}
	return ret;
}

//muxする動画ファイルと音声ファイルからmux後ファイルの推定サイズを取得する
static AUO_RESULT get_expected_filesize(const PRM_ENC *pe, BOOL enable_aud_mux, UINT64 *_expected_filesize) {
	*_expected_filesize = 0;
	//動画ファイルのサイズ
	UINT64 vid_size = 0;
	if (!PathFileExists(pe->temp_filename)) {
		error_no_vid_file(); return AUO_RESULT_ERROR;
	}
	if (!GetFileSizeUInt64(pe->temp_filename, &vid_size)) {
		warning_failed_get_vid_size(); return AUO_RESULT_WARNING;
	}
	//音声ファイルのサイズ
	if (enable_aud_mux) {
		UINT64 aud_size = 0;
		char audfile[MAX_PATH_LEN] = { 0 };
		get_aud_filename(audfile, _countof(audfile), pe);
		if (!PathFileExists(audfile)) {
			error_no_vid_file(); return AUO_RESULT_ERROR;
		}
		if (!GetFileSizeUInt64(audfile, &aud_size)) {
			warning_failed_get_aud_size(); return AUO_RESULT_WARNING;
		}
		*_expected_filesize += aud_size;
	}
	*_expected_filesize += vid_size;
	return AUO_RESULT_SUCCESS;
}

//mux後ファイルが存在する他とファイルサイズをチェック
//大丈夫そうならTRUEを返す
static AUO_RESULT check_muxout_filesize(const char *muxout, UINT64 expected_filesize) {
	const double FILE_SIZE_THRESHOLD_MULTI = 0.95;
	UINT64 muxout_filesize = 0;
	if (!PathFileExists(muxout)) {
		error_check_muxout_exist();
		return AUO_RESULT_ERROR;
	}
	//推定ファイルサイズの取得に失敗していたら終了
	if (expected_filesize <= 0)
		return AUO_RESULT_WARNING;
	if (GetFileSizeUInt64(muxout, &muxout_filesize)) {
		//ファイルサイズの取得に成功したら、予想サイズとの比較を行う
		if (((double)muxout_filesize) <= ((double)expected_filesize * FILE_SIZE_THRESHOLD_MULTI * (1.0 - exp(-1.0 * (double)expected_filesize / (128.0 * 1024.0))))) {
			error_check_muxout_too_small((int)(expected_filesize / 1024), (int)(muxout_filesize / 1024));
			return AUO_RESULT_ERROR;
		}
		return AUO_RESULT_SUCCESS;
	}
	warning_failed_check_muxout_filesize();
	return AUO_RESULT_WARNING;
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

	sprintf_s(buf, _countof(buf), "%d", conf_tmp.sar.x);
	replace(cmd, nSize, "%{par_x}", buf);
	sprintf_s(buf, _countof(buf), "%d", conf_tmp.sar.y);
	replace(cmd, nSize, "%{par_y}", buf);
}

//不必要なチャプターコマンドを削除する
static void del_chap_cmd(char *cmd, BOOL apple_type_only) {
	char *ptr;
	if (!apple_type_only)
		del_arg(cmd, "-chap", TRUE);
	//%{chap_apple}直前の-add以下を削除する
	if ((ptr = strstr(cmd, "%{chap_apple}")) == NULL)
		return;
	if ((ptr = strnrstr(cmd, "-add", ptr - cmd)) == NULL)
		return;
	del_arg(ptr, "-add", TRUE);
}

static AUO_RESULT build_mux_cmd(char *cmd, size_t nSize, const CONF_X264GUIEX *conf, const OUTPUT_INFO *oip, const PRM_ENC *pe, 
						  const SYSTEM_DATA *sys_dat, const MUXER_SETTINGS *mux_stg, UINT64 expected_filesize) {
	strcpy_s(cmd, nSize, mux_stg->base_cmd);
	BOOL enable_aud_mux = (oip->flag & OUTPUT_INFO_FLAG_AUDIO) != 0;
	BOOL enable_tc_mux = (conf->vid.afs) != 0;
	const MUXER_CMD_EX *muxer_mode = &mux_stg->ex_cmd[(pe->muxer_to_be_used == MUXER_MKV) ? conf->mux.mkv_mode : conf->mux.mp4_mode];
	char *audstr = (enable_aud_mux) ? mux_stg->aud_cmd : "";
	char *tcstr  = (enable_tc_mux) ? mux_stg->tc_cmd : "";
	char *exstr  = (conf->mux.apple_mode && str_has_char(muxer_mode->cmd_apple)) ? muxer_mode->cmd_apple : muxer_mode->cmd;
	//音声用コマンド
	replace(cmd, nSize, "%{au_cmd}",  audstr);
	//タイムコード用
	replace(cmd, nSize, "%{tc_cmd}",  tcstr);
	//一時ファイル(空き容量等)のチェックを行う
	AUO_RESULT mux_check = check_mux_disk_space(mux_stg, sys_dat->exstg->s_local.custom_mp4box_tmp_dir, conf, pe, expected_filesize);
	switch (mux_check) {
		case AUO_RESULT_SUCCESS:
			if (conf->mux.mp4_temp_dir) {
				//一時フォルダ指定を行う
				replace(cmd, nSize, "%{tmp_cmd}", mux_stg->tmp_cmd);
				char m_tmp_dir[MAX_PATH_LEN];
				strcpy_s(m_tmp_dir, _countof(m_tmp_dir), sys_dat->exstg->s_local.custom_mp4box_tmp_dir);
				PathForceRemoveBackSlash(m_tmp_dir);
				replace(cmd, nSize, "%{m_tmpdir}", m_tmp_dir);
				break;
			}
			//下へフォールスルー(一時フォルダ指定を行わない)
		case AUO_RESULT_WARNING: //一時フォルダ指定を行えない
			replace(cmd, nSize, "%{tmp_cmd}", "");
			break;
		case AUO_RESULT_ERROR://一時ファイル関連のチェックでエラー
		default:
			return AUO_RESULT_ERROR;
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
		del_chap_cmd(cmd, FALSE);
	} else {
		replace(cmd, nSize, "%{chapter}", chap_file);
		//mp4系ならapple形式チャプター追加も考慮する
		if (pe->muxer_to_be_used != MUXER_MKV) {
			//apple形式チャプターファイルへの置換が行われたら、apple形式チャプターファイルを作成する
			if (strstr(cmd, "%{chap_apple}")) {
				AuoChapStatus sts = convert_chapter(chap_apple, chap_file, CODE_PAGE_UNSET, get_duration(conf, sys_dat, pe, oip));
				if (sts != AUO_CHAP_ERR_NONE) {
					warning_mux_chapter(sts);
					del_chap_cmd(cmd, TRUE);
				} else {
					replace(cmd, nSize, "%{chap_apple}", chap_apple);
				}
			}
		}
	}
	//tc2mp4ならmp4boxの場所を指定
	if (pe->muxer_to_be_used == MUXER_TC2MP4) {
		//ここで使用するmp4boxのパスはtc2mp4modからの相対パスになってしまうので、
		//絶対パスに変換しておく
		char mp4box_fullpath[MAX_PATH_LEN];
		if (PathIsRelative(sys_dat->exstg->s_mux[MUXER_MP4].fullpath) == FALSE)
			strcpy_s(mp4box_fullpath, sizeof(mp4box_fullpath), sys_dat->exstg->s_mux[MUXER_MP4].fullpath);
		else
			_fullpath(mp4box_fullpath, sys_dat->exstg->s_mux[MUXER_MP4].fullpath, sizeof(mp4box_fullpath));
		sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " -L \"%s\"", mp4box_fullpath);
	}
	//アスペクト比
	replace_par(cmd, nSize, conf, oip);
	//その他の置換を実行
	cmd_replace(cmd, nSize, pe, sys_dat, oip->savefile);
	//情報表示
	show_mux_info(mux_stg->dispname, enable_aud_mux, enable_tc_mux, muxer_mode->name);
	return AUO_RESULT_SUCCESS;
}

static void change_mux_vid_filename(const char *muxout, const PRM_ENC *pe) {
	char vidfile_append[MAX_APPENDIX_LEN];
	strcpy_s(vidfile_append, _countof(vidfile_append), "_video");
	strcat_s(vidfile_append, _countof(vidfile_append), PathFindExtension(pe->temp_filename));
	char vidfile_newname[MAX_PATH_LEN];
	apply_appendix(vidfile_newname, _countof(vidfile_newname), pe->temp_filename, vidfile_append);
	rename(pe->temp_filename, vidfile_newname);
	rename(muxout, pe->temp_filename);
}

AUO_RESULT mux(const CONF_X264GUIEX *conf, const OUTPUT_INFO *oip, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
	AUO_RESULT ret = AUO_RESULT_SUCCESS;
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
	UINT64 expected_filesize = 0;
	char muxcmd[MAX_CMD_LEN]  = { 0 };
	char muxargs[MAX_CMD_LEN] = { 0 };
	char muxdir[MAX_PATH_LEN] = { 0 };
	char muxout[MAX_PATH_LEN] = { 0 };
	DWORD mux_priority = GetExePriority(conf->mux.priority, pe->h_p_aviutl);
	get_muxout_filename(muxout, _countof(muxout), pe->temp_filename);

	PROCESS_INFORMATION pi_mux;
	int rp_ret;

	PathGetDirectory(muxdir, _countof(muxdir), mux_stg->fullpath);

	//mux終了後の予想サイズを取得
	ret |= get_expected_filesize(pe, (oip->flag & OUTPUT_INFO_FLAG_AUDIO) != 0, &expected_filesize);
	if (ret & AUO_RESULT_ERROR)
		return AUO_RESULT_ERROR;

	//コマンドライン生成・情報表示
	ret |= build_mux_cmd(muxcmd, _countof(muxcmd), conf, oip, pe, sys_dat, mux_stg, expected_filesize);
	if (ret & AUO_RESULT_ERROR)
		return AUO_RESULT_ERROR; //エラーメッセージはbuild_mux_cmd関数内で吐かれる
	sprintf_s(muxargs, _countof(muxargs), "\"%s\" %s", mux_stg->fullpath, muxcmd);

	if ((rp_ret = RunProcess(muxargs, muxdir, &pi_mux, NULL, mux_priority, FALSE, conf->mux.minimized)) != RP_SUCCESS) {
		//エラー
		ret |= AUO_RESULT_ERROR; error_run_process(mux_stg->dispname, rp_ret);
	} else {
		while (WaitForSingleObject(pi_mux.hProcess, LOG_UPDATE_INTERVAL) == WAIT_TIMEOUT)
			log_process_events();

		ret |= check_muxout_filesize(muxout, expected_filesize);
		if (ret == AUO_RESULT_SUCCESS) {
			remove(pe->temp_filename);
			rename(muxout, pe->temp_filename);
		} else if (ret & AUO_RESULT_ERROR) {
			error_mux_failed(mux_stg->dispname, muxargs);
			if (PathFileExists(muxout))
				remove(muxout);
		} else {
			//AUO_RESULT_WARNING
			change_mux_vid_filename(muxout, pe);
		}
		CloseHandle(pi_mux.hProcess);
		CloseHandle(pi_mux.hThread);
	}

	set_window_title(AUO_FULL_NAME, PROGRESSBAR_DISABLED);

	return ret;
}