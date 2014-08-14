//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx/2.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

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

#endif //_H264_LEVEL_H_
