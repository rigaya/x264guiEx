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
#include <stdlib.h>
#include <Math.h>
#include <limits.h>
#include <vector>

#include "h264_level.h"
#include "auo_util.h"
#include "auo_conf.h"
#include "auo_options.h"


//オプションの種類(色々便利なので1から始める)
static const DWORD OPTION_TYPE_BOOL             =  1;
static const DWORD OPTION_TYPE_BOOL_REVERSE     =  2;
static const DWORD OPTION_TYPE_INT              =  3;
static const DWORD OPTION_TYPE_FLOAT            =  4;
static const DWORD OPTION_TYPE_INT2             =  5;
static const DWORD OPTION_TYPE_FLOAT2           =  6;
static const DWORD OPTION_TYPE_BOOL2_REVERSE    =  7;
static const DWORD OPTION_TYPE_LIST             =  8;
//個別
static const DWORD OPTION_TYPE_CRF              =  9;
static const DWORD OPTION_TYPE_BITRATE          = 10;
static const DWORD OPTION_TYPE_QP               = 11;
static const DWORD OPTION_TYPE_KEYINT           = 12;
static const DWORD OPTION_TYPE_DEBLOCK          = 13;
static const DWORD OPTION_TYPE_CQM              = 14;
static const DWORD OPTION_TYPE_TCFILE_IN        = 15;
static const DWORD OPTION_TYPE_INPUT_DEPTH      = 16;
static const DWORD OPTION_TYPE_PASS             = 17;
static const DWORD OPTION_TYPE_MB_PARTITION     = 18;
static const DWORD OPTION_TYPE_TFF              = 19;
static const DWORD OPTION_TYPE_BFF              = 20;
static const DWORD OPTION_TYPE_TIMEBASE         = 21;
static const DWORD OPTION_TYPE_LEVEL            = 22;

//値を取らないオプションタイプのリスト
static const DWORD OPTION_NO_VALUE[] = { 
	OPTION_TYPE_BOOL, 
	OPTION_TYPE_BOOL_REVERSE, 
	OPTION_TYPE_BOOL2_REVERSE,
	OPTION_TYPE_TFF,
	OPTION_TYPE_BFF,
	NULL
};

//オプション解析用の設定
static guiEx_settings *ex_stg;

static X264_OPTIONS x264_options_table[] = {
	{ "input-depth",      "",   OPTION_TYPE_INPUT_DEPTH,   NULL,                 offsetof(CONF_X264, use_10bit_depth) },
	{ "output-csp",       "",   OPTION_TYPE_LIST,          list_output_csp,      offsetof(CONF_X264, output_csp     ) },
	{ "pass",             "p",  OPTION_TYPE_PASS,          NULL,                 offsetof(CONF_X264, pass           ) },
	{ "slow-firstpass",   "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, slow_first_pass) },
	//{ "stats",            "",   OPTION_TYPE_STATS,         NULL,                 NULL                                  },
	{ "preset",           "",   OPTION_TYPE_LIST,          NULL,                 offsetof(CONF_X264, preset         ) },
	{ "tune",             "",   OPTION_TYPE_LIST,          NULL,                 offsetof(CONF_X264, tune           ) },
	{ "profile",          "",   OPTION_TYPE_LIST,          NULL,                 offsetof(CONF_X264, profile        ) },
	{ "crf",              "",   OPTION_TYPE_CRF,           NULL,                 NULL                                 },
	{ "bitrate",          "B",  OPTION_TYPE_BITRATE,       NULL,                 NULL                                 },
	{ "qp",               "q",  OPTION_TYPE_QP,            NULL,                 NULL                                 },
	{ "ipratio",         "",    OPTION_TYPE_FLOAT,         NULL,                 offsetof(CONF_X264, ip_ratio       ) },
	{ "pbratio",         "",    OPTION_TYPE_FLOAT,         NULL,                 offsetof(CONF_X264, pb_ratio       ) },
	{ "qpmin",            "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, qp_min         ) },
	{ "qpmax",            "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, qp_max         ) },
	{ "qpstep",           "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, qp_step        ) },
	{ "qcomp",            "",   OPTION_TYPE_FLOAT,         NULL,                 offsetof(CONF_X264, qp_compress    ) },
	{ "chroma-qp-offset", "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, chroma_qp_offset) },
	{ "no-mbtree",        "",   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_X264, mbtree         ) },
	{ "mbtree",           "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, mbtree         ) },
	{ "rc-lookahead",     "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, rc_lookahead   ) },
	{ "vbv-bufsize",      "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, vbv_bufsize    ) },
	{ "vbv-maxrate",      "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, vbv_maxrate    ) },
	{ "aq-mode",          "",   OPTION_TYPE_INT,           list_aq,              offsetof(CONF_X264, aq_mode        ) },
	{ "aq-strength",      "",   OPTION_TYPE_FLOAT,         NULL,                 offsetof(CONF_X264, aq_strength    ) },
	{ "psy-rd",           "",   OPTION_TYPE_FLOAT2,        NULL,                 offsetof(CONF_X264, psy_rd         ) },
	{ "psy",              "",   OPTION_TYPE_FLOAT2,        NULL,                 offsetof(CONF_X264, psy_rd         ) },
	{ "no-psy",           "",   OPTION_TYPE_BOOL2_REVERSE, NULL,                 offsetof(CONF_X264, psy_rd         ) },
	{ "scenecut",         "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, scenecut       ) },
	{ "no-scenecut",      "",   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_X264, scenecut       ) },
	{ "keyint",           "I",  OPTION_TYPE_KEYINT,        NULL,                 offsetof(CONF_X264, keyint_max     ) },
	{ "min-keyint",       "i",  OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, keyint_min     ) },
	{ "open-gop",         "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, open_gop       ) },
	{ "no-cabac",         "",   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_X264, cabac          ) },
	{ "cabac",            "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, cabac          ) },
	{ "bframes",          "b",  OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, bframes        ) },
	{ "b-adapt",          "",   OPTION_TYPE_INT,           list_b_adpat,         offsetof(CONF_X264, b_adapt        ) },
	{ "no-b-adapt",       "",   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_X264, b_adapt        ) },
	{ "b-bias",           "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, b_bias         ) },
	{ "b-pyramid",        "",   OPTION_TYPE_LIST,          list_b_pyramid,       offsetof(CONF_X264, b_pyramid      ) },
	{ "slices",           "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, slice_n        ) },
	{ "deblock",          "f",  OPTION_TYPE_DEBLOCK,       NULL,                 NULL                                 },
	{ "no-deblock",       "",   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_X264, use_deblock    ) },
	{ "no-interlace",     "",   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_X264, interlaced     ) },
	{ "tff",              "",   OPTION_TYPE_TFF,           NULL,                 NULL                                 },
	{ "bff",              "",   OPTION_TYPE_BFF,           NULL,                 NULL                                 },
	{ "partitions",       "A",  OPTION_TYPE_MB_PARTITION,  NULL,                 offsetof(CONF_X264, mb_partition   ) },
	{ "no-8x8dct",        "",   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_X264, dct8x8         ) },
	{ "8x8dct",           "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, dct8x8         ) },
	{ "me",               "",   OPTION_TYPE_LIST,          list_me,              offsetof(CONF_X264, me             ) },
	{ "subme",            "m",  OPTION_TYPE_INT,           list_subme,           offsetof(CONF_X264, subme          ) },
	{ "merange",          "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, me_range       ) },
	{ "no-chroma-me",     "",   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_X264, chroma_me      ) },
	{ "chroma-me",        "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, chroma_me      ) },
	{ "direct",           "",   OPTION_TYPE_LIST,          list_direct,          offsetof(CONF_X264, direct_mv      ) },
	{ "ref",              "r",  OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, ref_frames     ) },
	{ "no-mixed-ref",     "",   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_X264, mixed_ref      ) },
	{ "mixed-ref",        "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, mixed_ref      ) },
	{ "no-weightb",       "",   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_X264, weight_b       ) },
	{ "weightb",          "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, weight_b       ) },
	{ "weightp",          "",   OPTION_TYPE_INT,           list_weightp,         offsetof(CONF_X264, weight_p       ) },
	{ "nr",               "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, noise_reduction) },
	{ "no-fast-pskip",    "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, no_fast_pskip  ) },
	{ "no-dct-decimate",  "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, no_dct_decimate) },
	{ "trellis",          "t",  OPTION_TYPE_INT,           list_trellis,         offsetof(CONF_X264, trellis        ) },
	{ "cqm",              "",   OPTION_TYPE_CQM,           list_cqm,             offsetof(CONF_X264, cqm            ) },
	//{ "cqmfile",          "",   OPTION_TYPE_CQMFILE,       NULL,                 NULL                                 },
	{ "colormatrix",      "",   OPTION_TYPE_LIST,          list_colormatrix,     offsetof(CONF_X264, colormatrix    ) },
	{ "colorprim",        "",   OPTION_TYPE_LIST,          list_colorprim,       offsetof(CONF_X264, colorprim      ) },
	{ "transfer",         "",   OPTION_TYPE_LIST,          list_transfer,        offsetof(CONF_X264, transfer       ) },
	{ "input-range",      "",   OPTION_TYPE_LIST,          list_input_range,     offsetof(CONF_X264, input_range    ) },
	{ "sar",              "",   OPTION_TYPE_INT2,          NULL,                 offsetof(CONF_X264, sar            ) },
	{ "level",            "",   OPTION_TYPE_LEVEL,         list_x264guiEx_level, offsetof(CONF_X264, h264_level     ) },
	{ "videoformat",      "",   OPTION_TYPE_LIST,          list_videoformat,     offsetof(CONF_X264, videoformat    ) },
	{ "aud",              "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, aud            ) },
	{ "pic-struct",       "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, pic_struct     ) },
	{ "nal-hrd",          "",   OPTION_TYPE_LIST,          list_nal_hrd,         offsetof(CONF_X264, nal_hrd        ) },
	{ "bluray-compat",    "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, bluray_compat  ) },
	{ "threads",          "",   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_X264, threads        ) },
	{ "sliced-threads",   "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, sliced_threading) },
	{ "log-level",        "",   OPTION_TYPE_LIST,          list_log_type,       offsetof(CONF_X264, log_mode       ) },
	{ "psnr",             "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, psnr           ) },
	{ "ssim",             "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, ssim           ) },
	{ "tcfile-in",        "",   OPTION_TYPE_TCFILE_IN,     NULL,                 NULL                                 },
	{ "timebase",         "",   OPTION_TYPE_TIMEBASE,      NULL,                 NULL                                 },
	{ "progress",         "",   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_X264, disable_progress) },
	{ "no-progress",      "",   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_X264, disable_progress) },
	{ NULL,               NULL, NULL,                      NULL,                 NULL                                 }
};

static BOOL x264guiEx_strtol(int *i, const char *str, DWORD len) {
	char *eptr = NULL;
	int v;
	BOOL ret = TRUE;
	if (len == NULL) len = ULONG_MAX;
	if (*str != '{') {
		v = strtol(str, &eptr, 0);
		if (*eptr == '\0' || (DWORD)(eptr - str) == len) { *i = v; } else { ret = FALSE; }
	} else {
		str++;
		BOOL multi = (*str == '*');
		v = strtol(str + multi, &eptr, 0);
		if (*eptr == '}') { (multi) ? *i *= v : *i += v; } else { ret = FALSE; }
	}
	return ret;
}

static BOOL x264guiEx_strtof(float *f, const char *str, DWORD len) {
	char *eptr = NULL;
	float v;
	BOOL ret = TRUE;
	if (len == NULL) len = ULONG_MAX;
	if (*str != '{') {
		v = (float)strtod(str, &eptr);
		if (*eptr == '\0' || (DWORD)(eptr - str) == len) { *f = v; } else { ret = FALSE; }
	} else {
		str++;
		BOOL multi = (*str == '*');
		v = (float)strtod(str + multi, &eptr);
		if (*eptr == '}') { (multi) ? *f *= v : *f += v; } else { ret = FALSE; }
	}
	return ret;
}

static BOOL x264guiEx_parse_int(int *i, const char *value, DWORD len) {
	BOOL ret;
	if ((ret = x264guiEx_strtol(i, value, len)) == FALSE) {
		size_t len = strlen(value);
		if (*value == '[' && value[len-1] == ']') {
			const char *a, *b, *c;
			if ((a = strstr(value, "if>")) != NULL && (b = strstr(value, "else")) != NULL) {
				int v;
				c = a + strlen("if>");
				ret |= x264guiEx_strtol(&v, c, b-c);
				b += strlen("else");
				if (*i > v)
					c = value+1, len = a - c;
				else
					c = b, len = (value + len - 1) - c;
				ret &= x264guiEx_strtol(i, c, len);
			}
		}
	}
	return ret;
}

static BOOL x264guiEx_parse_float(float *f, const char *value, DWORD len) {
	BOOL ret;
	if ((ret = x264guiEx_strtof(f, value, len)) == FALSE) {
		size_t len = strlen(value);
		if (*value == '[' && value[len-1] == ']') {
			const char *a, *b, *c;
			if ((a = strstr(value, "if>")) != NULL && (b = strstr(value, "else")) != NULL) {
				float v;
				c = a + strlen("if>");
				ret |= x264guiEx_strtof(&v, c, b-c);
				b += strlen("else");
				if (*f > v)
					c = value+1, len = a - c;
				else
					c = b, len = (value + len - 1) - c;
				ret &= x264guiEx_strtof(f, c, len);
			}
		}
	}
	return ret;
}

//以下部分的にwarning C4100を黙らせる
//C4100 : 引数は関数の本体部で 1 度も参照されません。
#pragma warning( push )
#pragma warning( disable: 4100 )

static BOOL set_bool(void *b, const char *value, const X264_OPTION_STR *list) {
	*(BOOL*)b = TRUE;
	return TRUE;
}

static BOOL set_bool_reverse(void *b, const char *value, const X264_OPTION_STR *list) {
	*(BOOL*)b = FALSE;
	return TRUE;
}

static BOOL set_bool2_reverse(void *b, const char *value, const X264_OPTION_STR *list) {
	((INT2*)b)->x = FALSE;
	((INT2*)b)->y = FALSE;
	return TRUE;
}

static BOOL set_int(void *i, const char *value, const X264_OPTION_STR *list) {
	return x264guiEx_parse_int((int *)i, value, NULL);
}

static BOOL set_float(void *f, const char *value, const X264_OPTION_STR *list) {
	return x264guiEx_parse_float((float *)f, value, NULL);
}

static BOOL set_int2(void *i, const char *value, const X264_OPTION_STR *list) {
	const size_t len = strlen(value);
	//一度値をコピーして分析
	BOOL ret = FALSE;
	for (size_t j = 0; j < len; j++) {
		if (value[j] == ':' || value[j] == '|' || value[j] == ',' || value[j] == '/') {
			ret = TRUE;
			if (!(j == strlen("<unset>") && _strnicmp(value, "<unset>", strlen("<unset>")) == NULL))
				ret &= x264guiEx_parse_int(&((INT2 *)i)->x, value, j);
			if (_stricmp(&value[j+1], "<unset>") != NULL)
				ret &= x264guiEx_parse_int(&((INT2 *)i)->y, &value[j+1], 0);
			break;
		}
	}
	return ret;
}

static BOOL set_float2(void *f, const char *value, const X264_OPTION_STR *list) {
	const size_t len = strlen(value);
	BOOL ret = FALSE;
	for (size_t j = 0; j < len; j++) {
		if (value[j] == ':' || value[j] == '|' || value[j] == ',' || value[j] == '/') {
			ret = TRUE;
			if (!(j == strlen("<unset>") && _strnicmp(value, "<unset>", strlen("<unset>")) == NULL))
				ret &= x264guiEx_parse_float(&((FLOAT2 *)f)->x, value, j);
			if (_stricmp(&value[j+1], "<unset>") != NULL)
				ret &= x264guiEx_parse_float(&((FLOAT2 *)f)->y, &value[j+1], 0);
			break;
		}
	}
	return ret;
}

static BOOL set_list(void *i, const char *value, const X264_OPTION_STR *list) {
	BOOL ret = FALSE;
	for (int j = 0; list[j].name; j++)
		if (_stricmp(value, list[j].name) == NULL) {
			*(int*)i = j;
			ret = TRUE;
			break;
		}
	//数値での指定に対応
	//int k = -1;
	//x264guiEx_parse_int(&k, value);
	//if (-1 < k && k < j)
	//	*(int*)i = k;
	return ret; 
}

static BOOL set_crf(void *cx, const char *value, const X264_OPTION_STR *list) {
	((CONF_X264 *)cx)->rc_mode = X264_RC_CRF;
	float f = 23.0f;
	x264guiEx_strtof(&f, value, NULL);
	((CONF_X264 *)cx)->crf = (int)(f * 100 + 0.5);
	return TRUE;
}
static BOOL set_bitrate(void *cx, const char *value, const X264_OPTION_STR *list) {
	((CONF_X264 *)cx)->rc_mode = X264_RC_BITRATE;
	return x264guiEx_strtol(&((CONF_X264 *)cx)->bitrate, value, NULL);
}
static BOOL set_qp(void *cx, const char *value, const X264_OPTION_STR *list) {
	((CONF_X264 *)cx)->rc_mode = X264_RC_QP;
	return x264guiEx_strtol(&((CONF_X264 *)cx)->qp, value, NULL);
}
static BOOL set_keyint(void *i, const char *value, const X264_OPTION_STR *list) {
	if ((*(int*)i = _stricmp(value, "infinite")) != NULL)
		return x264guiEx_parse_int((int *)i, value, NULL);
	return TRUE;
}
static BOOL set_deblock(void *cx, const char *value, const X264_OPTION_STR *list) {
	((CONF_X264 *)cx)->use_deblock = TRUE;
	return set_int2(&((CONF_X264 *)cx)->deblock, value, list);
}
static BOOL set_input_depth(void *b, const char *value, const X264_OPTION_STR *list) {
	*(BOOL*)b = (atoi(value) == 10) ? TRUE : FALSE;
	return TRUE;
}
static BOOL set_mb_partitions(void *cx, const char *value, const X264_OPTION_STR *list) {
	BOOL ret = TRUE;
	*(DWORD*)cx = MB_PARTITION_NONE;
	if (stristr(value, "all")) {
		*(DWORD*)cx = MB_PARTITION_ALL;
	} else if (stristr(value, "none")) {
		;//なにもしない
	} else {
		BOOL fin = FALSE;
		for (const char *p = value, *q = value; !fin; p++) {
			if (*p == '\0') fin = TRUE;
			if (fin || *p == ',' || *p == '/' || *p == ';' || *p == ':') {
				int len = p - q;
				if (     !_strnicmp(q, "p8x8", len))
					*(DWORD*)cx |= MB_PARTITION_P8x8;
				else if (!_strnicmp(q, "b8x8", len))
					*(DWORD*)cx |= MB_PARTITION_B8x8;
				else if (!_strnicmp(q, "p4x4", len))
					*(DWORD*)cx |= MB_PARTITION_P4x4;
				else if (!_strnicmp(q, "i8x8", len))
					*(DWORD*)cx |= MB_PARTITION_I8x8;
				else if (!_strnicmp(q, "i4x4", len))
					*(DWORD*)cx |= MB_PARTITION_I4x4;
				else
					ret = FALSE;
				q = p + 1;
			}
		}
	}
	return ret;
}
static BOOL set_tff(void *cx, const char *value, const X264_OPTION_STR *list) {
	((CONF_X264 *)cx)->interlaced = TRUE;
	((CONF_X264 *)cx)->tff = TRUE;
	return TRUE;
}
static BOOL set_bff(void *cx, const char *value, const X264_OPTION_STR *list) {
	((CONF_X264 *)cx)->interlaced = TRUE;
	((CONF_X264 *)cx)->tff = FALSE;
	return TRUE;
}
static BOOL set_timebase(void *cx, const char *value, const X264_OPTION_STR *list) {
	((CONF_X264 *)cx)->use_timebase = TRUE;
	return set_int2(&((CONF_X264 *)cx)->timebase, value, list);
}
static BOOL set_level(void *cx, const char *value, const X264_OPTION_STR *list) {
	BOOL ret = FALSE;
	size_t len = strlen(value);
	const char *tmp = value + len - 1;
	if (*tmp != '0')
		ret = set_list(cx, value, list);
	if (!ret) {
		//"5.0"とかを"5"に整形
		char *copy = (char *)malloc((len + 1) * sizeof(copy[0]));
		memcpy(copy, value, (len + 1) * sizeof(copy[0]));
		char *p = copy + len - 1;
		while (*p == '0' && p >= copy)
			p--;
		if (*p == '.') p--; //最後に'.'が残ったら消す
		*(p + 1) = '\0';
		ret = set_list(cx, copy, list);
		free(copy);
	}
	//整数指定を解析
	if (!ret) {
		int i = 0;
		if (x264guiEx_strtol(&i, value, NULL)) {
			char buf[16];
			if (i == 9)
				strcpy_s(buf, _countof(buf), "1b");
			else {
				size_t len = sprintf_s(buf, _countof(buf), "%.1f", i / 10.0);
				char *p = buf + len - 1;
				while (*p == '0' && p >= buf)
					p--;
				if (*p == '.') p--; //最後に'.'が残ったら消す
				*(p + 1) = '\0';
			}
			ret = set_list(cx, buf, list);
		}
	}
	return ret;
}
static BOOL set_do_nothing(void *cx, const char *value, const X264_OPTION_STR *list) {
	return FALSE;
}


static void write_bool(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	BOOL *bptr = (BOOL*)((BYTE*)cx + options->p_offset);
	BOOL *defptr = (BOOL*)((BYTE*)def + options->p_offset);
	if ((write_all || *bptr != *defptr) && *bptr)
		sprintf_s(cmd, nSize, " --%s", options->long_name);
}

static void write_bool_reverse(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	BOOL *bptr = (BOOL*)((BYTE*)cx + options->p_offset);
	BOOL *defptr = (BOOL*)((BYTE*)def + options->p_offset);
	if ((write_all || *bptr != *defptr) && !(*bptr))
		sprintf_s(cmd, nSize, " --%s", options->long_name);
}

static void write_bool2_reverse(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	INT2 *bptr = (INT2*)((BYTE*)cx + options->p_offset);
	if (!(bptr->x | bptr->y))
		sprintf_s(cmd, nSize, " --%s", options->long_name);
}

static void write_int(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	int *iptr = (int*)((BYTE*)cx + options->p_offset);
	int *defptr = (int*)((BYTE*)def + options->p_offset);
	if (write_all || *iptr != *defptr)
		sprintf_s(cmd, nSize, " --%s %d", options->long_name, *iptr);
}
//小数表示の鬱陶しい0を消す
static inline void write_float_ex(char *cmd, size_t nSize, float f) {
	size_t last_len = strlen(cmd);
	double d = (int)(f * 1000.0 + 0.5 - (f<0)) / 1000.0; //これを入れないと22.2が22.19999とかになる
	sprintf_s(cmd + last_len, nSize - last_len, "%lf", d);
	char *p = cmd + strlen(cmd) - 1;
	while (*p == '0' && p >= cmd + last_len)
		p--;
	if (*p == '.') p--; //最後に'.'が残ったら消す
	*(p + 1) = '\0';
}

static void write_float(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	float *fptr = (float*)((BYTE*)cx + options->p_offset);
	float *defptr = (float*)((BYTE*)def + options->p_offset);
	if (write_all || abs(*fptr - *defptr) > EPS_FLOAT) {
		sprintf_s(cmd, nSize, " --%s ", options->long_name);
		write_float_ex(cmd, nSize, *fptr);
	}
}

static void write_int2(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	INT2 *iptr = (INT2*)((BYTE*)cx + options->p_offset);
	INT2 *defptr = (INT2*)((BYTE*)def + options->p_offset);
	if (write_all || iptr->x != defptr->x || iptr->y != defptr->y)
		sprintf_s(cmd, nSize, " --%s %d:%d", options->long_name, iptr->x, iptr->y);
}

static void write_float2(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	FLOAT2 *fptr = (FLOAT2*)((BYTE*)cx + options->p_offset);
	FLOAT2 *defptr = (FLOAT2*)((BYTE*)def + options->p_offset);
	if (write_all || fptr->x != defptr->x || fptr->y != defptr->y) {
		sprintf_s(cmd, nSize, " --%s ", options->long_name);
		write_float_ex(cmd, nSize, fptr->x);
		strcat_s(cmd, nSize, ":");
		write_float_ex(cmd, nSize, fptr->y);
	}
}

static void write_list(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	int *iptr = (int*)((BYTE*)cx + options->p_offset);
	int *defptr = (int*)((BYTE*)def + options->p_offset);
	if (write_all || *iptr != *defptr)
		sprintf_s(cmd, nSize, " --%s %s", options->long_name, options->list[*iptr]);
}

static void write_crf(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	if (cx->rc_mode == X264_RC_CRF) {
		sprintf_s(cmd, nSize, " --%s ", options->long_name);
		write_float_ex(cmd, nSize, cx->crf / 100.0f);
	}
}
static void write_bitrate(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	if (cx->rc_mode == X264_RC_BITRATE) {
		sprintf_s(cmd, nSize, " --%s %d", options->long_name, cx->bitrate);
		if (cx->pass) {
			sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " --pass %d", cx->pass);
			sprintf_s(cmd + strlen(cmd), nSize - strlen(cmd), " --stats \"%s\"", vid->stats);
		}
	}
}
static void write_qp(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	if (cx->rc_mode == X264_RC_QP)
		sprintf_s(cmd, nSize, " --%s %d", options->long_name, cx->qp);
}
static void write_keyint(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	int *iptr = (int*)((BYTE*)cx + options->p_offset);
	int *defptr = (int*)((BYTE*)def + options->p_offset);
	if (write_all || *iptr != *defptr) {
		if (*iptr != 0)
			sprintf_s(cmd, nSize, " --%s %d", options->long_name, *iptr);
		else
			strcpy_s(cmd, nSize, " --keyint infinite");
	}
}
static void write_deblock(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	if (cx->use_deblock)
		if (write_all || cx->deblock.x != def->deblock.x || cx->deblock.y != def->deblock.y)
			sprintf_s(cmd, nSize, " --%s %d:%d", options->long_name, cx->deblock.x, cx->deblock.y);
}
static void write_cqm(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	if (cx->cqm < 2)
		write_list(cmd, nSize, options, cx, def, vid, write_all);
	else
		sprintf_s(cmd, nSize, " --cqmfile \"%s\"", vid->cqmfile);
}
static void write_tcfilein(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	if (cx->use_tcfilein)
		sprintf_s(cmd, nSize, " --tcfile-in \"%s\"", vid->tcfile_in);
}
static void write_input_depth(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	if (cx->use_10bit_depth)
		strcpy_s(cmd, nSize, " --input-depth 10");
}
static void write_mb_partitions(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {	
	DWORD *dwptr = (DWORD*)((BYTE*)cx + options->p_offset);
	DWORD *defptr = (DWORD*)((BYTE*)def + options->p_offset);
	const char * const arg = " --partitions ";
	if ((write_all || *dwptr != *defptr) && nSize > strlen(arg)) {
		strcpy_s(cmd, nSize, arg);
		cmd += strlen(arg); nSize -= strlen(arg);
		if (*dwptr == MB_PARTITION_ALL)
			strcpy_s(cmd, nSize, "all");
		else {
			strcpy_s(cmd, nSize, "none");
			if (*dwptr != MB_PARTITION_NONE) {
				size_t len = 0;
#define APPEND_OPT(v) { strcpy_s(cmd + len, nSize - len, v); len += strlen(v); }
				if (*dwptr & MB_PARTITION_P8x8) APPEND_OPT("p8x8,");
				if (*dwptr & MB_PARTITION_B8x8) APPEND_OPT("b8x8,");
				if (*dwptr & MB_PARTITION_P4x4) APPEND_OPT("p4x4,");
				if (*dwptr & MB_PARTITION_I8x8) APPEND_OPT("i8x8,");
				if (*dwptr & MB_PARTITION_I4x4) APPEND_OPT("i4x4,");
#undef APPEND_OPT
				if (len)
					cmd[len-1] = '\0'; //最後の","を取る
			}
		}
	}
}
static void write_tff(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	if (cx->interlaced)
		strcpy_s(cmd, nSize, (cx->tff) ? " --tff" : " --bff");
}
static void write_timebase(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	if (cx->use_timebase)
		if (write_all || cx->timebase.x != def->timebase.x || cx->timebase.y != def->timebase.y)
			if (cx->timebase.x > 0 && cx->timebase.y > 0)
				sprintf_s(cmd, nSize, " --%s %d/%d", options->long_name, cx->timebase.x, cx->timebase.y);
}
static void write_do_nothing(char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all) {
	return;
}

#pragma warning( pop ) //( disable: 4100 ) 終了

//この配列に従って各関数に飛ばされる
typedef BOOL (*SET_VALUE) (void *cx, const char *value, const X264_OPTION_STR *list);
const SET_VALUE set_value[] = {
	NULL,
	set_bool,
	set_bool_reverse,
	set_int,
	set_float,
	set_int2,
	set_float2,
	set_bool2_reverse,
	set_list,
	set_crf,
	set_bitrate,
	set_qp,
	set_keyint,
	set_deblock,
	set_list,
	set_do_nothing,
	set_input_depth,
	set_int,
	set_mb_partitions,
	set_tff,
	set_bff,
	set_timebase,
	set_level
};

//この配列に従って各関数に飛ばされる
typedef void (*WRITE_CMD) (char *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_X264 *cx, const CONF_X264 *def, const CONF_VIDEO *vid, BOOL write_all);
const WRITE_CMD write_cmd[] = {
	NULL, 
	write_bool,
	write_bool_reverse,
	write_int,
	write_float,
	write_int2,
	write_float2,
	write_bool2_reverse,
	write_list,
	write_crf,
	write_bitrate,
	write_qp,
	write_keyint,
	write_deblock,
	write_cqm,
	write_tcfilein,
	write_input_depth,
	write_do_nothing,
	write_mb_partitions,
	write_tff,
	write_do_nothing,
	write_timebase,
	write_list
};

static void parse_arg(char *cmd, size_t cmd_len, std::vector<CMD_ARG> *cmd_arg_list) {
	BOOL dQB = FALSE;
	BOOL space_flag = TRUE;
	BOOL next_option_flag = FALSE;
	CMD_ARG cmd_arg = { 0 };
	const char *cmd_fin = cmd + cmd_len;
	while (cmd < cmd_fin) {
		if (*cmd == '"') dQB = !dQB;
		if (!dQB) {
			if ((*cmd == ' ' || *cmd == '\r' || *cmd == '\n')) {
				*cmd = '\0';
				space_flag = TRUE;
			} else if (space_flag) {
				if (*cmd == '-' && !isdigit(*(cmd+1))) { //isdigitは負数を避けるため
					if (cmd_arg.arg_type) {
						cmd_arg_list->push_back(cmd_arg);
						ZeroMemory(&cmd_arg, sizeof(CMD_ARG));
					}
					cmd_arg.value = NULL;
					cmd++;
					if (*cmd == '-') {
						cmd++;
						cmd_arg.arg_type = ARG_TYPE_LONG;
					} else {
						cmd_arg.arg_type = ARG_TYPE_SHORT;
					}
					cmd_arg.option_name = cmd;
					next_option_flag = FALSE;
				} else if (!next_option_flag) {
					cmd_arg.value = cmd;
					next_option_flag = TRUE;
				}
				space_flag = FALSE;
			}
		} else if (space_flag) {
			if (!next_option_flag) {
				cmd_arg.value = cmd;
				next_option_flag = TRUE;
			}
			space_flag = FALSE;
		}
		cmd++;
	}
	if (cmd_arg.arg_type)
		cmd_arg_list->push_back(cmd_arg);

	//最初と最後の'"'を落とす
	foreach(std::vector<CMD_ARG>, it_arg, cmd_arg_list) {
		if (it_arg->value == NULL)
			continue;
		size_t value_len = strlen(it_arg->value);
		if (it_arg->value[0] == '"' && it_arg->value[value_len-1] == '"') {
			it_arg->value[value_len-1] = '\0';
			it_arg->value++;
			it_arg->value_had_dQB = TRUE; //落としたことを記憶
		}
	}
}

static void set_setting_list() {
	if (ex_stg->get_reset_s_x264_referesh()) {
		for (DWORD i = 0, check = 0x00; check != (0x04|0x02|0x01) && x264_options_table[i].long_name; i++) {
			if        (!(check & 0x01) && strcmp(x264_options_table[i].long_name, "preset") == NULL) {
				check |= 0x01; x264_options_table[i].list = ex_stg->s_x264.preset.name;
			} else if (!(check & 0x02) && strcmp(x264_options_table[i].long_name, "tune") == NULL) {
				check |= 0x02; x264_options_table[i].list = ex_stg->s_x264.tune.name;
			} else if (!(check & 0x04) && strcmp(x264_options_table[i].long_name, "profile") == NULL) {
				check |= 0x04; x264_options_table[i].list = ex_stg->s_x264.profile.name;
			}
		}
	}
}

static inline BOOL option_has_no_value(DWORD type) {
	for (int i = 0; OPTION_NO_VALUE[i]; i++)
		if (type == OPTION_NO_VALUE[i])
			return TRUE;
	return FALSE;
}

static void set_conf(std::vector<CMD_ARG> *cmd_arg_list, CONF_X264 *conf_set) {
	int i;
	foreach(std::vector<CMD_ARG>, it_arg, cmd_arg_list) {
		for (i = 0; x264_options_table[i].long_name; i++) {
			if (strcmp(it_arg->option_name, ((it_arg->arg_type == ARG_TYPE_LONG) ? x264_options_table[i].long_name : x264_options_table[i].short_name)) == NULL) {
				it_arg->ret = (option_has_no_value(x264_options_table[i].type) == FALSE && it_arg->value == NULL) ? FALSE : TRUE;
				break;
			}
		}
		if (it_arg->ret)
			it_arg->ret = set_value[x264_options_table[i].type]((void *)((BYTE *)conf_set + x264_options_table[i].p_offset), it_arg->value, x264_options_table[i].list);
	}
}

void set_cmd_to_conf(char *cmd, CONF_X264 *conf_set, size_t cmd_len, BOOL build_not_imported_cmd) {
	std::vector<CMD_ARG> cmd_arg_list;
	set_setting_list();
	parse_arg(cmd, cmd_len, &cmd_arg_list);
	set_conf(&cmd_arg_list, conf_set);
	if (build_not_imported_cmd) {
		//コマンドラインの再構築
		//it_arg->option_name のポインタが指すのはcmd上の文字列なので、
		//直接cmdに書き込むとおかしなことになる。
		//そこで一時領域を確保する
		char * const tmp = (char *)calloc((cmd_len + 1) * sizeof(tmp[0]), 1);
		size_t new_len = 0;
		const_foreach(std::vector<CMD_ARG>, it_arg, &cmd_arg_list) {
			if (!it_arg->ret) {
				new_len += sprintf_s(tmp + new_len, cmd_len+1 - new_len, "%s%s%s",
					(new_len) ? " " : "", (it_arg->arg_type == ARG_TYPE_LONG) ? "--" : "-", it_arg->option_name);
				if (it_arg->value) {
					const char * const dqb = (it_arg->value_had_dQB) ? "\"" : "";
					new_len += sprintf_s(tmp + new_len, cmd_len+1 - new_len, " %s%s%s", dqb, it_arg->value, dqb);
				}
			}
		}
		memcpy(cmd, tmp, (new_len + 1) * sizeof(cmd[0]));
		free(tmp);
	}
}

void set_cmd_to_conf(const char *cmd_src, CONF_X264 *conf_set) {
	//parse_argでコマンドラインは書き変えられるので、
	//一度コピーしておく
	size_t cmd_len = strlen(cmd_src) + 1;
	char * const cmd = (char *)malloc((cmd_len + 1) * sizeof(cmd[0]));
	memcpy(cmd, cmd_src, (cmd_len + 1) * sizeof(cmd[0]));
	set_cmd_to_conf(cmd, conf_set, cmd_len, FALSE); //parse_arg
	free(cmd);
}

void get_default_conf_x264(CONF_X264 *conf_set, BOOL use_10bit) {
	ZeroMemory(conf_set, sizeof(CONF_X264));
	set_cmd_to_conf(ex_stg->s_x264.default_cmd, conf_set);
	if (use_10bit)
		set_cmd_to_conf(ex_stg->s_x264.default_cmd_10bit, conf_set);
}

void set_preset_to_conf(CONF_X264 *conf_set, int preset_index) {
	set_cmd_to_conf(ex_stg->s_x264.preset.cmd[preset_index], conf_set);
}

void set_tune_to_conf(CONF_X264 *conf_set, int tune_index) {
	set_cmd_to_conf(ex_stg->s_x264.tune.cmd[tune_index], conf_set);
}

void set_profile_to_conf(CONF_X264 *conf_set, int profile_index) {
	set_cmd_to_conf(ex_stg->s_x264.profile.cmd[profile_index], conf_set);
}

void apply_presets(CONF_X264 *conf_set) {
	set_preset_to_conf(conf_set, conf_set->preset);
	set_tune_to_conf(conf_set, conf_set->tune);
	set_profile_to_conf(conf_set, conf_set->profile);
}

int check_profile(const CONF_X264 *conf_set) {
	CONF_X264 check;
	int profile_index;
	for (profile_index = 0; ex_stg->s_x264.profile.cmd[profile_index]; profile_index++) {
		memcpy(&check, conf_set, sizeof(CONF_X264));
		set_cmd_to_conf(ex_stg->s_x264.profile.cmd[profile_index], &check);
		if (memcmp(&check, conf_set, sizeof(CONF_X264)) == NULL)
			return profile_index;
	}
	return profile_index - 1;
}

void build_cmd_from_conf(char *cmd, size_t nSize, const CONF_X264 *conf, const void *_vid, BOOL write_all) {
	size_t len = 0;
	CONF_X264 x264def;
	CONF_X264 *def = &x264def;
	CONF_VIDEO *vid = (CONF_VIDEO *)_vid;
	get_default_conf_x264(def, conf->use_10bit_depth);
	set_preset_to_conf(def, conf->preset);
	set_tune_to_conf(def, conf->tune);
	set_profile_to_conf(def, conf->profile);

	for (X264_OPTIONS *opt = x264_options_table; opt->long_name; opt++) {
		write_cmd[opt->type](cmd, nSize, opt, conf, def, vid, write_all);
		len = strlen(cmd);
		nSize -= len;
		cmd += len;
		if (opt->p_offset && (opt->p_offset == (opt+1)->p_offset) && !write_all)
			opt++;
	}
}

static void set_guiEx_auto_sar(int *sar_x, int *sar_y, int width, int height) {
	if (width > 0 && height > 0 && *sar_x < 0 && *sar_y < 0) {
		int x = -1 * *sar_x * height;
		int y = -1 * *sar_y * width;
		if (abs(y - x) > -16 * *sar_y) {
			int gcd = get_gcd(x, y);
			*sar_x = x / gcd;
			*sar_y = y / gcd;
		} else {
			*sar_x = *sar_y = 1;
		}
	} else if (*sar_x * *sar_y < 0) {
		*sar_x = *sar_y = 0;
	}
}
static void set_guiEx_auto_sar(CONF_X264 *cx, int width, int height) {
	set_guiEx_auto_sar(&cx->sar.x, &cx->sar.y, width, height);
}

static void set_guiEx_auto_colormatrix(CONF_X264 *cx, int height) {
	int auto_matrix = (height >= COLOR_MATRIX_THRESHOLD) ? COLOR_MATRIX_HD : COLOR_MATRIX_SD;
	if (cx->colormatrix == COLOR_MATRIX_AUTO)
		cx->colormatrix = auto_matrix;
	if (cx->colorprim == COLOR_MATRIX_AUTO)
		cx->colorprim = auto_matrix;
	if (cx->transfer == COLOR_MATRIX_AUTO)
		cx->transfer = auto_matrix;
}

static void set_x264guiEx_auto_vbv(CONF_X264 *cx, int width, int height, int fps_num, int fps_den) {
	if (cx->vbv_bufsize < 0 || cx->vbv_maxrate < 0) {
		int profile_index = check_profile(cx);
		int level_index   = cx->h264_level;
		if (!level_index)
			level_index = calc_auto_level(width, height, cx->ref_frames, cx->interlaced, fps_num, fps_den, cx->vbv_maxrate, cx->vbv_bufsize);
		int *vbv_buf = (cx->vbv_bufsize < 0) ? &cx->vbv_bufsize : NULL;
		int *vbv_max = (cx->vbv_maxrate < 0) ? &cx->vbv_maxrate : NULL;
		get_vbv_value(vbv_max, vbv_buf, level_index, profile_index, cx->use_10bit_depth, ex_stg);
	}
}

static void set_guiEx_auto_keyint(CONF_X264 *cx, int fps_num, int fps_den) {
	if (cx->keyint_max < 0) {
		cx->keyint_max = (int)((fps_num + (fps_den - 1)) / fps_den) * 10; // 60000/1001 fpsの時に 600になるように最後に10倍する (599とか嫌すぎる)
	}
}

void apply_guiEx_auto_settings(CONF_X264 *cx, int width, int height, int fps_num, int fps_den) {
	set_guiEx_auto_sar(cx, width, height);
	set_guiEx_auto_colormatrix(cx, height);
	set_guiEx_auto_keyint(cx, fps_num, fps_den);
	set_x264guiEx_auto_vbv(cx, width, height, fps_num, fps_den);
}

const X264_OPTION_STR * get_option_list(const char *option_name) {
	for (int i = 0; x264_options_table[i].long_name; i++)
		if (x264_options_table[i].type == OPTION_TYPE_LIST || x264_options_table[i].type == OPTION_TYPE_INT)
			if (strcmp(x264_options_table[i].long_name, option_name) == NULL)
				return x264_options_table[i].list;
	return NULL;
}

void set_ex_stg_ptr(guiEx_settings *_ex_stg) {
	ex_stg = _ex_stg;
}

int get_option_value(const char *cmd_src, const char *target_option_name, char *buffer, size_t nSize) {
	//初期化
	buffer[0] = '\0';
	//LONG_OPTIONかSHORT_OPTIONかの判定
	if (target_option_name[0] != '-')
		return -1;
	target_option_name++;
	BOOL target_arg_type = (target_option_name[0] == '-');
	target_option_name += target_arg_type;

	int ret = 1;
	std::vector<CMD_ARG> cmd_arg_list;
	//parse_argでコマンドラインは書き変えられるので、
	//一度コピーしておく
	size_t cmd_len = strlen(cmd_src);
	char * const cmd = (char *)malloc((cmd_len + 1) * sizeof(cmd[0]));
	memcpy(cmd, cmd_src, (cmd_len + 1) * sizeof(cmd[0]));
	set_setting_list();
	parse_arg(cmd, cmd_len, &cmd_arg_list);
	foreach(std::vector<CMD_ARG>, it_arg, &cmd_arg_list) {
		if (it_arg->arg_type == target_arg_type) {
			if (NULL == strcmp(it_arg->option_name, target_option_name)) {
				if (it_arg->value) {
					strcpy_s(buffer, nSize, it_arg->value);
					ret = 0; //正常に取得
				}
				break;
			}
		}
	}
	free(cmd);
	return ret;
}
