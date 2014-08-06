//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _AUO_SETTINGS_H_
#define _AUO_SETTINGS_H_

#include <Windows.h>
#include <vector>
#include "auo.h"

typedef struct {
	char *name; //x264でのオプション名
	WCHAR *desc; //GUIでの表示用
} X264_OPTION_STR;

const int FAW_INDEX_ERROR = -1;

const int AUTO_SAVE_LOG_OUTPUT_DIR = 0;
const int AUTO_SAVE_LOG_CUSTOM = 1;

//メモリーを切り刻みます。
class mem_cutter {
private:
	char *init_ptr;
	char *mp;
	size_t mp_init_size;
	size_t mp_size;
public:
	mem_cutter() {
		mp = NULL;
		init_ptr = NULL;
		mp_init_size = 0;
		mp_size = mp_init_size;
	};
	~mem_cutter() {
		clear();
	};
	void init(int size) {
		clear();
		mp_init_size = size;
		mp_size = mp_init_size;
		init_ptr = (char*)calloc(mp_init_size, 1);
		mp = init_ptr;
	};
	void clear() {
		if (init_ptr) free(init_ptr); init_ptr = NULL;
		mp = NULL;
		mp_size = 0;
	};
	void *CutMem(int size) {
		if (mp_size - size < 0)
			return NULL;
		void *ptr = mp;
		mp += size;
		mp_size -= size;
		return ptr;
	};
	char *SetPrivateProfileString(const char *section, const char *keyname, const char *defaultString, const char *ini_file) {
		if (mp_size > 0) {
			size_t len = GetPrivateProfileString(section, keyname, defaultString, mp, mp_size, ini_file) + 1;
			char *ptr = mp;
			mp += len;
			mp_size -= len;
			return ptr;
		} else {
			return NULL;
		}
	};
	void *GetPtr() {
		return mp;
	};
	size_t GetRemain() {
		return mp_size;
	};
	void CutString() {
		size_t len = strlen(mp) + 1;
		mp += len;
		mp_size -= len;
	};
};

typedef struct {
	char *name;          //名前
	char *cmd;           //コマンドライン
	BOOL bitrate;        //ビットレート指定モード
	int bitrate_min;     //ビットレートの最小値
	int bitrate_max;     //ビットレートの最大値
	int bitrate_default; //ビットレートのデフォルト値
	int bitrate_step;    //クリックでの変化幅
	int enc_2pass;       //2passエンコを行う
	int use_8bit;        //8bitwavを入力する
	char *disp_list;     //表示名のリスト
	char *cmd_list;      //コマンドラインのリスト
} AUDIO_ENC_MODE;

typedef struct {
	char *keyName;               //iniファイルでのセクション名
	char *dispname;              //名前
	char *filename;              //拡張子付き名前
	char fullpath[MAX_PATH_LEN]; //エンコーダの場所(フルパス)
	char *aud_appendix;          //作成する音声ファイル名に追加する文字列
	int pipe_input;              //パイプ入力が可能
	char *cmd_base;              //1st pass用コマンドライン
	char *cmd_2pass;             //2nd pass用コマンドライン
	int mode_count;              //エンコードモードの数
	AUDIO_ENC_MODE *mode;        //エンコードモードの設定
} AUDIO_SETTINGS;

typedef struct {
	char *name;  //拡張オプションの名前
	char *cmd;   //拡張オプションのコマンドライン
} MUXER_CMD_EX;

typedef struct {
	char *keyName;                //iniファイルでのセクション名
	char *dispname;               //名前
	char *filename;               //拡張子付き名前
	char fullpath[MAX_PATH_LEN];  //エンコーダの場所(フルパス)
	char *base_cmd;               //もととなるコマンドライン
	char *aud_cmd;                //音声mux用のコマンドライン
	char *tc_cmd;                 //タイムコードmux用のコマンドライン
	char *tmp_cmd;                //一時フォルダ指定用コマンドライン
	int ex_count;                 //拡張オプションの数
	MUXER_CMD_EX *ex_cmd;         //拡張オプション
} MUXER_SETTINGS;

typedef struct {
	X264_OPTION_STR *name;  //各種設定用x264コマンドの名前(配列、最後はnull)
	char **cmd;   //各種設定用x264コマンド(配列、最後はnull)
} X264_CMD;

typedef struct {
	char fullpath[MAX_PATH_LEN];       //x264の場所(フルパス)
	char fullpath_10bit[MAX_PATH_LEN]; //x264の場所(フルパス) 10bit用
	char *default_cmd;                 //デフォルト設定用コマンドライン
	char *default_cmd_10bit;           //10bit depthデフォルト設定用追加コマンドライン
	int preset_count;                  //presetの数
	int tune_count;                    //tuneの数
	int profile_count;                 //profileの数
	int default_preset;                //デフォルトpresetのインデックス
	int default_tune;                  //デフォルトtuneのインデックス
	int default_profile;               //デフォルトprofileのインデックス
	X264_CMD preset;                   //presetコマンドライン集
	X264_CMD tune;                     //tuneコマンドライン集
	X264_CMD profile;                  //profileコマンドライン集
	float *profile_vbv_multi;          //profileによるvbv倍率
} X264_SETTINGS;

typedef struct {
	char *from; //置換元文字列
	char *to;   //置換先文字列
} FILENAME_REPLACE;

typedef struct {
	BOOL minimized;                        //最小化で起動
	BOOL transparent;                      //半透明で表示
	BOOL auto_save_log;                    //ログ自動保存を行うかどうか
	int  auto_save_log_mode;               //ログ自動保存のモード
	char auto_save_log_path[MAX_PATH_LEN]; //ログ自動保存ファイル名
	BOOL show_status_bar;                  //ステータスバーの表示
	BOOL taskbar_progress;                 //タスクバーに進捗を表示
} LOG_WINDOW_SETTINGS;

typedef struct {
	BOOL   calc_bitrate;  //ビットレート計算モード
	double initial_size;  //初期サイズ
} BITRATE_CALC_SETTINGS;

typedef struct {
	BOOL  large_cmdbox;                        //拡大サイズでコマンドラインプレビューを行う
	DWORD audio_buffer_size;                   //音声用バッファサイズ
	BOOL  auto_afs_disable;                    //自動的にafsを無効化
	BOOL  auto_del_stats;                      //自動マルチパス時、ステータスファイルを自動的に削除
	char  custom_tmp_dir[MAX_PATH_LEN];        //一時フォルダ
	char  custom_audio_tmp_dir[MAX_PATH_LEN];  //音声用一時フォルダ
	char  custom_mp4box_tmp_dir[MAX_PATH_LEN]; //mp4box用一時フォルダ
	char  stg_dir[MAX_PATH_LEN];               //プロファイル設定ファイル保存フォルダ
	char  app_dir[MAX_PATH_LEN];               //実行ファイルのフォルダ
} LOCAL_SETTINGS;

typedef struct {
	char aud[MAX_APPENDIX_LEN];  //音声ファイル名に追加する文字列
	char tc[MAX_APPENDIX_LEN];   //タイムコードファイル名に追加する文字列
	char qp[MAX_APPENDIX_LEN];   //qpファイル名に追加する文字列
	char chap[MAX_APPENDIX_LEN]; //チャプターファイル名に追加する文字列
	char wav[MAX_APPENDIX_LEN];  //一時wavファイル名に追加する文字列
} FILE_APPENDIX;

class guiEx_settings {
private:
	mem_cutter s_x264_mc;
	mem_cutter fn_rep_mc;
	mem_cutter s_aud_mc;
	mem_cutter s_mux_mc;

	static BOOL  init;                        //静的確保したものが初期化
	static char  auo_path[MAX_PATH_LEN];      //自分(auo)のフルパス
	static char  ini_fileName[MAX_PATH_LEN];  //iniファイル(読み込み用)の場所
	static char  conf_fileName[MAX_PATH_LEN]; //configファイル(読み書き用)の場所
	static DWORD ini_filesize;                //iniファイル(読み込み用)のサイズ

	void load_x264_cmd(X264_CMD *x264cmd, int *count, int *default_index, const char *section);  //x264コマンドライン設定の読み込み
	void clear_x264_cmd(X264_CMD *x264cmd, int count);                                             //x264コマンドライン設定の消去

	void load_aud();          //音声エンコーダ関連の設定の読み込み・更新
	void load_mux();          //muxerの設定の読み込み・更新
	void load_x264();         //x264関連の設定の読み込み・更新
	void load_local();        //ファイルの場所等の設定の読み込み・更新

	int get_faw_index();             //FAWのインデックスを取得する
	BOOL s_x264_refresh;             //x264設定の再ロード

	void make_default_stg_dir(char *default_stg_dir, DWORD nSize); //プロファイル設定ファイルの保存場所の作成
	BOOL check_inifile();            //iniファイルが読めるかテスト

public:
	int s_aud_count;                 //音声エンコーダの数
	int s_mux_count;                 //muxerの数 (基本3固定)
	AUDIO_SETTINGS *s_aud;           //音声エンコーダの設定
	MUXER_SETTINGS *s_mux;           //muxerの設定
	X264_SETTINGS  s_x264;           //x264関連の設定
	LOCAL_SETTINGS s_local;          //ファイルの場所等
	std::vector<FILENAME_REPLACE> fn_rep;  //一時ファイル名置換
	LOG_WINDOW_SETTINGS s_log;       //ログウィンドウ関連の設定
	FILE_APPENDIX s_append;          //各種ファイルに追加する名前
	BITRATE_CALC_SETTINGS s_fbc;    //簡易ビットレート計算機設定

	int s_aud_faw_index;            //FAWのインデックス

	guiEx_settings();
	guiEx_settings(BOOL disable_loading);
	~guiEx_settings();

	BOOL get_init_success();  //iniファイルが存在し、正しいバージョンだったか
	void load_encode_stg();   //映像・音声・動画関連の設定の読み込み・更新
	void load_fn_replace();   //一時ファイル名置換等の設定の読み込み・更新
	void load_log_win();      //ログウィンドウ等の設定の読み込み・更新
	void load_append();       //各種ファイルの設定の読み込み・更新
	void load_fbc();          //簡易ビットレート計算機設定の読み込み・更新

	void save_local();        //ファイルの場所等の設定の保存
	void save_log_win();      //ログウィンドウ等の設定の保存
	void save_fbc();          //簡易ビットレート計算機設定の保存

	void apply_fn_replace(char *target_filename, DWORD nSize);  //一時ファイル名置換の適用

	BOOL get_reset_s_x264_referesh(); //s_x264が更新されたか

private:
	void initialize(BOOL disable_loading);

	void clear_aud();         //音声エンコーダ関連の設定の消去
	void clear_mux();         //muxerの設定の消去
	void clear_x264();        //x264関連の設定の消去
	void clear_local();       //ファイルの場所等の設定の消去
	void clear_fn_replace();  //一時ファイル名置換等の消去
	void clear_log_win();     //ログウィンドウ等の設定の消去
	void clear_append();      //各種ファイルの設定の消去
	void clear_fbc();         //簡易ビットレート計算機設定のクリア
};

#endif //_AUO_SETTINGS_H_