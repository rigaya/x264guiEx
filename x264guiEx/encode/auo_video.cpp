//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#pragma comment(lib, "user32.lib") //WaitforInputIdle
#include <stdlib.h>
#include <stdio.h>
#include <Process.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") 
#include <limits.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <vector>

#include "output.h"
#include "convert.h"
#include "vphelp_client.h"

#pragma warning( push )
#pragma warning( disable: 4127 )
#include "afs_client.h"
#pragma warning( pop )

#include "auo.h"
#include "auo_frm.h"
#include "auo_pipe.h"
#include "auo_error.h"
#include "auo_conf.h"
#include "auo_util.h"
#include "auo_system.h"
#include "auo_version.h"

#include "auo_encode.h"
#include "auo_video.h"

const int DROP_FRAME_FLAG = INT_MAX;

static const char * specify_x264_input_csp(int output_csp) {
	return specify_csp[output_csp];
}

int get_aviutl_color_format(int use_10bit, int output_csp) {
	//Aviutlからの入力に使用するフォーマット
	switch (output_csp) {
		case OUT_CSP_YUV444:
			return CF_YC48;
		case OUT_CSP_RGB:
			return CF_RGB;
		case OUT_CSP_YUV420:
		case OUT_CSP_YUV422:
		default:
			return (use_10bit) ? CF_YC48 : CF_YUY2;
	}
}

static int calc_input_frame_size(int width, int height, int color_format) {
	width = (color_format == CF_RGB) ? (width+3) & ~3 : (width+1) & ~1;
	return width * height * COLORFORMATS[color_format].size;
}

BOOL setup_afsvideo(const OUTPUT_INFO *oip, CONF_X264GUIEX *conf, PRM_ENC *pe, BOOL auto_afs_disable) {
	//すでに初期化してある
	if (pe->afs_init || pe->video_out_type == VIDEO_OUTPUT_DISABLED)
		return TRUE;

	int color_format = get_aviutl_color_format(conf->x264.use_10bit_depth, conf->x264.output_csp);
	int frame_size = calc_input_frame_size(oip->w, oip->h, color_format);
	//Aviutl(自動フィールドシフト)からの映像入力
	if (afs_vbuf_setup((OUTPUT_INFO *)oip, conf->vid.afs, frame_size, COLORFORMATS[color_format].FOURCC)) {
		pe->afs_init = TRUE;
		return TRUE;
	} else if (conf->vid.afs && auto_afs_disable) {
		afs_vbuf_release(); //一度解放
		//afs無効で再初期化
		if (afs_vbuf_setup((OUTPUT_INFO *)oip, FALSE, frame_size, COLORFORMATS[color_format].FOURCC)) {
			warning_auto_afs_disable();
			conf->vid.afs = FALSE;
			//再度使用するmuxerをチェックする
			pe->muxer_to_be_used = check_muxer_to_be_used(conf, pe->video_out_type, (oip->flag & OUTPUT_INFO_FLAG_AUDIO) != 0);
			pe->afs_init = TRUE;
			return TRUE;
		}
	}
	//エラー
	error_afs_setup(conf->vid.afs, auto_afs_disable);
	return FALSE;
}

void close_afsvideo(PRM_ENC *pe) {
	if (!pe->afs_init || pe->video_out_type == VIDEO_OUTPUT_DISABLED)
		return;

	afs_vbuf_release();

	pe->afs_init = FALSE;
}

static DWORD check_cmdex(CONF_X264GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
	DWORD ret = OUT_RESULT_SUCCESS;
	int color_format = get_aviutl_color_format(conf->x264.use_10bit_depth, conf->x264.output_csp); //現在の色形式を保存
	if (conf->oth.disable_guicmd) 
		get_default_conf_x264(&conf->x264, FALSE); //CLIモード時はとりあえず、デフォルトを呼んでおく
	//cmdexを適用
	set_cmd_to_conf(conf->vid.cmdex, &conf->x264);

	if (color_format != get_aviutl_color_format(conf->x264.use_10bit_depth, conf->x264.output_csp)) {
		//cmdexで入力色形式が変更になる場合、再初期化
		close_afsvideo(pe);
		if (!setup_afsvideo(oip, conf, pe, sys_dat->exstg->s_local.auto_afs_disable)) {
			ret |= OUT_RESULT_ERROR; //Aviutl(afs)からのフレーム読み込みに失敗
		}
	}
	return ret;
}

static DWORD tcfile_out(int *jitter, int frame_n, double fps, BOOL afs, const PRM_ENC *pe) {
	DWORD ret = OUT_RESULT_SUCCESS;
	char auotcfile[MAX_PATH_LEN];
	FILE *tcfile = NULL;

	if (afs)
		fps *= 4; //afsなら4倍精度
	double tm_multi = 1000.0 / fps;

	//ファイル名作成
	apply_appendix(auotcfile, sizeof(auotcfile), pe->temp_filename, pe->append.tc);

	if (fopen_s(&tcfile, auotcfile, "wb") == NULL) {
		fprintf(tcfile, "# timecode format v2\r\n");
		if (afs) {
			for (int i = 0; i < frame_n; i++)
				if (jitter[i] != DROP_FRAME_FLAG)
					fprintf(tcfile, "%.6lf\r\n", (i * 4 + jitter[i]) * tm_multi);
		} else {
			for (int i = 0; i < frame_n; i++)
				fprintf(tcfile, "%.6lf\r\n", i * tm_multi);
		}
		fclose(tcfile);
	} else {
		ret |= OUT_RESULT_ERROR; warning_auo_tcfile_failed();
	}
	return ret;
}

//Aviutlのキーフレーム検出からqpfile作成
static DWORD check_keyframe_flag(const OUTPUT_INFO *oip, const PRM_ENC *pe) {
	DWORD ret = OUT_RESULT_SUCCESS;
	char auoqpfile[MAX_PATH_LEN] = { 0 };
	DWORD tm = 0, tm_prev = 0;
	const char *mes_head = "Aviutl キーフレーム検出中…";
	std::vector<int> keyframe_list;

	//ファイル名作成
	apply_appendix(auoqpfile, sizeof(auoqpfile), pe->temp_filename, pe->append.qp);
	if (PathFileExists(auoqpfile))
		remove(auoqpfile);

	set_window_title(mes_head, PROGRESSBAR_CONTINUOUS);

	//検出ループ
	for (int i = 0; i < oip->n; i++) {
		//中断
		if (oip->func_is_abort()) {
			ret |= OUT_RESULT_ABORT; write_log_auo_line(LOG_INFO, "Aviutl キーフレーム検出を中断しました。");
			break;
		}
		//フラグ検出
		if (oip->func_get_flag(i) & OUTPUT_INFO_FRAME_FLAG_KEYFRAME)
			keyframe_list.push_back(i);
		//進捗表示
		if ((tm = timeGetTime()) - tm_prev > LOG_UPDATE_INTERVAL * 5) {
			set_log_progress(i / (double)oip->n);
			tm_prev = tm;
		}
	}

	//キーフレームが検出されればファイル出力
	if (keyframe_list.size()) {
		write_log_auo_line_fmt(LOG_INFO, "Aviutlから %d箇所 キーフレーム設定を検出しました。", keyframe_list.size());
		FILE *qpfile = NULL;
		if (fopen_s(&qpfile, auoqpfile, "wb") != NULL) {
			ret |= OUT_RESULT_ERROR; warning_auto_qpfile_failed();
		} else {
			foreach(std::vector<int>, it_keyframe, &keyframe_list)
				fprintf_s(qpfile, "%d I\r\n", *it_keyframe);
			fclose(qpfile);
		}
	} else {
		write_log_auo_line(LOG_INFO, "キーフレーム探索を行いましたが、キーフレーム設定を検出できませんでした。");
	}
	set_window_title(mes_head, PROGRESSBAR_DISABLED);
	return ret;
}

//auo_pipe.cppのread_from_pipeのx264用特別版
static int ReadLogX264(PIPE_SET *pipes, int total_drop, int current_frames) {
	DWORD pipe_read = 0;
	if (!PeekNamedPipe(pipes->stdErr.h_read, NULL, 0, NULL, &pipe_read, NULL))
		return -1;
	if (pipe_read) {
		ReadFile(pipes->stdErr.h_read, pipes->read_buf + pipes->buf_len, sizeof(pipes->read_buf) - pipes->buf_len - 1, &pipe_read, NULL);
		pipes->buf_len += pipe_read;
		write_log_x264_mes(pipes->read_buf, &pipes->buf_len, total_drop, current_frames);
	} else {
		log_process_events();
	}
	return pipe_read;
}

static void convert_func_info(const CONF_X264 *cx) {
	const char *input_fmt = NULL;
	const char *output_fmt = NULL;
	static const char * const USE_SIMD[]   = { "", ", using" };
	static const char * const USE_SSE2[]   = { "", " SSE2" };
	static const char * const USE_SSSE3[]  = { "", " SSSE3" };
	static const char * const USE_SSE4_1[] = { "", " SSE4.1" };
	BOOL use_sse2 = FALSE;
	BOOL use_ssse3 = FALSE;
	BOOL use_sse4_1 = FALSE;
	const char *ip_mode = "";
	switch (cx->output_csp) {
		case OUT_CSP_YUV444:
			input_fmt = "YC48";
			output_fmt = "YUV444";
			use_sse2 = check_sse2();
			break;
		case OUT_CSP_RGB:
			if ((use_ssse3 = (check_ssse3())) == FALSE)
				use_sse2 = check_sse2();
			write_log_auo_line_fmt(LOG_INFO, "Sorting RGB%s%s%s", 
				USE_SIMD[(use_sse2 | use_ssse3) != 0], USE_SSE2[use_sse2 != 0], USE_SSSE3[use_ssse3 != 0]);
			return;
		case OUT_CSP_YUV420:
			ip_mode = (cx->interlaced) ? "i" : "p";
			//下へフォールスルー
		case OUT_CSP_YUV422:
		default:
			input_fmt = (cx->use_10bit_depth) ? "YC48" : "YUY2";
			output_fmt = specify_x264_input_csp(cx->output_csp);
			use_sse2 = check_sse2();
			use_ssse3 = (cx->use_10bit_depth && check_ssse3());
			use_sse4_1 = (cx->use_10bit_depth && check_sse4_1());
			break;
	}
	const char *bit_depth  = (cx->use_10bit_depth) ? "(10bit)" : "";
	const char *ycc        = (cx->use_10bit_depth && cx->fullrange) ? " fullrange mode" : "";
	//以上情報を表示
	write_log_auo_line_fmt(LOG_INFO, "converting %s -> %s%s%s%s%s%s%s%s", input_fmt, output_fmt, ip_mode, bit_depth, ycc, 
		USE_SIMD[(use_sse2 | use_ssse3 | use_sse4_1) != 0], USE_SSE2[use_sse2 != 0], USE_SSSE3[use_ssse3 != 0], USE_SSE4_1[use_sse4_1 != 0]); 
}

static void build_full_cmd(char *cmd, size_t nSize, const CONF_X264GUIEX *conf, const OUTPUT_INFO *oip, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const char *input) {
	CONF_X264GUIEX prm;
	//パラメータをコピー
	memcpy(&prm, conf, sizeof(CONF_X264GUIEX));
	//共通置換を実行
	cmd_replace(prm.vid.cmdex,     sizeof(prm.vid.cmdex),     pe, sys_dat, oip->savefile);
	cmd_replace(prm.vid.stats,     sizeof(prm.vid.stats),     pe, sys_dat, oip->savefile);
	cmd_replace(prm.vid.tcfile_in, sizeof(prm.vid.tcfile_in), pe, sys_dat, oip->savefile);
	cmd_replace(prm.vid.cqmfile,   sizeof(prm.vid.cqmfile),   pe, sys_dat, oip->savefile);
	//cliモードでない
	if (!prm.oth.disable_guicmd) {
		//自動設定の適用
		apply_guiEx_auto_settings(&prm.x264, oip->w, oip->h, oip->rate, oip->scale);
		//GUI部のコマンドライン生成
		build_cmd_from_conf(cmd, nSize, &prm.x264, &prm.vid, FALSE);
	}
	//cmdex追加
	if (strlen(prm.vid.cmdex))
		sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " %s", prm.vid.cmdex);
	//メッセージの発行
	if ((conf->x264.vbv_bufsize != 0 || conf->x264.vbv_maxrate != 0) && prm.vid.afs)
		write_log_auo_line(LOG_INFO, "自動フィールドシフト使用時はvbv設定は正確に反映されません。");
	//AviUtlのkeyframe指定があれば、それをqpfileで読み込む
	char auoqpfile[MAX_PATH_LEN];
	apply_appendix(auoqpfile, sizeof(auoqpfile), pe->temp_filename, pe->append.qp);
	if (prm.vid.check_keyframe && FileExistsAndHasSize(auoqpfile) && strstr(cmd, "--qpfile") == NULL)
		sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " --qpfile \"%s\"", auoqpfile);
	//1pass目でafsでない、--framesがなければ--framesを指定
	if ((!prm.vid.afs || pe->current_x264_pass > 1) && strstr(cmd, "--frames") == NULL)
		sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " --frames %d", oip->n - pe->drop_count);
	//解像度情報追加(--input-res)
	if (strcmp(input, PIPE_FN) == NULL)
		sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " --input-res %dx%d", oip->w, oip->h);
	//rawの形式情報追加
	sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " --input-csp %s", specify_x264_input_csp(prm.x264.output_csp));
	//fps//tcfile-inが指定されていた場合、fpsの自動付加を停止]
	if (!prm.x264.use_tcfilein && strstr(cmd, "--tcfile-in") == NULL)
		sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " --fps %d/%d", oip->rate, oip->scale);
	//出力ファイル
	char *outfile = (prm.x264.nul_out) ? "nul" : pe->temp_filename;
	sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " -o \"%s\"", outfile);
	//入力
	sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " \"%s\"", input);
}

static void set_pixel_data(CONVERT_CF_DATA *pixel_data, const CONF_X264GUIEX *conf, int w, int h) {
	int byte_per_pixel = (conf->x264.use_10bit_depth) ? sizeof(short) : sizeof(BYTE);
	ZeroMemory(pixel_data,  sizeof(CONVERT_CF_DATA));
	switch (conf->x264.output_csp) {
		case OUT_CSP_YUV422: //nv16 (YUV422)
			pixel_data->count = 2;
			pixel_data->size[0] = w * h * byte_per_pixel;
			pixel_data->size[1] = pixel_data->size[0];
			break;
		case OUT_CSP_YUV444: //i444 (YUV444 planar)
			pixel_data->count = 3;
			pixel_data->size[0] = w * h * byte_per_pixel;
			pixel_data->size[1] = pixel_data->size[0];
			pixel_data->size[2] = pixel_data->size[0];
			break;
		case OUT_CSP_RGB: //RGB packed
			pixel_data->count = 1;
			pixel_data->size[0] = w * h * 3 * sizeof(BYTE); //8bit only
			break;
		case OUT_CSP_YUV420: //nv12 (YUV420)
		default:
			pixel_data->count = 2;
			pixel_data->size[0] = w * h * byte_per_pixel;
			pixel_data->size[1] = pixel_data->size[0] / 2;
			break;
	}
	//サイズの総和計算
	for (int i = 0; i < pixel_data->count; i++)
		pixel_data->total_size += pixel_data->size[i];
}

static void check_x264_priority(HANDLE h_aviutl, HANDLE h_x264, DWORD priority) {
	if (priority == AVIUTLSYNC_PRIORITY_CLASS)
		priority = GetPriorityClass(h_aviutl);
	SetPriorityClass(h_x264, priority);
}

static DWORD x264_out(CONF_X264GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
	DWORD ret = OUT_RESULT_SUCCESS;
	PIPE_SET pipes = { 0 };
	PROCESS_INFORMATION pi_x264 = { 0 };

	char x264cmd[MAX_CMD_LEN]  = { 0 };
	char x264args[MAX_CMD_LEN] = { 0 };
	char x264dir[MAX_PATH_LEN] = { 0 };
	char *x264fullpath = (conf->x264.use_10bit_depth) ? sys_dat->exstg->s_x264.fullpath_10bit : sys_dat->exstg->s_x264.fullpath;
	
	const BOOL afs = conf->vid.afs != 0;
	CONVERT_CF_DATA pixel_data;
	set_pixel_data(&pixel_data, conf, oip->w, oip->h);
	
	int *jitter = NULL;
	int rp_ret;

	//x264優先度関連の初期化
	DWORD set_priority = (pe->h_p_aviutl || conf->vid.priority != AVIUTLSYNC_PRIORITY_CLASS) ? priority_table[conf->vid.priority].value : NORMAL_PRIORITY_CLASS;

	//プロセス用情報準備
	if (!PathFileExists(x264fullpath)) {
		ret |= OUT_RESULT_ERROR; error_no_exe_file("x264", x264fullpath);
		return ret;
	}
	PathGetDirectory(x264dir, sizeof(x264dir), x264fullpath);

    //YUY2/YC48->NV12/YUV444, RGBコピー用関数
	const func_convert_frame convert_frame = get_convert_func(oip->w, conf->x264.use_10bit_depth, conf->x264.interlaced, conf->x264.output_csp, conf->x264.fullrange);
	convert_func_info(&conf->x264);
	//映像バッファ用メモリ確保
	if (!malloc_pixel_data(&pixel_data, oip->w, oip->h, conf->x264.output_csp, conf->x264.use_10bit_depth)) {
		ret |= OUT_RESULT_ERROR; error_malloc_pixel_data();
		return ret;
	}

	//パイプの設定
	pipes.stdIn.enable = TRUE;
	pipes.stdErr.enable = TRUE;
	pipes.stdIn.bufferSize = pixel_data.total_size * 2;

	//コマンドライン生成
	build_full_cmd(x264cmd, sizeof(x264cmd), conf, oip, pe, sys_dat, PIPE_FN);
	write_log_auo_line(LOG_INFO, "arguments passed...");
	write_args(x264cmd);
	sprintf_s(x264args, sizeof(x264args), "\"%s\" %s", x264fullpath, x264cmd);
	
	//jitter用領域確保
	if ((jitter = (int *)calloc(oip->n + 1, sizeof(int))) == NULL) {
		ret |= OUT_RESULT_ERROR; error_malloc_tc();
	//Aviutl(afs)からのフレーム読み込み
	} else if (!setup_afsvideo(oip, conf, pe, sys_dat->exstg->s_local.auto_afs_disable)) {
		ret |= OUT_RESULT_ERROR; //Aviutl(afs)からのフレーム読み込みに失敗
	//x264プロセス開始
	} else if ((rp_ret = RunProcess(x264args, x264dir, &pi_x264, &pipes, (set_priority == AVIUTLSYNC_PRIORITY_CLASS) ? GetPriorityClass(pe->h_p_aviutl) : set_priority, TRUE, FALSE)) != RP_SUCCESS) {
		ret |= OUT_RESULT_ERROR; error_run_process("x264", rp_ret);
	} else {
		//全て正常
		int i;
		void *frame = NULL;
		int *next_jitter = NULL;
		BOOL enc_pause = FALSE, copy_frame = FALSE, drop = FALSE;

		//x264が待機に入るまでこちらも待機
		while (WaitForInputIdle(pi_x264.hProcess, LOG_UPDATE_INTERVAL) == WAIT_TIMEOUT)
			log_process_events();

		//ログウィンドウ側から制御を可能に
		DWORD tm_x264enc_start = timeGetTime();
		enable_x264_control(&set_priority, &enc_pause, afs, afs && pe->current_x264_pass == 1, &tm_x264enc_start, oip->n);

		//------------メインループ------------
		for (i = 0, next_jitter = jitter + 1, pe->drop_count = 0; i < oip->n; i++, next_jitter++) {
			//中断を確認
			if (oip->func_is_abort()) {
				ret |= OUT_RESULT_ABORT;
				break;
			}

			//x264が実行中なら、メッセージを取得・ログウィンドウに表示
			if (ReadLogX264(&pipes, pe->drop_count, i) < 0) {
				//勝手に死んだ...
				ret |= OUT_RESULT_ERROR; error_x264_dead();
				break;
			}

			//一時停止
			while (enc_pause) {
				Sleep(LOG_UPDATE_INTERVAL);
				log_process_events();
			}

			if (!(i & 7)) {
				//Aviutlの進捗表示を更新
				oip->func_rest_time_disp(i + oip->n * (pe->current_x264_pass - 1), oip->n * pe->total_x264_pass);

				//x264優先度
				check_x264_priority(pe->h_p_aviutl, pi_x264.hProcess, set_priority);
			}

			//Aviutl(afs)からフレームをもらう
			if ((frame = afs_get_video((OUTPUT_INFO *)oip, i, &drop, next_jitter)) == NULL) {
				ret |= OUT_RESULT_ERROR; error_afs_get_frame();
				break;
			}

			//コピーフレームフラグ処理
			copy_frame = (i && (oip->func_get_flag(i) & OUTPUT_INFO_FRAME_FLAG_COPYFRAME));
			drop |= (afs & copy_frame);

			if (!drop) {
				//コピーフレームの場合は、映像バッファの中身を更新せず、そのままパイプに流す
				if (!copy_frame)
					convert_frame(frame, &pixel_data, oip->w, oip->h);  /// YUY2/YC48->NV12/YUV444変換, RGBコピー
				//映像データをパイプに
				for (int j = 0; j < pixel_data.count; j++)
					_fwrite_nolock((void *)pixel_data.data[j], 1, pixel_data.size[j], pipes.f_stdin);
			} else {
				*(next_jitter - 1) = DROP_FRAME_FLAG;
				pe->drop_count++;
			}

			// 「表示 -> セーブ中もプレビュー表示」がチェックされていると
			// func_update_preview() の呼び出しによって func_get_video_ex() の
			// 取得したバッファが書き換えられてしまうので、呼び出し位置を移動 (拡張AVI出力 plus より)
			oip->func_update_preview();
		}
		//------------メインループここまで--------------

		//ログウィンドウからのx264制御を無効化
		disable_x264_control();

		//パイプを閉じる
		CloseStdIn(&pipes);

		if (!ret) {
			oip->func_rest_time_disp(oip->n * pe->current_x264_pass, oip->n * pe->total_x264_pass);

			//タイムコード出力
			if (afs || conf->vid.auo_tcfile_out)
				tcfile_out(jitter, oip->n, (double)oip->rate / (double)oip->scale, afs, pe);
		}

		//x264終了待機
		while (WaitForSingleObject(pi_x264.hProcess, LOG_UPDATE_INTERVAL) == WAIT_TIMEOUT)
			ReadLogX264(&pipes, pe->drop_count, i);

		DWORD tm_x264enc_fin = timeGetTime();

		//最後にx264のメッセージを取得
		while (ReadLogX264(&pipes, pe->drop_count, i) > 0);

		if (!(ret & OUT_RESULT_ERROR) && afs)
			write_log_auo_line_fmt(LOG_INFO, "drop %d / %d frames", pe->drop_count, i);

		write_log_auo_enc_time("x264エンコード時間", tm_x264enc_fin - tm_x264enc_start);
	}

	set_window_title(AUO_FULL_NAME, PROGRESSBAR_DISABLED);

	//解放処理
	if (pipes.stdErr.enable)
		CloseHandle(pipes.stdErr.h_read);
	CloseHandle(pi_x264.hProcess);
	CloseHandle(pi_x264.hThread);

	free_pixel_data(&pixel_data);
	if (jitter) free(jitter); 

	return ret;
}

static void set_window_title_x264(PRM_ENC *pe) {
	char mes[256];
	strcpy_s(mes, sizeof(mes), "x264エンコード");
	if (pe->total_x264_pass > 1)
		sprintf_s(mes + strlen(mes), sizeof(mes) - strlen(mes), "   %d / %d pass", pe->current_x264_pass, pe->total_x264_pass);
	set_window_title(mes, PROGRESSBAR_CONTINUOUS);
}

DWORD video_output(CONF_X264GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
	DWORD ret = OUT_RESULT_SUCCESS;
	//動画エンコードの必要がなければ終了
	if (pe->video_out_type == VIDEO_OUTPUT_DISABLED)
		return ret;

	//追加コマンドをパラメータに適用する
	ret |= check_cmdex(conf, oip, pe, sys_dat);

	//キーフレーム検出
	if (!ret && conf->vid.check_keyframe && !conf->vid.afs && strstr(conf->vid.cmdex, "--qpfile") == NULL)
		check_keyframe_flag(oip, pe);

	for (; !ret && pe->current_x264_pass <= pe->total_x264_pass; pe->current_x264_pass++) {
		if (conf->x264.use_auto_npass) {
			//自動npass出力
			switch (pe->current_x264_pass) {
				case 1:
					conf->x264.pass = 1;
					break;
				case 2:
					if (conf->vid.afs && conf->vid.afs_bitrate_correction)
						conf->x264.bitrate = (conf->x264.bitrate * oip->n) / (oip->n - pe->drop_count);
					//下へフォールスルー
				default:
					open_log_window(oip->savefile, pe->current_x264_pass, pe->total_x264_pass);
					if (pe->current_x264_pass == pe->total_x264_pass)
						conf->x264.nul_out = FALSE;
					conf->x264.pass = 3;
					break;
			}
		}
		set_window_title_x264(pe);
		ret |= x264_out(conf, oip, pe, sys_dat);
	}

	return ret;
}