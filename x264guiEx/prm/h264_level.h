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

#ifndef _H264_LEVEL_H_
#define _H264_LEVEL_H_

#include "auo.h"
#include "auo_settings.h"
#include "auo_options.h"
#include "rgy_tchar.h"

#if ENCODER_X264

static const ENC_OPTION_STR list_x264guiEx_level[] = {
    { _T("auto"), AUO_OPTION_LEVEL_AUTO, L"自動" },
    { _T("1"),    AUO_MES_UNKNOWN, L"1"    },
    { _T("1b"),   AUO_MES_UNKNOWN, L"1b"   },
    { _T("1.1"),  AUO_MES_UNKNOWN, L"1.1"  },
    { _T("1.2"),  AUO_MES_UNKNOWN, L"1.2"  },
    { _T("1.3"),  AUO_MES_UNKNOWN, L"1.3"  },
    { _T("2"),    AUO_MES_UNKNOWN, L"2"    },
    { _T("2.1"),  AUO_MES_UNKNOWN, L"2.1"  },
    { _T("2.2"),  AUO_MES_UNKNOWN, L"2.2"  },
    { _T("3"),    AUO_MES_UNKNOWN, L"3"    },
    { _T("3.1"),  AUO_MES_UNKNOWN, L"3.1"  },
    { _T("3.2"),  AUO_MES_UNKNOWN, L"3.2"  },
    { _T("4"),    AUO_MES_UNKNOWN, L"4"    },
    { _T("4.1"),  AUO_MES_UNKNOWN, L"4.1"  },
    { _T("4.2"),  AUO_MES_UNKNOWN, L"4.2"  },
    { _T("5"),    AUO_MES_UNKNOWN, L"5"    },
    { _T("5.1"),  AUO_MES_UNKNOWN, L"5.1"  },
    { _T("5.2"),  AUO_MES_UNKNOWN, L"5.2"  },
    { _T("6"),    AUO_MES_UNKNOWN, L"6"    },
    { _T("6.1"),  AUO_MES_UNKNOWN, L"6.1"  },
    { _T("6.2"),  AUO_MES_UNKNOWN, L"6.2"  },
    { NULL,   AUO_MES_UNKNOWN, NULL }
};

int calc_auto_level(int width, int height, int ref, BOOL interlaced, int fps_num, int fps_den, int vbv_max, int vbv_buf);
void get_vbv_value(int *vbv_max, int *vbv_buf, int level, int profile_index, int use_highbit, guiEx_settings *ex_stg);
int get_ref_limit(int level, int width, int height, BOOL interlaced);

#endif

#endif //_H264_LEVEL_H_
