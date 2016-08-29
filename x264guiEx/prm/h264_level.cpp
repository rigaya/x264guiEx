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

#include "auo.h"
#include "auo_util.h"
#include "h264_level.h"

const int MAX_REF_FRAMES = 16;
const int PROGRESSIVE    = 1;
const int INTERLACED     = 2;
const int LEVEL_COLUMNS  = 7;
const int COLUMN_VBVMAX  = 4;
const int COLUMN_VBVBUF  = 5;

static const int H264_LEVEL_LIMITS[][LEVEL_COLUMNS] =
{   //interlaced, MaxMBpsec, MaxMBpframe, MaxDpbMbs, MaxVBVMaxrate, MaxVBVBuf,    end,  level
    { PROGRESSIVE,        -1,          -1,        -1,             0,         0,  NULL}, // auto
    { PROGRESSIVE,      1485,          99,       396,            64,       175,  NULL}, // 1
    { PROGRESSIVE,      1485,          99,       396,           128,       350,  NULL}, // 1b
    { PROGRESSIVE,      3000,         396,       900,           192,       500,  NULL}, // 1.1
    { PROGRESSIVE,      6000,         396,      2376,           384,      1000,  NULL}, // 1.2
    { PROGRESSIVE,     11880,         396,      2376,           768,      2000,  NULL}, // 1.3
    { PROGRESSIVE,     11880,         396,      2376,          2000,      2000,  NULL}, // 2
    {  INTERLACED,     19800,         792,      4752,          4000,      4000,  NULL}, // 2.1
    {  INTERLACED,     20250,        1620,      8100,          4000,      4000,  NULL}, // 2.2
    {  INTERLACED,     40500,        1620,      8100,         10000,     10000,  NULL}, // 3
    {  INTERLACED,    108000,        3600,     18000,         14000,     14000,  NULL}, // 3.1
    {  INTERLACED,    216000,        5120,     20480,         20000,     20000,  NULL}, // 3.2
    {  INTERLACED,    245760,        8192,     32768,         20000,     25000,  NULL}, // 4
    {  INTERLACED,    245760,        8192,     32768,         50000,     62500,  NULL}, // 4.1
    {  INTERLACED,    522240,        8704,     34816,         50000,     62500,  NULL}, // 4.2
    { PROGRESSIVE,    589824,       22080,    110400,        135000,    135000,  NULL}, // 5
    { PROGRESSIVE,    983040,       36864,    184320,        240000,    240000,  NULL}, // 5.1
    { PROGRESSIVE,   2073600,       36864,    184320,        240000,    240000,  NULL}, // 5.2
    {        NULL,      NULL,        NULL,      NULL,          NULL,      NULL,  NULL}, // end
};

//必要なLevelを計算する, 適合するLevelがなければ 0 を返す
int calc_auto_level(int width, int height, int ref, BOOL interlaced, int fps_num, int fps_den, int vbv_max, int vbv_buf) {
    int i, j = (interlaced) ? INTERLACED : PROGRESSIVE;
    int MB_frame = ceil_div_int(width, 16) * (j * ceil_div_int(height, 16*j));
    int data[LEVEL_COLUMNS] = {
        j,
        (int)ceil_div_int64((UINT64)MB_frame * fps_num, fps_den),
        MB_frame,
        MB_frame * ref,
        vbv_max, 
        vbv_buf,
        NULL
    };

    //あとはひたすら比較
    i = 1, j = 0; // i -> 行(Level), j -> 列(項目)
    while (H264_LEVEL_LIMITS[i][j])
        (data[j] > H264_LEVEL_LIMITS[i][j]) ? i++ : j++;
    //一番右の列まで行き着いてればそれが求めるレベル 一応インターレースについても再チェック
    return (j == (LEVEL_COLUMNS-1) && data[0] <= H264_LEVEL_LIMITS[i][0]) ? i : 0;
}

//vbv値を求める *vbv_max と *vbv_buf はNULLでもOK
void get_vbv_value(int *vbv_max, int *vbv_buf, int level, int profile_index, int use_highbit, guiEx_settings *ex_stg) {
    if (level > 0 && H264_LEVEL_LIMITS[level][1] > 0) {
        //high10 profileを使うかどうか
        if (use_highbit && _stricmp(ex_stg->s_x264.profile.name[profile_index].name, "high") == NULL) {
            for (int i = 0; i < ex_stg->s_x264.profile_count; i++)
                if (_stricmp(ex_stg->s_x264.profile.name[i].name, "high10") == NULL) {
                    profile_index = i;
                    break;
                }
        }
        float profile_multi = ex_stg->s_x264.profile_vbv_multi[profile_index];

        if (vbv_max)
            *vbv_max = (int)(H264_LEVEL_LIMITS[level][COLUMN_VBVMAX] * profile_multi);
        if (vbv_buf)
            *vbv_buf = (int)(H264_LEVEL_LIMITS[level][COLUMN_VBVBUF] * profile_multi);
    } else {
        if (vbv_max)
            *vbv_max = 0;
        if (vbv_buf)
            *vbv_buf = 0;
    }
    return;
}

int get_ref_limit(int level, int width, int height, BOOL interlaced) {
    if (level <= 0)
        return 16;

    const int j = (interlaced) ? INTERLACED : PROGRESSIVE;
    const int MB_frame = ceil_div_int(width, 16) * (j * ceil_div_int(height, 16*j));

    return min(H264_LEVEL_LIMITS[level][3] / MB_frame, MAX_REF_FRAMES);
}
