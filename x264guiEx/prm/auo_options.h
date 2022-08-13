// -----------------------------------------------------------------------------------------
// x264guiEx/x265guiEx/svtAV1guiEx/ffmpegOut/QSVEnc/NVEnc/VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2010-2022 rigaya
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

#ifndef _AUO_OPTIONS_H_
#define _AUO_OPTIONS_H_

#include <Windows.h>
#include <vector>

#include "auo.h"
#include "auo_settings.h"

//エンコードモード
enum {
    X264_RC_CRF = 0,
    X264_RC_BITRATE,
    X264_RC_QP,
};

//QPの最大値
const int X264_QP_MAX_8BIT  = 69;
const int X264_QP_MAX_10BIT = 81;

//差がこのくらいなら等しいとみなす(オプション用なのでこのくらいで十分)
const float EPS_FLOAT = 1.0e-4f;

//マクロブロックタイプの一般的なオプション
enum {
    MB_PARTITION_NONE = 0x00000000,
    MB_PARTITION_P8x8 = 0x00000001,
    MB_PARTITION_B8x8 = 0x00000002,
    MB_PARTITION_P4x4 = 0x00000004,
    MB_PARTITION_I8x8 = 0x00000008,
    MB_PARTITION_I4x4 = 0x00000010,
    MB_PARTITION_ALL  = 0x0000001F,
};

enum {
    OUT_CSP_NV12,
    OUT_CSP_NV16,
    OUT_CSP_YUV444,
    OUT_CSP_RGB,
    OUT_CSP_YUY2,
    OUT_CSP_YV12,
    OUT_CSP_YUV422,
    OUT_CSP_YUV400,
};

enum {
    YC48_COLMAT_CONV_AUTO,
    YC48_COLMAT_CONV_NONE,
    YC48_COLMAT_CONV_BT709,
};

enum {
    AUTO_NPASS_NONE = 0,
    AUTO_NPASS_AMP,
    AUTO_NPASS_CRF,
};

//x264のinput-cspとして使用するもの
//OUT_CSP_NV12, OUT_CSP_YUV444, OUT_CSP_RGB に合わせる
static const char * const specify_csp[] = {
    "nv12", //OUT_CSP_NV12
    "nv16", //OUT_CSP_NV16
    "i444", //OUT_CSP_YUV444
    "rgb"   //OUT_CSP_RGB
};
//文字列を引数にとるオプションの引数リスト
//OUT_CSP_NV12, OUT_CSP_YUV444, OUT_CSP_RGB に合わせる
const ENC_OPTION_STR list_output_csp[] = {
    { "i420", AUO_MES_UNKNOWN, L"i420" },
    { "i422", AUO_MES_UNKNOWN, L"i422" },
    { "i444", AUO_MES_UNKNOWN, L"i444" },
    { "rgb",  AUO_MES_UNKNOWN, L"rgb"  },
    { NULL,   AUO_MES_UNKNOWN, NULL }
};
const ENC_OPTION_STR list_aq[] = { 
    { NULL, AUO_MES_UNKNOWN, L"none"    },
    { NULL, AUO_MES_UNKNOWN, L"VAQ"     },
    { NULL, AUO_MES_UNKNOWN, L"AutoVAQ" },
    { NULL, AUO_MES_UNKNOWN, L"MixAQ"   },
    { NULL, AUO_MES_UNKNOWN, NULL }
};
const ENC_OPTION_STR list_weightp[] = { 
    { NULL, AUO_MES_UNKNOWN, L"Disabled" }, 
    { NULL, AUO_MES_UNKNOWN, L"Simple"   }, 
    { NULL, AUO_MES_UNKNOWN, L"Smart"    }, 
    { NULL, AUO_MES_UNKNOWN, NULL } 
};
const ENC_OPTION_STR list_b_adpat[] = {
    { NULL, AUO_OPTION_BADAPT_DISABLED, L"無効" },
    { NULL, AUO_OPTION_BADAPT_SIMPLE,   L"簡易" },
    { NULL, AUO_OPTION_BADAPT_FULL,     L"完全" },
    { NULL, AUO_MES_UNKNOWN,            NULL }
};
const ENC_OPTION_STR list_trellis[] = {
    { NULL, AUO_MES_UNKNOWN, L"Disabled"        },
    { NULL, AUO_MES_UNKNOWN, L"Final MB encode" },
    { NULL, AUO_MES_UNKNOWN, L"All"             },
    { NULL, AUO_MES_UNKNOWN, NULL }
};
const ENC_OPTION_STR list_me[] = {
    { "dia",  AUO_OPTION_ME_DIA,  L"Diamond Search (高速)" },
    { "hex",  AUO_OPTION_ME_HEX,  L"Hexagonal Search"      },
    { "umh",  AUO_OPTION_ME_UMH,  L"Uneven Multi-Hexagon"  },
    { "esa",  AUO_OPTION_ME_ESA,  L"Exaustive Search"      },
    { "tesa", AUO_OPTION_ME_TESA, L"Hadamard ESA (低速)"   },
    { NULL,   AUO_MES_UNKNOWN,    NULL }
};
const ENC_OPTION_STR list_subme[] = {
    { NULL, AUO_OPTION_SUBME_00, L" 0 (fullpell only)"                },
    { NULL, AUO_OPTION_SUBME_01, L" 1 (高速)"                         },
    { NULL, AUO_OPTION_SUBME_02, L" 2"                                },
    { NULL, AUO_OPTION_SUBME_03, L" 3"                                },
    { NULL, AUO_OPTION_SUBME_04, L" 4"                                },
    { NULL, AUO_OPTION_SUBME_05, L" 5"                                },
    { NULL, AUO_OPTION_SUBME_06, L" 6 (RDO for I/P frames)"           },
    { NULL, AUO_OPTION_SUBME_07, L" 7 (RDO for all frames)"           },
    { NULL, AUO_OPTION_SUBME_08, L" 8 (RD refinement for I/P frames)" },
    { NULL, AUO_OPTION_SUBME_09, L" 9 (RD refinement for all frames)" },
    { NULL, AUO_OPTION_SUBME_10, L"10 (QP-RD)"                        },
    { NULL, AUO_OPTION_SUBME_11, L"11 (Full RD)"                      },
    { NULL, AUO_MES_UNKNOWN,     NULL }
};
const ENC_OPTION_STR list_direct[] = {
    { "none",     AUO_MES_UNKNOWN, L"none"     },
    { "spatial",  AUO_MES_UNKNOWN, L"spatial"  },
    { "temporal", AUO_MES_UNKNOWN, L"temporal" },
    { "auto",     AUO_MES_UNKNOWN, L"auto"     },
    { NULL,       AUO_MES_UNKNOWN, NULL }
};
const ENC_OPTION_STR list_cqm[] = {
    { "flat", AUO_MES_UNKNOWN, NULL },
    { "jvt",  AUO_MES_UNKNOWN, NULL },
    { NULL,   AUO_MES_UNKNOWN, NULL }
};
const ENC_OPTION_STR list_input_range[] = {
    { "auto", AUO_MES_UNKNOWN, L"auto" },
    { "pc",   AUO_MES_UNKNOWN, L"pc" },
    { NULL,   AUO_MES_UNKNOWN, NULL }
};
const ENC_OPTION_STR list_colorprim[] = {
    { "undef",     AUO_OPTION_VUI_UNDEF, L"指定なし"  },
    { "auto",      AUO_OPTION_VUI_AUTO, L"自動"      },
    { "bt709",     AUO_MES_UNKNOWN, L"bt709"     },
    { "smpte170m", AUO_MES_UNKNOWN, L"smpte170m" },
    { "bt470m",    AUO_MES_UNKNOWN, L"bt470m"    },
    { "bt470bg",   AUO_MES_UNKNOWN, L"bt470bg"   },
    { "smpte240m", AUO_MES_UNKNOWN, L"smpte240m" },
    { "film",      AUO_MES_UNKNOWN, L"film"      },
    { "bt2020",    AUO_MES_UNKNOWN, L"bt2020"    },
    { "smpte428",  AUO_MES_UNKNOWN, L"smpte428"  },
    { "smpte431",  AUO_MES_UNKNOWN, L"smpte431"  },
    { "smpte432",  AUO_MES_UNKNOWN, L"smpte432"  },
    { NULL,        AUO_MES_UNKNOWN, NULL }
};
const ENC_OPTION_STR list_transfer[] = {
    { "undef",        AUO_OPTION_VUI_UNDEF, L"指定なし" },
    { "auto",         AUO_OPTION_VUI_AUTO, L"自動"      },
    { "bt709",        AUO_MES_UNKNOWN, L"bt709"        },
    { "smpte170m",    AUO_MES_UNKNOWN, L"smpte170m"    },
    { "bt470m",       AUO_MES_UNKNOWN, L"bt470m"       },
    { "bt470bg",      AUO_MES_UNKNOWN, L"bt470bg"      },
    { "smpte240m",    AUO_MES_UNKNOWN, L"smpte240m"    },
    { "linear",       AUO_MES_UNKNOWN, L"linear"       },
    { "log100",       AUO_MES_UNKNOWN, L"log100"       },
    { "log316",       AUO_MES_UNKNOWN, L"log316"       },
    { "iec61966-2-4", AUO_MES_UNKNOWN, L"iec61966-2-4" },
    { "bt1361e",      AUO_MES_UNKNOWN, L"bt1361e"      },
    { "iec61966-2-1", AUO_MES_UNKNOWN, L"iec61966-2-1" },
    { "bt2020-10",    AUO_MES_UNKNOWN, L"bt2020-10"    },
    { "bt2020-12",    AUO_MES_UNKNOWN, L"bt2020-12"    },
    { "smpte2084",    AUO_MES_UNKNOWN, L"smpte2084"    },
    { "smpte428",     AUO_MES_UNKNOWN, L"smpte428"     },
    { "arib-std-b67", AUO_MES_UNKNOWN, L"arib-std-b67" },
    { NULL, AUO_MES_UNKNOWN,NULL }
};
const ENC_OPTION_STR list_colormatrix[] = {
    { "undef",     AUO_OPTION_VUI_UNDEF, L"指定なし"  },
    { "auto",      AUO_OPTION_VUI_AUTO, L"自動"      },
    { "bt709",     AUO_MES_UNKNOWN, L"bt709"     },
    { "smpte170m", AUO_MES_UNKNOWN, L"smpte170m" },
    { "bt470bg",   AUO_MES_UNKNOWN, L"bt470bg"   },
    { "smpte240m", AUO_MES_UNKNOWN, L"smpte240m" },
    { "YCgCo",     AUO_MES_UNKNOWN, L"YCgCo"     },
    { "fcc",       AUO_MES_UNKNOWN, L"fcc"       },
    { "GBR",       AUO_MES_UNKNOWN, L"GBR"       },
    { "bt2020nc",  AUO_MES_UNKNOWN, L"bt2020nc"  },
    { "bt2020c",   AUO_MES_UNKNOWN, L"bt2020c"   },
    { "smpte2085", AUO_MES_UNKNOWN, L"smpte2085" },
    { "chroma-derived-nc", AUO_MES_UNKNOWN,L"chroma-derived-nc" },
    { "chroma-derived-c",  AUO_MES_UNKNOWN,L"chroma-derived-c" },
    { "ICtCp",     AUO_MES_UNKNOWN, L"ICtCp" },
    { NULL, AUO_MES_UNKNOWN, NULL }
};
const ENC_OPTION_STR list_videoformat[] = {
    { "undef",     AUO_OPTION_VUI_UNDEF, L"指定なし"  },
    { "ntsc",      AUO_MES_UNKNOWN, L"ntsc"      },
    { "component", AUO_MES_UNKNOWN, L"component" },
    { "pal",       AUO_MES_UNKNOWN, L"pal"       },
    { "secam",     AUO_MES_UNKNOWN, L"secam"     },
    { "mac",       AUO_MES_UNKNOWN, L"mac"       },
    { NULL, AUO_MES_UNKNOWN, NULL } 
};
const ENC_OPTION_STR list_log_type[] = {
    { "none",    AUO_MES_UNKNOWN, L"none"    },
    { "error",   AUO_MES_UNKNOWN, L"error"   },
    { "warning", AUO_MES_UNKNOWN, L"warning" },
    { "info",    AUO_MES_UNKNOWN, L"info"    },
    { "debug",   AUO_MES_UNKNOWN, L"debug"   },
    { NULL, AUO_MES_UNKNOWN, NULL }
};
const ENC_OPTION_STR list_b_pyramid[] = {
    { "none",   AUO_MES_UNKNOWN, L"none"   },
    { "strict", AUO_MES_UNKNOWN, L"strict" },
    { "normal", AUO_MES_UNKNOWN, L"normal" },
    { NULL, AUO_MES_UNKNOWN, NULL }
};
const ENC_OPTION_STR list_nal_hrd[] = {
    { "none",   AUO_MES_UNKNOWN, L"none" },
    { "vbr",    AUO_MES_UNKNOWN, L"vbr"  },
    { "cbr",    AUO_MES_UNKNOWN, L"cbr"  },
    { NULL, AUO_MES_UNKNOWN,NULL }
};

//色についてのオプション設定(0がデフォルトとなるよう、x264とは並び順が異なる)
//まあGUIで順番なんてどうでも…
enum {
    COLOR_MATRIX_DEFAULT   = 0,
    COLOR_MATRIX_AUTO      = 1,
    COLOR_MATRIX_HD        = 2,
    COLOR_MATRIX_SD        = 3,
};
const int COLOR_MATRIX_THRESHOLD = 720;

typedef struct {
    int x;
    int y;
} INT2;

typedef struct {
    float x;
    float y;
} FLOAT2;

#pragma pack(push,4)
//x264パラメータ構造体
//guiExで使いやすいよう、適当に
//並び順に意味はない
typedef struct {
    BOOL    use_highbit_depth;
    int     output_csp;
    int     pass;
    BOOL    slow_first_pass;
    BOOL    use_auto_npass;
    int     auto_npass;
    BOOL    nul_out;
    int     preset;
    int     tune;
    int     profile;
    int     rc_mode;
    int     bitrate;
    int     crf;
    int     qp;
    float   ip_ratio;
    float   pb_ratio;
    int     qp_min;
    int     qp_max;
    int     qp_step;
    float   qp_compress;
    int     chroma_qp_offset;
    BOOL    mbtree;
    int     rc_lookahead;
    int     vbv_bufsize;
    int     vbv_maxrate;
    int     aq_mode;
    float   aq_strength;
    FLOAT2  psy_rd;
    int     scenecut;
    int     keyint_min;
    int     keyint_max;
    BOOL    open_gop;
    BOOL    cabac;
    int     bframes;
    int     b_adapt;
    int     b_bias;
    int     b_pyramid;
    int     slice_n;
    BOOL    use_deblock;
    INT2    deblock;
    BOOL    interlaced;
    BOOL    tff;
    DWORD   mb_partition;
    BOOL    dct8x8;
    int     me;
    int     subme;
    int     me_range;
    BOOL    chroma_me;
    int     direct_mv;
    int     ref_frames;
    BOOL    mixed_ref;
    BOOL    weight_b;
    int     weight_p;
    int     noise_reduction;
    BOOL    no_fast_pskip;
    BOOL    no_dct_decimate;
    int     trellis;
    int     cqm;
    int     colormatrix;
    int     colorprim;
    int     transfer;
    int     input_range;
    INT2    sar;
    int     h264_level;
    int     videoformat;
    BOOL    aud;
    BOOL    pic_struct;
    int     nal_hrd;
    BOOL    bluray_compat;
    int     threads;
    int     sliced_threading;
    int     log_mode;
    BOOL    psnr;
    BOOL    ssim;
    BOOL    use_tcfilein;
    BOOL    use_timebase;
    INT2    timebase;
    BOOL    disable_progress;
    int     lookahead_threads;
} CONF_X264;
#pragma pack(pop)

typedef struct {
    char *long_name;
    char *short_name;
    DWORD type;
    const ENC_OPTION_STR *list;
    size_t p_offset;
} X264_OPTIONS;

enum {
    ARG_TYPE_UNKNOWN = 0,
    ARG_TYPE_LONG    = 1,
    ARG_TYPE_SHORT   = 2,
};

typedef struct CMD_ARG {
    int arg_type;       //LONGかSHORTか
    char *option_name;  //オプション名(最初の"--"なし)
    char *value;        //オプションの値
    BOOL value_had_dQB; //前後に'"'があったか
    BOOL ret;           //構造体に読み込まれたかどうか
    BOOL type_mediainfo; //MediaInfoの書式だったかどうか
} CMD_ARG;

//コマンドラインの解析・生成
void set_cmd_to_conf(const char *cmd_src, CONF_X264 *conf_set);
void set_cmd_to_conf(char *cmd, CONF_X264 *conf_set, size_t cmd_len, BOOL build_not_imported_cmd);
void get_default_conf_x264(CONF_X264 *conf_set, BOOL use_highbit);
//void set_preset_to_conf(CONF_X264 *conf_set, int preset_index);
//void set_tune_to_conf(CONF_X264 *conf_set, int tune_index);
void set_profile_to_conf(CONF_X264 *conf_set, int profile_index);
void apply_presets(CONF_X264 *conf_set);
int check_profile(const CONF_X264 *conf_set);
void build_cmd_from_conf(char *cmd, size_t nSize, const CONF_X264 *conf, const void *_vid, BOOL write_all);
void set_guiEx_auto_sar(int *sar_x, int *sar_y, int width, int height);
void set_guiEx_auto_keyint(CONF_X264 *cx, int fps_num, int fps_den);
void apply_guiEx_auto_settings(CONF_X264 *cx, int width, int height, int fps_num, int fps_den, BOOL ref_limit_by_level);
const ENC_OPTION_STR * get_option_list(const char *option_name);
void set_ex_stg_ptr(guiEx_settings *_ex_stg);
int get_option_value(const char *cmd_src, const char *target_option_name, char *buffer, size_t nSize);
int get_list_index(const ENC_OPTION_STR *list, const char *name);

#endif //_AUO_OPTIONS_H_
