// -----------------------------------------------------------------------------------------
// x264guiEx by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2010-2017 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// --------------------------------------------------------------------------------------------

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

int get_encoder_send_bitdepth(const CONF_ENC *cnf);

//オプションの種類(色々便利なので1から始める)
enum {
    OPTION_TYPE_BOOL = 1,
    OPTION_TYPE_BOOL_REVERSE,
    OPTION_TYPE_INT,
    OPTION_TYPE_FLOAT,
    OPTION_TYPE_INT2,
    OPTION_TYPE_FLOAT2,
    OPTION_TYPE_BOOL2_REVERSE,
    OPTION_TYPE_LIST,
    //個別
    OPTION_TYPE_CRF,
    OPTION_TYPE_BITRATE,
    OPTION_TYPE_QP,
    OPTION_TYPE_KEYINT,
    OPTION_TYPE_DEBLOCK,
    OPTION_TYPE_CQM,
    OPTION_TYPE_TCFILE_IN,
    OPTION_TYPE_INPUT_DEPTH,
    OPTION_TYPE_OUTPUT_DEPTH,
    OPTION_TYPE_PASS,
    OPTION_TYPE_MB_PARTITION,
    OPTION_TYPE_TFF,
    OPTION_TYPE_BFF,
    OPTION_TYPE_TIMEBASE,
    OPTION_TYPE_LEVEL,
    OPTION_TYPE_ANALYSE,
    OPTION_TYPE_RC,
    OPTION_TYPE_AQ,
    OPTION_TYPE_INTERLACED,
    OPTION_TYPE_PSY,
};

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
    { _T("input-depth"),      _T(""),   OPTION_TYPE_INPUT_DEPTH,   NULL,                 offsetof(CONF_ENC, use_highbit_depth) },
    { _T("output-depth"),     _T(""),   OPTION_TYPE_OUTPUT_DEPTH,  NULL,                 offsetof(CONF_ENC, use_highbit_depth) },
    { _T("output-csp"),       _T(""),   OPTION_TYPE_LIST,          list_output_csp,      offsetof(CONF_ENC, output_csp     ) },
    { _T("pass"),            _T("p"),  OPTION_TYPE_PASS,          NULL,                 offsetof(CONF_ENC, pass           ) },
    { _T("slow-firstpass"),   _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, slow_first_pass) },
    //{ _T("stats"),            _T(""),   OPTION_TYPE_STATS,         NULL,                 NULL                                  },
    { _T("preset"),           _T(""),   OPTION_TYPE_LIST,          NULL,                 offsetof(CONF_ENC, preset         ) },
    { _T("tune"),             _T(""),   OPTION_TYPE_LIST,          NULL,                 offsetof(CONF_ENC, tune           ) },
    { _T("profile"),          _T(""),   OPTION_TYPE_LIST,          NULL,                 offsetof(CONF_ENC, profile        ) },
    { _T("crf"),              _T(""),   OPTION_TYPE_CRF,           NULL,                 NULL                                 },
    { _T("bitrate"),          _T("B"),  OPTION_TYPE_BITRATE,       NULL,                 NULL                                 },
    { _T("qp"),               _T("q"),  OPTION_TYPE_QP,            NULL,                 NULL                                 },
    { _T("ipratio"),          _T(""),   OPTION_TYPE_FLOAT,         NULL,                 offsetof(CONF_ENC, ip_ratio       ) },
    { _T("pbratio"),          _T(""),   OPTION_TYPE_FLOAT,         NULL,                 offsetof(CONF_ENC, pb_ratio       ) },
    { _T("qpmin"),            _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, qp_min         ) },
    { _T("qpmax"),            _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, qp_max         ) },
    { _T("qpstep"),           _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, qp_step        ) },
    { _T("qcomp"),            _T(""),   OPTION_TYPE_FLOAT,         NULL,                 offsetof(CONF_ENC, qp_compress    ) },
    { _T("chroma-qp-offset"), _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, chroma_qp_offset) },
    { _T("no-mbtree"),        _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, mbtree         ) },
    { _T("mbtree"),           _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, mbtree         ) },
    { _T("rc-lookahead"),     _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, rc_lookahead   ) },
    { _T("vbv-bufsize"),      _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, vbv_bufsize    ) },
    { _T("vbv-maxrate"),      _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, vbv_maxrate    ) },
    { _T("aq-mode"),          _T(""),   OPTION_TYPE_INT,           list_aq,              offsetof(CONF_ENC, aq_mode        ) },
    { _T("aq-strength"),      _T(""),   OPTION_TYPE_FLOAT,         NULL,                 offsetof(CONF_ENC, aq_strength    ) },
    { _T("psy-rd"),           _T(""),   OPTION_TYPE_FLOAT2,        NULL,                 offsetof(CONF_ENC, psy_rd         ) },
    { _T("psy"),              _T(""),   OPTION_TYPE_PSY,           NULL,                 offsetof(CONF_ENC, psy_rd         ) },
    { _T("no-psy"),           _T(""),   OPTION_TYPE_BOOL2_REVERSE, NULL,                 offsetof(CONF_ENC, psy_rd         ) },
    { _T("scenecut"),         _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, scenecut       ) },
    { _T("no-scenecut"),      _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, scenecut       ) },
    { _T("keyint"),           _T("I"),  OPTION_TYPE_KEYINT,        NULL,                 offsetof(CONF_ENC, keyint_max     ) },
    { _T("min-keyint"),       _T("i"),  OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, keyint_min     ) },
    { _T("open-gop"),         _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, open_gop       ) },
    { _T("no-cabac"),         _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, cabac          ) },
    { _T("cabac"),            _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, cabac          ) },
    { _T("bframes"),          _T("b"),  OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, bframes        ) },
    { _T("b-adapt"),          _T(""),   OPTION_TYPE_INT,           list_b_adpat,         offsetof(CONF_ENC, b_adapt        ) },
    { _T("no-b-adapt"),       _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, b_adapt        ) },
    { _T("b-bias"),           _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, b_bias         ) },
    { _T("b-pyramid"),        _T(""),   OPTION_TYPE_LIST,          list_b_pyramid,       offsetof(CONF_ENC, b_pyramid      ) },
    { _T("slices"),           _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, slice_n        ) },
    { _T("deblock"),          _T("f"),  OPTION_TYPE_DEBLOCK,       NULL,                 NULL                                 },
    { _T("no-deblock"),       _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, use_deblock    ) },
    { _T("no-interlace"),     _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, interlaced     ) },
    { _T("tff"),              _T(""),   OPTION_TYPE_TFF,           NULL,                 NULL                                 },
    { _T("bff"),              _T(""),   OPTION_TYPE_BFF,           NULL,                 NULL                                 },
    { _T("partitions"),       _T("A"),  OPTION_TYPE_MB_PARTITION,  NULL,                 offsetof(CONF_ENC, mb_partition   ) },
    { _T("no-8x8dct"),        _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, dct8x8         ) },
    { _T("8x8dct"),           _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, dct8x8         ) },
    { _T("me"),               _T(""),   OPTION_TYPE_LIST,          list_me,              offsetof(CONF_ENC, me             ) },
    { _T("subme"),            _T("m"),  OPTION_TYPE_INT,           list_subme,           offsetof(CONF_ENC, subme          ) },
    { _T("merange"),          _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, me_range       ) },
    { _T("no-chroma-me"),     _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, chroma_me      ) },
    { _T("chroma-me"),        _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, chroma_me      ) },
    { _T("direct"),           _T(""),   OPTION_TYPE_LIST,          list_direct,          offsetof(CONF_ENC, direct_mv      ) },
    { _T("ref"),              _T("r"),  OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, ref_frames     ) },
    { _T("no-mixed-refs"),    _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, mixed_ref      ) },
    { _T("mixed-refs"),       _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, mixed_ref      ) },
    { _T("no-weightb"),       _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, weight_b       ) },
    { _T("weightb"),          _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, weight_b       ) },
    { _T("weightp"),          _T(""),   OPTION_TYPE_INT,           list_weightp,         offsetof(CONF_ENC, weight_p       ) },
    { _T("nr"),               _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, noise_reduction) },
    { _T("no-fast-pskip"),    _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, no_fast_pskip  ) },
    { _T("no-dct-decimate"),  _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, no_dct_decimate) },
    { _T("trellis"),          _T("t"),  OPTION_TYPE_INT,           list_trellis,         offsetof(CONF_ENC, trellis        ) },
    { _T("cqm"),              _T(""),   OPTION_TYPE_CQM,           list_cqm,             offsetof(CONF_ENC, cqm            ) },
    //{ _T("cqmfile"),          _T(""),   OPTION_TYPE_CQMFILE,       NULL,                 NULL                                 },
    { _T("colormatrix"),      _T(""),   OPTION_TYPE_LIST,          list_colormatrix,     offsetof(CONF_ENC, colormatrix    ) },
    { _T("colorprim"),        _T(""),   OPTION_TYPE_LIST,          list_colorprim,       offsetof(CONF_ENC, colorprim      ) },
    { _T("transfer"),         _T(""),   OPTION_TYPE_LIST,          list_transfer,        offsetof(CONF_ENC, transfer       ) },
    { _T("input-range"),      _T(""),   OPTION_TYPE_LIST,          list_input_range,     offsetof(CONF_ENC, input_range    ) },
    { _T("sar"),              _T(""),   OPTION_TYPE_INT2,          NULL,                 offsetof(CONF_ENC, sar            ) },
    { _T("level"),            _T(""),   OPTION_TYPE_LEVEL,         list_x264guiEx_level, offsetof(CONF_ENC, h264_level     ) },
    { _T("videoformat"),      _T(""),   OPTION_TYPE_LIST,          list_videoformat,     offsetof(CONF_ENC, videoformat    ) },
    { _T("aud"),              _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, aud            ) },
    { _T("pic-struct"),       _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, pic_struct     ) },
    { _T("nal-hrd"),          _T(""),   OPTION_TYPE_LIST,          list_nal_hrd,         offsetof(CONF_ENC, nal_hrd        ) },
    { _T("bluray-compat"),    _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, bluray_compat  ) },
    { _T("threads"),          _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, threads        ) },
    { _T("lookahead-threads"),_T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, lookahead_threads) },
    { _T("sliced-threads"),   _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, sliced_threading) },
    { _T("log-level"),        _T(""),   OPTION_TYPE_LIST,          list_log_type,        offsetof(CONF_ENC, log_mode       ) },
    { _T("psnr"),             _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, psnr           ) },
    { _T("ssim"),             _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, ssim           ) },
    { _T("tcfile-in"),        _T(""),   OPTION_TYPE_TCFILE_IN,     NULL,                 NULL                                 },
    { _T("timebase"),         _T(""),   OPTION_TYPE_TIMEBASE,      NULL,                 NULL                                 },
    { _T("progress"),         _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, disable_progress) },
    { _T("no-progress"),      _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, disable_progress) },
    //書き出しの場合はここまでで終わり
    { NULL,                   NULL, NULL,                      NULL,                 NULL                                 },
    //MediaInfoの書式を回収する
    { _T("analyse"),          _T(""),   OPTION_TYPE_ANALYSE,       NULL,                 offsetof(CONF_ENC, mb_partition   ) },
    { _T("psy_rd"),           _T(""),   OPTION_TYPE_FLOAT2,        NULL,                 offsetof(CONF_ENC, psy_rd         ) },
    { _T("mixed_ref"),        _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, mixed_ref      ) },
    { _T("me_range"),         _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, me_range       ) },
    { _T("chroma_me"),        _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, chroma_me      ) },
    { _T("fast_pskip"),       _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, no_fast_pskip  ) },
    { _T("chroma_qp_offset"), _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, chroma_qp_offset) },
    { _T("lookahead_threads"),_T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, lookahead_threads) },
    { _T("sliced_threads"),   _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, sliced_threading) },
    { _T("decimate"),         _T(""),   OPTION_TYPE_BOOL_REVERSE,  NULL,                 offsetof(CONF_ENC, no_dct_decimate) },
    { _T("bluray_compat"),    _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, bluray_compat  ) },
    { _T("b_pyramid"),        _T(""),   OPTION_TYPE_LIST,          list_b_pyramid,       offsetof(CONF_ENC, b_pyramid      ) },
    { _T("b_adapt"),          _T(""),   OPTION_TYPE_INT,           list_b_adpat,         offsetof(CONF_ENC, b_adapt        ) },
    { _T("b_bias"),           _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, b_bias         ) },
    { _T("open_gop"),         _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, open_gop       ) },
    { _T("keyint_min"),       _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, keyint_min     ) },
    { _T("rc_lookahead"),     _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, rc_lookahead   ) },
    { _T("nal_hrd"),          _T(""),   OPTION_TYPE_LIST,          list_nal_hrd,         offsetof(CONF_ENC, nal_hrd        ) },
    { _T("ip_ratio"),         _T(""),   OPTION_TYPE_FLOAT,         NULL,                 offsetof(CONF_ENC, ip_ratio       ) },
    { _T("pb_ratio"),         _T(""),   OPTION_TYPE_FLOAT,         NULL,                 offsetof(CONF_ENC, pb_ratio       ) },
    { _T("vbv_maxrate"),      _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, vbv_maxrate    ) },
    { _T("vbv_bufsize"),      _T(""),   OPTION_TYPE_INT,           NULL,                 offsetof(CONF_ENC, vbv_bufsize    ) },
    { _T("interlaced"),       _T(""),   OPTION_TYPE_INTERLACED,    NULL,                 NULL                                 },
    { _T("mbaff"),            _T(""),   OPTION_TYPE_INTERLACED,    NULL,                 NULL                                 },
    { _T("rc"),               _T(""),   OPTION_TYPE_RC,            NULL,                 NULL                                 },
    { _T("aq"),               _T(""),   OPTION_TYPE_AQ,            NULL,                 NULL                                 },
    { _T("wpredb"),           _T(""),   OPTION_TYPE_BOOL,          NULL,                 offsetof(CONF_ENC, weight_b       ) },
    { _T("wpredp"),           _T(""),   OPTION_TYPE_INT,           list_weightp,         offsetof(CONF_ENC, weight_p       ) },
};

static BOOL x264guiEx_strtol(int *i, const TCHAR *str, DWORD len) {
    TCHAR *eptr = NULL;
    int v;
    BOOL ret = TRUE;
    if (len == NULL) len = ULONG_MAX;
    if (*str != L'{') {
        v = _tcstol(str, &eptr, 0);
        if (*eptr == L'\0' || (DWORD)(eptr - str) == len) { *i = v; } else { ret = FALSE; }
    } else {
        str++;
        BOOL multi = (*str == L'*');
        v = _tcstol(str + multi, &eptr, 0);
        if (*eptr == L'}') { (multi) ? *i *= v : *i += v; } else { ret = FALSE; }
    }
    return ret;
}

static BOOL x264guiEx_strtof(float *f, const TCHAR *str, DWORD len) {
    TCHAR *eptr = NULL;
    float v;
    BOOL ret = TRUE;
    if (len == NULL) len = ULONG_MAX;
    if (*str != L'{') {
        v = (float)_tcstod(str, &eptr);
        if (*eptr == L'\0' || (DWORD)(eptr - str) == len) { *f = v; } else { ret = FALSE; }
    } else {
        str++;
        BOOL multi = (*str == L'*');
        v = (float)_tcstod(str + multi, &eptr);
        if (*eptr == L'}') { (multi) ? *f *= v : *f += v; } else { ret = FALSE; }
    }
    return ret;
}

static BOOL x264guiEx_parse_int(int *i, const TCHAR *value, DWORD len) {
    BOOL ret;
    if ((ret = x264guiEx_strtol(i, value, len)) == FALSE) {
        size_t val_len = _tcslen(value);
        if (*value == L'[' && value[val_len -1] == L']') {
            const TCHAR *a, *b, *c;
            if ((a = _tcsstr(value, _T("if>"))) != NULL && (b = _tcsstr(value, _T("else"))) != NULL) {
                int v;
                c = a + _tcslen(_T("if>"));
                ret |= x264guiEx_strtol(&v, c, (DWORD)(b-c));
                b += _tcslen(_T("else"));
                if (*i > v)
                    c = value+1, val_len = a - c;
                else
                    c = b, val_len = (value + val_len - 1) - c;
                ret &= x264guiEx_strtol(i, c, (DWORD)val_len);
            }
        }
    }
    return ret;
}

static BOOL x264guiEx_parse_float(float *f, const TCHAR *value, DWORD len) {
    BOOL ret;
    if ((ret = x264guiEx_strtof(f, value, len)) == FALSE) {
        size_t val_len = _tcslen(value);
        if (*value == L'[' && value[val_len -1] == L']') {
            const TCHAR *a, *b, *c;
            if ((a = _tcsstr(value, _T("if>"))) != NULL && (b = _tcsstr(value, _T("else"))) != NULL) {
                float v;
                c = a + _tcslen(_T("if>"));
                ret |= x264guiEx_strtof(&v, c, (DWORD)(b-c));
                b += _tcslen(_T("else"));
                if (*f > v)
                    c = value+1, val_len = a - c;
                else
                    c = b, val_len = (value + val_len - 1) - c;
                ret &= x264guiEx_strtof(f, c, (DWORD)val_len);
            }
        }
    }
    return ret;
}

//以下部分的にwarning C4100を黙らせる
//C4100 : 引数は関数の本体部で 1 度も参照されません。
#pragma warning( push )
#pragma warning( disable: 4100 )

static BOOL set_bool(void *b, const TCHAR *value, const ENC_OPTION_STR *list) {
    BOOL ret = TRUE;
    if (value) {
        int i = -1;
        if (FALSE != (ret = x264guiEx_parse_int(&i, value, NULL)))
            if (FALSE != (ret = check_range(i, FALSE, TRUE)))
                *(int *)b = i;
    } else {
        *(BOOL*)b = TRUE;
    }
    return ret;
}

static BOOL set_bool_reverse(void *b, const TCHAR *value, const ENC_OPTION_STR *list) {
    BOOL ret = TRUE;
    if (value) {
        int i = -1;
        if (FALSE != (ret = x264guiEx_parse_int(&i, value, NULL)))
            if (FALSE != (ret = check_range(i, FALSE, TRUE)))
                *(int *)b = !i;
    } else {
        *(BOOL*)b = FALSE;
    }
    return ret;
}
static BOOL set_int(void *i, const TCHAR *value, const ENC_OPTION_STR *list) {
    return x264guiEx_parse_int((int *)i, value, NULL);
}

static BOOL set_float(void *f, const TCHAR *value, const ENC_OPTION_STR *list) {
    return x264guiEx_parse_float((float *)f, value, NULL);
}

static BOOL set_int2(void *i, const TCHAR *value, const ENC_OPTION_STR *list) {
    const size_t len = _tcslen(value);
    //一度値をコピーして分析
    BOOL ret = FALSE;
    for (DWORD j = 0; j < len; j++) {
        if (value[j] == L':' || value[j] == L'|' || value[j] == L',' || value[j] == L'/') {
            ret = TRUE;
            if (!(j == _tcslen(_T("<unset>")) && _tcsncicmp(value, _T("<unset>"), _tcslen(_T("<unset>"))) == NULL))
                ret &= x264guiEx_parse_int(&((INT2 *)i)->x, value, j);
            if (_tcsicmp(&value[j+1], _T("<unset>")) != NULL)
                ret &= x264guiEx_parse_int(&((INT2 *)i)->y, &value[j+1], 0);
            break;
        }
    }
    return ret;
}

static BOOL set_bool2_reverse(void *b, const TCHAR *value, const ENC_OPTION_STR *list) {
    BOOL ret = TRUE;
    if (value) {
        INT2 i_value = { 0, 0 };
        if (FALSE != (ret = set_int2(&i_value, value, NULL))) {
            if (i_value.x == 0) ((INT2*)b)->x = FALSE;
            if (i_value.y == 0) ((INT2*)b)->y = FALSE;
        }
    } else {
        ((INT2*)b)->x = FALSE;
        ((INT2*)b)->y = FALSE;
    }
    return TRUE;
}

static BOOL set_float2(void *f, const TCHAR *value, const ENC_OPTION_STR *list) {
    const size_t len = _tcslen(value);
    BOOL ret = FALSE;
    for (DWORD j = 0; j < len; j++) {
        if (value[j] == L':' || value[j] == L'|' || value[j] == L',' || value[j] == L'/') {
            ret = TRUE;
            if (!(j == _tcslen(_T("<unset>")) && _tcsncicmp(value, _T("<unset>"), _tcslen(_T("<unset>"))) == NULL))
                ret &= x264guiEx_parse_float(&((FLOAT2 *)f)->x, value, j);
            if (_tcsicmp(&value[j+1], _T("<unset>")) != NULL)
                ret &= x264guiEx_parse_float(&((FLOAT2 *)f)->y, &value[j+1], 0);
            break;
        }
    }
    return ret;
}

static BOOL set_list(void *i, const TCHAR *value, const ENC_OPTION_STR *list) {
    BOOL ret = FALSE;
    for (int j = 0; list[j].name; j++) {
        if (_tcsicmp(value, list[j].name) == NULL) {
            *(int*)i = j;
            ret = TRUE;
            break;
        }
    }
    //数値での指定に対応
    if (!ret) {
        int k = -1;
        if (FALSE != (ret = x264guiEx_parse_int(&k, value, NULL))) {
            //取得した数が、listに存在するか確認する
            ret = FALSE;
            for (int i_check = 0; list[i_check].name; i_check++) {
                if (i_check == k) {
                    *(int*)i = k;
                    ret = TRUE;
                    break;
                }
            }
        }
    }
    return ret; 
}
static BOOL set_crf(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    ((CONF_ENC *)cx)->rc_mode = ENC_RC_CRF;
    float f = 23.0f;
    x264guiEx_strtof(&f, value, NULL);
    ((CONF_ENC *)cx)->crf = (int)(f * 100 + 0.5);
    return TRUE;
}
static BOOL set_bitrate(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    ((CONF_ENC *)cx)->rc_mode = ENC_RC_BITRATE;
    return x264guiEx_strtol(&((CONF_ENC *)cx)->bitrate, value, NULL);
}
static BOOL set_qp(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    ((CONF_ENC *)cx)->rc_mode = ENC_RC_QP;
    return x264guiEx_strtol(&((CONF_ENC *)cx)->qp, value, NULL);
}
static BOOL set_keyint(void *i, const TCHAR *value, const ENC_OPTION_STR *list) {
    if ((*(int*)i = _tcsicmp(value, _T("infinite"))) != NULL)
        return x264guiEx_parse_int((int *)i, value, NULL);
    return TRUE;
}
static BOOL set_deblock(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    BOOL ret = FALSE;
    int a, b, c;
    if (3 == _stscanf_s(value, _T("%d:%d:%d"), &a, &b, &c)) {
        //type mediainfo
        ((CONF_ENC *)cx)->use_deblock = !!a;
        ((CONF_ENC *)cx)->deblock.x = b;
        ((CONF_ENC *)cx)->deblock.y = c;
        ret = TRUE;
    }
    if (!ret)
        if (FALSE != (ret = set_int2(&((CONF_ENC *)cx)->deblock, value, list)))
            ((CONF_ENC *)cx)->use_deblock = TRUE;
    return ret;
}
static BOOL set_tcfilein(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    ((CONF_ENC *)cx)->use_tcfilein = TRUE;
    return TRUE;
}
static BOOL set_input_depth(void *b, const TCHAR *value, const ENC_OPTION_STR *list) {
    *(BOOL*)b = (_tstoi(value) > 8) ? TRUE : FALSE;
    return TRUE;
}
static BOOL set_output_depth(void *b, const TCHAR *value, const ENC_OPTION_STR *list) {
    *(BOOL*)b = (_tstoi(value) > 8) ? TRUE : FALSE;
    return TRUE;
}
static BOOL set_mb_partitions(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    BOOL ret = TRUE;
    *(DWORD*)cx = MB_PARTITION_NONE;
    if (_tcsstr(value, _T("all"))) {
        *(DWORD*)cx = MB_PARTITION_ALL;
    } else if (_tcsstr(value, _T("none"))) {
        ;//なにもしない
    } else {
        BOOL fin = FALSE;
        for (const TCHAR *p = value, *q = value; !fin; p++) {
            if (*p == L'\0') fin = TRUE;
            if (fin || *p == L',' || *p == L'/' || *p == L';' || *p == L':') {
                int len = (int)(p - q);
                if (     !_tcsncicmp(q, _T("p8x8"), len))
                    *(DWORD*)cx |= MB_PARTITION_P8x8;
                else if (!_tcsncicmp(q, _T("b8x8"), len))
                    *(DWORD*)cx |= MB_PARTITION_B8x8;
                else if (!_tcsncicmp(q, _T("p4x4"), len))
                    *(DWORD*)cx |= MB_PARTITION_P4x4;
                else if (!_tcsncicmp(q, _T("i8x8"), len))
                    *(DWORD*)cx |= MB_PARTITION_I8x8;
                else if (!_tcsncicmp(q, _T("i4x4"), len))
                    *(DWORD*)cx |= MB_PARTITION_I4x4;
                else
                    ret = FALSE;
                q = p + 1;
            }
        }
    }
    return ret;
}
static BOOL set_tff(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    ((CONF_ENC *)cx)->interlaced = TRUE;
    ((CONF_ENC *)cx)->tff = TRUE;
    return TRUE;
}
static BOOL set_bff(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    ((CONF_ENC *)cx)->interlaced = TRUE;
    ((CONF_ENC *)cx)->tff = FALSE;
    return TRUE;
}
static BOOL set_timebase(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    ((CONF_ENC *)cx)->use_timebase = TRUE;
    return set_int2(&((CONF_ENC *)cx)->timebase, value, list);
}
static BOOL set_level(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    BOOL ret = FALSE;
    size_t len = _tcslen(value);
    const TCHAR *tmp = value + len - 1;
    if (*tmp != L'0')
        ret = set_list(cx, value, list);
    if (!ret) {
        //"5.0"とかを"5"に整形
        TCHAR *copy = (TCHAR *)malloc((len + 1) * sizeof(copy[0]));
        memcpy(copy, value, (len + 1) * sizeof(copy[0]));
        TCHAR *p = copy + len - 1;
        while (*p == L'0' && p >= copy)
            p--;
        if (*p == L'.') p--; //最後に'.'が残ったら消す
        *(p + 1) = L'\0';
        ret = set_list(cx, copy, list);
        free(copy);
    }
    //整数指定を解析
    if (!ret) {
        int i = 0;
        if (x264guiEx_strtol(&i, value, NULL)) {
            TCHAR buf[16];
            if (i == 9)
                _tcscpy_s(buf, _countof(buf), _T("1b"));
            else {
                size_t buf_len = _stprintf_s(buf, _countof(buf), _T("%.1f"), i / 10.0);
                TCHAR *p = buf + buf_len - 1;
                while (*p == L'0' && p >= buf)
                    p--;
                if (*p == L'.') p--; //最後に'.'が残ったら消す
                *(p + 1) = L'\0';
            }
            ret = set_list(cx, buf, list);
        }
    }
    return ret;
}
static BOOL set_analyse(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    INT2 i_val = { 0, 0 };
    BOOL ret = set_int2(&i_val, value, list);
    if (ret) {
        *(DWORD*)cx = MB_PARTITION_NONE;
        if ((DWORD)i_val.y & 0x0100) *(DWORD*)cx |= MB_PARTITION_B8x8;
        if ((DWORD)i_val.y & 0x0020) *(DWORD*)cx |= MB_PARTITION_P4x4;
        if ((DWORD)i_val.y & 0x0010) *(DWORD*)cx |= MB_PARTITION_P8x8;
        if ((DWORD)i_val.y & 0x0002) *(DWORD*)cx |= MB_PARTITION_I8x8;
        if ((DWORD)i_val.y & 0x0001) *(DWORD*)cx |= MB_PARTITION_I4x4;
    }
    return ret; 
}
static BOOL set_rc(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    BOOL ret = TRUE;
    if (NULL == _tcsncmp(value, _T("2pass"), _tcslen(_T("2pass")))) {
        ((CONF_ENC *)cx)->rc_mode = ENC_RC_BITRATE;
        ((CONF_ENC *)cx)->use_auto_npass = TRUE;
        ((CONF_ENC *)cx)->auto_npass = 2;
    } else if (NULL == _tcsncmp(value, _T("crf"), _tcslen(_T("crf")))) {
        ((CONF_ENC *)cx)->rc_mode = ENC_RC_CRF;
    } else if (NULL == _tcsncmp(value, _T("cbr"), _tcslen(_T("cbr")))
            || NULL == _tcsncmp(value, _T("abr"), _tcslen(_T("abr")))) {
        ((CONF_ENC *)cx)->rc_mode = ENC_RC_BITRATE;
        ((CONF_ENC *)cx)->use_auto_npass = FALSE;
    } else if (NULL == _tcsncmp(value, _T("cqp"), _tcslen(_T("cqp")))) {
        ((CONF_ENC *)cx)->rc_mode = ENC_RC_QP;
    } else {
        ret = FALSE;
    }
    return ret; 
}
static BOOL set_aq(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    FLOAT2 f_val = { 0, 0 };
    BOOL ret = set_float2(&f_val, value, list);
    if (ret) {
        ((CONF_ENC *)cx)->aq_mode = ((int)(f_val.x + 0.5));
        ((CONF_ENC *)cx)->aq_strength = f_val.y;
    }
    return ret; 
}
static BOOL set_interlaced(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    BOOL ret = TRUE;
    if (!value) {
        ((CONF_ENC *)cx)->interlaced = TRUE;
        ((CONF_ENC *)cx)->tff = TRUE;
    } else if (NULL == _tcsncmp(value, _T("tff"), _tcslen(_T("tff")))) {
        ((CONF_ENC *)cx)->interlaced = TRUE;
        ((CONF_ENC *)cx)->tff = TRUE;
    } else if (NULL == _tcsncmp(value, _T("bff"), _tcslen(_T("bff")))) {
        ((CONF_ENC *)cx)->interlaced = TRUE;
        ((CONF_ENC *)cx)->tff = FALSE;
    } else if (NULL == _tcsncmp(value, _T("0"), _tcslen(_T("0")))) {
        ((CONF_ENC *)cx)->interlaced = FALSE;
        ((CONF_ENC *)cx)->tff = FALSE;
    } else {
        ret = FALSE;
    }
    return ret; 
}
static BOOL set_psy(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    BOOL ret = TRUE;
    if (value) {
        if (NULL == _tcscmp(value, _T("0"))) {
            ((FLOAT2 *)cx)->x = 0.0;
            ((FLOAT2 *)cx)->y = 0.0;
        } else if (NULL == _tcscmp(value, _T("1"))) {
            ; //何もしない
        } else {
            ret = set_float2(cx, value, list);
        }
    }
    return ret;
}
static BOOL set_do_nothing(void *cx, const TCHAR *value, const ENC_OPTION_STR *list) {
    return FALSE;
}


static int write_bool(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    BOOL *bptr = (BOOL*)((BYTE*)cx + options->p_offset);
    BOOL *defptr = (BOOL*)((BYTE*)def + options->p_offset);
    if ((write_all || *bptr != *defptr) && *bptr)
        return _stprintf_s(cmd, nSize, _T(" --%s"), options->long_name);
    return 0;
}

static int write_bool_reverse(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    BOOL *bptr = (BOOL*)((BYTE*)cx + options->p_offset);
    BOOL *defptr = (BOOL*)((BYTE*)def + options->p_offset);
    if ((write_all || *bptr != *defptr) && !(*bptr))
        return _stprintf_s(cmd, nSize, _T(" --%s"), options->long_name);
    return 0;
}

static int write_bool2_reverse(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    INT2 *bptr = (INT2*)((BYTE*)cx + options->p_offset);
    if (!(bptr->x | bptr->y))
        return _stprintf_s(cmd, nSize, _T(" --%s"), options->long_name);
    return 0;
}

static int write_int(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    int *iptr = (int*)((BYTE*)cx + options->p_offset);
    int *defptr = (int*)((BYTE*)def + options->p_offset);
    if (write_all || *iptr != *defptr)
        return _stprintf_s(cmd, nSize, _T(" --%s %d"), options->long_name, *iptr);
    return 0;
}
//小数表示の鬱陶しい0を消す
static inline int write_float_ex(TCHAR *cmd, size_t nSize, float f) {
    double d = (int)(f * 1000.0 + 0.5 - (f<0)) / 1000.0; //これを入れないと22.2が22.19999とかになる
    int len = _stprintf_s(cmd, nSize, _T("%lf"), d);
    TCHAR *p = cmd + len - 1;
    while (*p == L'0' && p >= cmd)
        p--;
    if (*p == L'.') p--; //最後に'.'が残ったら消す
    p++;
    *p = L'\0';
    return (int)(p - cmd);
}

static int write_float(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    float *fptr = (float*)((BYTE*)cx + options->p_offset);
    float *defptr = (float*)((BYTE*)def + options->p_offset);
    if (write_all || abs(*fptr - *defptr) > EPS_FLOAT) {
        int len = _stprintf_s(cmd, nSize, _T(" --%s "), options->long_name);
        return len + write_float_ex(cmd + len, nSize - len, *fptr);
    }
    return 0;
}

static int write_int2(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    INT2 *iptr = (INT2*)((BYTE*)cx + options->p_offset);
    INT2 *defptr = (INT2*)((BYTE*)def + options->p_offset);
    if (write_all || iptr->x != defptr->x || iptr->y != defptr->y)
        return _stprintf_s(cmd, nSize, _T(" --%s %d:%d"), options->long_name, iptr->x, iptr->y);
    return 0;
}

static int write_float2(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    FLOAT2 *fptr = (FLOAT2*)((BYTE*)cx + options->p_offset);
    FLOAT2 *defptr = (FLOAT2*)((BYTE*)def + options->p_offset);
    if (write_all || fptr->x != defptr->x || fptr->y != defptr->y) {
        int len = _stprintf_s(cmd, nSize, _T(" --%s "), options->long_name);
        len += write_float_ex(cmd + len, nSize - len, fptr->x);
        _tcscpy_s(cmd + len, nSize - len, _T(":"));
        len += (int)_tcslen(_T(":"));
        return len + write_float_ex(cmd + len, nSize - len, fptr->y);
    }
    return 0;
}

static int write_list(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    int *iptr = (int*)((BYTE*)cx + options->p_offset);
    int *defptr = (int*)((BYTE*)def + options->p_offset);
    if (write_all || *iptr != *defptr)
        return _stprintf_s(cmd, nSize, _T(" --%s %s"), options->long_name, options->list[*iptr].name);
    return 0;
}

static int write_crf(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    if (cx->rc_mode == ENC_RC_CRF) {
        int len = _stprintf_s(cmd, nSize, _T(" --%s "), options->long_name);
        return len + write_float_ex(cmd + len, nSize - len, cx->crf / 100.0f);
    }
    return 0;
}
static int write_bitrate(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    if (cx->rc_mode == ENC_RC_BITRATE) {
        int len = _stprintf_s(cmd, nSize, _T(" --%s %d"), options->long_name, cx->bitrate);
        if (cx->pass) {
            len += _stprintf_s(cmd + _tcslen(cmd), nSize - _tcslen(cmd), _T(" --pass %d"), cx->pass);
            len += _stprintf_s(cmd + _tcslen(cmd), nSize - _tcslen(cmd), _T(" --stats \"%s\""), vid->stats);
        }
        return len;
    }
    return 0;
}
static int write_qp(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    if (cx->rc_mode == ENC_RC_QP)
        return _stprintf_s(cmd, nSize, _T(" --%s %d"), options->long_name, cx->qp);
    return 0;
}
static int write_keyint(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    int *iptr = (int*)((BYTE*)cx + options->p_offset);
    int *defptr = (int*)((BYTE*)def + options->p_offset);
    if (write_all || *iptr != *defptr) {
        return (*iptr != 0) ? _stprintf_s(cmd, nSize, _T(" --%s %d"), options->long_name, *iptr)
                            : _stprintf_s(cmd, nSize, _T(" --%s %s"), options->long_name, _T("infinite"));
    }
    return 0;
}
static int write_deblock(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    if (cx->use_deblock)
        if (write_all || cx->deblock.x != def->deblock.x || cx->deblock.y != def->deblock.y)
            return _stprintf_s(cmd, nSize, _T(" --%s %d:%d"), options->long_name, cx->deblock.x, cx->deblock.y);
    return 0;
}
static int write_cqm(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    if (cx->cqm < 2) {
        return write_list(cmd, nSize, options, cx, def, vid, write_all);
    } else {
        TCHAR aviutl_dir[MAX_PATH_LEN] = { 0 };
        get_aviutl_dir(aviutl_dir, _countof(aviutl_dir));
        return _stprintf_s(cmd, nSize, _T(" --cqmfile \"%s\""), GetFullPathFrom(vid->cqmfile, aviutl_dir).c_str());
    }
}
static int write_tcfilein(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    if (cx->use_tcfilein) {
        TCHAR aviutl_dir[MAX_PATH_LEN] = { 0 };
        get_aviutl_dir(aviutl_dir, _countof(aviutl_dir));
        return _stprintf_s(cmd, nSize, _T(" --tcfile-in \"%s\""), GetFullPathFrom(vid->tcfile_in, aviutl_dir).c_str());
    }
    return 0;
}
static int write_input_depth(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    if (cx->use_highbit_depth)
        return _stprintf_s(cmd, nSize, _T(" --input-depth %d --output-depth 10"), get_encoder_send_bitdepth(cx));
    return 0;
}
static int write_mb_partitions(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {    
    DWORD *dwptr = (DWORD*)((BYTE*)cx + options->p_offset);
    DWORD *defptr = (DWORD*)((BYTE*)def + options->p_offset);
    if (write_all || *dwptr != *defptr) {
        int len = 0;
#define APPEND_OPT(v) { _tcscpy_s(cmd + len, nSize - len, v); len += (int)_tcslen(v); }
        APPEND_OPT(_T(" --partitions "));
        if (*dwptr == MB_PARTITION_ALL) {
            APPEND_OPT(_T("all"));
        } else if (*dwptr == MB_PARTITION_NONE) {
            APPEND_OPT(_T("none"));
        } else {
            if (*dwptr & MB_PARTITION_P8x8) APPEND_OPT(_T("p8x8,"));
            if (*dwptr & MB_PARTITION_B8x8) APPEND_OPT(_T("b8x8,"));
            if (*dwptr & MB_PARTITION_P4x4) APPEND_OPT(_T("p4x4,"));
            if (*dwptr & MB_PARTITION_I8x8) APPEND_OPT(_T("i8x8,"));
            if (*dwptr & MB_PARTITION_I4x4) APPEND_OPT(_T("i4x4,"));
            //最後の","を取る
            len--;
            cmd[len] = L'\0'; 
        }
        return len;
#undef APPEND_OPT
    }
    return 0;
}
static int write_tff(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    if (cx->interlaced)
        return _stprintf_s(cmd, nSize, _T(" --%s "), (cx->tff) ? _T("tff") : _T("bff"));
    return 0;
}
static int write_timebase(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    if (cx->use_timebase)
        if (write_all || cx->timebase.x != def->timebase.x || cx->timebase.y != def->timebase.y)
            if (cx->timebase.x > 0 && cx->timebase.y > 0)
                return _stprintf_s(cmd, nSize, _T(" --%s %d/%d"), options->long_name, cx->timebase.x, cx->timebase.y);
    return 0;
}
static int write_do_nothing(TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all) {
    return 0;
}

#pragma warning( pop ) //( disable: 4100 ) 終了

//この配列に従って各関数に飛ばされる
typedef BOOL (*SET_VALUE) (void *cx, const TCHAR *value, const ENC_OPTION_STR *list);
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
    set_tcfilein,
    set_input_depth,
    set_output_depth,
    set_int,
    set_mb_partitions,
    set_tff,
    set_bff,
    set_timebase,
    set_level,
    set_analyse,
    set_rc,
    set_aq,
    set_interlaced,
    set_psy
};

//この配列に従って各関数に飛ばされる
typedef int (*WRITE_CMD) (TCHAR *cmd, size_t nSize, const X264_OPTIONS *options, const CONF_ENC *cx, const CONF_ENC *def, const CONF_VIDEO *vid, BOOL write_all);
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
    write_do_nothing,
    write_mb_partitions,
    write_tff,
    write_do_nothing,
    write_timebase,
    write_list,
    write_do_nothing,
    write_do_nothing,
    write_do_nothing,
    write_do_nothing,
    write_do_nothing
};

//MediaInfoからの情報で無視するもの
static BOOL is_arg_ignore_mediainfo(const TCHAR *arg) {
    static const TCHAR * const IGNORE_ARGS[] = { _T("threads"), _T("lookahead_threads") };
    for (int i = 0; i < _countof(IGNORE_ARGS); i++)
        if (NULL == _tcscmp(arg, IGNORE_ARGS[i]))
            return TRUE;
    return FALSE;
}

static void parse_arg(TCHAR *cmd, size_t cmd_len, std::vector<CMD_ARG> &cmd_arg_list) {
    BOOL dQB = FALSE;
    BOOL space_flag = TRUE;
    BOOL next_option_flag = TRUE;
    CMD_ARG cmd_arg = { 0 };
    const TCHAR *cmd_fin = cmd + cmd_len;
    while (cmd < cmd_fin) {
        if (*cmd == L'"') dQB = !dQB;
        if (!dQB) {
            if ((*cmd == L' ' || *cmd == L'\r' || *cmd == L'\n')) {
                *cmd = L'\0';
                space_flag = TRUE;
            } else if (space_flag) {
                space_flag = FALSE;
                if (*cmd == L'-' && !iswdigit(*(cmd+1))) { //iswdigitは負数を避けるため
                    if (cmd_arg.arg_type) {
                        cmd_arg_list.push_back(cmd_arg);
                        ZeroMemory(&cmd_arg, sizeof(CMD_ARG));
                    }
                    cmd_arg.value = NULL;
                    cmd++;
                    if (*cmd == L'-') {
                        cmd++;
                        cmd_arg.arg_type = ARG_TYPE_LONG;
                    } else {
                        cmd_arg.arg_type = ARG_TYPE_SHORT;
                        if (cmd[1] != L' ')
                            cmd_arg.value = cmd + 1;
                    }
                    cmd_arg.option_name = cmd;
                    next_option_flag = FALSE;
                } else if (!next_option_flag) {
                    cmd_arg.value = cmd;
                    next_option_flag = TRUE;
                } else if (cmd[0] == L'/' && cmd[1] == L' ') {
                    ;
                } else {
                    //オプション名でもなく、オプション値でもない
                    //MediaInfoのオプションかどうかをチェック
                    BOOL b_QB = FALSE;
                    for (TCHAR *ptr = cmd; ptr <= cmd_fin; ptr++) {
                        if (*ptr == L'"') b_QB = !b_QB;
                        if (!b_QB) {
                            if ((*ptr == L' ' || *ptr == L'\r' || *ptr == L'\n' || ptr == cmd_fin)) {
                                TCHAR *equal_ptr = NULL;
                                if (NULL != (equal_ptr = _tcsrchr(cmd, L'='))) {
                                    //mediainfoの書式
                                    *equal_ptr = L'\0';
                                    CMD_ARG cmd_arg_media_info = { 0 };
                                    TCHAR *r_space_ptr = _tcsrchr(equal_ptr, L' ');
                                    cmd_arg_media_info.option_name = (r_space_ptr) ? r_space_ptr + 1 : cmd;
                                    if (!is_arg_ignore_mediainfo(cmd_arg_media_info.option_name)) {
                                        cmd_arg_media_info.arg_type = ARG_TYPE_LONG;
                                        cmd_arg_media_info.value = equal_ptr + 1;
                                        cmd_arg_media_info.type_mediainfo = TRUE;
                                        cmd_arg_list.push_back(cmd_arg_media_info);
                                    }
                                }
                                cmd = ptr;
                                *cmd = L'\0';
                                space_flag = TRUE;
                                next_option_flag = TRUE;
                                break;
                            }
                        }
                    }
                }
            }
        } else if (space_flag) {
            space_flag = FALSE;
            if (!next_option_flag) {
                cmd_arg.value = cmd;
                next_option_flag = TRUE;
            }
        }
        cmd++;
    }
    if (cmd_arg.arg_type)
        cmd_arg_list.push_back(cmd_arg);

    //最初と最後の'"'を落とす
    foreach (it_arg, cmd_arg_list) {
        if (it_arg->value == NULL)
            continue;
        size_t value_len = _tcslen(it_arg->value);
        if (it_arg->value[0] == L'"' && it_arg->value[value_len-1] == L'"') {
            it_arg->value[value_len-1] = L'\0';
            it_arg->value++;
            it_arg->value_had_dQB = TRUE; //落としたことを記憶
        }
    }
}

static void set_setting_list() {
    if (ex_stg->get_reset_s_enc_referesh()) {
        for (DWORD i = 0, check = 0x00; check != (0x04|0x02|0x01) && x264_options_table[i].long_name; i++) {
            if        (!(check & 0x01) && _tcscmp(x264_options_table[i].long_name, _T("preset")) == NULL) {
                check |= 0x01; x264_options_table[i].list = ex_stg->s_enc.preset.name;
            } else if (!(check & 0x02) && _tcscmp(x264_options_table[i].long_name, _T("tune")) == NULL) {
                check |= 0x02; x264_options_table[i].list = ex_stg->s_enc.tune.name;
            } else if (!(check & 0x04) && _tcscmp(x264_options_table[i].long_name, _T("profile")) == NULL) {
                check |= 0x04; x264_options_table[i].list = ex_stg->s_enc.profile.name;
            }
        }
    }
}

//MediaInfoからの情報の補正を行う
static void check_values_from_mediainfo(const std::vector<CMD_ARG> &cmd_arg_list, CONF_ENC *conf_set) {
    BOOL keyint_from_mediainfo = FALSE;
    BOOL keyint_min_from_mediainfo = FALSE;
    BOOL chroma_qp_offset_from_mediainfo = FALSE;
    //2重に指定された場合などを考え、一度すべて見てみてから判定
    for (auto arg : cmd_arg_list) {
        if (arg.ret) {
            if        (NULL == _tcscmp(arg.option_name, _T("chroma_qp_offset"))) {
                chroma_qp_offset_from_mediainfo = arg.type_mediainfo;
            } else if (NULL == _tcscmp(arg.option_name, _T("keyint"))) {
                keyint_from_mediainfo = arg.type_mediainfo;
            } else if (NULL == _tcscmp(arg.option_name, _T("keyint_min"))) {
                keyint_min_from_mediainfo = arg.type_mediainfo;
            }
        }
    }
    if (chroma_qp_offset_from_mediainfo) {
        conf_set->chroma_qp_offset += ((conf_set->psy_rd.x > 0.0) + (conf_set->psy_rd.x > 0.25));
        conf_set->chroma_qp_offset += ((conf_set->psy_rd.y > 0.0) + (conf_set->psy_rd.y > 0.25));
    }
    if (keyint_from_mediainfo && keyint_min_from_mediainfo)
        if (conf_set->keyint_max / 10 == conf_set->keyint_min)
            conf_set->keyint_min = 0;

    if (keyint_from_mediainfo)
        if (conf_set->keyint_max % 10 == 0)
            conf_set->keyint_max = -1;
}

static inline BOOL option_has_no_value(DWORD type) {
    for (int i = 0; OPTION_NO_VALUE[i]; i++)
        if (type == OPTION_NO_VALUE[i])
            return TRUE;
    return FALSE;
}

static void set_conf(std::vector<CMD_ARG> &cmd_arg_list, CONF_ENC *conf_set) {
    foreach (it_arg, cmd_arg_list) {
        int i;
        for (i = 0; x264_options_table[i].long_name; i++) {
            if (NULL == ((it_arg->arg_type == ARG_TYPE_LONG) ? _tcscmp(it_arg->option_name, x264_options_table[i].long_name) : 
                                                               _tcsncmp(it_arg->option_name, x264_options_table[i].short_name, 1))) {
                it_arg->ret = (option_has_no_value(x264_options_table[i].type) == FALSE && it_arg->value == NULL) ? FALSE : TRUE;
                break;
            }
        }
        if (it_arg->ret)
            it_arg->ret = set_value[x264_options_table[i].type]((void *)((BYTE *)conf_set + x264_options_table[i].p_offset), it_arg->value, x264_options_table[i].list);
    }
    check_values_from_mediainfo(cmd_arg_list, conf_set);
}

void set_cmd_to_conf(TCHAR *cmd, CONF_ENC *conf_set, size_t cmd_len, BOOL build_not_imported_cmd) {
    std::vector<CMD_ARG> cmd_arg_list;
    set_setting_list();
    parse_arg(cmd, cmd_len, cmd_arg_list);
    set_conf(cmd_arg_list, conf_set);
    if (build_not_imported_cmd) {
        //コマンドラインの再構築
        //it_arg->option_name のポインタが指すのはcmd上の文字列なので、
        //直接cmdに書き込むとおかしなことになる。
        //そこで一時領域を確保する
        TCHAR * const tmp = (TCHAR *)calloc((cmd_len + 1) * sizeof(tmp[0]), 1);
        size_t new_len = 0;
        for (auto arg : cmd_arg_list) {
            //正常に読み込まれていない、かつMediaInfoの書式でないものを再構成する
            if (!arg.ret && !arg.type_mediainfo) {
                new_len += _stprintf_s(tmp + new_len, cmd_len+1 - new_len, L"%s%s%s",
                    (new_len) ? L" " : _T(""), (arg.arg_type == ARG_TYPE_LONG) ? L"--" : L"-", arg.option_name);
                if (arg.value) {
                    const TCHAR * const dqb = (arg.value_had_dQB) ? L"\"" : _T("");
                    new_len += _stprintf_s(tmp + new_len, cmd_len+1 - new_len, L" %s%s%s", dqb, arg.value, dqb);
                }
            }
        }
        memcpy(cmd, tmp, (new_len + 1) * sizeof(cmd[0]));
        free(tmp);
    }
}

void set_cmd_to_conf(const TCHAR *cmd_src, CONF_ENC *conf_set) {
    //parse_argでコマンドラインは書き変えられるので、
    //一度コピーしておく
    size_t cmd_len = _tcslen(cmd_src) + 1;
    TCHAR * const cmd = (TCHAR *)malloc((cmd_len + 1) * sizeof(cmd[0]));
    memcpy(cmd, cmd_src, (cmd_len + 1) * sizeof(cmd[0]));
    set_cmd_to_conf(cmd, conf_set, cmd_len, FALSE); //parse_arg
    free(cmd);
}

void get_default_conf(CONF_ENC *conf_set, BOOL use_highbit) {
    ZeroMemory(conf_set, sizeof(CONF_ENC));
    set_cmd_to_conf(ex_stg->s_enc.default_cmd, conf_set);
    if (use_highbit)
        set_cmd_to_conf(ex_stg->s_enc.default_cmd_highbit, conf_set);
}

void set_preset_to_conf(CONF_ENC *conf_set, int preset_index) {
    set_cmd_to_conf(ex_stg->s_enc.preset.cmd[preset_index], conf_set);
}

void set_tune_to_conf(CONF_ENC *conf_set, int tune_index) {
    set_cmd_to_conf(ex_stg->s_enc.tune.cmd[tune_index], conf_set);
}

void set_profile_to_conf(CONF_ENC *conf_set, int profile_index) {
    set_cmd_to_conf(ex_stg->s_enc.profile.cmd[profile_index], conf_set);
}

void set_cmd_to_conf_full(const TCHAR *cmd_src, CONF_ENC *conf_set) {
    // 一度ロードし、プロファイル等を再設定する
    CONF_ENC preload;
    get_default_conf(&preload, FALSE); //デフォルトを呼んでおく
    set_cmd_to_conf(cmd_src, &preload);
    memcpy(conf_set, &preload, sizeof(CONF_ENC));
    get_default_conf(conf_set, preload.use_highbit_depth);
    set_preset_to_conf(conf_set, preload.preset);
    set_tune_to_conf(conf_set, preload.tune);
    set_profile_to_conf(conf_set, preload.profile);
    set_cmd_to_conf(cmd_src, conf_set);
}

void apply_presets(CONF_ENC *conf_set) {
    set_preset_to_conf(conf_set, conf_set->preset);
    set_tune_to_conf(conf_set, conf_set->tune);
    set_profile_to_conf(conf_set, conf_set->profile);
}

int check_profile(const CONF_ENC *conf_set) {
    CONF_ENC check;
    int profile_index;
    for (profile_index = 0; ex_stg->s_enc.profile.cmd[profile_index]; profile_index++) {
        memcpy(&check, conf_set, sizeof(CONF_ENC));
        set_cmd_to_conf(ex_stg->s_enc.profile.cmd[profile_index], &check);
        if (memcmp(&check, conf_set, sizeof(CONF_ENC)) == NULL)
            return profile_index;
    }
    return profile_index - 1;
}

void build_cmd_from_conf(TCHAR *cmd, size_t nSize, const CONF_ENC *conf, const void *_vid, BOOL write_all) {
    CONF_ENC x264def;
    CONF_ENC *def = &x264def;
    CONF_VIDEO *vid = (CONF_VIDEO *)_vid;
    get_default_conf(def, conf->use_highbit_depth);
    set_preset_to_conf(def, conf->preset);
    set_tune_to_conf(def, conf->tune);
    set_profile_to_conf(def, conf->profile);

    for (X264_OPTIONS *opt = x264_options_table; opt->long_name; opt++) {
        int len = write_cmd[opt->type](cmd, nSize, opt, conf, def, vid, write_all);
        len = (int)_tcslen(cmd);
        nSize -= len;
        cmd += len;
        if (opt->p_offset && (opt->p_offset == (opt+1)->p_offset) && len && !write_all)
            opt++;
    }
}

static void set_guiEx_auto_sar(CONF_ENC *cx, int width, int height) {
    set_guiEx_auto_sar(&cx->sar.x, &cx->sar.y, width, height);
}

static void set_guiEx_auto_colormatrix(CONF_ENC *cx, int height) {
    int auto_matrix = (height >= COLOR_MATRIX_THRESHOLD) ? COLOR_MATRIX_HD : COLOR_MATRIX_SD;
    if (cx->colormatrix == COLOR_MATRIX_AUTO)
        cx->colormatrix = auto_matrix;
    if (cx->colorprim == COLOR_MATRIX_AUTO)
        cx->colorprim = auto_matrix;
    if (cx->transfer == COLOR_MATRIX_AUTO)
        cx->transfer = auto_matrix;
}

static void set_x264guiEx_auto_vbv(CONF_ENC *cx, int width, int height, int fps_num, int fps_den, BOOL ref_limit_by_level) {
    if (cx->vbv_bufsize < 0 || cx->vbv_maxrate < 0) {
        int profile_index = check_profile(cx);
        int level_index = cx->h264_level;
        if (!level_index)
            level_index = calc_auto_level(width, height, (ref_limit_by_level) ? 0 : cx->ref_frames, cx->interlaced, fps_num, fps_den, cx->vbv_maxrate, cx->vbv_bufsize);
        int *vbv_buf = (cx->vbv_bufsize < 0) ? &cx->vbv_bufsize : NULL;
        int *vbv_max = (cx->vbv_maxrate < 0) ? &cx->vbv_maxrate : NULL;
        get_vbv_value(vbv_max, vbv_buf, level_index, profile_index, cx->use_highbit_depth, ex_stg);
    }
}

void set_guiEx_auto_keyint(CONF_ENC *cx, int fps_num, int fps_den) {
    if (cx->keyint_max < 0) {
        cx->keyint_max = (int)((fps_num + (fps_den - 1)) / fps_den) * 10; // 60000/1001 fpsの時に 600になるように最後に10倍する (599とか嫌すぎる)
    }
}

static void set_guiEx_auto_ref_limit_by_level(CONF_ENC *cx, int width, int height, int fps_num, int fps_den, BOOL ref_limit_by_level) {
    if (!ref_limit_by_level)
        return;
    int level_index = cx->h264_level;
    if (!level_index)
        level_index = calc_auto_level(width, height, 0, cx->interlaced, fps_num, fps_den, cx->vbv_maxrate, cx->vbv_bufsize);
    cx->ref_frames = std::max(1, std::min(cx->ref_frames, get_ref_limit(level_index, width, height, cx->interlaced)));
}

void apply_guiEx_auto_settings(CONF_ENC *cx, int width, int height, int fps_num, int fps_den, BOOL ref_limit_by_level) {
    set_guiEx_auto_sar(cx, width, height);
    set_guiEx_auto_colormatrix(cx, height);
    set_guiEx_auto_keyint(cx, fps_num, fps_den);
    set_x264guiEx_auto_vbv(cx, width, height, fps_num, fps_den, ref_limit_by_level);
    set_guiEx_auto_ref_limit_by_level(cx, width, height, fps_num, fps_den, ref_limit_by_level);
}

const ENC_OPTION_STR * get_option_list(const TCHAR *option_name) {
    for (int i = 0; x264_options_table[i].long_name; i++)
        if (x264_options_table[i].type == OPTION_TYPE_LIST || x264_options_table[i].type == OPTION_TYPE_INT)
            if (_tcscmp(x264_options_table[i].long_name, option_name) == NULL)
                return x264_options_table[i].list;
    return NULL;
}

void set_ex_stg_ptr(guiEx_settings *_ex_stg) {
    ex_stg = _ex_stg;
}

int get_option_value(const TCHAR *cmd_src, const TCHAR *target_option_name, TCHAR *buffer, size_t nSize) {
    //初期化
    buffer[0] = L'\0';
    //LONG_OPTIONかSHORT_OPTIONかの判定
    if (target_option_name[0] != L'-')
        return -1;
    target_option_name++;
    BOOL target_arg_type = (target_option_name[0] == L'-');
    target_option_name += target_arg_type;

    int ret = 1;
    std::vector<CMD_ARG> cmd_arg_list;
    //parse_argでコマンドラインは書き変えられるので、
    //一度コピーしておく
    size_t cmd_len = _tcslen(cmd_src);
    TCHAR * const cmd = (TCHAR *)malloc((cmd_len + 1) * sizeof(cmd[0]));
    memcpy(cmd, cmd_src, (cmd_len + 1) * sizeof(cmd[0]));
    set_setting_list();
    parse_arg(cmd, cmd_len, cmd_arg_list);
    for (auto arg : cmd_arg_list) {
        if (arg.arg_type == target_arg_type) {
            if (NULL == _tcscmp(arg.option_name, target_option_name)) {
                if (arg.value) {
                    _tcscpy_s(buffer, nSize, arg.value);
                    ret = 0; //正常に取得
                }
                break;
            }
        }
    }
    free(cmd);
    return ret;
}

int get_list_index(const ENC_OPTION_STR *list, const TCHAR *name) {
    for (int i = 0; list[i].name; i++) {
        if (_tcsstr(list[i].name, name)) {
            return i;
        }
    }
    return 0;
}
