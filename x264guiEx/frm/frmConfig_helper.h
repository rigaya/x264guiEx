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

#pragma once

using namespace System;
using namespace System::Text;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Threading;
using namespace System::Diagnostics;

#include "auo_options.h"
#include "auo_mes.h"

namespace x264guiEx {

    ref class LocalSettings
    {
    public:
        String^ x264ExeName;
        String^ x264Path;
        List<String^>^ audEncName;
        List<String^>^ audEncExeName;
        List<String^>^ audEncPath;
        String^ MP4MuxerExeName;
        String^ MP4MuxerPath;
        String^ MKVMuxerExeName;
        String^ MKVMuxerPath;
        String^ TC2MP4ExeName;
        String^ TC2MP4Path;
        String^ MPGMuxerExeName;
        String^ MPGMuxerPath;
        String^ MP4RawExeName;
        String^ MP4RawPath;
        String^ CustomTmpDir;
        String^ CustomAudTmpDir;
        String^ CustomMP4TmpDir;
        String^ LastAppDir;
        String^ LastBatDir;

        LocalSettings() {
            audEncName = gcnew List<String^>();
            audEncExeName = gcnew List<String^>();
            audEncPath = gcnew List<String^>();
        }
        ~LocalSettings() {
            delete audEncName;
            delete audEncExeName;
            delete audEncPath;
        }
    };

    ref class TBValueBitrateConvert
    {
        value struct TBData {
            int bitrate;
            int step;
            int count;
        };
        array<TBData>^ data;
        ~TBValueBitrateConvert() {
            delete data;
        }
    public:
        TBValueBitrateConvert() {
            data = gcnew array<TBData> {
                {      0,       5,   0 }, //    0 - 1000 までのstepは  5単位
                {   1000,      50,   0 }, // 1000 - 2000 までのstepは 50単位
                {   2000,     100,   0 }, //以下同様
                {   4000,     200,   0 },
                {   8000,    1000,   0 },
                {  64000,    4000,   0 },
                { 256000,   32000,   0 },
                { 256000, INT_MAX,   0 }
            };
            //そこまでのカウント数を計算しておく
            for (int i = 0; i < data->Length - 2; i++)
                data[i+1].count = data[i].count + (data[i+1].bitrate - data[i].bitrate) / data[i].step;
        };
        int getMaxCount() //最大のカウント数(TBの最大値)
        {
            return data[data->Length - 2].count;
        };
        int BitrateToTB(int rate) //ビットレートをTBの値に換算
        {
            for (int i = data->Length - 2; i >= 0; i--)
                if (data[i].bitrate <= (int)min((DWORD)INT_MAX, (DWORD)rate)) //int:-1をDWORD:0xffffffffとして扱い、最大値として設定
                    return data[i].count + (int)((rate - data[i].bitrate) / (double)data[i].step);
            return 0;
        };
        int TBToBitrate(int TBValue) //TBの値からビットレートに
        {
            for (int i = data->Length - 2; i >= 0; i--)
                if (data[i].count <= TBValue)
                    return data[i].bitrate + (TBValue - data[i].count) * data[i].step;
            return 0;
        };
    };
//
//    ref class stgFileController
//    {
//        List<String^>^ stgPath; //stgファイルのフルパスリスト
//        List<String^>^ stgName; //stgファイル名(拡張子なし)のリスト
//        String^ stgDir; //stgファイルのディレクトリ
//
//        System::Boolean CheckStgDir() //stgファイルのディレクトリが存在するか、なければ作成
//        {
//            if (stgDir == nullptr)
//                return false;
//            if (!Directory::Exists(stgDir)) {
//                try {
//                    Directory::CreateDirectory(stgDir);
//                } catch (...) {
//                    return false;
//                }
//            }
//            return true;
//        }
//        System::Void Clear() //リストをクリア
//        {
//            stgPath->Clear();
//            stgName->Clear();
//        }
//
//    public:
//        stgFileController(String^ _stgDir)
//        {
//            stgDir = _stgDir;
//            stgPath = gcnew List<String^>();
//            stgName = gcnew List<String^>();
//        }
//        ~stgFileController()
//        {
//            delete stgPath;
//            delete stgName;
//        }
//        System::Void ReLoad()
//        {
//            Clear();
//            if (CheckStgDir()) {
//                stgPath->AddRange(Directory::GetFiles(stgDir, L"*.stg", SearchOption::AllDirectories));
//                for (int i = 0; i < stgPath->Count; i++)
//                    stgName->Add(Path::GetFileNameWithoutExtension(stgPath[i]));
//            }
//        }
//        System::Void ReLoad(String^ _stgDir)
//        {
//            stgDir = _stgDir;
//            ReLoad();
//        }
//        List<String^>^ getStgNameList() //stgファイル名(拡張子なし)のリストを取得
//        {
//            return stgName;
//        }
//        String^ getStgFullPath(int index) //stgフルパスを返す
//        {
//            return (index >= 0 && index < stgPath->Count) ? stgPath[index] : L"";
//        }
//        int AddStgToList(String^ _stgName) //ファイル名を追加、すでに存在するファイル名ならそのインデックスを返す
//        {
//            //_stgNameには拡張子なしのものを渡す
//            //すでに存在するか確認
//            for (int i = 0; i < stgName->Count; i++)
//                if (String::Compare(stgName[i], _stgName, true) == 0)
//                    return i;
//            stgPath->Add(Path::Combine(stgDir, _stgName + L".stg"));
//            stgName->Add(_stgName);
//            return stgName->Count - 1;
//        }
//        void DeleteFromList(int index) //インデックスのファイルを削除
//        {
//            stgPath->RemoveAt(index);
//            stgName->RemoveAt(index);
//        }
//    };


    value struct ExeControls
    {
        String^ Name;
        String^ Path;
        const char* args;
    };

    const int fcgTBQualityTimerLatency = 600;
    const int fcgTBQualityTimerPeriod = 40;
    const int fcgTXCmdfulloffset = 57;
    const int fcgCXAudioEncModeSmallWidth = 189;
    const int fcgCXAudioEncModeLargeWidth = 237;
};

//コンボボックスの表示名

#define STR_BITRATE_START L"-1: "
#define STR_BITRATE_AUTO (STR_BITRATE_START + LOAD_CLI_STRING(AUO_CONFIG_BITRATE_AUTO))

const int x264_encmode_to_RCint[] = {
    X264_RC_BITRATE,
    X264_RC_QP,
    X264_RC_CRF,
    X264_RC_BITRATE,
    X264_RC_BITRATE,
    X264_RC_BITRATE,
    X264_RC_CRF,
    NULL
};

const int x264_encmode_to_passint[] = {
    0,
    0,
    0,
    1,
    3,
    3,
    NULL
};

static const X264_OPTION_STR x264_encodemode_desc[] = {
    { NULL, AUO_CONFIG_CX_X264_ENCMODE_SINGLE_CBR,     L"シングルパス - ビットレート指定"        },
    { NULL, AUO_CONFIG_CX_X264_ENCMODE_SINGLE_QP,      L"シングルパス - 固定量子化量"            },
    { NULL, AUO_CONFIG_CX_X264_ENCMODE_SINGLE_CRF,     L"シングルパス - 品質基準VBR(可変レート)" },
    { NULL, AUO_CONFIG_CX_X264_ENCMODE_MULTI_1PASS,    L"マルチパス - 1pass"                     },
    { NULL, AUO_CONFIG_CX_X264_ENCMODE_MULTI_NPASS,    L"マルチパス - Npass"                     },
    { NULL, AUO_CONFIG_CX_X264_ENCMODE_AMP,            L"自動マルチパス"                         },
    { NULL, AUO_CONFIG_CX_X264_ENCMODE_CRF_WITH_CHECK, L"サイズ確認付 品質基準VBR(可変レート)"   },
    { NULL, AUO_MES_UNKNOWN, NULL }
};

static const X264_OPTION_STR aspect_desc[] = {
    { NULL, AUO_CONFIG_CX_ASPECT_SAR, L"SAR比を指定 (デフォルト)" },
    { NULL, AUO_CONFIG_CX_ASPECT_DAR, L"画面比から自動計算"       },
    { NULL, AUO_MES_UNKNOWN, NULL }
};

static const X264_OPTION_STR tempdir_desc[] = {
    { NULL, AUO_CONFIG_CX_TEMPDIR_OUTDIR, L"出力先と同じフォルダ (デフォルト)" },
    { NULL, AUO_CONFIG_CX_TEMPDIR_SYSTEM, L"システムの一時フォルダ"            },
    { NULL, AUO_CONFIG_CX_TEMPDIR_CUSTOM, L"カスタム"                          },
    { NULL, AUO_MES_UNKNOWN, NULL }
};

static const X264_OPTION_STR audtempdir_desc[] = {
    { NULL, AUO_CONFIG_CX_AUDTEMP_DEFAULT, L"変更しない" },
    { NULL, AUO_CONFIG_CX_AUDTEMP_CUSTOM,  L"カスタム"   },
    { NULL, AUO_MES_UNKNOWN, NULL }
};

static const X264_OPTION_STR mp4boxtempdir_desc[] = {
    { NULL, AUO_CONFIG_CX_MP4BOXTEMP_DEFAULT, L"指定しない" },
    { NULL, AUO_CONFIG_CX_MP4BOXTEMP_CUSTOM,  L"カスタム"   },
    { NULL, AUO_MES_UNKNOWN, NULL }
};

static const X264_OPTION_STR interlaced_desc[] = {
    { NULL, AUO_CONFIG_CX_INTERLACE_PROGRESSIVE, L"プログレッシブ" },
    { NULL, AUO_CONFIG_CX_INTERLACE_TFF,         L"インタレ (tff)" },
    { NULL, AUO_CONFIG_CX_INTERLACE_BFF,         L"インタレ (bff)" },
    { NULL, AUO_MES_UNKNOWN, NULL }
};

static const X264_OPTION_STR audio_enc_timing_desc[] = {
    { NULL, AUO_CONFIG_CX_AUD_ENC_ORDER_AFTER,    L"後"   },
    { NULL, AUO_CONFIG_CX_AUD_ENC_ORDER_BEFORE,   L"前"   },
    { NULL, AUO_CONFIG_CX_AUD_ENC_ORDER_PARALLEL, L"同時" },
    { NULL, AUO_MES_UNKNOWN, NULL }
};


//メモ表示用 RGB
const int StgNotesColor[][3] = {
    {  80,  72,  92 },
    { 120, 120, 120 }
};

const WCHAR * const DefaultStgNotes = L"メモ...";
const WCHAR * const DefaultStatusFilePath = L"%{savfile}.stats";
const WCHAR * const DefaultTcFilePath = L"%{savfile}_tc.txt";

typedef struct {
    WCHAR *string;
    AuoMes mes;
    WCHAR *desc;
} REPLACE_STRINGS;

const REPLACE_STRINGS REPLACE_STRINGS_LIST[] = {
    { L"%{vidpath}",           AUO_CONFIG_CX_REPLACE_VID_PATH,         L"一時動画ファイル名(フルパス)" },
    { L"%{audpath}",           AUO_CONFIG_CX_REPLACE_AUD_PATH,         L"一時音声ファイル名(フルパス)" },
    { L"%{tmpdir}",            AUO_CONFIG_CX_REPLACE_TMPDIR,           L"一時フォルダ名(最後の\\無し)" },
    { L"%{tmpfile}",           AUO_CONFIG_CX_REPLACE_TMPFILE,          L"一時ファイル名(フルパス・拡張子除く)" },
    { L"%{tmpname}",           AUO_CONFIG_CX_REPLACE_TMPNAME,          L"一時ファイル名(ファイル名のみ・拡張子除く)" },
    { L"%{savpath}",           AUO_CONFIG_CX_REPLACE_SAVPATH,          L"出力ファイル名(フルパス)" },
    { L"%{savfile}",           AUO_CONFIG_CX_REPLACE_SAVFILE,          L"出力ファイル名(フルパス・拡張子除く)" },
    { L"%{savname}",           AUO_CONFIG_CX_REPLACE_SAVNAME,          L"出力ファイル名(ファイル名のみ・拡張子除く)" },
    { L"%{savdir}",            AUO_CONFIG_CX_REPLACE_SAVDIR,           L"出力フォルダ名(最後の\\無し)" },
    { L"%{aviutldir}",         AUO_CONFIG_CX_REPLACE_AVIUTLDIR,        L"Aviutl.exeのフォルダ名(最後の\\無し)" },
    { L"%{chpath}",            AUO_CONFIG_CX_REPLACE_CHPATH,           L"チャプターファイル名(フルパス)" },
    { L"%{tcpath}",            AUO_CONFIG_CX_REPLACE_TCPATH,           L"タイムコードファイル名(フルパス)" },
    { L"%{muxout}",            AUO_CONFIG_CX_REPLACE_MUXOUT,           L"muxで作成する一時ファイル名(フルパス)" },
    { L"%{x264path}",          AUO_CONFIG_CX_REPLACE_X264PATH,         L"指定された x264.exe のパス" },
    { L"%{x264_10path}",       AUO_CONFIG_CX_REPLACE_X264_10PATH,      L"指定された x264.exe(10bit版) のパス" },
    { L"%{audencpath}",        AUO_CONFIG_CX_REPLACE_AUDENCPATH,       L"実行された音声エンコーダのパス" },
    { L"%{mp4muxerpath}",      AUO_CONFIG_CX_REPLACE_MP4MUXERPATH,     L"mp4 muxerのパス" },
    { L"%{mkvmuxerpath}",      AUO_CONFIG_CX_REPLACE_MKVMUXERPATH,     L"mkv muxerのパス" },
    { L"%{fps_scale}",         AUO_CONFIG_CX_REPLACE_FPS_SCALE,        L"フレームレート(分母)" },
    { L"%{fps_rate}",          AUO_CONFIG_CX_REPLACE_FPS_RATE,         L"フレームレート(分子)" },
    { L"%{fps_rate_times_4}",  AUO_CONFIG_CX_REPLACE_FPS_RATE_TIMES_4, L"フレームレート(分子)×4" },
    { L"%{sar_x}",             AUO_CONFIG_CX_REPLACE_SAR_X,            L"サンプルアスペクト比 (横)" },
    { L"%{sar_y}",             AUO_CONFIG_CX_REPLACE_SAR_Y,            L"サンプルアスペクト比 (縦)" },
    { L"%{dar_x}",             AUO_CONFIG_CX_REPLACE_DAR_X,            L"画面アスペクト比 (横)" },
    { L"%{dar_y}",             AUO_CONFIG_CX_REPLACE_DAR_Y,            L"画面アスペクト比 (縦)" },

    { NULL, AUO_MES_UNKNOWN, NULL }
};
