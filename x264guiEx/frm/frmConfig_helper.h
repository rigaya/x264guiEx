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

namespace x264guiEx {

    ref class LocalSettings 
    {
    public:
        String^ x264ExeName;
        String^ x264Path;
        String^ x264Pathhighbit;
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
                {     0,       5,   0 }, //    0 - 1000 までのstepは  5単位
                {  1000,      50,   0 }, // 1000 - 2000 までのstepは 50単位
                {  2000,     100,   0 }, //以下同様
                {  4000,     200,   0 },
                {  8000,    1000,   0 },
                { 54000, INT_MAX,   0 },
                { 54000, INT_MAX,   0 }
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

    public delegate void DelegateMessage(String^ mes);
    public delegate void DelegateProcessFin(String^ mes);

    ref class auoSetupControl {
    private:
        //各実行ファイルのバージョン情報
        value struct VersionInfo {
            String^ name;        //名前
            String^ currentVer;  //現在指定されているexeファイルのバージョン
            String^ LatestVer;   //最新のバージョン
        };
        Thread^ thRun;                    //更新実行用のスレッド
        Thread^ thCheck;                  //更新チェック用のスレッド
        List<VersionInfo>^ verInfo;       //バージョン情報のリスト
        DataTable^ dataTableVersionInfo;  //バージョン情報をDataTableに変換したもの
        DelegateMessage^ mesFunc;         //auoSetupのメッセージを転送する
        DelegateProcessFin^ finFuncCheck; //更新・更新チェック終了時に行うDelegate
        DelegateProcessFin^ finFuncRun;   //更新・更新チェック終了時に行うDelegate
        String^ aviutlDir;                //Aviutlフォルダ
        String^ iniFileName;              //対象iniファイル名
        String^ auoDir;                   //auoのあるフォルダ
        String^ auoSetupFilePath;         //auoSetupのパス
        String^ auoSetupDir;              //auoSetupを入れたフォルダのひとつ上のフォルダ
        StringBuilder^ exeMesBuffer;      //auoSetupの出力を更新チェック時に受け取るバッファ
        HANDLE hEventAbort;               //auoSetupを停止するためのイベント

        static initonly array<String^>^ TARGET_LIST = { L"x264", L"x265", L"qaac", L"l-smash" };
        static initonly String^ VERSION_SAME_AS_CURRENT = L" ⇐";
        static initonly array<String^>^ RESULT_MES = {
            L"更新は必要ありません。", L"更新することができます。", L"エラーが発生しました。", L"中止しました。"
        };
    public:
        static initonly array<String^>^ RESULT_STRING = { L"完了", L"完了", L"失敗", L"中止" };
        enum class auoSetupResult {
            unknown = -1,
            success = 0,
            success_with_info = 1,
            error = 2,
            abort = 3,
        };
    public:
        static auoSetupResult strToResult(String^ str) {
            for (int i = 0; i < RESULT_STRING->Length; i++)
                if (0 <= str->IndexOf(RESULT_STRING[i]))
                    return static_cast<auoSetupResult>(i);
            return auoSetupResult::unknown;
        }
        //更新チェック時のメッセージから更新が可能かを返す
        static bool checkIfUpdateAvailable(String^ message) {
            for (int i = 0; i < auoSetupControl::RESULT_MES->Length; i++) {
                if (0 == String::Compare(message, auoSetupControl::RESULT_MES[i])) {
                    return (i == static_cast<int>(auoSetupResult::success_with_info));
                }
            }
            return false;
        }
    public:
        auoSetupControl() {
            mesFunc = nullptr;
            finFuncRun = nullptr;
            finFuncCheck = nullptr;
            verInfo = nullptr;
            thCheck = nullptr;
            thRun = nullptr;
            hEventAbort = NULL;
            dataTableVersionInfo = nullptr;
            exeMesBuffer = nullptr;
        }
        ~auoSetupControl() {
            close();
        }
    private:
        void addToBuffer(String^ mes) {
            if (nullptr == mes)
                return;

            if (nullptr == exeMesBuffer) {
                exeMesBuffer = gcnew StringBuilder();
            }
            exeMesBuffer->Append(mes);
        }
    private:
        static System::Boolean directoryRequiresAdmin(String^ Dir) {
            if (nullptr == Dir)
                return false;
            String^ FullDirectory;
            try {
                if (Path::IsPathRooted(Dir)) {
                    FullDirectory = Dir;
                } else {
                    FullDirectory = Path::GetFullPath(Dir);
                }
            } catch (...) {
                return true;
            }
            bool required = false;
            List<String^>^ blackListDir = gcnew List<String^>();
            blackListDir->Add(System::Environment::ExpandEnvironmentVariables(L"%ProgramFiles%"));
            blackListDir->Add(System::Environment::ExpandEnvironmentVariables(L"%ProgramFiles(x86)%"));
            blackListDir->Add(System::Environment::ExpandEnvironmentVariables(L"%ProgramW6432%"));
            blackListDir->Add(System::Environment::ExpandEnvironmentVariables(L"%SystemRoot%"));
            for (int i = 0; i < blackListDir->Count; i++) {
                if (0 <= FullDirectory->IndexOf(blackListDir[i], StringComparison::OrdinalIgnoreCase)) {
                    required = true;
                    break;
                }
            }
            delete blackListDir;
            return required;
        }
    private:
        //auoSetup.exe用のフォルダを作成し、
        //リソースからauoSetup.exeを取り出す
        bool createAuoSetupExeFile() {
            bool result = true;
            if (directoryRequiresAdmin(auoDir)) {
                addToBuffer(L"Aviutlが管理者権限の必要な場所にインストールされています。\r\n");
                addToBuffer(L"更新を実行できません。\r\n");
                return false;
            }
            //x264guiExのあるところから2階層掘って、そこにauoSetup.exeを作成
            //2階層掘らないとauoSetup.exeの仕様上、うまく動かない
            //zipを展開したフォルダ内のx264guiEx.auoと、インストール先のx264guiEx.auoを区別する
            //searchFileExceptInstallDirが誤動作する
            try {
                auoSetupDir = Path::Combine(auoDir, L"setup");
                if (!Directory::Exists(auoSetupDir))
                    Directory::CreateDirectory(auoSetupDir);
                auoSetupFilePath = Path::Combine(auoSetupDir, L"setup");
                if (!Directory::Exists(auoSetupFilePath))
                    Directory::CreateDirectory(auoSetupFilePath);
                auoSetupFilePath = Path::Combine(auoSetupFilePath, L"auoSetup.exe");

                char auoSetup_file_path[MAX_PATH_LEN] ={ 0 };
                GetCHARfromString(auoSetup_file_path, _countof(auoSetup_file_path), auoSetupFilePath);
                if (AUO_RESULT_SUCCESS != create_auoSetup(auoSetup_file_path)) {
                    result = false;
                }
            } catch (...) {
                addToBuffer(L"更新用の一時フォルダの作成に失敗しました。\r\n");
                addToBuffer(L"更新を実行できません。\r\n");
                result = false;
            }
            return result;
        }
    public:
        void init() {
            close();

            dataTableVersionInfo = gcnew DataTable();
            dataTableVersionInfo->Columns->Add(L"実行ファイル");
            dataTableVersionInfo->Columns->Add(L"現在のバージョン");
            dataTableVersionInfo->Columns->Add(L"最新バージョン");

            char aviutldir[MAX_PATH_LEN] ={ 0 };
            get_aviutl_dir(aviutldir, _countof(aviutldir));
            aviutlDir = String(aviutldir).ToString();

            char ini_filename[MAX_PATH_LEN] = { 0 };
            char auo_dir[MAX_PATH_LEN] = { 0 };
            get_auo_path(auo_dir, _countof(auo_dir));
            apply_appendix(ini_filename, _countof(ini_filename), auo_dir, ".ini");
            iniFileName = String(ini_filename).ToString();

            PathRemoveFileSpecFixed(auo_dir);
            auoDir = String(auo_dir).ToString();
        }
    private:
        void setVerionInfoToDataTable() {
            //読み取り結果をdataTableにセット
            for each (auto info in verInfo) {
                //すでに行が存在すれば、その値を書き換える
                bool target_found = false;
                for (int i_row = 0; i_row < dataTableVersionInfo->Rows->Count; i_row++) {
                    DataRow^ row = dataTableVersionInfo->Rows[i_row];
                    if (0 == String::Compare(row[0]->ToString(), info.name)) {
                        row[1] = info.currentVer;
                        row[2] = info.LatestVer;
                        target_found = true;
                        break;
                    }
                }
                //存在しなければ、新しい行を追加する
                if (!target_found) {
                    DataRow^ drb = dataTableVersionInfo->NewRow();
                    drb[0] = info.name;
                    drb[1] = info.currentVer;
                    drb[2] = info.LatestVer;
                    dataTableVersionInfo->Rows->Add(drb);
                }
            }
        }
    public:
        DataTable^ getVersionInfoDataTable() {
            return dataTableVersionInfo;
        }
    public:
        //非同期で更新チェック、終了時にfuncFinを実行する
        int checkAsync(DelegateProcessFin^ funcFin) {
            mesFunc = gcnew DelegateMessage(this, &auoSetupControl::addToBuffer);
            finFuncCheck = funcFin;
            thCheck = gcnew Thread(gcnew ThreadStart(this, &auoSetupControl::check));
            thCheck->Start();
            return 0;
        }
    public:
        //非同期で更新、終了時にfuncFinを実行する
        int runAsync(DelegateMessage^ _mesFunc, DelegateProcessFin^ _funcFin) {
            mesFunc = _mesFunc + gcnew DelegateMessage(this, &auoSetupControl::addToBuffer);
            finFuncRun = _funcFin;
            thCheck = gcnew Thread(gcnew ThreadStart(this, &auoSetupControl::run));
            thCheck->Start();
            return 0;
        }
    public:
        System::Void abortAuoSetup() {
            if (NULL != hEventAbort)
                SetEvent(hEventAbort);
        }
    private:
        System::Void check() {
            auoSetupResult result = auoSetupResult::error;
            if (runAuoSetup(L"-nogui -check-only")) {
                array<String^>^ delimiterLine = { L"\r\n", L"\r", L"\n" };
                if (nullptr != exeMesBuffer && exeMesBuffer->ToString()->Length > 0) {
                    array<String^>^ exeMesLines = exeMesBuffer->ToString()->Split(delimiterLine, StringSplitOptions::None);
                    result = auoSetupResult::success;

                    if (nullptr != verInfo) { delete verInfo; }
                    verInfo = gcnew List<VersionInfo>();
                    //各実行ファイルの情報をチェック
                    //各行ごとに、各実行ファイルについてチェック
                    for each (auto targetLine in exeMesLines) {
                        for each (auto targetName in TARGET_LIST) {
                            if (0 <= targetLine->IndexOf(targetName)) {
                                VersionInfo info;
                                info.name = targetName;
                                array<String^>^ delimiterMes = { L"。", L"→" };
                                array<String^>^ splittedMes = targetLine->Trim()->Split(delimiterMes, StringSplitOptions::None);
                                if (2 <= splittedMes->Length) {
                                    if (3 <= splittedMes->Length
                                        && 0 <= splittedMes[0]->IndexOf(L"更新します")) {
                                        info.currentVer = splittedMes[1]->Trim();
                                        info.LatestVer = splittedMes[2]->Trim();
                                        verInfo->Add(info);
                                        result = auoSetupResult::success_with_info;
                                    } else if (0 <= splittedMes[0]->IndexOf(L"ダウンロードします")) {
                                        info.currentVer = L" - ";
                                        info.LatestVer = splittedMes[1]->Trim();
                                        verInfo->Add(info);
                                        result = auoSetupResult::success_with_info;
                                    } else if (0 <= splittedMes[0]->IndexOf(L"必要ありません")) {
                                        info.currentVer = splittedMes[1]->Trim();
                                        info.LatestVer = VERSION_SAME_AS_CURRENT;
                                        verInfo->Add(info);
                                    }
                                }
                                break;
                            }
                        }
                    }
                    setVerionInfoToDataTable();
                }
            }
            finFuncCheck(RESULT_MES[static_cast<int>(result)]);
        }
    public:
        System::Void run() {
            //オプションの決定
            String ^options = L"-nogui -update-exe-only -no-nero";
            {
                guiEx_settings ex_stg;
                ex_stg.load_encode_stg();
                if (ex_stg.s_local.update_overwrite) {
                    options += L" -update-overwrite";
                }
            }
            for each (auto info in verInfo) {
                if (0 == String::Compare(info.LatestVer, VERSION_SAME_AS_CURRENT)) {
                    options += L" -no-" + info.name;
                }
            }
            //実行
            runAuoSetup(options);
            //メッセージ解析
            auoSetupResult result = auoSetupResult::unknown;
            array<String^>^ delimiterLine = { L"\r\n", L"\r", L"\n" };
            if (nullptr != exeMesBuffer && exeMesBuffer->ToString()->Length > 0) {
                array<String^>^ exeMesLines = exeMesBuffer->ToString()->Split(delimiterLine, StringSplitOptions::None);
                //最後の5行分ぐらいには結果があるはず
                for (int i = 0, i_line = exeMesLines->Length - 1; 0 > static_cast<int>(result) && i < 5 && i_line >= 0; i++, i_line--)
                    for (int j = 0; j < RESULT_STRING->Length; j++)
                        if (0 <= exeMesLines[i_line]->IndexOf(RESULT_STRING[j]))
                            result = static_cast<auoSetupResult>(j);
                //unknownのままなのもおかしいのでこれもエラー
                if (auoSetupResult::unknown == result)
                    result = auoSetupResult::error;
                //終了処理
                finFuncRun(RESULT_STRING[static_cast<int>(result)]);
                //DataTableの更新
                switch (result) {
                case auoSetupResult::success:
                case auoSetupResult::success_with_info: //完了
                    for (int i = 0; i < verInfo->Count; i++) {
                        if (0 != String::Compare(verInfo[i].LatestVer, VERSION_SAME_AS_CURRENT)) {
                            VersionInfo info = verInfo[i];
                            info.currentVer = info.LatestVer;
                            info.LatestVer = VERSION_SAME_AS_CURRENT;
                            verInfo[i] = info;
                        }
                    }
                    break;
                case auoSetupResult::abort: //中止
                    break;
                case auoSetupResult::error:
                default: //エラー
                         //チェックを実行する前にDelegateを書き換えておく
                    mesFunc = gcnew DelegateMessage(this, &auoSetupControl::addToBuffer);
                    finFuncCheck = gcnew DelegateProcessFin(this, &auoSetupControl::addToBuffer);
                    check();
                    break;
                }
                setVerionInfoToDataTable();
            }
        }
    private:
        bool runAuoSetup(String^ args) {
            bool result = false;
            if (createAuoSetupExeFile()) {
                hEventAbort = NULL;
                ProcessStartInfo ^psInfo = gcnew ProcessStartInfo();
                psInfo->FileName = auoSetupFilePath;
                psInfo->Arguments = args + L" -dir \"" + aviutlDir + L"\" -ini \"" + iniFileName + "\"";
                psInfo->WorkingDirectory = Path::GetDirectoryName(auoSetupFilePath);
                psInfo->CreateNoWindow = true;
                psInfo->UseShellExecute = false;
                psInfo->RedirectStandardOutput = true;

                Process^ processAuoSetup = nullptr;
                try {
                    processAuoSetup = Process::Start(psInfo);
                    char event_name[1024];
                    sprintf_s(event_name, "%s_%d", AUOSETUP_EVENT_ABORT, processAuoSetup->Id);
                    int countGetEvent = 0;
                    try {
                        while (!processAuoSetup->HasExited) {
                            //停止制御用のイベントを開く
                            if (NULL == hEventAbort && countGetEvent < 200) {
                                countGetEvent++;
                                Thread::Sleep(100);
                                hEventAbort = OpenEvent(EVENT_ALL_ACCESS, FALSE, event_name);
                            }
                            mesFunc(processAuoSetup->StandardOutput->ReadLine() + L"\r\n");
                            Thread::Sleep(50);
                        }
                        processAuoSetup->WaitForExit();
                        mesFunc(processAuoSetup->StandardOutput->ReadToEnd());
                    } catch (...) {
                        ;//なぜここに来た?
                    }
                    if (NULL != hEventAbort)
                        CloseHandle(hEventAbort);
                    hEventAbort = NULL;
                    processAuoSetup->Close();
                    result = true;
                } catch (...) {
                    if (nullptr != mesFunc)
                        mesFunc(L"更新の実行に失敗しました。\r\n");
                } finally {
                    if (nullptr != processAuoSetup)
                        delete processAuoSetup;
                }
                delete psInfo;
            }
            try {
                //if (Directory::Exists(auoSetupDir))
                //    Directory::Delete(auoSetupDir, true);
            } catch (...) {
                //特に何かする必要はない
            }
            return result;
        }
    private:
        void close() {
            if (nullptr != thCheck) {
                if (thCheck->IsAlive) {
                    abortAuoSetup();
                    thCheck->Join();
                }
                delete thCheck;
                thCheck = nullptr;
            }
            if (nullptr != thRun) {
                if (thRun->IsAlive) {
                    abortAuoSetup();
                    thRun->Join();
                }
                delete thRun;
                thRun = nullptr;
            }
            if (nullptr != verInfo) {
                delete verInfo;
                verInfo = nullptr;
            }
            if (nullptr != dataTableVersionInfo) {
                delete dataTableVersionInfo;
                dataTableVersionInfo = nullptr;
            }
            if (nullptr != exeMesBuffer) {
                delete exeMesBuffer;
                exeMesBuffer = nullptr;
            }
            //delegateはスレッド終了後に無効化
            hEventAbort = NULL;
            mesFunc = nullptr;
            finFuncRun = nullptr;
            finFuncCheck = nullptr;
        }
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
const WCHAR * const x264_encodemode_desc[] = {
    L"シングルパス - ビットレート指定",
    L"シングルパス - 固定量子化量",
    L"シングルパス - 品質基準VBR(可変レート)",
    L"マルチパス - 1pass",
    L"マルチパス - Npass",
    L"自動マルチパス",
    L"サイズ確認付 品質基準VBR(可変レート)",
    NULL
};

#define STR_BITRATE_AUTO (L"-1: 自動 ")

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

const WCHAR * const aspect_desc[] = {
    L"SAR比を指定 (デフォルト)",
    L"画面比から自動計算",
    NULL
};

const WCHAR * const tempdir_desc[] = {
    L"出力先と同じフォルダ (デフォルト)",
    L"システムの一時フォルダ",
    L"カスタム",
    NULL
};

const WCHAR * const audtempdir_desc[] = {
    L"変更しない",
    L"カスタム",
    NULL
};

const WCHAR * const mp4boxtempdir_desc[] = {
    L"指定しない",
    L"カスタム",
    NULL
};

const WCHAR * const interlaced_desc[] = {
    L"プログレッシブ",
    L"インタレ (tff)",
    L"インタレ (bff)",
    NULL
};

const WCHAR * const audio_enc_timing_desc[] = {
    L"後",
    L"前",
    L"同時",
    NULL
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
    WCHAR *desc;
} REPLACE_STRINGS;

const REPLACE_STRINGS REPLACE_STRINGS_LIST[] = {
    { L"%{vidpath}",          L"一時動画ファイル名(フルパス)" },
    { L"%{audpath}",          L"一時音声ファイル名(フルパス)" },
    { L"%{tmpdir}",           L"一時フォルダ名(最後の\\無し)" },
    { L"%{tmpfile}",          L"一時ファイル名(フルパス・拡張子除く)" },
    { L"%{tmpname}",          L"一時ファイル名(ファイル名のみ・拡張子除く)" },
    { L"%{savpath}",          L"出力ファイル名(フルパス)" },
    { L"%{savfile}",          L"出力ファイル名(フルパス・拡張子除く)" },
    { L"%{savname}",          L"出力ファイル名(ファイル名のみ・拡張子除く)" },
    { L"%{savdir}",           L"出力フォルダ名(最後の\\無し)" },
    { L"%{aviutldir}",        L"Aviutl.exeのフォルダ名(最後の\\無し)" },
    { L"%{chpath}",           L"チャプターファイル名(フルパス)" },
    { L"%{tcpath}",           L"タイムコードファイル名(フルパス)" },
    { L"%{muxout}",           L"muxで作成する一時ファイル名(フルパス)" },
    //{ L"%{x264path}",         L"指定された x264.exe のパス" },
    //{ L"%{x264_10path}",      L"指定された x264.exe(10bit版) のパス" },
    //{ L"%{audencpath}",       L"実行された音声エンコーダのパス" },
    //{ L"%{mp4muxerpath}",     L"mp4 muxerのパス" },
    //{ L"%{mkvmuxerpath}",     L"mkv muxerのパス" },
    { L"%{fps_scale}",        L"フレームレート(分母)" },
    { L"%{fps_rate}",         L"フレームレート(分子)" },
    { L"%{fps_rate_times_4}", L"フレームレート(分子)×4" },
    { L"%{sar_x}",            L"サンプルアスペクト比 (横)" },
    { L"%{sar_y}",            L"サンプルアスペクト比 (縦)" },
    { L"%{dar_x}",            L"画面アスペクト比 (横)" },
    { L"%{dar_y}",            L"画面アスペクト比 (縦)" },
    { NULL, NULL }
};
