//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <Windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include "auo_util.h"
#include "auo_conf.h"
#include "auo_options.h"

const int guiEx_config::conf_block_data[CONF_BLOCK_COUNT] = {
	 sizeof(CONF_X264),
	 sizeof(CONF_VIDEO),
	 sizeof(CONF_AUDIO),
	 sizeof(CONF_MUX),
	 sizeof(CONF_OTHER)
};

const size_t guiEx_config::conf_block_pointer[CONF_BLOCK_COUNT] = {
	offsetof(CONF_X264GUIEX, x264),
	offsetof(CONF_X264GUIEX, vid),
	offsetof(CONF_X264GUIEX, aud),
	offsetof(CONF_X264GUIEX, mux),
	offsetof(CONF_X264GUIEX, oth)
};

guiEx_config::guiEx_config() { }

int guiEx_config::load_x264guiEx_conf(CONF_X264GUIEX *conf, const char *stg_file) {
	size_t conf_size = 0;
	BYTE *dst, *filedat;
	//初期化
	ZeroMemory(conf, sizeof(CONF_X264GUIEX));
	//ファイルからロード
	FILE *fp = NULL;
	if (fopen_s(&fp, stg_file, "rb") != NULL)
		return CONF_ERROR_FILE_OPEN;
	//設定ファイルチェック
	char conf_name[CONF_NAME_BLOCK_LEN + 32];
	fread(&conf_name, sizeof(char), CONF_NAME_BLOCK_LEN, fp);
	if (strcmp(CONF_NAME, conf_name)) {
		fclose(fp);
		return CONF_ERROR_FILE_OPEN;
	}
	fread(&conf_size, sizeof(int), 1, fp);
	BYTE *dat = (BYTE*)calloc(conf_size, 1);
	init_CONF_X264GUIEX(conf, FALSE);
	fseek(fp, 0, SEEK_SET);
	fread(dat, conf_size, 1, fp);
	fclose(fp);

	//ブロックサイズチェック
	if (((CONF_X264GUIEX *)dat)->block_count > CONF_BLOCK_COUNT)
		return CONF_ERROR_BLOCK_SIZE;

	//サイズ情報部分のコピーはしない
	dst = (BYTE *)conf;
	//filedat = (BYTE *)data;
	//memcpy(dst, filedat, data->head_size);
	dst += CONF_HEAD_SIZE;

	//ブロック部分のコピー
	for (int i = 0; i < ((CONF_X264GUIEX *)dat)->block_count; ++i) {
		filedat = dat + ((CONF_X264GUIEX *)dat)->block_head_p[i];
		memcpy(dst, filedat, ((CONF_X264GUIEX *)dat)->block_size[i]);
		dst += conf_block_data[i];
	}

	//初期化するかどうかで使うので。
	conf->size_all = CONF_INITIALIZED;
	free(dat);
	return 0;
}

int guiEx_config::save_x264guiEx_conf(const CONF_X264GUIEX *conf, const char *stg_file) {
	CONF_X264GUIEX save_conf;
	memcpy(&save_conf, conf, sizeof(CONF_X264GUIEX));
	ZeroMemory(&save_conf.block_count, sizeof(save_conf.block_count));

	//展開したコマンドライン
	char cmd_all[MAX_CMD_LEN] = { 0 };
	build_cmd_from_conf(cmd_all, sizeof(cmd_all), &conf->x264, &conf->vid, TRUE);
	DWORD cmd_all_len = strlen(cmd_all) + 1;

	//設定ファイルのブロックごとの大きさを書き込み
	sprintf_s(save_conf.conf_name, sizeof(save_conf.conf_name), CONF_NAME);
	save_conf.size_all = sizeof(CONF_X264GUIEX) + cmd_all_len;
	save_conf.head_size = CONF_HEAD_SIZE;
	save_conf.block_count = CONF_BLOCK_COUNT;
	for (int i = 0; i < CONF_BLOCK_COUNT; ++i) {
		save_conf.block_size[i] = conf_block_data[i];
		save_conf.block_head_p[i] = conf_block_pointer[i];
	}
	//最後に展開したコマンドラインを追加する
	save_conf.block_size[CONF_BLOCK_COUNT]   = cmd_all_len;
	save_conf.block_head_p[CONF_BLOCK_COUNT] = sizeof(CONF_X264GUIEX);

	//ファイルへ書きこみ
	FILE *fp = NULL;
	if (fopen_s(&fp, stg_file, "wb") != NULL)
		return CONF_ERROR_FILE_OPEN;
	fwrite(&save_conf, sizeof(CONF_X264GUIEX), 1, fp);
	fwrite(cmd_all,    cmd_all_len,            1, fp);
	fclose(fp);
	return 0;
}