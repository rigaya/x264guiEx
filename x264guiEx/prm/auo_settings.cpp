//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "auo_util.h"
#include "auo_settings.h"
#include "auo_version.h"

static const int INI_SECTION_BUFSIZE = 32768;
static const int INI_KEY_MAX_LEN = 256;

static const int INI_VER = 1;

static const char * const INI_APPENDIX  = ".ini";
static const char * const CONF_APPENDIX = ".conf";

static const char * const STG_DEFAULT_DIRECTORY_NAME = "x264guiEx_stg";

static const char * const INI_SECTION_MAIN         = "X264GUIEX";
static const char * const INI_SECTION_APPENDIX     = "APPENDIX";
static const char * const INI_SECTION_AUD          = "AUDIO";
static const char * const INI_SECTION_X264         = "X264";
static const char * const INI_SECTION_X264_DEFAULT = "X264_DEFAULT";
static const char * const INI_SECTION_X264_PRESET  = "X264_PRESET";
static const char * const INI_SECTION_X264_TUNE    = "X264_TUNE";
static const char * const INI_SECTION_X264_PROFILE = "X264_PROFILE";
static const char * const INI_SECTION_MUX          = "MUXER";
static const char * const INI_SECTION_FN           = "FILENAME_REPLACE";
static const char * const INI_SECTION_PREFIX       = "SETTING_";
static const char * const INI_SECTION_MODE         = "MODE_";
static const char * const INI_SECTION_FBC          = "BITRATE_CALC";


static double GetPrivateProfileDouble(const char *section, const char *keyname, const char *defaultString, const char *ini_file) {
	char buf[256], *eptr;
	double d;
	GetPrivateProfileString(section, keyname, defaultString, buf, sizeof(buf), ini_file);
	d = strtod(buf, &eptr);
	if (*eptr == '\0') return d;
	d = strtod(defaultString, &eptr);
	if (*eptr == '\0') return d;
	d = 0.0; return d;
}

static inline void WritePrivateProfileInt(const char *section, const char *keyname, int value, const char *ini_file) {
	char tmp[22];
	sprintf_s(tmp, sizeof(tmp), "%d", value);
	WritePrivateProfileString(section, keyname, tmp, ini_file);
}

static inline void WritePrivateProfileDouble(const char *section, const char *keyname, double value, const char *ini_file) {
	char tmp[64];
	sprintf_s(tmp, sizeof(tmp), "%lf", value);
	WritePrivateProfileString(section, keyname, tmp, ini_file);
}


BOOL    guiEx_settings::init = FALSE;
char    guiEx_settings::auo_path[MAX_PATH_LEN] = { 0 };
char    guiEx_settings::ini_fileName[MAX_PATH_LEN] = { 0 };
char    guiEx_settings::conf_fileName[MAX_PATH_LEN] = { 0 };
DWORD   guiEx_settings::ini_filesize = 0;

guiEx_settings::guiEx_settings() {
	initialize(false);
}

guiEx_settings::guiEx_settings(BOOL disable_loading) {
	initialize(disable_loading);
}

void guiEx_settings::initialize(BOOL disable_loading) {
	s_aud_count = 0;
	s_mux_count = 0;
	s_aud = NULL;
	s_mux = NULL;
	ZeroMemory(&s_x264, sizeof(s_x264));
	ZeroMemory(&s_local, sizeof(s_local));
	ZeroMemory(&s_log, sizeof(s_log));
	ZeroMemory(&s_append, sizeof(s_append));
	s_aud_faw_index = FAW_INDEX_ERROR;
	if (!init) {
		get_auo_path(auo_path, sizeof(auo_path));
		apply_appendix(ini_fileName,  sizeof(ini_fileName),  auo_path, INI_APPENDIX);
		apply_appendix(conf_fileName, sizeof(conf_fileName), auo_path, CONF_APPENDIX);
		init = check_inifile() && !disable_loading;
		if (init) {
			load_encode_stg();
			load_fn_replace();
			load_log_win();
			load_append();
		}
	}
}

guiEx_settings::~guiEx_settings() {
	clear_aud();
	clear_mux();
	clear_x264();
	clear_local();
	clear_fn_replace();
	clear_log_win();
	clear_append();
	clear_fbc();
}

BOOL guiEx_settings::check_inifile() {
	BOOL ret = (INI_VER == GetPrivateProfileInt(INI_SECTION_MAIN, "ini_ver", 0, ini_fileName));
	if (ret && !GetFileSizeInt(ini_fileName, &ini_filesize))
		ret = FALSE;
	return ret;
}

BOOL guiEx_settings::get_init_success() {
	if (!init) {
		char mes[1024];
		char title[256];
		strcpy_s(mes, sizeof(mes), AUO_NAME);
		sprintf_s(PathFindExtension(mes), sizeof(mes) - strlen(mes), ".iniが存在しないか、iniファイルが古いです。\n%s を開始できません。", AUO_FULL_NAME);
		sprintf_s(title, sizeof(title), "%s - エラー", AUO_FULL_NAME);
		MessageBox(NULL, mes, title, MB_ICONERROR);
	}
	return init;
}

int guiEx_settings::get_faw_index() {
	for (int i = 0; i < s_aud_count; i++)
		if (stristr(s_aud[i].filename, "faw"))
			return i;
	return FAW_INDEX_ERROR;
}

void guiEx_settings::load_encode_stg() {
	load_aud();
	load_mux();
	load_x264();
	load_local(); //fullpathの情報がきちんと格納されるよう、最後に呼ぶ
}

void guiEx_settings::load_aud() {
	int i, j, k;
	char encoder_section[INI_KEY_MAX_LEN];
	char key[INI_KEY_MAX_LEN];
	int keybase_len;

	clear_aud();

	s_aud_count = GetPrivateProfileInt(INI_SECTION_AUD, "count", 0, ini_fileName);

	s_aud_mc.init(ini_filesize + s_aud_count * sizeof(AUDIO_SETTINGS));
	s_aud = (AUDIO_SETTINGS *)s_aud_mc.CutMem(s_aud_count * sizeof(AUDIO_SETTINGS));
	for (i = 0; i < s_aud_count; i++) {
		sprintf_s(key, sizeof(key), "audio_encoder_%d", i+1);
		s_aud[i].keyName = s_aud_mc.SetPrivateProfileString(INI_SECTION_AUD, key, "key", ini_fileName);
		sprintf_s(encoder_section, sizeof(encoder_section), "%s%s", INI_SECTION_PREFIX, s_aud[i].keyName);
		s_aud[i].dispname     = s_aud_mc.SetPrivateProfileString(encoder_section, "dispname",     "", ini_fileName);
		s_aud[i].filename     = s_aud_mc.SetPrivateProfileString(encoder_section, "filename",     "", ini_fileName);
		s_aud[i].aud_appendix = s_aud_mc.SetPrivateProfileString(encoder_section, "aud_appendix", "", ini_fileName);
		s_aud[i].cmd_base     = s_aud_mc.SetPrivateProfileString(encoder_section, "base_cmd",     "", ini_fileName);
		s_aud[i].cmd_2pass    = s_aud_mc.SetPrivateProfileString(encoder_section, "2pass_cmd",    "", ini_fileName);
		s_aud[i].pipe_input   = GetPrivateProfileInt(            encoder_section, "pipe_input",    0, ini_fileName);

		sprintf_s(encoder_section, sizeof(encoder_section), "%s%s", INI_SECTION_MODE, s_aud[i].keyName);
		int tmp_count = GetPrivateProfileInt(encoder_section, "count", 0, ini_fileName);
		//置き換えリストの影響で、この段階ではAUDIO_ENC_MODEが最終的に幾つになるのかわからない
		//とりあえず、一時的に読み込んでみる
		s_aud[i].mode_count = tmp_count;
		AUDIO_ENC_MODE *tmp_mode = (AUDIO_ENC_MODE *)s_aud_mc.CutMem(tmp_count * sizeof(AUDIO_ENC_MODE));
		for (j = 0; j < tmp_count; j++) {
			sprintf_s(key, sizeof(key), "mode_%d", j+1);
			tmp_mode[j].name = s_aud_mc.SetPrivateProfileString(encoder_section, key, "", ini_fileName);
			keybase_len = strlen(key);
			strcpy_s(key + keybase_len, sizeof(key) - keybase_len, "_cmd");
			tmp_mode[j].cmd = s_aud_mc.SetPrivateProfileString(encoder_section, key, "", ini_fileName);
			strcpy_s(key + keybase_len, sizeof(key) - keybase_len, "_2pass");
			tmp_mode[j].enc_2pass = GetPrivateProfileInt(encoder_section, key, 0, ini_fileName);
			strcpy_s(key + keybase_len, sizeof(key) - keybase_len, "_convert8bit");
			tmp_mode[j].use_8bit =  GetPrivateProfileInt(encoder_section, key, 0, ini_fileName);
			strcpy_s(key + keybase_len, sizeof(key) - keybase_len, "_bitrate");
			tmp_mode[j].bitrate = GetPrivateProfileInt(encoder_section, key, 0, ini_fileName);
			if (tmp_mode[j].bitrate) {
				strcpy_s(key + keybase_len, sizeof(key) - keybase_len, "_bitrate_min");
				tmp_mode[j].bitrate_min = GetPrivateProfileInt(encoder_section, key, 0, ini_fileName);
				strcpy_s(key + keybase_len, sizeof(key) - keybase_len, "_bitrate_max");
				tmp_mode[j].bitrate_max = GetPrivateProfileInt(encoder_section, key, 0, ini_fileName);
				strcpy_s(key + keybase_len, sizeof(key) - keybase_len, "_bitrate_step");
				tmp_mode[j].bitrate_step = GetPrivateProfileInt(encoder_section, key, 0, ini_fileName);
				strcpy_s(key + keybase_len, sizeof(key) - keybase_len, "_bitrate_default");
				tmp_mode[j].bitrate_default = GetPrivateProfileInt(encoder_section, key, 0, ini_fileName);
			} else {
				strcpy_s(key + keybase_len, sizeof(key) - keybase_len, "_dispList");
				tmp_mode[j].disp_list = s_aud_mc.SetPrivateProfileString(encoder_section, key, "", ini_fileName);
				s_aud_mc.CutMem(1);
				strcpy_s(key + keybase_len, sizeof(key) - keybase_len, "_cmdList");
				tmp_mode[j].cmd_list = s_aud_mc.SetPrivateProfileString(encoder_section, key, "", ini_fileName);
				s_aud_mc.CutMem(1);
				//リストのcmd置き換えリストの","の数分AUDIO_ENC_MODEは増える
				if (!tmp_mode[j].bitrate)
					s_aud[i].mode_count += countchr(tmp_mode[j].cmd_list, ',');
			}
		}
		s_aud[i].mode = (AUDIO_ENC_MODE *)s_aud_mc.CutMem(s_aud[i].mode_count * sizeof(AUDIO_ENC_MODE));
		j = 0;
		for (int tmp_index = 0; tmp_index < tmp_count; tmp_index++) {
			if (tmp_mode[tmp_index].bitrate) {
				memcpy(&s_aud[i].mode[j], &tmp_mode[tmp_index], sizeof(AUDIO_ENC_MODE));
				j++;
			} else {
				//置き換えリストを分解する
				char *p, *q;
				int list_count = countchr(tmp_mode[tmp_index].cmd_list, ',') + 1;
				//分解した先頭へのポインタへのポインタ用領域を確保
				char **cmd_list  = (char**)s_aud_mc.CutMem(sizeof(char*) * list_count);
				char **disp_list = (char**)s_aud_mc.CutMem(sizeof(char*) * list_count);
				//cmdの置き換えリストを","により分解
				cmd_list[0] = tmp_mode[tmp_index].cmd_list;
				for (k = 0, p = cmd_list[0];  (cmd_list[k] = strtok_s(p, ",", &q))  != NULL; k++)
					p = NULL;
				//同様に表示用リストを分解
				disp_list[0] = tmp_mode[tmp_index].disp_list;
				for (k = 0, p = disp_list[0]; (disp_list[k] = strtok_s(p, ",", &q)) != NULL; k++)
					p = NULL;
				//リストの個数分、置き換えを行ったAUDIO_ENC_MODEを作成する
				for (k = 0; k < list_count; j++, k++) {
					memcpy(&s_aud[i].mode[j], &tmp_mode[tmp_index], sizeof(AUDIO_ENC_MODE));

					if (cmd_list[k]) {
						strcpy_s((char *)s_aud_mc.GetPtr(), s_aud_mc.GetRemain(), s_aud[i].mode[j].cmd);
						replace((char *)s_aud_mc.GetPtr(), s_aud_mc.GetRemain(), "%{cmdList}", cmd_list[k]);
						s_aud[i].mode[j].cmd = (char *)s_aud_mc.GetPtr();
						s_aud_mc.CutString();
					}

					if (disp_list[k]) {
						strcpy_s((char *)s_aud_mc.GetPtr(), s_aud_mc.GetRemain(), s_aud[i].mode[j].name);
						replace((char *)s_aud_mc.GetPtr(), s_aud_mc.GetRemain(), "%{dispList}", disp_list[k]);
						s_aud[i].mode[j].name = (char *)s_aud_mc.GetPtr();
						s_aud_mc.CutString();
					}
				}
			}
		}
	}
	s_aud_faw_index = get_faw_index();
}

void guiEx_settings::load_mux() {
	int i, j, len;
	char muxer_section[INI_KEY_MAX_LEN];
	char key[INI_KEY_MAX_LEN];

	static const int MUX_COUNT = 3;
	static const char * MUXER_TYPE[MUX_COUNT] = { "MUXER_MP4", "MUXER_MKV", "MUXER_TC2MP4" };

	clear_mux();


	s_mux_count = MUX_COUNT;
	s_mux_mc.init(ini_filesize + s_mux_count * sizeof(MUXER_SETTINGS));
	s_mux = (MUXER_SETTINGS *)s_mux_mc.CutMem(s_mux_count * sizeof(MUXER_SETTINGS));
	for (i = 0; i < s_mux_count; i++) {
		sprintf_s(muxer_section, sizeof(muxer_section), "%s%s", INI_SECTION_PREFIX, MUXER_TYPE[i]);
		len = strlen(MUXER_TYPE[i]) + 1;
		s_mux[i].keyName = (char *)s_mux_mc.CutMem(len);
		memcpy(s_mux[i].keyName, MUXER_TYPE[i], len);
		s_mux[i].dispname = s_mux_mc.SetPrivateProfileString(muxer_section, "dispname", "", ini_fileName);
		s_mux[i].filename = s_mux_mc.SetPrivateProfileString(muxer_section, "filename", "", ini_fileName);
		s_mux[i].base_cmd = s_mux_mc.SetPrivateProfileString(muxer_section, "base_cmd", "", ini_fileName);
		s_mux[i].aud_cmd  = s_mux_mc.SetPrivateProfileString(muxer_section, "au_cmd",   "", ini_fileName);
		s_mux[i].tc_cmd   = s_mux_mc.SetPrivateProfileString(muxer_section, "tc_cmd",   "", ini_fileName);
		s_mux[i].tmp_cmd  = s_mux_mc.SetPrivateProfileString(muxer_section, "tmp_cmd",  "", ini_fileName);

		sprintf_s(muxer_section, sizeof(muxer_section), "%s%s", INI_SECTION_MODE, s_mux[i].keyName);
		s_mux[i].ex_count = GetPrivateProfileInt(muxer_section, "count", 0, ini_fileName);
		s_mux[i].ex_cmd = (MUXER_CMD_EX *)s_mux_mc.CutMem(s_mux[i].ex_count * sizeof(MUXER_CMD_EX));
		for (j = 0; j < s_mux[i].ex_count; j++) {
			sprintf_s(key, sizeof(key), "ex_cmd_%d", j+1);
			s_mux[i].ex_cmd[j].cmd  = s_mux_mc.SetPrivateProfileString(muxer_section, key, "", ini_fileName);
			strcat_s(key, sizeof(key), "_name");
			s_mux[i].ex_cmd[j].name = s_mux_mc.SetPrivateProfileString(muxer_section, key, "", ini_fileName);
		}
	}
}

void guiEx_settings::load_fn_replace() {
	clear_fn_replace();

	fn_rep_mc.init(ini_filesize);

	char *ptr = (char *)fn_rep_mc.GetPtr();
	size_t len = GetPrivateProfileSection(INI_SECTION_FN, ptr, fn_rep_mc.GetRemain(), ini_fileName) + 1;
	fn_rep_mc.CutMem(len);
	for (; *ptr != NULL; ptr += strlen(ptr) + 1) {
		FILENAME_REPLACE rep = { 0 };
		char *p = strchr(ptr, '=');
		rep.from = (p) ? p + 1 : ptr - 1;
		p = strchr(ptr, ':');
		if (p) *p = '\0';
		rep.to   = (p) ? p + 1 : ptr - 1;
		fn_rep.push_back(rep);
	}
}

void guiEx_settings::load_x264_cmd(X264_CMD *x264cmd, int *count, int *default_index, const char *section) {
	int i;
	char *p, *q;
	char key[INI_KEY_MAX_LEN];
	char *name = s_x264_mc.SetPrivateProfileString(section, "name", "", ini_fileName);
	s_x264_mc.CutMem(1);
	*count = countchr(name, ',') + 1;
	x264cmd->name = (X264_OPTION_STR *)s_x264_mc.CutMem(sizeof(X264_OPTION_STR) * (*count + 1));
	ZeroMemory(x264cmd->name, sizeof(X264_OPTION_STR) * (*count + 1));
	x264cmd->cmd = (char **)s_x264_mc.CutMem(sizeof(char *) * (*count + 1));

	x264cmd->name[0].name = name;
	for (i = 0, p = x264cmd->name[0].name; (x264cmd->name[i].name = strtok_s(p, ",", &q)) != NULL; i++)
		p = NULL;

	setlocale(LC_ALL, "japanese");
	for (int i = 0; x264cmd->name[i].name; i++) {
		size_t w_len = 0;
		x264cmd->name[i].desc = (WCHAR *)s_x264_mc.GetPtr();
		mbstowcs_s(&w_len, x264cmd->name[i].desc, s_x264_mc.GetRemain() / sizeof(WCHAR), x264cmd->name[i].name, _TRUNCATE);
		s_x264_mc.CutMem((w_len + 1) * sizeof(WCHAR));
	}

	*default_index = 0;
	char *def = s_x264_mc.SetPrivateProfileString(section, "disp", "", ini_fileName);
	sprintf_s(key,  sizeof(key), "cmd_");
	int keybase_len = strlen(key);
	for (i = 0; x264cmd->name[i].name; i++) {
		strcpy_s(key + keybase_len, sizeof(key) - keybase_len, x264cmd->name[i].name);
		x264cmd->cmd[i] = s_x264_mc.SetPrivateProfileString(section, key, "", ini_fileName);
		if (_stricmp(x264cmd->name[i].name, def) == NULL)
			*default_index = i;
	}
}

void guiEx_settings::load_x264() {
	char key[INI_KEY_MAX_LEN];

	clear_x264();

	s_x264_mc.init(ini_filesize);

	s_x264.default_cmd       = s_x264_mc.SetPrivateProfileString(INI_SECTION_X264_DEFAULT, "cmd_default",       "", ini_fileName);
	s_x264.default_cmd_10bit = s_x264_mc.SetPrivateProfileString(INI_SECTION_X264_DEFAULT, "cmd_default_10bit", "", ini_fileName);

	load_x264_cmd(&s_x264.preset,  &s_x264.preset_count,  &s_x264.default_preset,  INI_SECTION_X264_PRESET);
	load_x264_cmd(&s_x264.tune,    &s_x264.tune_count,    &s_x264.default_tune,    INI_SECTION_X264_TUNE);
	load_x264_cmd(&s_x264.profile, &s_x264.profile_count, &s_x264.default_profile, INI_SECTION_X264_PROFILE);

	s_x264.profile_vbv_multi = (float *)s_x264_mc.CutMem(sizeof(float) * s_x264.profile_count);
	for (int i = 0; i < s_x264.profile_count; i++) {
		sprintf_s(key, sizeof(key), "vbv_multi_%s", s_x264.profile.name[i]);
		s_x264.profile_vbv_multi[i] = (float)GetPrivateProfileDouble(INI_SECTION_X264_PROFILE, key, "1.0", ini_fileName);
	}

	s_x264_refresh = TRUE;
}

void guiEx_settings::make_default_stg_dir(char *default_stg_dir, DWORD nSize) {
	strcpy_s(default_stg_dir, nSize, auo_path);
	DWORD buf_space = nSize - (strlen(default_stg_dir) - strlen(PathFindFileName(default_stg_dir)));
	strcpy_s(PathFindFileName(default_stg_dir), buf_space, STG_DEFAULT_DIRECTORY_NAME);
}

void guiEx_settings::load_local() {
	char default_stg_dir[MAX_PATH_LEN];
	make_default_stg_dir(default_stg_dir, sizeof(default_stg_dir));

	clear_local();

	s_local.large_cmdbox = GetPrivateProfileInt(INI_SECTION_MAIN,  "large_cmdbox",        0,                    conf_fileName);
	GetPrivateProfileString(INI_SECTION_MAIN, "custom_tmp_dir",        "", s_local.custom_tmp_dir,        sizeof(s_local.custom_tmp_dir),        conf_fileName);
	GetPrivateProfileString(INI_SECTION_MAIN, "custom_audio_tmp_dir",  "", s_local.custom_audio_tmp_dir,  sizeof(s_local.custom_audio_tmp_dir),  conf_fileName);
	GetPrivateProfileString(INI_SECTION_MAIN, "custom_mp4box_tmp_dir", "", s_local.custom_mp4box_tmp_dir, sizeof(s_local.custom_mp4box_tmp_dir), conf_fileName);
	GetPrivateProfileString(INI_SECTION_MAIN, "stg_dir",  default_stg_dir, s_local.stg_dir,               sizeof(s_local.stg_dir),               conf_fileName);
	GetPrivateProfileString(INI_SECTION_MAIN, "last_app_dir",          "", s_local.app_dir,               sizeof(s_local.custom_mp4box_tmp_dir), conf_fileName);

	s_local.audio_buffer_size   = min(GetPrivateProfileInt(INI_SECTION_MAIN, "audio_buffer",        AUDIO_BUFFER_DEFAULT, conf_fileName), AUDIO_BUFFER_MAX);

	GetPrivateProfileString(INI_SECTION_X264,    "X264",           "", s_x264.fullpath,       sizeof(s_x264.fullpath),       conf_fileName);
	GetPrivateProfileString(INI_SECTION_X264,    "X264_10bit",     "", s_x264.fullpath_10bit, sizeof(s_x264.fullpath_10bit), conf_fileName);
	for (int i = 0; i < s_aud_count; i++)
		GetPrivateProfileString(INI_SECTION_AUD, s_aud[i].keyName, "", s_aud[i].fullpath,     sizeof(s_aud[i].fullpath),     conf_fileName);
	for (int i = 0; i < s_mux_count; i++)
		GetPrivateProfileString(INI_SECTION_MUX, s_mux[i].keyName, "", s_mux[i].fullpath,     sizeof(s_mux[i].fullpath),     conf_fileName);
}

void guiEx_settings::load_log_win() {
	clear_log_win();
	s_log.minimized        = GetPrivateProfileInt(INI_SECTION_MAIN, "log_start_minimized",  0, conf_fileName);
	s_log.transparent      = GetPrivateProfileInt(INI_SECTION_MAIN, "log_transparent",      1, conf_fileName);
	s_log.auto_save_log    = GetPrivateProfileInt(INI_SECTION_MAIN, "log_auto_save",        0, conf_fileName);
	s_log.show_status_bar  = GetPrivateProfileInt(INI_SECTION_MAIN, "log_show_status_bar",  1, conf_fileName);
	s_log.taskbar_progress = GetPrivateProfileInt(INI_SECTION_MAIN, "log_taskbar_progress", 1, conf_fileName);
}

void guiEx_settings::load_append() {
	clear_append();
	GetPrivateProfileString(INI_SECTION_APPENDIX, "tc_appendix",   "_tc.txt",      s_append.tc,   sizeof(s_append.tc),   ini_fileName);
	GetPrivateProfileString(INI_SECTION_APPENDIX, "qp_appendix",   "_qp.txt",      s_append.qp,   sizeof(s_append.qp),   ini_fileName);
	GetPrivateProfileString(INI_SECTION_APPENDIX, "chap_appendix", "_chapter.txt", s_append.chap, sizeof(s_append.chap), ini_fileName);
	GetPrivateProfileString(INI_SECTION_APPENDIX, "wav_appendix",  "_tmp.wav",     s_append.wav,  sizeof(s_append.wav),  ini_fileName);
}

void guiEx_settings::load_fbc() {
	clear_fbc();
	s_fbc.calc_bitrate = GetPrivateProfileInt(   INI_SECTION_FBC, "calc_bitrate",       1, conf_fileName);
	s_fbc.initial_size = GetPrivateProfileDouble(INI_SECTION_FBC, "initial_size", "39.80", conf_fileName);
}

void guiEx_settings::save_local() {
	WritePrivateProfileInt(INI_SECTION_MAIN,    "large_cmdbox",          s_local.large_cmdbox,          conf_fileName);

	PathRemoveBlanks(s_local.custom_tmp_dir);
	PathRemoveBackslash(s_local.custom_tmp_dir);
	WritePrivateProfileString(INI_SECTION_MAIN, "custom_tmp_dir",        s_local.custom_tmp_dir,        conf_fileName);

	PathRemoveBlanks(s_local.custom_audio_tmp_dir);
	PathRemoveBackslash(s_local.custom_audio_tmp_dir);
	WritePrivateProfileString(INI_SECTION_MAIN, "custom_audio_tmp_dir",  s_local.custom_audio_tmp_dir,  conf_fileName);

	PathRemoveBlanks(s_local.custom_mp4box_tmp_dir);
	PathRemoveBackslash(s_local.custom_mp4box_tmp_dir);
	WritePrivateProfileString(INI_SECTION_MAIN, "custom_mp4box_tmp_dir", s_local.custom_mp4box_tmp_dir, conf_fileName);

	PathRemoveBlanks(s_local.stg_dir);
	PathRemoveBackslash(s_local.stg_dir);
	WritePrivateProfileString(INI_SECTION_MAIN, "stg_dir",               s_local.stg_dir,               conf_fileName);

	PathRemoveBlanks(s_local.app_dir);
	PathRemoveBackslash(s_local.app_dir);
	WritePrivateProfileString(INI_SECTION_MAIN, "last_app_dir",          s_local.app_dir,               conf_fileName);

	PathRemoveBlanks(s_x264.fullpath);
	PathRemoveBlanks(s_x264.fullpath_10bit);
	WritePrivateProfileString(INI_SECTION_X264,    "X264",           s_x264.fullpath,       conf_fileName);
	WritePrivateProfileString(INI_SECTION_X264,    "X264_10bit",     s_x264.fullpath_10bit, conf_fileName);
	for (int i = 0; i < s_aud_count; i++) {
		PathRemoveBlanks(s_aud[i].fullpath);
		WritePrivateProfileString(INI_SECTION_AUD, s_aud[i].keyName, s_aud[i].fullpath, conf_fileName);
	}
	for (int i = 0; i < s_mux_count; i++) {
		PathRemoveBlanks(s_mux[i].fullpath);
		WritePrivateProfileString(INI_SECTION_MUX, s_mux[i].keyName, s_mux[i].fullpath, conf_fileName);
	}
}

void guiEx_settings::save_log_win() {
	WritePrivateProfileInt(INI_SECTION_MAIN, "log_start_minimized",   s_log.minimized,        conf_fileName);
	WritePrivateProfileInt(INI_SECTION_MAIN, "log_transparent",       s_log.transparent,      conf_fileName);
	WritePrivateProfileInt(INI_SECTION_MAIN, "log_auto_save",         s_log.auto_save_log,    conf_fileName);
	WritePrivateProfileInt(INI_SECTION_MAIN, "log_show_status_bar",   s_log.show_status_bar,  conf_fileName);
	WritePrivateProfileInt(INI_SECTION_MAIN, "log_taskbar_progress",  s_log.taskbar_progress, conf_fileName);
}

void guiEx_settings::save_fbc() {
	WritePrivateProfileInt(   INI_SECTION_FBC, "calc_bitrate", s_fbc.calc_bitrate, conf_fileName);
	WritePrivateProfileDouble(INI_SECTION_FBC, "initial_size", s_fbc.initial_size, conf_fileName);
}

BOOL guiEx_settings::get_reset_s_x264_referesh() {
	BOOL refresh = s_x264_refresh;
	s_x264_refresh = FALSE;
	return refresh;
}

void guiEx_settings::clear_aud() {
	s_aud_mc.clear();
	s_aud_count = 0;
	s_aud_faw_index = FAW_INDEX_ERROR;
}

void guiEx_settings::clear_mux() {
	s_mux_mc.clear();
	s_mux_count = 0;
}

void guiEx_settings::clear_x264() {
	s_x264_mc.clear();
	s_x264_refresh = TRUE;
}

void guiEx_settings::clear_local() {
	ZeroMemory(&s_local, sizeof(s_local));
}

void guiEx_settings::clear_fn_replace() {
	fn_rep_mc.clear();
	fn_rep.clear();
}

void guiEx_settings::clear_log_win() {
	ZeroMemory(&s_log, sizeof(s_log));
}

void guiEx_settings::clear_fbc() {
	ZeroMemory(&s_fbc, sizeof(s_fbc));
}

void guiEx_settings::clear_append() {
	ZeroMemory(&s_append, sizeof(s_append));
}

void guiEx_settings::apply_fn_replace(char *target_filename, DWORD nSize) {
	foreach(std::vector<FILENAME_REPLACE>, it_rep, &fn_rep)
		replace(target_filename, nSize, it_rep->from, it_rep->to);
}