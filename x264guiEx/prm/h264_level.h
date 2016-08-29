// -----------------------------------------------------------------------------------------
// x264guiEx by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2010-2016 rigaya
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

const X264_OPTION_STR list_x264guiEx_level[] = { 
    { "auto", L"自動" },
    { "1",    L"1"    },
    { "1b",   L"1b"   },
    { "1.1",  L"1.1"  },
    { "1.2",  L"1.2"  },
    { "1.3",  L"1.3"  },
    { "2",    L"2"    },
    { "2.1",  L"2.1"  },
    { "2.2",  L"2.2"  },
    { "3",    L"3"    },
    { "3.1",  L"3.1"  },
    { "3.2",  L"3.2"  },
    { "4",    L"4"    },
    { "4.1",  L"4.1"  },
    { "4.2",  L"4.2"  },
    { "5",    L"5"    },
    { "5.1",  L"5.1"  },
    { "5.2",  L"5.2"  },
    { NULL, NULL }
};

int calc_auto_level(int width, int height, int ref, BOOL interlaced, int fps_num, int fps_den, int vbv_max, int vbv_buf);
void get_vbv_value(int *vbv_max, int *vbv_buf, int level, int profile_index, int use_highbit, guiEx_settings *ex_stg);
int get_ref_limit(int level, int width, int height, BOOL interlaced);

#endif //_H264_LEVEL_H_
