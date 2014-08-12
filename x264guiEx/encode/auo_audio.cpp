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
#pragma comment(lib, "user32.lib") //WaitforInputIdle

#include "output.h"
#include "auo.h"
#include "auo_version.h"
#include "auo_convert.h"
#include "auo_frm.h"
#include "auo_pipe.h"
#include "auo_error.h"
#include "auo_conf.h"
#include "auo_util.h"
#include "auo_system.h"
#include "fawcheck.h"
#include "auo_faw2aac.h"

#include "auo_encode.h"

const int WAVE_HEADER_SIZE = 44;
const int RIFF_SIZE_POS    = 4;
const int WAVE_SIZE_POS    = WAVE_HEADER_SIZE - 4;

static void auo_faw_check(CONF_AUDIO *aud, const OUTPUT_INFO *oip, const guiEx_settings *ex_stg) {
	if (ex_stg->s_aud_faw_index == FAW_INDEX_ERROR) {
		write_log_auo_line(LOG_WARNING, "FAWCheck : "AUO_NAME_WITHOUT_EXT".iniからのFAWの情報取得に失敗したため、判定を中止しました。");
		return;
	}
	int n = 0;
	short *dat = (short *)oip->func_get_audio(0, min(oip->audio_n, 10 * oip->audio_rate), &n);
	int ret = FAWCheck(dat, n, oip->audio_rate, oip->audio_size);
	switch (ret) {
		case NON_FAW:
			write_log_auo_line(LOG_INFO, "FAWCheck : non-FAW");
			break;
		case FAW_FULL:
		case FAW_HALF:
			aud->encoder   = ex_stg->s_aud_faw_index;
			aud->enc_mode  = ret - FAW_FULL;
			aud->use_2pass = ex_stg->s_aud[ex_stg->s_aud_faw_index].mode[aud->enc_mode].enc_2pass;
			aud->use_wav   = !ex_stg->s_aud[ex_stg->s_aud_faw_index].pipe_input;
			write_log_auo_line_fmt(LOG_INFO, "FAWCheck : FAW, %s size", (ret == FAW_FULL) ? "full" : "half");
			break;
		case FAWCHECK_ERROR_TOO_SHORT:
			write_log_auo_line(LOG_WARNING, "FAWCheck : 音声が短すぎ、判定できません。");
			break;
		case FAWCHECK_ERROR_OTHER:
		default:
			write_log_auo_line(LOG_WARNING, "FAWCheck : エラーが発生し、判定できません。");
			break;
	}
}

static void show_audio_enc_info(AUDIO_SETTINGS *aud_stg, CONF_AUDIO *cnf_aud) {
	char bitrate[128] = { 0 };
	if (aud_stg->mode[cnf_aud->enc_mode].bitrate)
		sprintf_s(bitrate, _countof(bitrate), ", %dkbps", cnf_aud->bitrate);
	char *use2pass = (cnf_aud->use_2pass) ? ", 2pass" : "";
	write_log_auo_line_fmt(LOG_INFO, "%s で音声エンコードを行います。%s%s%s", aud_stg->dispname, aud_stg->mode[cnf_aud->enc_mode].name, bitrate, use2pass);
}

static void build_wave_header(BYTE *head, const OUTPUT_INFO *oip, BOOL use_8bit, int sample_n) {
	static const char * const RIFF_HEADER = "RIFF";
	static const char * const WAVE_HEADER = "WAVE";
	static const char * const FMT_CHUNK   = "fmt ";
	static const char * const DATA_CHUNK  = "data";
	const DWORD FMT_SIZE    = 16;
	const short FMT_ID      = 1;
	const int   size        = (use_8bit) ? sizeof(BYTE) : sizeof(short);

	memcpy(   head +  0, RIFF_HEADER, strlen(RIFF_HEADER));
	*(DWORD*)(head +  4) = sample_n * (size * oip->audio_ch) + WAVE_HEADER_SIZE - 8;
	memcpy(   head +  8, WAVE_HEADER, strlen(WAVE_HEADER));
	memcpy(   head + 12, FMT_CHUNK, strlen(FMT_CHUNK));
	*(DWORD*)(head + 16) = FMT_SIZE;
	*(short*)(head + 20) = FMT_ID;
	*(short*)(head + 22) = (short)oip->audio_ch;
	*(DWORD*)(head + 24) = oip->audio_rate;
	*(DWORD*)(head + 28) = oip->audio_rate * oip->audio_ch * size;
	*(short*)(head + 32) = (short)(size * oip->audio_ch);
	*(short*)(head + 34) = (short)(size * 8);
	memcpy(   head + 36, DATA_CHUNK, strlen(DATA_CHUNK));
	*(DWORD*)(head + 40) = sample_n * (size * oip->audio_ch);
	//計44byte(WAVE_HEADER_SIZE)
}

static void correct_header(FILE *f_out, int data_size) {
	//2箇所の出力データサイズ部分を書き換え
	int riff_size = data_size + (WAVE_SIZE_POS - RIFF_SIZE_POS);
	_fseeki64(f_out, RIFF_SIZE_POS, SEEK_SET);
	fwrite(&riff_size, sizeof(int), 1, f_out);
	_fseeki64(f_out, WAVE_SIZE_POS - RIFF_SIZE_POS, SEEK_CUR);
	fwrite(&data_size, sizeof(int), 1, f_out);
}

static void write_wav_header(FILE *f_out, const OUTPUT_INFO *oip, BOOL use_8bit) {
	BYTE head[WAVE_HEADER_SIZE];
	build_wave_header(head, oip, use_8bit, oip->audio_n);
	_fwrite_nolock(&head, sizeof(head), 1, f_out);
}

static void make_wavfilename(char *wavfile, size_t nSize, BOOL use_pipe, const char *tempfilename, const char *append_wav) {
	if (use_pipe)
		strcpy_s(wavfile, nSize, PIPE_FN);
	else
		apply_appendix(wavfile, nSize, tempfilename, append_wav);
}

static void build_audcmd(char *cmd, size_t nSize, const CONF_X264GUIEX *conf, const AUDIO_SETTINGS *aud_stg, const PRM_ENC *pe, const SYSTEM_DATA *sys_dat, const char *wavfile, const OUTPUT_INFO *oip) {
	strcpy_s(cmd, nSize, aud_stg->cmd_base);
	//%{2pass_cmd}
	replace(cmd, nSize, "%{2pass_cmd}",  (conf->aud.use_2pass) ? aud_stg->cmd_2pass : "");
	//%{mode}
	replace(cmd, nSize, "%{mode}",  aud_stg->mode[conf->aud.enc_mode].cmd);
	//%{wavpath}
	replace(cmd, nSize, "%{wavpath}",  wavfile);
	//%{rate}
	char tmp[22];
	sprintf_s(tmp, _countof(tmp), "%d", conf->aud.bitrate);
	replace(cmd, nSize, "%{rate}", tmp);

	cmd_replace(cmd, nSize, pe, sys_dat, conf, oip);
}

static void show_progressbar(BOOL use_pipe, const char *enc_name, int progress_mode) {
	char mes[1024];
	if (use_pipe)
		sprintf_s(mes, _countof(mes), "%s でエンコード中...", enc_name);
	else
		strcpy_s(mes, _countof(mes), "wav出力中...");
	set_window_title(mes, progress_mode);
}

static AUO_RESULT wav_output(const OUTPUT_INFO *oip,  const char *wavfile, BOOL wav_8bit, int bufsize, 
						PROCESS_INFORMATION *pi_aud, const char *auddispname, char *audargs, 
						const char *auddir, DWORD encoder_priority) 
{
	AUO_RESULT ret = AUO_RESULT_SUCCESS;
	PIPE_SET pipes = { 0 };
	BYTE *buf8bit = NULL;
	FILE *f_out = NULL;
	const func_audio_16to8 audio_16to8 = get_audio_16to8_func();
	const BOOL use_pipe = (strcmp(wavfile, PIPE_FN) == NULL); 
	int rp_ret;

	//8bitを使用する場合のメモリ確保
	if (wav_8bit && (buf8bit = (BYTE*)malloc(bufsize * oip->audio_ch * sizeof(BYTE))) == NULL) {
		ret |= AUO_RESULT_ERROR; error_malloc_8bit();
		return ret;
	}

	//パイプ or ファイルオープン
	if (use_pipe) {
		//パイプ準備
		pipes.stdIn.enable = TRUE;
		pipes.stdIn.bufferSize = bufsize * 2;
		//エンコーダ準備
		if ((rp_ret = RunProcess(audargs, auddir, pi_aud, &pipes, encoder_priority, TRUE, FALSE)) != RP_SUCCESS) {
			ret |= AUO_RESULT_ERROR; error_run_process(auddispname, rp_ret);
		} else {
			f_out = pipes.f_stdin;
			while (WaitForInputIdle(pi_aud->hProcess, LOG_UPDATE_INTERVAL) == WAIT_TIMEOUT)
				log_process_events();
		}
	} else if (fopen_s(&f_out, wavfile, "wbS")) {
		ret |= AUO_RESULT_ERROR; error_open_wavfile();
	}

	if (!ret) {
		//wavヘッダ出力
		write_wav_header(f_out, oip, wav_8bit);

		//メッセージ
		show_progressbar(use_pipe, auddispname, PROGRESSBAR_CONTINUOUS);

		//wav出力
		const int wav_sample_size = oip->audio_ch * ((wav_8bit) ? sizeof(BYTE) : sizeof(short));
		void *audio_dat = NULL;
		int samples_read = 0, samples_get = bufsize;
		//wav出力ループ
		while (oip->audio_n - samples_read > 0 && samples_get) {
			//中断
			if (oip->func_is_abort()) {
				ret |= AUO_RESULT_ABORT;
				break;
			}
			audio_dat = oip->func_get_audio(samples_read, min(oip->audio_n - samples_read, bufsize), &samples_get);
			samples_read += samples_get;
			set_log_progress(samples_read / (double)oip->audio_n);

			if (wav_8bit)
				audio_16to8(buf8bit, (short*)audio_dat, samples_get * oip->audio_ch);

			_fwrite_nolock((wav_8bit) ? buf8bit : audio_dat, samples_get * wav_sample_size, 1, f_out);
		}
		//終了処理
		if (!use_pipe && oip->audio_n != samples_read)
			correct_header(f_out, samples_read * wav_sample_size);
		(use_pipe) ? CloseStdIn(&pipes) : fclose(f_out);
	}
	if (buf8bit) free(buf8bit);

	//wavファイル出力が成功したか確認
	if (!use_pipe && !FileExistsAndHasSize(wavfile)) {
		ret |= AUO_RESULT_ERROR; error_no_wavefile();
	}

	return ret;
}

AUO_RESULT audio_output(CONF_X264GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
	AUO_RESULT ret = AUO_RESULT_SUCCESS;
	//音声エンコードの必要がなければ終了
	if (!(oip->flag & OUTPUT_INFO_FLAG_AUDIO))
		return ret;
	//FAWCheck
	if (conf->aud.faw_check)
		auo_faw_check(&conf->aud, oip, sys_dat->exstg);
	if (conf->aud.encoder == sys_dat->exstg->s_aud_faw_index)
		if (AUO_RESULT_SUCCESS == audio_faw2aac(conf, oip, pe, sys_dat))
			return ret;

	//使用するエンコーダの設定を選択
	AUDIO_SETTINGS *aud_stg = &sys_dat->exstg->s_aud[conf->aud.encoder];

	char wavfile[MAX_PATH_LEN] = { 0 };
	char audfile[MAX_PATH_LEN] = { 0 };
	char audcmd[MAX_CMD_LEN]   = { 0 };
	char audargs[MAX_CMD_LEN]  = { 0 };
	char auddir[MAX_PATH_LEN]  = { 0 };

	PROCESS_INFORMATION pi_aud = { 0 };
	const BOOL use_pipe = (!conf->aud.use_wav && !conf->aud.use_2pass) ? TRUE : FALSE;
	DWORD encoder_priority = GetExePriority(conf->aud.priority, pe->h_p_aviutl);

	//実行ファイルチェック(filenameが空文字列なら実行しない)
	if (str_has_char(aud_stg->filename) && !PathFileExists(aud_stg->fullpath)) {
		error_no_exe_file(aud_stg->dispname, aud_stg->fullpath);
		return AUO_RESULT_ERROR;
	}

	//wavfile名作成
	make_wavfilename(wavfile, _countof(wavfile), use_pipe, pe->temp_filename, pe->append.wav);

	//audfile名作成
	strcpy_s(pe->append.aud, _countof(pe->append.aud), aud_stg->aud_appendix); //pe一時パラメータにコピーしておく
	get_aud_filename(audfile, _countof(audfile), pe);

	//情報表示
	show_audio_enc_info(aud_stg, &conf->aud);

	//auddir作成
	PathGetDirectory(auddir, _countof(auddir), aud_stg->fullpath);

	//コマンドライン作成
	build_audcmd(audcmd, _countof(audcmd), conf, aud_stg, pe, sys_dat, wavfile, oip);
	sprintf_s(audargs, _countof(audargs), "\"%s\" %s", aud_stg->fullpath, audcmd);

	//wav出力
	ret |= wav_output(oip, wavfile, aud_stg->mode[conf->aud.enc_mode].use_8bit, sys_dat->exstg->s_local.audio_buffer_size,
		&pi_aud, aud_stg->dispname, audargs, auddir, encoder_priority);
	
	//音声エンコード(filenameが空文字列なら実行しない)
	if (!ret && !use_pipe && str_has_char(aud_stg->filename)) {
		show_progressbar(use_pipe, aud_stg->dispname, PROGRESSBAR_MARQUEE);
		int rp_ret;
		if ((rp_ret = RunProcess(audargs, auddir, &pi_aud, NULL, encoder_priority, FALSE, conf->aud.minimized)) != RP_SUCCESS) {
			ret |= AUO_RESULT_ERROR; error_run_process(aud_stg->dispname, rp_ret);
		}
	}

	//終了待機、メッセージ取得(filenameが空文字列なら実行しない)
	if (!ret && str_has_char(aud_stg->filename)) {
		while (WaitForSingleObject(pi_aud.hProcess, LOG_UPDATE_INTERVAL) == WAIT_TIMEOUT)
			log_process_events();

		UINT64 audfilesize = 0; 
		if (!PathFileExists(audfile) || 
			(GetFileSizeUInt64(audfile, &audfilesize) && audfilesize == 0)) {
			//エラーが発生した場合
			ret |= AUO_RESULT_ERROR; error_audenc_failed(aud_stg->dispname, audargs);
		} else {
			if (FileExistsAndHasSize(audfile))
				remove(wavfile); //ゴミ掃除
		}
	}

	CloseHandle(pi_aud.hProcess);
	CloseHandle(pi_aud.hThread);

	set_window_title(AUO_FULL_NAME, PROGRESSBAR_DISABLED);

	return ret;
}