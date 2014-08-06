//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") 

#include "output.h"
#include "auo.h"
#include "auo_frm.h"
#include "auo_util.h"
#include "auo_error.h"
#include "auo_version.h"
#include "auo_conf.h"
#include "auo_system.h"

#include "auo_video.h"
#include "auo_audio.h"
#include "auo_mux.h"
#include "auo_encode.h"

//---------------------------------------------------------------------
//		関数プロトタイプ宣言
//---------------------------------------------------------------------
static BOOL check_output(const OUTPUT_INFO *oip, const PRM_ENC *pe);
static void set_enc_prm(PRM_ENC *pe, const OUTPUT_INFO *oip);


//---------------------------------------------------------------------
//		出力プラグイン構造体定義
//---------------------------------------------------------------------
OUTPUT_PLUGIN_TABLE output_plugin_table = {
	NULL,                         // フラグ
	AUO_FULL_NAME,                // プラグインの名前
	AUO_EXT_FILTER,               // 出力ファイルのフィルタ
	AUO_VERSION_INFO,             // プラグインの情報
	func_init,                    // DLL開始時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	func_exit,                    // DLL終了時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	func_output,                  // 出力時に呼ばれる関数へのポインタ
	func_config,                  // 出力設定のダイアログを要求された時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	func_config_get,              // 出力設定データを取得する時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
	func_config_set,              // 出力設定データを設定する時に呼ばれる関数へのポインタ (NULLなら呼ばれません)
};


//---------------------------------------------------------------------
//		出力プラグイン構造体のポインタを渡す関数
//---------------------------------------------------------------------
EXTERN_C OUTPUT_PLUGIN_TABLE __declspec(dllexport) * __stdcall GetOutputPluginTable( void )
{
	return &output_plugin_table;
}


//---------------------------------------------------------------------
//		出力プラグイン内部変数
//---------------------------------------------------------------------

static CONF_X264GUIEX conf;
static SYSTEM_DATA sys_dat = { 0 };


//---------------------------------------------------------------------
//		出力プラグイン出力関数
//---------------------------------------------------------------------
//
	//int		flag;			//	フラグ
	//						//	OUTPUT_INFO_FLAG_VIDEO	: 画像データあり
	//						//	OUTPUT_INFO_FLAG_AUDIO	: 音声データあり
	//						//	OUTPUT_INFO_FLAG_BATCH	: バッチ出力中
	//int		w,h;			//	縦横サイズ
	//int		rate,scale;		//	フレームレート
	//int		n;				//	フレーム数
	//int		size;			//	１フレームのバイト数
	//int		audio_rate;		//	音声サンプリングレート
	//int		audio_ch;		//	音声チャンネル数
	//int		audio_n;		//	音声サンプリング数
	//int		audio_size;		//	音声１サンプルのバイト数
	//LPSTR	savefile;		//	セーブファイル名へのポインタ
	//void	*(*func_get_video)( int frame );
	//						//	DIB形式(RGB24bit)の画像データへのポインタを取得します。
	//						//	frame	: フレーム番号
	//						//	戻り値	: データへのポインタ
	//						//			  画像データポインタの内容は次に外部関数を使うかメインに処理を戻すまで有効
	//void	*(*func_get_audio)( int start,int length,int *readed );
	//						//	16bitPCM形式の音声データへのポインタを取得します。
	//						//	start	: 開始サンプル番号
	//						//	length	: 読み込むサンプル数
	//						//	readed	: 読み込まれたサンプル数
	//						//	戻り値	: データへのポインタ
	//						//			  音声データポインタの内容は次に外部関数を使うかメインに処理を戻すまで有効
	//BOOL	(*func_is_abort)( void );
	//						//	中断するか調べます。
	//						//	戻り値	: TRUEなら中断
	//BOOL	(*func_rest_time_disp)( int now,int total );
	//						//	残り時間を表示させます。
	//						//	now		: 処理しているフレーム番号
	//						//	total	: 処理する総フレーム数
	//						//	戻り値	: TRUEなら成功
	//int		(*func_get_flag)( int frame );
	//						//	フラグを取得します。
	//						//	frame	: フレーム番号
	//						//	戻り値	: フラグ
	//						//  OUTPUT_INFO_FRAME_FLAG_KEYFRAME		: キーフレーム推奨
	//						//  OUTPUT_INFO_FRAME_FLAG_COPYFRAME	: コピーフレーム推奨
	//BOOL	(*func_update_preview)( void );
	//						//	プレビュー画面を更新します。
	//						//	最後にfunc_get_videoで読み込まれたフレームが表示されます。
	//						//	戻り値	: TRUEなら成功
	//void	*(*func_get_video_ex)( int frame,DWORD format );
	//						//	DIB形式の画像データを取得します。
	//						//	frame	: フレーム番号
	//						//	format	: 画像フォーマット( NULL = RGB24bit / 'Y''U''Y''2' = YUY2 / 'Y''C''4''8' = PIXEL_YC )
	//						//			  ※PIXEL_YC形式 は YUY2フィルタモードでは使用出来ません。
	//						//	戻り値	: データへのポインタ
	//						//			  画像データポインタの内容は次に外部関数を使うかメインに処理を戻すまで有効

BOOL func_init() 
{
	return TRUE;
}

BOOL func_exit() 
{
	delete_SYSTEM_DATA(&sys_dat);
	return TRUE;
}

BOOL func_output( OUTPUT_INFO *oip ) 
{
	DWORD ret = OUT_RESULT_SUCCESS;
	static const encode_task task[2][2] = { { video_output, audio_output }, { audio_output, video_output } };
	PRM_ENC pe = { 0 };
	const DWORD tm_start_enc = timeGetTime();

	//データの初期化
	init_SYSTEM_DATA(&sys_dat);
	if (!sys_dat.exstg->get_init_success()) return FALSE;

	//ログウィンドウを開く
	open_log_window(oip->savefile, 1, (conf.x264.use_auto_npass) ? conf.x264.auto_npass : 1);
	set_prevent_log_close(TRUE, NULL); //※1 start

	//各種設定を行う
	set_enc_prm(&pe, oip);
	pe.h_p_aviutl = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId()); //※2 start

	//チェックを行い、エンコード可能ならエンコードを開始する
	if (check_output(oip, &pe) && setup_afsvideo(oip, &conf, &pe)) { //※3 start

		for (int i = 0; !ret && i < 2; i++)
			ret |= task[conf.aud.audio_encode_first != FALSE][i](&conf, oip, &pe, &sys_dat);

		if (!ret)
			ret |= mux(&conf, oip, &pe, &sys_dat);

		ret |= move_temporary_files(&conf, &pe, oip->savefile, ret);

		write_log_auo_enc_time("総エンコード時間  ", timeGetTime() - tm_start_enc);

		close_afsvideo(&pe); //※3 end

	} else {
		ret |= OUT_RESULT_ERROR;
	}

	if (ret & OUT_RESULT_ABORT) info_encoding_aborted();

	CloseHandle(pe.h_p_aviutl); //※2 end
	set_prevent_log_close(FALSE, oip->savefile); //※1 end, 自動ログ保存も行われる

	return (ret & OUT_RESULT_ERROR) ? FALSE : TRUE;
}

//---------------------------------------------------------------------
//		出力プラグイン設定関数
//---------------------------------------------------------------------
//以下部分的にwarning C4100を黙らせる
//C4100 : 引数は関数の本体部で 1 度も参照されません。
#pragma warning( push )
#pragma warning( disable: 4100 )
BOOL func_config(HWND hwnd, HINSTANCE dll_hinst)
{
	init_SYSTEM_DATA(&sys_dat);
	if (sys_dat.exstg->get_init_success())
		ShowfrmConfig(&conf, &sys_dat);
	return TRUE;
}
#pragma warning( pop )

int func_config_get( void *data, int size )
{
	if (data && size == sizeof(CONF_X264GUIEX))
		memcpy(data, &conf, sizeof(conf));
	return sizeof(conf);
}

int func_config_set( void *data,int size )
{
	init_SYSTEM_DATA(&sys_dat);
	init_CONF_X264GUIEX(&conf, FALSE);
	if (size != sizeof(CONF_X264GUIEX) || 
		((CONF_X264GUIEX *)data)->size_all != CONF_INITIALIZED)
		return NULL;
	memcpy(&conf, data, size);
	return size;
}


//---------------------------------------------------------------------
//		x264guiExのその他の関数
//---------------------------------------------------------------------
void init_SYSTEM_DATA(SYSTEM_DATA *_sys_dat) {
	if (_sys_dat->init)
		return;
	get_auo_path(_sys_dat->auo_path, sizeof(_sys_dat->auo_path));
	get_aviutl_dir(_sys_dat->aviutl_dir, sizeof(_sys_dat->aviutl_dir));
	_sys_dat->exstg = new guiEx_settings();
	set_ex_stg_ptr(_sys_dat->exstg);
	_sys_dat->init = TRUE;
}
void delete_SYSTEM_DATA(SYSTEM_DATA *_sys_dat) {
	if (_sys_dat->init) {
		delete _sys_dat->exstg;
		_sys_dat->exstg = NULL;
		set_ex_stg_ptr(_sys_dat->exstg);
	}
	_sys_dat->init = FALSE;
}
void init_CONF_X264GUIEX(CONF_X264GUIEX *conf, BOOL use_10bit) {
	ZeroMemory(conf, sizeof(CONF_X264GUIEX));
	get_default_conf_x264(&conf->x264, use_10bit);
	conf->size_all = CONF_INITIALIZED;
}
void write_log_auo_line_fmt(int log_type_index, const char *format, ... ) {
	va_list args;
	int len;
	char *buffer;
	va_start(args, format);
	len = _vscprintf(format, args) // _vscprintf doesn't count
		                      + 1; // terminating '\0'
	buffer = (char *)malloc(len * sizeof(char));
	vsprintf_s(buffer, len, format, args);
	write_log_auo_line(log_type_index, buffer);
	free(buffer);
}
//エンコード時間の表示
void write_log_auo_enc_time(const char *mes, DWORD time) {
	write_log_auo_line_fmt(LOG_INFO, "%s : %d時間%2d分%2d.%1d秒", 
		mes, 
		time / (60*60*1000),
		(time % (60*60*1000)) / (60*1000), 
		(time % (60*1000)) / 1000,
		((time % 1000) + 50) / 100);
}

static BOOL check_muxer_exist(MUXER_SETTINGS *muxer_stg) {
	if (PathFileExists(muxer_stg->fullpath)) 
		return TRUE;
	error_no_exe_file(muxer_stg->filename, muxer_stg->fullpath);
	return FALSE;
}

static BOOL check_output(const OUTPUT_INFO *oip, const PRM_ENC *pe) {
	BOOL check = TRUE;
	//ファイル名長さ
	if (strlen(oip->savefile) > (MAX_PATH_LEN - MAX_APPENDIX_LEN - 1)) {
		error_filename_too_long();
		check = FALSE;
	}

	//解像度
	int w_mul = 1, h_mul = 1;
	switch (conf.x264.output_csp) {
		case OUT_CSP_YUV444:
		case OUT_CSP_RGB:
			w_mul = 1, h_mul = 1;
			break;
		case OUT_CSP_YUV422:
			w_mul = 2, h_mul = 1;
			break;
		case OUT_CSP_YUV420:
		default:
			w_mul = 2; h_mul = 2;
			break;
	}
	if (conf.x264.interlaced) h_mul *= 2;
	if (oip->w % w_mul) {
		error_invalid_resolution(TRUE,  w_mul, oip->w, oip->h);
		check = FALSE;
	}
	if (oip->h % h_mul) {
		error_invalid_resolution(FALSE, h_mul, oip->w, oip->h);
		check = FALSE;
	}

	//出力するもの
	if (pe->video_out_type == VIDEO_OUTPUT_DISABLED && !(oip->flag & OUTPUT_INFO_FLAG_AUDIO)) {
		error_nothing_to_output();
		check = FALSE;
	}

	if (conf.oth.out_audio_only)
		write_log_auo_line(LOG_INFO, "音声のみ出力を行います。");

	//必要な実行ファイル
	//x264
	if (!conf.oth.disable_guicmd) {
		char *x264fullpath = (conf.x264.use_10bit_depth) ? sys_dat.exstg->s_x264.fullpath_10bit : sys_dat.exstg->s_x264.fullpath;
		if (pe->video_out_type != VIDEO_OUTPUT_DISABLED && !PathFileExists(x264fullpath)) {
			error_no_exe_file("x264.exe", x264fullpath);
			check = FALSE;
		}
	}

	//音声エンコーダ
	if ((oip->flag & OUTPUT_INFO_FLAG_AUDIO) && !PathFileExists(sys_dat.exstg->s_aud[conf.aud.encoder].fullpath)) {
		error_no_exe_file(sys_dat.exstg->s_aud[conf.aud.encoder].filename, sys_dat.exstg->s_aud[conf.aud.encoder].fullpath);
		check = FALSE;
	}

	//muxer
	switch (pe->muxer_to_be_used) {
		case MUXER_TC2MP4:
			check &= check_muxer_exist(&sys_dat.exstg->s_mux[MUXER_MP4]); //tc2mp4使用時は追加でmp4boxも必要
			//下へフォールスルー
		case MUXER_MP4:
		case MUXER_MKV:
			check &= check_muxer_exist(&sys_dat.exstg->s_mux[pe->muxer_to_be_used]);
			break;
		default:
			break;
	}

	return check;
}

void open_log_window(const char *savefile, int current_pass, int total_pass) {
	char mes[MAX_PATH_LEN + 256];
	char *newLine = (get_current_log_len(current_pass)) ? "\r\n\r\n" : ""; //必要なら行送り
	static const char *SEPARATOR = "--------------------------------------------------------------------------------------------------";
	if (total_pass < 2)
		sprintf_s(mes, sizeof(mes), "%s%s\r\n[%s]\r\n%s", newLine, SEPARATOR, savefile, SEPARATOR);
	else
		sprintf_s(mes, sizeof(mes), "%s%s\r\n[%s] (%d / %d pass)\r\n%s", newLine, SEPARATOR, savefile, current_pass, total_pass, SEPARATOR);
	
	show_log_window();
	write_log_line(LOG_INFO, mes);
}

static int check_video_ouput(const OUTPUT_INFO *oip) {
	if ((oip->flag & OUTPUT_INFO_FLAG_VIDEO) && !conf.oth.out_audio_only)
		return (check_ext(oip->savefile, ".mp4")) ? VIDEO_OUTPUT_MP4 : ((check_ext(oip->savefile, ".mkv")) ? VIDEO_OUTPUT_MKV : VIDEO_OUTPUT_RAW);
	return VIDEO_OUTPUT_DISABLED;
}

static int check_muxer_to_be_used(int video_output_type, BOOL audio_output) {
	//if (conf.vid.afs)
	//	conf.mux.disable_mp4ext = conf.mux.disable_mkvext = FALSE; //afsなら外部muxerを強制する

	//音声なし、afsなしならmuxしない
	if (!audio_output && !conf.vid.afs)
		return MUXER_DISABLED;

	if (video_output_type == VIDEO_OUTPUT_MP4 && !conf.mux.disable_mp4ext)
		return (conf.vid.afs) ? MUXER_TC2MP4 : MUXER_MP4;
	else if (video_output_type == VIDEO_OUTPUT_MKV && !conf.mux.disable_mkvext)
		return MUXER_MKV;
	else
		return MUXER_DISABLED;
}

static void set_tmpdir(PRM_ENC *pe, int tmp_dir_index, const char *savefile) {
	if (tmp_dir_index < TMP_DIR_OUTPUT || TMP_DIR_CUSTOM < tmp_dir_index)
		tmp_dir_index = TMP_DIR_OUTPUT;

	if (tmp_dir_index == TMP_DIR_SYSTEM) {
		//システムの一時フォルダを取得
		if (GetTempPath(sizeof(pe->temp_filename), pe->temp_filename) != NULL) {
			PathRemoveBackslash(pe->temp_filename);
			write_log_auo_line_fmt(LOG_INFO, "一時フォルダ : %s", pe->temp_filename);
		} else {
			warning_failed_getting_temp_path();
			tmp_dir_index = TMP_DIR_OUTPUT;
		}
	}
	if (tmp_dir_index == TMP_DIR_CUSTOM) {
		//指定されたフォルダ
		if (DirectoryExistsOrCreate(sys_dat.exstg->s_local.custom_tmp_dir)) {
			strcpy_s(pe->temp_filename, sizeof(pe->temp_filename), sys_dat.exstg->s_local.custom_tmp_dir);
			PathRemoveBackslash(pe->temp_filename);
			write_log_auo_line_fmt(LOG_INFO, "一時フォルダ : %s", pe->temp_filename);
		} else {
			warning_no_temp_root(sys_dat.exstg->s_local.custom_tmp_dir);
			tmp_dir_index = TMP_DIR_OUTPUT;
		}
	}
	if (tmp_dir_index == TMP_DIR_OUTPUT) {
		//出力フォルダと同じ("\"なし)
		strcpy_s(pe->temp_filename, sizeof(pe->temp_filename), savefile);
		PathRemoveFileSpec(pe->temp_filename);
	}
}

static void set_enc_prm(PRM_ENC *pe, const OUTPUT_INFO *oip) {
	//初期化
	ZeroMemory(pe, sizeof(PRM_ENC));
	//設定更新
	sys_dat.exstg->load_encode_stg();
	sys_dat.exstg->load_append();
	sys_dat.exstg->load_fn_replace();
	
	pe->video_out_type = check_video_ouput(oip);
	pe->muxer_to_be_used = check_muxer_to_be_used(pe->video_out_type, (oip->flag & OUTPUT_INFO_FLAG_AUDIO) != 0);
	pe->total_x264_pass = (conf.x264.use_auto_npass && !conf.oth.disable_guicmd) ? conf.x264.auto_npass : 1;
	pe->current_x264_pass = 1;
	pe->drop_count = 0;
	memcpy(&pe->append, &sys_dat.exstg->s_append, sizeof(FILE_APPENDIX));

	//一時フォルダ決定
	char filename_replace[MAX_PATH_LEN];

	//一時フォルダの決定
	set_tmpdir(pe, conf.oth.temp_dir, oip->savefile);

	//音声一時フォルダの決定
	char *cus_aud_tdir = pe->temp_filename;
	if (conf.aud.aud_temp_dir)
		if (DirectoryExistsOrCreate(sys_dat.exstg->s_local.custom_audio_tmp_dir)) {
			cus_aud_tdir = sys_dat.exstg->s_local.custom_audio_tmp_dir;
			write_log_auo_line_fmt(LOG_INFO, "音声一時フォルダ : %s", cus_aud_tdir);
		} else
			warning_no_aud_temp_root(sys_dat.exstg->s_local.custom_audio_tmp_dir);
	strcpy_s(pe->aud_temp_dir, sizeof(pe->aud_temp_dir), cus_aud_tdir);

	//ファイル名置換を行い、一時ファイル名を作成
	strcpy_s(filename_replace, sizeof(filename_replace), PathFindFileName(oip->savefile));
	sys_dat.exstg->apply_fn_replace(filename_replace, sizeof(filename_replace));
	PathCombineLong(pe->temp_filename, sizeof(pe->temp_filename), pe->temp_filename, filename_replace);
}