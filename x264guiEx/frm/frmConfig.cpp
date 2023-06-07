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

//以下warning C4100を黙らせる
//C4100 : 引数は関数の本体部で 1 度も参照されません。
#pragma warning( push )
#pragma warning( disable: 4100 )

#include "auo_version.h"
#include "auo_frm.h"
#include "auo_faw2aac.h"
#include "frmConfig.h"
#include "frmSaveNewStg.h"
#include "frmOtherSettings.h"
#include "frmBitrateCalculator.h"

using namespace x264guiEx;

/// -------------------------------------------------
///     設定画面の表示
/// -------------------------------------------------
[STAThreadAttribute]
void ShowfrmConfig(CONF_GUIEX *conf, const SYSTEM_DATA *sys_dat) {
    if (!sys_dat->exstg->s_local.disable_visual_styles)
        System::Windows::Forms::Application::EnableVisualStyles();
    System::IO::Directory::SetCurrentDirectory(String(sys_dat->aviutl_dir).ToString());
    frmConfig frmConf(conf, sys_dat);
    frmConf.ShowDialog();
}

/// -------------------------------------------------
///     frmSaveNewStg 関数
/// -------------------------------------------------
System::Boolean frmSaveNewStg::checkStgFileName(String^ stgName) {
    String^ fileName;
    if (stgName->Length == 0)
        return false;

    if (!ValidiateFileName(stgName)) {
        MessageBox::Show(LOAD_CLI_STRING(AUO_CONFIG_ERR_INVALID_CHAR), LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OK, MessageBoxIcon::Error);
        return false;
    }
    if (String::Compare(Path::GetExtension(stgName), L".stg", true))
        stgName += L".stg";
    if (File::Exists(fileName = Path::Combine(fsnCXFolderBrowser->GetSelectedFolder(), stgName)))
        if (MessageBox::Show(stgName + LOAD_CLI_STRING(AUO_CONFIG_ALREADY_EXISTS), LOAD_CLI_STRING(AUO_CONFIG_OVERWRITE_CHECK), MessageBoxButtons::YesNo, MessageBoxIcon::Question)
            != System::Windows::Forms::DialogResult::Yes)
            return false;
    StgFileName = fileName;
    return true;
}

System::Void frmSaveNewStg::setStgDir(String^ _stgDir) {
    StgDir = _stgDir;
    fsnCXFolderBrowser->SetRootDirAndReload(StgDir);
}


/// -------------------------------------------------
///     frmBitrateCalculator 関数
/// -------------------------------------------------
System::Void frmBitrateCalculator::Init(int VideoBitrate, int AudioBitrate, bool BTVBEnable, bool BTABEnable, int ab_max, const AuoTheme themeTo, const DarkenWindowStgReader *dwStg) {
    guiEx_settings exStg(true);
    exStg.load_fbc();
    enable_events = false;
    dwStgReader = dwStg;
    CheckTheme(themeTo);
    fbcTXSize->Text = exStg.s_fbc.initial_size.ToString("F2");
    fbcChangeTimeSetMode(exStg.s_fbc.calc_time_from_frame != 0);
    fbcRBCalcRate->Checked = exStg.s_fbc.calc_bitrate != 0;
    fbcRBCalcSize->Checked = !fbcRBCalcRate->Checked;
    fbcTXMovieFrameRate->Text = Convert::ToString(exStg.s_fbc.last_fps);
    fbcNUMovieFrames->Value = exStg.s_fbc.last_frame_num;
    fbcNULengthHour->Value = Convert::ToDecimal((int)exStg.s_fbc.last_time_in_sec / 3600);
    fbcNULengthMin->Value = Convert::ToDecimal((int)(exStg.s_fbc.last_time_in_sec % 3600) / 60);
    fbcNULengthSec->Value =  Convert::ToDecimal((int)exStg.s_fbc.last_time_in_sec % 60);
    SetBTVBEnabled(BTVBEnable);
    SetBTABEnabled(BTABEnable, ab_max);
    SetNUVideoBitrate(VideoBitrate);
    SetNUAudioBitrate(AudioBitrate);
    enable_events = true;
}
System::Void frmBitrateCalculator::CheckTheme(const AuoTheme themeTo) {
    //変更の必要がなければ終了
    if (themeTo == themeMode) return;

    //一度ウィンドウの再描画を完全に抑止する
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 0, 0);
    SetAllColor(this, themeTo, this->GetType(), dwStgReader);
    SetAllMouseMove(this, themeTo);
    //一度ウィンドウの再描画を再開し、強制的に再描画させる
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 1, 0);
    this->Refresh();
    themeMode = themeTo;
}
System::Void frmBitrateCalculator::frmBitrateCalculator_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
    guiEx_settings exStg(true);
    exStg.load_fbc();
    exStg.s_fbc.calc_bitrate = fbcRBCalcRate->Checked;
    exStg.s_fbc.calc_time_from_frame = fbcPNMovieFrames->Visible;
    exStg.s_fbc.last_fps = Convert::ToDouble(fbcTXMovieFrameRate->Text);
    exStg.s_fbc.last_frame_num = Convert::ToInt32(fbcNUMovieFrames->Value);
    exStg.s_fbc.last_time_in_sec = Convert::ToInt32(fbcNULengthHour->Value) * 3600
                                 + Convert::ToInt32(fbcNULengthMin->Value) * 60
                                 + Convert::ToInt32(fbcNULengthSec->Value);
    if (fbcRBCalcRate->Checked)
        exStg.s_fbc.initial_size = Convert::ToDouble(fbcTXSize->Text);
    exStg.save_fbc();
    frmConfig^ fcg = dynamic_cast<frmConfig^>(this->Owner);
    if (fcg != nullptr)
        fcg->InformfbcClosed();
}
System::Void frmBitrateCalculator::fbcRBCalcRate_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
    if (fbcRBCalcRate->Checked && Convert::ToDouble(fbcTXSize->Text) <= 0.0) {
        guiEx_settings exStg(true);
        exStg.load_fbc();
        fbcTXSize->Text = exStg.s_fbc.initial_size.ToString("F2");
    }
}
System::Void frmBitrateCalculator::fbcBTVBApply_Click(System::Object^  sender, System::EventArgs^  e) {
    frmConfig^ fcg = dynamic_cast<frmConfig^>(this->Owner);
    if (fcg != nullptr)
        fcg->SetVideoBitrate((int)fbcNUBitrateVideo->Value);
}
System::Void frmBitrateCalculator::fbcBTABApply_Click(System::Object^  sender, System::EventArgs^  e) {
    frmConfig^ fcg = dynamic_cast<frmConfig^>(this->Owner);
    if (fcg != nullptr)
        fcg->SetAudioBitrate((int)fbcNUBitrateAudio->Value);
}
System::Void frmBitrateCalculator::fbcMouseEnter_SetColor(System::Object^  sender, System::EventArgs^  e) {
    fcgMouseEnterLeave_SetColor(sender, themeMode, DarkenWindowState::Hot, dwStgReader);
}
System::Void frmBitrateCalculator::fbcMouseLeave_SetColor(System::Object^  sender, System::EventArgs^  e) {
    fcgMouseEnterLeave_SetColor(sender, themeMode, DarkenWindowState::Normal, dwStgReader);
}
System::Void frmBitrateCalculator::SetAllMouseMove(Control ^top, const AuoTheme themeTo) {
    if (themeTo == themeMode) return;
    System::Type^ type = top->GetType();
    if (type == CheckBox::typeid) {
        top->MouseEnter += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcMouseEnter_SetColor);
        top->MouseLeave += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcMouseLeave_SetColor);
    }
    for (int i = 0; i < top->Controls->Count; i++) {
        SetAllMouseMove(top->Controls[i], themeTo);
    }
}

/// -------------------------------------------------
///     frmConfig 関数  (frmBitrateCalculator関連)
/// -------------------------------------------------
System::Void frmConfig::CloseBitrateCalc() {
    frmBitrateCalculator::Instance::get()->Close();
}
System::Void frmConfig::fcgTSBBitrateCalc_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
    if (fcgTSBBitrateCalc->Checked) {
        int videoBitrate = 0;
        bool videoBitrateMode = (x264_encmode_to_RCint[fcgCXX264Mode->SelectedIndex] == X264_RC_BITRATE);
        videoBitrateMode &= Int32::TryParse(fcgTXQuality->Text, videoBitrate);

        frmBitrateCalculator::Instance::get()->Init(
            (videoBitrateMode) ? videoBitrate : 0,
            (fcgNUAudioBitrate->Visible) ? (int)fcgNUAudioBitrate->Value : 0,
            videoBitrateMode,
            fcgNUAudioBitrate->Visible,
            (int)fcgNUAudioBitrate->Maximum,
            themeMode,
            dwStgReader
            );
        frmBitrateCalculator::Instance::get()->Owner = this;
        frmBitrateCalculator::Instance::get()->Show();
    } else {
        frmBitrateCalculator::Instance::get()->Close();
    }
}
System::Void frmConfig::SetfbcBTABEnable(bool enable, int max) {
    frmBitrateCalculator::Instance::get()->SetBTABEnabled(fcgNUAudioBitrate->Visible, max);
}
System::Void frmConfig::SetfbcBTVBEnable(bool enable) {
    frmBitrateCalculator::Instance::get()->SetBTVBEnabled(enable);
}

System::Void frmConfig::SetVideoBitrate(int bitrate) {
    if (x264_encmode_to_RCint[fcgCXX264Mode->SelectedIndex] == X264_RC_BITRATE)
        fcgTXQuality->Text = bitrate.ToString();
}

System::Void frmConfig::SetAudioBitrate(int bitrate) {
    SetNUValue(fcgNUAudioBitrate, bitrate);
}
System::Void frmConfig::InformfbcClosed() {
    fcgTSBBitrateCalc->Checked = false;
}

/// -------------------------------------------------
///     frmConfig 関数
/// -------------------------------------------------
/////////////   LocalStg関連  //////////////////////
System::Void frmConfig::LoadLocalStg() {
    guiEx_settings *_ex_stg = sys_dat->exstg;
    _ex_stg->load_encode_stg();
    LocalStg.x264ExeName     = String(_ex_stg->s_enc.filename).ToString();
    LocalStg.x264Path        = String(_ex_stg->s_enc.fullpath).ToString();
    LocalStg.CustomTmpDir    = String(_ex_stg->s_local.custom_tmp_dir).ToString();
    LocalStg.CustomAudTmpDir = String(_ex_stg->s_local.custom_audio_tmp_dir).ToString();
    LocalStg.CustomMP4TmpDir = String(_ex_stg->s_local.custom_mp4box_tmp_dir).ToString();
    LocalStg.LastAppDir      = String(_ex_stg->s_local.app_dir).ToString();
    LocalStg.LastBatDir      = String(_ex_stg->s_local.bat_dir).ToString();
    LocalStg.MP4MuxerExeName = String(_ex_stg->s_mux[MUXER_MP4].filename).ToString();
    LocalStg.MP4MuxerPath    = String(_ex_stg->s_mux[MUXER_MP4].fullpath).ToString();
    LocalStg.MKVMuxerExeName = String(_ex_stg->s_mux[MUXER_MKV].filename).ToString();
    LocalStg.MKVMuxerPath    = String(_ex_stg->s_mux[MUXER_MKV].fullpath).ToString();
    LocalStg.TC2MP4ExeName   = String(_ex_stg->s_mux[MUXER_TC2MP4].filename).ToString();
    LocalStg.TC2MP4Path      = String(_ex_stg->s_mux[MUXER_TC2MP4].fullpath).ToString();
    LocalStg.MPGMuxerExeName = String(_ex_stg->s_mux[MUXER_MPG].filename).ToString();
    LocalStg.MPGMuxerPath    = String(_ex_stg->s_mux[MUXER_MPG].fullpath).ToString();
    LocalStg.MP4RawExeName   = String(_ex_stg->s_mux[MUXER_MP4_RAW].filename).ToString();
    LocalStg.MP4RawPath      = String(_ex_stg->s_mux[MUXER_MP4_RAW].fullpath).ToString();

    LocalStg.audEncName->Clear();
    LocalStg.audEncExeName->Clear();
    LocalStg.audEncPath->Clear();
    for (int i = 0; i < _ex_stg->s_aud_count; i++) {
        LocalStg.audEncName->Add(String(_ex_stg->s_aud[i].dispname).ToString());
        LocalStg.audEncExeName->Add(String(_ex_stg->s_aud[i].filename).ToString());
        LocalStg.audEncPath->Add(String(_ex_stg->s_aud[i].fullpath).ToString());
    }
    if (_ex_stg->s_local.large_cmdbox)
        fcgTXCmd_DoubleClick(nullptr, nullptr); //初期状態は縮小なので、拡大
}

System::Boolean frmConfig::CheckLocalStg() {
    bool error = false;
    String^ err = "";
    //x264のチェック
    if (LocalStg.x264Path->Length > 0
        && !File::Exists(LocalStg.x264Path)) {
        error = true;
        err += LOAD_CLI_STRING(AUO_CONFIG_VID_ENC_NOT_EXIST) + L"\n [ " + LocalStg.x264Path + L" ]\n";
    }
    //音声エンコーダのチェック (実行ファイル名がない場合はチェックしない)
    if (LocalStg.audEncExeName[fcgCXAudioEncoder->SelectedIndex]->Length) {
        String^ AudioEncoderPath = LocalStg.audEncPath[fcgCXAudioEncoder->SelectedIndex];
        if (AudioEncoderPath->Length > 0
            && !File::Exists(AudioEncoderPath)
            && (fcgCXAudioEncoder->SelectedIndex != sys_dat->exstg->s_aud_faw_index)) {
            //音声実行ファイルがない かつ
            //選択された音声がfawでない または fawであってもfaw2aacがない
            if (error) err += L"\n\n";
            error = true;
            err += LOAD_CLI_STRING(AUO_CONFIG_AUD_ENC_NOT_EXIST) + L"\n [ " + AudioEncoderPath + L" ]\n";
        }
    }
    //FAWのチェック
    if (fcgCBFAWCheck->Checked) {
        if (sys_dat->exstg->s_aud_faw_index == FAW_INDEX_ERROR) {
            if (error) err += L"\n\n";
            error = true;
            err += LOAD_CLI_STRING(AUO_CONFIG_FAW_STG_NOT_FOUND_IN_INI1) + L"\n"
                +  LOAD_CLI_STRING(AUO_CONFIG_FAW_STG_NOT_FOUND_IN_INI2) + L"\n"
                +  LOAD_CLI_STRING(AUO_CONFIG_FAW_STG_NOT_FOUND_IN_INI3);
        }
    }
    //自動マルチパスの自動ビットレート設定のチェック
    if (fcgLBAMPAutoBitrate != nullptr && fcgLBAMPAutoBitrate->Visible) {
        if (error) err += L"\n\n";
        error = true;
        err += LOAD_CLI_STRING(AUO_CONFIG_AMP_STG_INVALID1) + L"\n"
            +  LOAD_CLI_STRING(AUO_CONFIG_AMP_STG_INVALID2) + L"\n"
            +  LOAD_CLI_STRING(AUO_CONFIG_AMP_STG_INVALID3) + L"\n"
            +  LOAD_CLI_STRING(AUO_CONFIG_AMP_STG_INVALID4);
    }
    if (error)
        MessageBox::Show(this, err, LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OK, MessageBoxIcon::Error);
    return error;
}

System::Void frmConfig::SaveLocalStg() {
    guiEx_settings *_ex_stg = sys_dat->exstg;
    _ex_stg->load_encode_stg();
    _ex_stg->s_local.large_cmdbox = fcgTXCmd->Multiline;
    GetCHARfromString(_ex_stg->s_enc.fullpath,               sizeof(_ex_stg->s_enc.fullpath),               LocalStg.x264Path);
    GetCHARfromString(_ex_stg->s_local.custom_tmp_dir,        sizeof(_ex_stg->s_local.custom_tmp_dir),        LocalStg.CustomTmpDir);
    GetCHARfromString(_ex_stg->s_local.custom_mp4box_tmp_dir, sizeof(_ex_stg->s_local.custom_mp4box_tmp_dir), LocalStg.CustomMP4TmpDir);
    GetCHARfromString(_ex_stg->s_local.custom_audio_tmp_dir,  sizeof(_ex_stg->s_local.custom_audio_tmp_dir),  LocalStg.CustomAudTmpDir);
    GetCHARfromString(_ex_stg->s_local.app_dir,               sizeof(_ex_stg->s_local.app_dir),               LocalStg.LastAppDir);
    GetCHARfromString(_ex_stg->s_local.bat_dir,               sizeof(_ex_stg->s_local.bat_dir),               LocalStg.LastBatDir);
    GetCHARfromString(_ex_stg->s_mux[MUXER_MP4].fullpath,     sizeof(_ex_stg->s_mux[MUXER_MP4].fullpath),     LocalStg.MP4MuxerPath);
    GetCHARfromString(_ex_stg->s_mux[MUXER_MKV].fullpath,     sizeof(_ex_stg->s_mux[MUXER_MKV].fullpath),     LocalStg.MKVMuxerPath);
    GetCHARfromString(_ex_stg->s_mux[MUXER_TC2MP4].fullpath,  sizeof(_ex_stg->s_mux[MUXER_TC2MP4].fullpath),  LocalStg.TC2MP4Path);
    GetCHARfromString(_ex_stg->s_mux[MUXER_MPG].fullpath,     sizeof(_ex_stg->s_mux[MUXER_MPG].fullpath),     LocalStg.MPGMuxerPath);
    GetCHARfromString(_ex_stg->s_mux[MUXER_MP4_RAW].fullpath, sizeof(_ex_stg->s_mux[MUXER_MP4_RAW].fullpath), LocalStg.MP4RawPath);
    for (int i = 0; i < _ex_stg->s_aud_count; i++)
        GetCHARfromString(_ex_stg->s_aud[i].fullpath,         sizeof(_ex_stg->s_aud[i].fullpath),             LocalStg.audEncPath[i]);
    _ex_stg->save_local();
}

System::Void frmConfig::SetLocalStg() {
    fcgLBX264Path->Text           = System::String(ENCODER_NAME).ToString() + L".exe" + LOAD_CLI_STRING(AUO_CONFIG_SPECIFY_EXE_PATH);
    fcgLBX264PathSub->Text        = System::String(ENCODER_NAME).ToString() + L".exe" + LOAD_CLI_STRING(AUO_CONFIG_SPECIFY_EXE_PATH);
    fcgTXX264Path->Text           = LocalStg.x264Path;
    fcgTXX264PathSub->Text        = LocalStg.x264Path;
    fcgTXMP4MuxerPath->Text       = LocalStg.MP4MuxerPath;
    fcgTXMKVMuxerPath->Text       = LocalStg.MKVMuxerPath;
    fcgTXTC2MP4Path->Text         = LocalStg.TC2MP4Path;
    fcgTXMPGMuxerPath->Text       = LocalStg.MPGMuxerPath;
    fcgTXMP4RawPath->Text         = LocalStg.MP4RawPath;
    fcgTXCustomAudioTempDir->Text = LocalStg.CustomAudTmpDir;
    fcgTXCustomTempDir->Text      = LocalStg.CustomTmpDir;
    fcgTXMP4BoxTempDir->Text      = LocalStg.CustomMP4TmpDir;
    fcgLBMP4MuxerPath->Text       = LocalStg.MP4MuxerExeName + LOAD_CLI_STRING(AUO_CONFIG_SPECIFY_EXE_PATH);
    fcgLBMKVMuxerPath->Text       = LocalStg.MKVMuxerExeName + LOAD_CLI_STRING(AUO_CONFIG_SPECIFY_EXE_PATH);
    fcgLBTC2MP4Path->Text         = LocalStg.TC2MP4ExeName   + LOAD_CLI_STRING(AUO_CONFIG_SPECIFY_EXE_PATH);
    fcgLBMPGMuxerPath->Text       = LocalStg.MPGMuxerExeName + LOAD_CLI_STRING(AUO_CONFIG_SPECIFY_EXE_PATH);
    fcgLBMP4RawPath->Text         = LocalStg.MP4RawExeName   + LOAD_CLI_STRING(AUO_CONFIG_SPECIFY_EXE_PATH);

    fcgTXX264Path->SelectionStart           = fcgTXX264Path->Text->Length;
    fcgTXX264PathSub->SelectionStart        = fcgTXX264PathSub->Text->Length;
    fcgTXMP4MuxerPath->SelectionStart       = fcgTXMP4MuxerPath->Text->Length;
    fcgTXTC2MP4Path->SelectionStart         = fcgTXTC2MP4Path->Text->Length;
    fcgTXMKVMuxerPath->SelectionStart       = fcgTXMKVMuxerPath->Text->Length;
    fcgTXMPGMuxerPath->SelectionStart       = fcgTXMPGMuxerPath->Text->Length;
    fcgTXMP4RawPath->SelectionStart         = fcgTXMP4RawPath->Text->Length;
}

//////////////   TrackBar用タイマー関連     /////////////////////////
System::Void frmConfig::qualityTimerChange(Object^ state) {
    this->Invoke(gcnew qualityTimerChangeDelegate(this, &frmConfig::fcgTBQualityChange));
}

System::Void frmConfig::fcgTBQualityChange() {
    int j = fcgTBQuality->Value;
    int i = j + timerChangeValue;
    i = clamp(i, fcgTBQuality->Minimum, fcgTBQuality->Maximum);
    fcgTBQuality->Value = i;
    if (i != j)
        fcgTBQuality_Scroll(nullptr, nullptr);
}

System::Void frmConfig::InitTimer() {
    qualityTimer = gcnew System::Threading::Timer(
        gcnew System::Threading::TimerCallback(this, &frmConfig::qualityTimerChange),
        nullptr, System::Threading::Timeout::Infinite, fcgTBQualityTimerPeriod);
    timerChangeValue = 1;
}


//////////////       その他イベント処理   ////////////////////////
System::Void frmConfig::ActivateToolTip(bool Enable) {
    fcgTTEx->Active = Enable;
    fcgTTX264->Active = Enable;
    fcgTTX264Version->Active = Enable;
}

System::Void frmConfig::fcgTSBOtherSettings_Click(System::Object^  sender, System::EventArgs^  e) {
    frmOtherSettings::Instance::get()->stgDir = String(sys_dat->exstg->s_local.stg_dir).ToString();
    frmOtherSettings::Instance::get()->SetTheme(themeMode, dwStgReader);
    frmOtherSettings::Instance::get()->ShowDialog();
    char buf[MAX_PATH_LEN];
    GetCHARfromString(buf, sizeof(buf), frmOtherSettings::Instance::get()->stgDir);
    if (_stricmp(buf, sys_dat->exstg->s_local.stg_dir)) {
        //変更があったら保存する
        strcpy_s(sys_dat->exstg->s_local.stg_dir, sizeof(sys_dat->exstg->s_local.stg_dir), buf);
        sys_dat->exstg->save_local();
        InitStgFileList();
    }
    //再読み込み
    guiEx_settings stg;
    stg.load_encode_stg();
    log_reload_settings();
    sys_dat->exstg->s_local.default_audio_encoder = stg.s_local.default_audio_encoder;
    sys_dat->exstg->s_local.get_relative_path = stg.s_local.get_relative_path;
    SetStgEscKey(stg.s_local.enable_stg_esc_key != 0);
    ActivateToolTip(stg.s_local.disable_tooltip_help == FALSE);
    if (str_has_char(stg.s_local.conf_font.name))
        SetFontFamilyToForm(this, gcnew FontFamily(String(stg.s_local.conf_font.name).ToString()), this->Font->FontFamily);
    LoadLangText();
}

System::Void frmConfig::fcgTSBCMDOnly_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
    //CLIモードとの切り替え
    //一度ウィンドウの再描画を完全に抑止する
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 0, 0);
    //なぜか知らんが、Visibleプロパティをfalseにするだけでは非表示にできない
    //しょうがないので参照の削除と挿入を行う
    if (fcgTSBCMDOnly->Checked) {
        fcgtabControlVideo->TabPages->RemoveAt(2);
        fcgtabControlVideo->TabPages->RemoveAt(1);
        fcgtabControlVideo->TabPages->RemoveAt(0);
        fcgtabPageExSettings->Text = LOAD_CLI_STRING(AUO_CONFIG_VIDEO);
        fcggroupBoxCmdEx->Text = LOAD_CLI_STRING(AUO_CONFIG_COMMANDLINE);
    } else {
        fcgtabControlVideo->TabPages->Insert(0, fcgtabPageX264Main);
        fcgtabControlVideo->TabPages->Insert(1, fcgtabPageX264RC);
        fcgtabControlVideo->TabPages->Insert(2, fcgtabPageX264Frame);
        fcgtabPageExSettings->Text = LOAD_CLI_STRING(AUO_CONFIG_EXTENSION);
        fcggroupBoxCmdEx->Text = LOAD_CLI_STRING(AUO_CONFIG_ADDITIONAL_COMMANDLINE);
    }
    //一度ウィンドウの再描画を再開し、強制的に再描画させる
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 1, 0);
    this->Refresh();

    fcgChangeEnabled(sender, e);
    fcgRebuildCmd(sender, e);
}

System::Void frmConfig::fcgCBUsehighbit_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
    //一度ウィンドウの再描画を完全に抑止する
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 0, 0);
    //8bit/highbitで異なるQPの最大最小を管理する
    int old_max = (int)fcgNUQpmax->Maximum;
    fcgNUQpmax->Maximum = (fcgCBUsehighbit->Checked) ? X264_QP_MAX_10BIT : X264_QP_MAX_8BIT;
    fcgNUQpmin->Maximum = fcgNUQpmax->Maximum;
    fcgNUQpstep->Maximum = fcgNUQpmax->Maximum;
    fcgNUChromaQp->Minimum = -1 * fcgNUQpmax->Maximum;
    fcgNUChromaQp->Maximum = fcgNUQpmax->Maximum;
    if ((int)fcgNUQpmax->Value == old_max)
        fcgNUQpmax->Value = fcgNUQpmax->Maximum;
    fcgCXX264Mode_SelectedIndexChanged(sender, e);
    //一度ウィンドウの再描画を再開し、強制的に再描画させる
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 1, 0);
    this->Refresh();
}

System::Void frmConfig::fcgCBAFS_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
    fcgCBAFSBitrateCorrection->Enabled = fcgCBAFS->Checked;
    fcgLBVBVafsWarning->Visible        = fcgCBAFS->Checked;
    if (fcgCBAFS->Checked) {
        //fcgCBMP4MuxerExt->Checked      = true;
        //fcgCBMKVMuxerExt->Checked      = true;
    }
    //fcgCBMP4MuxerExt->Enabled          = !fcgCBAFS->Checked;
    //fcgCBMKVMuxerExt->Enabled          = !fcgCBAFS->Checked;

    bool disable_keyframe_detect = fcgCBAFS->Checked && !sys_dat->exstg->s_local.set_keyframe_as_afs_24fps;
    if (disable_keyframe_detect)
        fcgCBCheckKeyframes->Checked   = false;
    fcgCBCheckKeyframes->Enabled       = !disable_keyframe_detect;

    int muxer_cmdex = fcgCXMP4CmdEx->SelectedIndex;
    setMuxerCmdExNames(fcgCXMP4CmdEx, (fcgCBAFS->Checked) ? MUXER_TC2MP4 : MUXER_MP4);
    fcgCXMP4CmdEx->SelectedIndex = muxer_cmdex;
}

System::Void frmConfig::fcgArrangeForAutoMultiPass(bool enable) {
    const int PNX264ModeOffset = +7;
    const int PNBitrateOffset  = -39;
    const int PNStatusFileOffset = -17;
    if (fcgLastX264ModeAsAMP == enable)
        return;
    Point NewPoint = fcgPNX264Mode->Location;
    NewPoint.Y += PNX264ModeOffset * ((enable) ? -1 : 1);
    fcgPNX264Mode->Location = NewPoint;
    fcgPNX264Mode->Height += (PNBitrateOffset - PNX264ModeOffset*2) * ((enable) ? -1 : 1);
    NewPoint = fcgPNBitrate->Location;
    NewPoint.Y += PNBitrateOffset * ((enable) ? -1 : 1);
    fcgPNBitrate->Location = NewPoint;
    NewPoint = fcgPNStatusFile->Location;
    NewPoint.Y += PNStatusFileOffset * ((enable) ? -1 : 1);
    fcgPNStatusFile->Location = NewPoint;
    fcgLastX264ModeAsAMP = enable;
    fcgCBAMPLimitBitrateLower->Visible = enable;
    fcgCBAMPLimitBitrateUpper->Visible = enable;
    fcgCBAMPLimitFileSize->Visible = enable;
    fcgNUAMPLimitBitrateLower->Visible = enable;
    fcgNUAMPLimitBitrateUpper->Visible = enable;
    fcgNUAMPLimitFileSize->Visible = enable;
}

System::Void frmConfig::fcgCheckAMPAutoBitrateEvent(System::Object^  sender, System::EventArgs^  e) {
    if (fcgLBAMPAutoBitrate == nullptr)
        return;
    if (fcgCXX264Mode->SelectedIndex == 5) {
        if (fcgTXQuality->Text->StartsWith(STR_BITRATE_START)
            || 0 == String::Compare(fcgTXQuality->Text, L"-1")) {
                if (!fcgCBAMPLimitBitrateUpper->Checked && !fcgCBAMPLimitFileSize->Checked) {
                fcgLBAMPAutoBitrate->Visible = true;
                return;
                }
        }
    }
    fcgLBAMPAutoBitrate->Visible = false;
}

System::Void frmConfig::AddCheckAMPAutoBitrateEvent() {
    fcgCBAMPLimitBitrateLower->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCheckAMPAutoBitrateEvent);
    fcgCBAMPLimitBitrateUpper->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCheckAMPAutoBitrateEvent);
    fcgCBAMPLimitFileSize->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCheckAMPAutoBitrateEvent);
    fcgCXX264Mode->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCheckAMPAutoBitrateEvent);
    fcgTXQuality->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgCheckAMPAutoBitrateEvent);
}

System::Void frmConfig::fcgCXX264Mode_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
    int index = fcgCXX264Mode->SelectedIndex;
    cnf_fcgTemp->rc_mode = x264_encmode_to_RCint[index];
    cnf_fcgTemp->use_auto_npass = (fcgCXX264Mode->SelectedIndex == 5 || fcgCXX264Mode->SelectedIndex == 6);
    switch (cnf_fcgTemp->rc_mode) {
        case X264_RC_BITRATE:
            fcgLBQuality->Text = (fcgCXX264Mode->SelectedIndex == 5) ? LOAD_CLI_STRING(AUO_CONFIG_MODE_TARGET_BITRATE) : LOAD_CLI_STRING(AUO_CONFIG_MODE_BITRATE);
            fcgLBQualityLeft->Text = LOAD_CLI_STRING(AUO_CONFIG_QUALITY_LOW);
            fcgLBQualityRight->Text = LOAD_CLI_STRING(AUO_CONFIG_QUALITY_HIGH);
            fcgTBQuality->Minimum = 0;
            fcgTBQuality->Maximum = TBBConvert.getMaxCount();
            cnf_fcgTemp->pass = x264_encmode_to_passint[index];
            if (fcgCXX264Mode->SelectedIndex >= 3) {
                fcgCBNulOut->Enabled = true;
                fcgCBNulOut->Checked = cnf_fcgTemp->nul_out != 0;
                if (fcgCXX264Mode->SelectedIndex == 4) {
                    fcgCBFastFirstPass->Enabled = false; //Enabledの変更が先
                    fcgCBFastFirstPass->Checked = false;
                } else {
                    fcgCBFastFirstPass->Enabled = true; //Enabledの変更が先
                    fcgCBFastFirstPass->Checked = !cnf_fcgTemp->slow_first_pass;
                }
            } else {
                fcgCBNulOut->Enabled = false; //Enabledの変更が先
                fcgCBNulOut->Checked = false;
                fcgCBFastFirstPass->Enabled = false; //Enabledの変更が先
                fcgCBFastFirstPass->Checked = false;
            }
            //自動(-1)から変更されたときの処置 → 1000に戻す
            if ((cnf_fcgTemp->bitrate == -1) && (fcgCXX264Mode->SelectedIndex != 5))
                cnf_fcgTemp->bitrate = 1000;
            //文字列を更新
            if ((cnf_fcgTemp->bitrate == -1) && (fcgCXX264Mode->SelectedIndex == 5))
                fcgTXQuality->Text = STR_BITRATE_AUTO; //-1の特例処置(-1: 自動)
            else
                fcgTXQuality->Text = Convert::ToString(cnf_fcgTemp->bitrate);
            SetfbcBTVBEnable(true);
            break;
        case X264_RC_QP:
            fcgLBQuality->Text = LOAD_CLI_STRING(AUO_CONFIG_MODE_QP);
            fcgLBQualityLeft->Text = LOAD_CLI_STRING(AUO_CONFIG_QUALITY_HIGH);
            fcgLBQualityRight->Text = LOAD_CLI_STRING(AUO_CONFIG_QUALITY_LOW);
            fcgTBQuality->Minimum = 0;
            fcgTBQuality->Maximum = 69;
            fcgCBNulOut->Enabled = false; //Enabledの変更が先
            fcgCBNulOut->Checked = false;
            fcgCBFastFirstPass->Enabled = false; //Enabledの変更が先
            fcgCBFastFirstPass->Checked = false;
            fcgTXQuality->Text = Convert::ToString(cnf_fcgTemp->qp);
            SetfbcBTVBEnable(false);
            break;
        case X264_RC_CRF:
        default:
            fcgLBQuality->Text = LOAD_CLI_STRING(AUO_CONFIG_MODE_CRF);
            fcgLBQualityLeft->Text = LOAD_CLI_STRING(AUO_CONFIG_QUALITY_HIGH);
            fcgLBQualityRight->Text = LOAD_CLI_STRING(AUO_CONFIG_QUALITY_LOW);
            fcgTBQuality->Minimum = (fcgCBUsehighbit->Checked) ? -12*2 : 0;
            fcgTBQuality->Maximum = 51*2;
            fcgCBNulOut->Enabled = false; //Enabledの変更が先
            fcgCBNulOut->Checked = false;
            fcgCBFastFirstPass->Enabled = false; //Enabledの変更が先
            fcgCBFastFirstPass->Checked = false;
            fcgTXQuality->Text = Convert::ToString(clamp(cnf_fcgTemp->crf / 100.0, (fcgCBUsehighbit->Checked) ? -12 : 0, 51));
            SetfbcBTVBEnable(false);
            break;
    }
    fcgNUAutoNPass->Enabled = (fcgCXX264Mode->SelectedIndex == 5);
    fcgArrangeForAutoMultiPass(cnf_fcgTemp->use_auto_npass != 0);
}

System::Void frmConfig::fcgTXQuality_Enter(System::Object^  sender, System::EventArgs^  e) {
    if (fcgTXQuality->Text->StartsWith(STR_BITRATE_START)) {
        fcgTXQuality->Text = L"-1";
        fcgTXQuality->Select(0, fcgTXQuality->Text->Length);
    }
}

System::Void frmConfig::fcgTXQuality_TextChanged(System::Object^  sender, System::EventArgs^  e) {
    if (fcgTXQuality->Text->Length == 0 || String::Compare(fcgTXQuality->Text, L"-") == 0)
        return;
    //自動モードの文字列に変更されたときの処理
    if (fcgTXQuality->Text->StartsWith(STR_BITRATE_START)) {
        fcgTXQuality->Text = STR_BITRATE_AUTO;
        cnf_fcgTemp->bitrate = -1;
        fcgTBQuality->Value = TBBConvert.BitrateToTB(cnf_fcgTemp->bitrate);
        lastQualityStr = fcgTXQuality->Text;
        fcgTXQuality->SelectionStart = fcgTXQuality->Text->Length;
        fcgTXQuality->SelectionLength = 0;
        return;
    }
    int c = fcgTXQuality->SelectionStart;
    int index = fcgCXX264Mode->SelectedIndex;
    bool restore = false;
    int i;
    double d;
    if (!Double::TryParse(fcgTXQuality->Text, d)) {
        fcgTXQuality->Text = lastQualityStr;
        restore = true;
    } else {
        switch (x264_encmode_to_RCint[index]) {
        case X264_RC_BITRATE:
            //自動マルチパス時は-1(自動)もあり得る
            if (Int32::TryParse(fcgTXQuality->Text, i) && i >= ((fcgCXX264Mode->SelectedIndex == 5) ? -1 : 0)) {
                cnf_fcgTemp->bitrate = i;
                fcgTXQuality->Text = i.ToString();
                fcgTBQuality->Value = TBBConvert.BitrateToTB(cnf_fcgTemp->bitrate);
            } else {
                fcgTXQuality->Text = lastQualityStr;
                restore = true;
            }
            break;
        case X264_RC_QP:
            if (Int32::TryParse(fcgTXQuality->Text, i)) {
                i = SetTBValue(fcgTBQuality, i);
                cnf_fcgTemp->qp = i;
                fcgTXQuality->Text = Convert::ToString(i);
            }
            break;
        case X264_RC_CRF:
        default:
            if (Double::TryParse(fcgTXQuality->Text, d)) {
                int TBmin = fcgTBQuality->Minimum * 50;
                int TBmax = fcgTBQuality->Maximum * 50;
                i = clamp((int)Math::Round(d * 100), TBmin, TBmax);
                cnf_fcgTemp->crf = i;
                //小数点以下3桁目カットを考慮
                if (d*1000 != cnf_fcgTemp->crf * 10)
                    fcgTXQuality->Text = Convert::ToString(i / 100.0);
            }
            fcgTBQuality->Value = (int)Math::Round(cnf_fcgTemp->crf / 50);
            break;
        }
    }
    //カーソルの位置を動かさないように   復元したのなら、直前の入力は無効のハズ
    fcgTXQuality->SelectionStart = clamp(c - Convert::ToInt32(restore), 0, fcgTXQuality->Text->Length);
    fcgTXQuality->SelectionLength = 0;
    lastQualityStr = fcgTXQuality->Text;
}

System::Void frmConfig::fcgTXQuality_Validating(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e) {
    switch (x264_encmode_to_RCint[fcgCXX264Mode->SelectedIndex]) {
        case X264_RC_BITRATE:
            //自動モードの場合は除く
            if (fcgCXX264Mode->SelectedIndex == 5 && cnf_fcgTemp->bitrate == -1) {
                fcgTXQuality->Text = STR_BITRATE_AUTO;
            } else
                fcgTXQuality->Text = Convert::ToString(cnf_fcgTemp->bitrate);
            break;
        case X264_RC_QP:
            fcgTXQuality->Text = Convert::ToString(cnf_fcgTemp->qp);
            break;
        case X264_RC_CRF:
            fcgTXQuality->Text = Convert::ToString(cnf_fcgTemp->crf / 100.0);
        default:
            break;
    }
}

System::Void frmConfig::SetTBValueToTextBox() {
    int index = fcgCXX264Mode->SelectedIndex;
    switch (x264_encmode_to_RCint[index]) {
        case X264_RC_BITRATE:
            cnf_fcgTemp->bitrate = TBBConvert.TBToBitrate(fcgTBQuality->Value);
            fcgTXQuality->Text = Convert::ToString(cnf_fcgTemp->bitrate);
            break;
        case X264_RC_QP:
            cnf_fcgTemp->qp = fcgTBQuality->Value;
            fcgTXQuality->Text = Convert::ToString(cnf_fcgTemp->qp);
            break;
        case X264_RC_CRF:
        default:
            cnf_fcgTemp->crf = fcgTBQuality->Value * 50;
            fcgTXQuality->Text = Convert::ToString(cnf_fcgTemp->crf / 100.0);
            break;
    }
}
System::Boolean frmConfig::EnableSettingsNoteChange(bool Enable) {
    if (fcgTSTSettingsNotes->Visible == Enable &&
        fcgTSLSettingsNotes->Visible == !Enable)
        return true;
    if (CountStringBytes(fcgTSTSettingsNotes->Text) > fcgTSTSettingsNotes->MaxLength - 1) {
        MessageBox::Show(this, LOAD_CLI_STRING(AUO_CONFIG_TEXT_LIMIT_LENGTH), LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OK, MessageBoxIcon::Error);
        fcgTSTSettingsNotes->Focus();
        fcgTSTSettingsNotes->SelectionStart = fcgTSTSettingsNotes->Text->Length;
        return false;
    }
    fcgTSTSettingsNotes->Visible = Enable;
    fcgTSLSettingsNotes->Visible = !Enable;
    if (Enable) {
        fcgTSTSettingsNotes->Text = fcgTSLSettingsNotes->Text;
        fcgTSTSettingsNotes->Focus();
        bool isDefaultNote = fcgTSLSettingsNotes->Overflow != ToolStripItemOverflow::Never;
        fcgTSTSettingsNotes->Select((isDefaultNote) ? 0 : fcgTSTSettingsNotes->Text->Length, fcgTSTSettingsNotes->Text->Length);
    } else {
        SetfcgTSLSettingsNotes(fcgTSTSettingsNotes->Text);
        CheckOtherChanges(nullptr, nullptr);
    }
    return true;
}


///////////////////  メモ関連  ///////////////////////////////////////////////
System::Void frmConfig::fcgTSLSettingsNotes_DoubleClick(System::Object^  sender, System::EventArgs^  e) {
    EnableSettingsNoteChange(true);
}
System::Void frmConfig::fcgTSTSettingsNotes_Leave(System::Object^  sender, System::EventArgs^  e) {
    EnableSettingsNoteChange(false);
}
System::Void frmConfig::fcgTSTSettingsNotes_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
    if (e->KeyCode == Keys::Return)
        EnableSettingsNoteChange(false);
}
System::Void frmConfig::fcgTSTSettingsNotes_TextChanged(System::Object^  sender, System::EventArgs^  e) {
    SetfcgTSLSettingsNotes(fcgTSTSettingsNotes->Text);
    CheckOtherChanges(nullptr, nullptr);
}

//////////////////// 追加コマンド関連 /////////////////////////////////////////
System::Void frmConfig::fcgCXCmdExInsert_FontChanged(System::Object^  sender, System::EventArgs^  e) {
    InitCXCmdExInsert();
}
System::Void frmConfig::fcgCXCmdExInsert_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
    String^ insertStr;
    if (       0 == fcgCXCmdExInsert->SelectedIndex) {
        //何もしない
    } else if (1 == fcgCXCmdExInsert->SelectedIndex) {
        //WinXPにおいて、OpenFileDialogはCurrentDirctoryを勝手に変更しやがるので、
        //一度保存し、あとから再適用する
        String^ CurrentDir = Directory::GetCurrentDirectory();
        OpenFileDialog^ ofd = gcnew OpenFileDialog();
        ofd->FileName = L"";
        ofd->Multiselect = false;
        ofd->Filter = L"(*.*)|*.*";
        bool ret = (ofd->ShowDialog() == System::Windows::Forms::DialogResult::OK);
        if (ret) {
            if (sys_dat->exstg->s_local.get_relative_path)
                ofd->FileName = GetRelativePath(ofd->FileName, CurrentDir);
            insertStr = ofd->FileName;
        }
        Directory::SetCurrentDirectory(CurrentDir);
    } else {
        insertStr = String(REPLACE_STRINGS_LIST[fcgCXCmdExInsert->SelectedIndex-2].string).ToString();
    }
    if (insertStr != nullptr && insertStr->Length > 0) {
        int current_selection = fcgTXCmdEx->SelectionStart;
        fcgTXCmdEx->Text = fcgTXCmdEx->Text->Insert(fcgTXCmdEx->SelectionStart, insertStr);
        fcgTXCmdEx->SelectionStart = current_selection + insertStr->Length; //たまに変なところへ行くので念のため必要
        fcgTXCmdEx->Focus();
    }
    fcgCXCmdExInsert->SelectedIndex = 0;
}

System::Void frmConfig::AdjustCXDropDownWidth(ComboBox^ CX) {
    System::Drawing::Graphics^ ds = CX->CreateGraphics();
    float maxwidth = 0.0;
    for (int i = 0; i < CX->Items->Count; i++)
        maxwidth = std::max(maxwidth, ds->MeasureString(CX->Items[i]->ToString(), CX->Font).Width);
    CX->DropDownWidth = (int)(maxwidth + 0.5);
    delete ds;
}

System::Void frmConfig::InitCXCmdExInsert() {
    fcgCXCmdExInsert->BeginUpdate();
    fcgCXCmdExInsert->Items->Clear();
    fcgCXCmdExInsert->Items->Add(LOAD_CLI_STRING(AUO_CONFIG_INSERT_STRING));
    fcgCXCmdExInsert->Items->Add(LOAD_CLI_STRING(AUO_CONFIG_FILE_FULL_PATH));
    System::Drawing::Graphics^ ds = fcgCXCmdExInsert->CreateGraphics();
    float max_width_of_string = 0;
    for (int i = 0; REPLACE_STRINGS_LIST[i].desc; i++)
        max_width_of_string = std::max(max_width_of_string, ds->MeasureString(String(REPLACE_STRINGS_LIST[i].string).ToString() + L" … ", fcgCXCmdExInsert->Font).Width);
    for (int i = 0; REPLACE_STRINGS_LIST[i].desc; i++) {
        String^ AppenStr = String(REPLACE_STRINGS_LIST[i].string).ToString();
        const int length_of_string = AppenStr->Length;
        AppenStr += L" … ";
        for (float current_width = 0.0; current_width < max_width_of_string; AppenStr = AppenStr->Insert(length_of_string, L" "))
            current_width = ds->MeasureString(AppenStr, fcgCXCmdExInsert->Font).Width;
        String^ descStr = LOAD_CLI_STRING(REPLACE_STRINGS_LIST[i].mes);
        if (descStr->Length == 0) {
            descStr = String(REPLACE_STRINGS_LIST[i].desc).ToString();
        }
        AppenStr += descStr;
        fcgCXCmdExInsert->Items->Add(AppenStr);
    }
    delete ds;
    fcgCXCmdExInsert->SelectedIndex = 0;
    AdjustCXDropDownWidth(fcgCXCmdExInsert);
    fcgCXCmdExInsert->EndUpdate();
}

/////////////    音声設定関連の関数    ///////////////
System::Void frmConfig::fcgCBAudio2pass_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
    if (fcgCBAudio2pass->Checked) {
        fcgCBAudioUsePipe->Checked = false;
        fcgCBAudioUsePipe->Enabled = false;
    } else if (CurrentPipeEnabled) {
        fcgCBAudioUsePipe->Checked = true;
        fcgCBAudioUsePipe->Enabled = true;
    }
}

System::Void frmConfig::fcgCXAudioEncoder_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
    setAudioDisplay();
}

System::Void frmConfig::fcgCXAudioEncMode_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
    AudioEncodeModeChanged();
}

System::Int32 frmConfig::GetCurrentAudioDefaultBitrate() {
    return sys_dat->exstg->s_aud[fcgCXAudioEncoder->SelectedIndex].mode[fcgCXAudioEncMode->SelectedIndex].bitrate_default;
}

System::Void frmConfig::setAudioDisplay() {
    AUDIO_SETTINGS *astg = &sys_dat->exstg->s_aud[fcgCXAudioEncoder->SelectedIndex];
    //～の指定
    if (str_has_char(astg->filename)) {
        fcgLBAudioEncoderPath->Text = String(astg->filename).ToString() + LOAD_CLI_STRING(AUO_CONFIG_SPECIFY_EXE_PATH);
        fcgTXAudioEncoderPath->Enabled = true;
        fcgTXAudioEncoderPath->Text = LocalStg.audEncPath[fcgCXAudioEncoder->SelectedIndex];
        fcgBTAudioEncoderPath->Enabled = true;
    } else {
        //filename空文字列(wav出力時)
        fcgLBAudioEncoderPath->Text = L"";
        fcgTXAudioEncoderPath->Enabled = false;
        fcgTXAudioEncoderPath->Text = L"";
        fcgBTAudioEncoderPath->Enabled = false;
    }
    fcgTXAudioEncoderPath->SelectionStart = fcgTXAudioEncoderPath->Text->Length;
    fcgCXAudioEncMode->BeginUpdate();
    fcgCXAudioEncMode->Items->Clear();
    for (int i = 0; i < astg->mode_count; i++)
        fcgCXAudioEncMode->Items->Add(String(astg->mode[i].name).ToString());
    fcgCXAudioEncMode->EndUpdate();
    bool pipe_enabled = (astg->pipe_input && (!(fcgCBAudio2pass->Checked && astg->mode[fcgCXAudioEncMode->SelectedIndex].enc_2pass != 0)));
    CurrentPipeEnabled = pipe_enabled;
    fcgCBAudioUsePipe->Enabled = pipe_enabled;
    fcgCBAudioUsePipe->Checked = pipe_enabled;
    if (fcgCXAudioEncMode->Items->Count > 0)
        fcgCXAudioEncMode->SelectedIndex = 0;
}

System::Void frmConfig::AudioEncodeModeChanged() {
    int index = fcgCXAudioEncMode->SelectedIndex;
    AUDIO_SETTINGS *astg = &sys_dat->exstg->s_aud[fcgCXAudioEncoder->SelectedIndex];
    if (astg->mode[index].bitrate) {
        fcgCXAudioEncMode->Width = fcgCXAudioEncModeSmallWidth;
        fcgLBAudioBitrate->Visible = true;
        fcgNUAudioBitrate->Visible = true;
        fcgNUAudioBitrate->Minimum = astg->mode[index].bitrate_min;
        fcgNUAudioBitrate->Maximum = astg->mode[index].bitrate_max;
        fcgNUAudioBitrate->Increment = astg->mode[index].bitrate_step;
        SetNUValue(fcgNUAudioBitrate, (conf->aud.bitrate != 0) ? conf->aud.bitrate : astg->mode[index].bitrate_default);
    } else {
        fcgCXAudioEncMode->Width = fcgCXAudioEncModeLargeWidth;
        fcgLBAudioBitrate->Visible = false;
        fcgNUAudioBitrate->Visible = false;
        fcgNUAudioBitrate->Minimum = 0;
        fcgNUAudioBitrate->Maximum = 1536; //音声の最大レートは1536kbps
    }
    fcgCBAudio2pass->Enabled = astg->mode[index].enc_2pass != 0;
    if (!fcgCBAudio2pass->Enabled) fcgCBAudio2pass->Checked = false;
    SetfbcBTABEnable(fcgNUAudioBitrate->Visible, (int)fcgNUAudioBitrate->Maximum);

    bool delay_cut_available = astg->mode[index].delay > 0;
    fcgLBAudioDelayCut->Visible = delay_cut_available;
    fcgCXAudioDelayCut->Visible = delay_cut_available;
    if (delay_cut_available) {
        const bool delay_cut_edts_available = str_has_char(astg->cmd_raw) && str_has_char(sys_dat->exstg->s_mux[MUXER_MP4_RAW].delay_cmd);
        const int current_idx = fcgCXAudioDelayCut->SelectedIndex;
        const int items_to_set = _countof(AUDIO_DELAY_CUT_MODE) - 1 - ((delay_cut_edts_available) ? 0 : 1);
        fcgCXAudioDelayCut->BeginUpdate();
        fcgCXAudioDelayCut->Items->Clear();
        for (int i = 0; i < items_to_set; i++) {
            String^ string = nullptr;
            if (AUDIO_DELAY_CUT_MODE[i].mes != AUO_MES_UNKNOWN) {
                string = LOAD_CLI_STRING(AUDIO_DELAY_CUT_MODE[i].mes);
            }
            if (string == nullptr || string->Length == 0) {
                string = String(AUDIO_DELAY_CUT_MODE[i].desc).ToString();
            }
            fcgCXAudioDelayCut->Items->Add(string);
        }
        fcgCXAudioDelayCut->EndUpdate();
        fcgCXAudioDelayCut->SelectedIndex = (current_idx >= items_to_set) ? 0 : current_idx;
    } else {
        fcgCXAudioDelayCut->SelectedIndex = 0;
    }
}

///////////////   設定ファイル関連   //////////////////////
System::Void frmConfig::CheckTSItemsEnabled(CONF_GUIEX *current_conf) {
    bool selected = (CheckedStgMenuItem != nullptr);
    fcgTSBSave->Enabled = (selected && memcmp(cnf_stgSelected, current_conf, sizeof(CONF_GUIEX)));
    fcgTSBDelete->Enabled = selected;
}

System::Void frmConfig::UncheckAllDropDownItem(ToolStripItem^ mItem) {
    ToolStripDropDownItem^ DropDownItem = dynamic_cast<ToolStripDropDownItem^>(mItem);
    if (DropDownItem == nullptr)
        return;
    for (int i = 0; i < DropDownItem->DropDownItems->Count; i++) {
        UncheckAllDropDownItem(DropDownItem->DropDownItems[i]);
        ToolStripMenuItem^ item = dynamic_cast<ToolStripMenuItem^>(DropDownItem->DropDownItems[i]);
        if (item != nullptr)
            item->Checked = false;
    }
}

System::Void frmConfig::CheckTSSettingsDropDownItem(ToolStripMenuItem^ mItem) {
    UncheckAllDropDownItem(fcgTSSettings);
    CheckedStgMenuItem = mItem;
    fcgTSSettings->Text = (mItem == nullptr) ? LOAD_CLI_STRING(AUO_CONFIG_PROFILE) : mItem->Text;
    if (mItem != nullptr)
        mItem->Checked = true;
    fcgTSBSave->Enabled = false;
    fcgTSBDelete->Enabled = (mItem != nullptr);
}

ToolStripMenuItem^ frmConfig::fcgTSSettingsSearchItem(String^ stgPath, ToolStripItem^ mItem) {
    if (stgPath == nullptr)
        return nullptr;
    ToolStripDropDownItem^ DropDownItem = dynamic_cast<ToolStripDropDownItem^>(mItem);
    if (DropDownItem == nullptr)
        return nullptr;
    for (int i = 0; i < DropDownItem->DropDownItems->Count; i++) {
        ToolStripMenuItem^ item = fcgTSSettingsSearchItem(stgPath, DropDownItem->DropDownItems[i]);
        if (item != nullptr)
            return item;
        item = dynamic_cast<ToolStripMenuItem^>(DropDownItem->DropDownItems[i]);
        if (item      != nullptr &&
            item->Tag != nullptr &&
            0 == String::Compare(item->Tag->ToString(), stgPath, true))
            return item;
    }
    return nullptr;
}

ToolStripMenuItem^ frmConfig::fcgTSSettingsSearchItem(String^ stgPath) {
    return fcgTSSettingsSearchItem((stgPath != nullptr && stgPath->Length > 0) ? Path::GetFullPath(stgPath) : nullptr, fcgTSSettings);
}

System::Void frmConfig::SaveToStgFile(String^ stgName) {
    size_t nameLen = CountStringBytes(stgName) + 1;
    char *stg_name = (char *)malloc(nameLen);
    GetCHARfromString(stg_name, nameLen, stgName);
    init_CONF_GUIEX(cnf_stgSelected, fcgCBUsehighbit->Checked);
    FrmToConf(cnf_stgSelected);
    String^ stgDir = Path::GetDirectoryName(stgName);
    if (!Directory::Exists(stgDir))
        Directory::CreateDirectory(stgDir);
    int result = guiEx_config::save_guiEx_conf(cnf_stgSelected, stg_name);
    free(stg_name);
    switch (result) {
        case CONF_ERROR_FILE_OPEN:
            MessageBox::Show(LOAD_CLI_STRING(AUO_CONFIG_ERR_OPEN_STG_FILE), LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OK, MessageBoxIcon::Error);
            return;
        case CONF_ERROR_INVALID_FILENAME:
            MessageBox::Show(LOAD_CLI_STRING(AUO_CONFIG_ERR_INVALID_CHAR), LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OK, MessageBoxIcon::Error);
            return;
        default:
            break;
    }
    init_CONF_GUIEX(cnf_stgSelected, fcgCBUsehighbit->Checked);
    FrmToConf(cnf_stgSelected);
}

System::Void frmConfig::fcgTSBSave_Click(System::Object^  sender, System::EventArgs^  e) {
    if (CheckedStgMenuItem != nullptr)
        SaveToStgFile(CheckedStgMenuItem->Tag->ToString());
    CheckTSSettingsDropDownItem(CheckedStgMenuItem);
}

System::Void frmConfig::fcgTSBSaveNew_Click(System::Object^  sender, System::EventArgs^  e) {
    frmSaveNewStg::Instance::get()->setStgDir(String(sys_dat->exstg->s_local.stg_dir).ToString());
    frmSaveNewStg::Instance::get()->SetTheme(themeMode, dwStgReader);
    if (CheckedStgMenuItem != nullptr)
        frmSaveNewStg::Instance::get()->setFilename(CheckedStgMenuItem->Text);
    frmSaveNewStg::Instance::get()->ShowDialog();
    String^ stgName = frmSaveNewStg::Instance::get()->StgFileName;
    if (stgName != nullptr && stgName->Length)
        SaveToStgFile(stgName);
    RebuildStgFileDropDown(nullptr);
    CheckTSSettingsDropDownItem(fcgTSSettingsSearchItem(stgName));
}

System::Void frmConfig::DeleteStgFile(ToolStripMenuItem^ mItem) {
    if (System::Windows::Forms::DialogResult::OK ==
        MessageBox::Show(LOAD_CLI_STRING(AUO_CONFIG_ASK_STG_FILE_DELETE) + L"[" + mItem->Text + L"]",
        LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OKCancel, MessageBoxIcon::Exclamation))
    {
        File::Delete(mItem->Tag->ToString());
        RebuildStgFileDropDown(nullptr);
        CheckTSSettingsDropDownItem(nullptr);
        SetfcgTSLSettingsNotes(L"");
    }
}

System::Void frmConfig::fcgTSBDelete_Click(System::Object^  sender, System::EventArgs^  e) {
    DeleteStgFile(CheckedStgMenuItem);
}

System::Void frmConfig::fcgTSSettings_DropDownItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e) {
    ToolStripMenuItem^ ClickedMenuItem = dynamic_cast<ToolStripMenuItem^>(e->ClickedItem);
    if (ClickedMenuItem == nullptr)
        return;
    if (ClickedMenuItem->Tag == nullptr || ClickedMenuItem->Tag->ToString()->Length == 0)
        return;
    CONF_GUIEX load_stg;
    char stg_path[MAX_PATH_LEN];
    GetCHARfromString(stg_path, sizeof(stg_path), ClickedMenuItem->Tag->ToString());
    if (guiEx_config::load_guiEx_conf(&load_stg, stg_path) == CONF_ERROR_FILE_OPEN) {
        if (MessageBox::Show(LOAD_CLI_STRING(AUO_CONFIG_ERR_OPEN_STG_FILE) + L"\n"
                           + LOAD_CLI_STRING(AUO_CONFIG_ASK_STG_FILE_DELETE),
                           LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::YesNo, MessageBoxIcon::Error)
                           == System::Windows::Forms::DialogResult::Yes)
            DeleteStgFile(ClickedMenuItem);
        return;
    }
    ConfToFrm(&load_stg, true);
    CheckTSSettingsDropDownItem(ClickedMenuItem);
    memcpy(cnf_stgSelected, &load_stg, sizeof(CONF_GUIEX));
}

System::Void frmConfig::RebuildStgFileDropDown(ToolStripDropDownItem^ TS, String^ dir) {
    array<String^>^ subDirs = Directory::GetDirectories(dir);
    for (int i = 0; i < subDirs->Length; i++) {
        ToolStripMenuItem^ DDItem = gcnew ToolStripMenuItem(L"[ " + subDirs[i]->Substring(dir->Length+1) + L" ]");
        DDItem->DropDownItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &frmConfig::fcgTSSettings_DropDownItemClicked);
        DDItem->ForeColor = Color::Blue;
        DDItem->Tag = nullptr;
        RebuildStgFileDropDown(DDItem, subDirs[i]);
        TS->DropDownItems->Add(DDItem);
    }
    array<String^>^ stgList = Directory::GetFiles(dir, L"*.stg");
    for (int i = 0; i < stgList->Length; i++) {
        ToolStripMenuItem^ mItem = gcnew ToolStripMenuItem(Path::GetFileNameWithoutExtension(stgList[i]));
        mItem->Tag = stgList[i];
        TS->DropDownItems->Add(mItem);
    }
}

System::Void frmConfig::RebuildStgFileDropDown(String^ stgDir) {
    fcgTSSettings->DropDownItems->Clear();
    if (stgDir != nullptr)
        CurrentStgDir = stgDir;
    if (!Directory::Exists(CurrentStgDir))
        Directory::CreateDirectory(CurrentStgDir);
    RebuildStgFileDropDown(fcgTSSettings, Path::GetFullPath(CurrentStgDir));
}

///////////////   言語ファイル関連   //////////////////////

System::Void frmConfig::CheckTSLanguageDropDownItem(ToolStripMenuItem^ mItem) {
    UncheckAllDropDownItem(fcgTSLanguage);
    fcgTSLanguage->Text = (mItem == nullptr) ? LOAD_CLI_STRING(AuofcgTSSettings) : mItem->Text;
    if (mItem != nullptr)
        mItem->Checked = true;
}
System::Void frmConfig::SetSelectedLanguage(const char *language_text) {
    for (int i = 0; i < fcgTSLanguage->DropDownItems->Count; i++) {
        ToolStripMenuItem^ item = dynamic_cast<ToolStripMenuItem^>(fcgTSLanguage->DropDownItems[i]);
        char item_text[MAX_PATH_LEN];
        GetCHARfromString(item_text, sizeof(item_text), item->Tag->ToString());
        if (strncmp(item_text, language_text, strlen(language_text)) == 0) {
            CheckTSLanguageDropDownItem(item);
            break;
        }
    }
}

System::Void frmConfig::SaveSelectedLanguage(const char *language_text) {
    sys_dat->exstg->set_and_save_lang(language_text);
}

System::Void frmConfig::fcgTSLanguage_DropDownItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e) {
    ToolStripMenuItem^ ClickedMenuItem = dynamic_cast<ToolStripMenuItem^>(e->ClickedItem);
    if (ClickedMenuItem == nullptr)
        return;
    if (ClickedMenuItem->Tag == nullptr || ClickedMenuItem->Tag->ToString()->Length == 0)
        return;

    char language_text[MAX_PATH_LEN];
    GetCHARfromString(language_text, sizeof(language_text), ClickedMenuItem->Tag->ToString());
    SaveSelectedLanguage(language_text);
    load_lng(language_text);
    overwrite_aviutl_ini_auo_info();
    LoadLangText();
    CheckTSLanguageDropDownItem(ClickedMenuItem);
}

System::Void frmConfig::InitLangList() {
    if (list_lng != nullptr) {
        delete list_lng;
    }
#define ENABLE_LNG_FILE_DETECT 1
#if ENABLE_LNG_FILE_DETECT
    auto lnglist = find_lng_files();
    list_lng = new std::vector<std::string>();
    for (const auto& lang : lnglist) {
        list_lng->push_back(lang);
    }
#endif

    fcgTSLanguage->DropDownItems->Clear();

    for (const auto& auo_lang : list_auo_languages) {
        String^ label = String(auo_lang.code).ToString() + L" (" + String(auo_lang.name).ToString() + L")";
        ToolStripMenuItem^ mItem = gcnew ToolStripMenuItem(label);
        mItem->DropDownItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &frmConfig::fcgTSLanguage_DropDownItemClicked);
        mItem->Tag = String(auo_lang.code).ToString();
        fcgTSLanguage->DropDownItems->Add(mItem);
    }
#if ENABLE_LNG_FILE_DETECT
    for (size_t i = 0; i < list_lng->size(); i++) {
        auto filename = String(PathFindFileNameA((*list_lng)[i].c_str())).ToString();
        ToolStripMenuItem^ mItem = gcnew ToolStripMenuItem(filename);
        mItem->DropDownItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &frmConfig::fcgTSLanguage_DropDownItemClicked);
        mItem->Tag = filename;
        fcgTSLanguage->DropDownItems->Add(mItem);
    }
#endif
    SetSelectedLanguage(sys_dat->exstg->get_lang());
}

//////////////   初期化関連     ////////////////
System::Void frmConfig::InitData(CONF_GUIEX *set_config, const SYSTEM_DATA *system_data) {
    if (set_config->size_all != CONF_INITIALIZED) {
        //初期化されていなければ初期化する
        init_CONF_GUIEX(set_config, FALSE);
    }
    conf = set_config;
    sys_dat = system_data;
}

System::Void frmConfig::InitComboBox() {
    //コンボボックスに値を設定する
    setComboBox(fcgCXAQMode,         list_aq);
    setComboBox(fcgCXAspectRatio,    aspect_desc);
    setComboBox(fcgCXAudioTempDir,   audtempdir_desc);
    setComboBox(fcgCXBAdpapt,        list_b_adpat);
    setComboBox(fcgCXBpyramid,       list_b_pyramid);
    setComboBox(fcgCXColorMatrix,    list_colormatrix);
    setComboBox(fcgCXColorPrim,      list_colorprim);
    setComboBox(fcgCXInputRange,     list_input_range);
    setComboBox(fcgCXDirectME,       list_direct);
    setComboBox(fcgCXLevel,          list_x264guiEx_level);
    setComboBox(fcgCXLogLevel,       list_log_type);
    setComboBox(fcgCXME,             list_me);
    setComboBox(fcgCXMP4BoxTempDir,  mp4boxtempdir_desc);
    setComboBox(fcgCXNalHrd,         list_nal_hrd);
    setComboBox(fcgCXOutputCsp,      list_output_csp);
    setComboBox(fcgCXPreset,         sys_dat->exstg->s_enc.preset.name);
    setComboBox(fcgCXProfile,        sys_dat->exstg->s_enc.profile.name);
    setComboBox(fcgCXTune,           sys_dat->exstg->s_enc.tune.name);
    setComboBox(fcgCXSubME,          list_subme);
    setComboBox(fcgCXTempDir,        tempdir_desc);
    setComboBox(fcgCXTransfer,       list_transfer);
    setComboBox(fcgCXTrellis,        list_trellis);
    setComboBox(fcgCXVideoFormat,    list_videoformat);
    setComboBox(fcgCXX264Mode,       x264_encodemode_desc);
    setComboBox(fcgCXWeightP,        list_weightp);
    setComboBox(fcgCXInterlaced,     interlaced_desc);

    setComboBox(fcgCXAudioEncTiming, audio_enc_timing_desc);
    setComboBox(fcgCXAudioDelayCut,  AUDIO_DELAY_CUT_MODE);

    InitCXCmdExInsert();

    setMuxerCmdExNames(fcgCXMP4CmdEx, MUXER_MP4);
    setMuxerCmdExNames(fcgCXMKVCmdEx, MUXER_MKV);
    setMuxerCmdExNames(fcgCXMPGCmdEx, MUXER_MPG);

    setAudioEncoderNames();

    setPriorityList(fcgCXX264Priority);
    setPriorityList(fcgCXMuxPriority);
    setPriorityList(fcgCXAudioPriority);
}

System::Void frmConfig::SetTXMaxLen(TextBox^ TX, int max_len) {
    TX->MaxLength = max_len;
    TX->Validating += gcnew System::ComponentModel::CancelEventHandler(this, &frmConfig::TX_LimitbyBytes);
}

System::Void frmConfig::SetTXMaxLenAll() {
    //MaxLengthに最大文字数をセットし、それをもとにバイト数計算を行うイベントをセットする。
    SetTXMaxLen(fcgTXCmdEx,                sizeof(conf->vid.cmdex) - 1);
    SetTXMaxLen(fcgTXX264Path,             sizeof(sys_dat->exstg->s_enc.fullpath) - 1);
    SetTXMaxLen(fcgTXX264PathSub,          sizeof(sys_dat->exstg->s_enc.fullpath) - 1);
    SetTXMaxLen(fcgTXAudioEncoderPath,     sizeof(sys_dat->exstg->s_aud[0].fullpath) - 1);
    SetTXMaxLen(fcgTXMP4MuxerPath,         sizeof(sys_dat->exstg->s_mux[MUXER_MP4].fullpath) - 1);
    SetTXMaxLen(fcgTXMKVMuxerPath,         sizeof(sys_dat->exstg->s_mux[MUXER_MKV].fullpath) - 1);
    SetTXMaxLen(fcgTXTC2MP4Path,           sizeof(sys_dat->exstg->s_mux[MUXER_TC2MP4].fullpath) - 1);
    SetTXMaxLen(fcgTXMPGMuxerPath,         sizeof(sys_dat->exstg->s_mux[MUXER_MPG].fullpath) - 1);
    SetTXMaxLen(fcgTXMP4RawPath,           sizeof(sys_dat->exstg->s_mux[MUXER_MP4_RAW].fullpath) - 1);
    SetTXMaxLen(fcgTXCustomTempDir,        sizeof(sys_dat->exstg->s_local.custom_tmp_dir) - 1);
    SetTXMaxLen(fcgTXCustomAudioTempDir,   sizeof(sys_dat->exstg->s_local.custom_audio_tmp_dir) - 1);
    SetTXMaxLen(fcgTXMP4BoxTempDir,        sizeof(sys_dat->exstg->s_local.custom_mp4box_tmp_dir) - 1);
    SetTXMaxLen(fcgTXStatusFile,           sizeof(conf->vid.stats) - 1);
    SetTXMaxLen(fcgTXTCIN,                 sizeof(conf->vid.tcfile_in) - 1);
    SetTXMaxLen(fcgTXCQM,                  sizeof(conf->vid.cqmfile) - 1);
    SetTXMaxLen(fcgTXBatBeforeAudioPath,   sizeof(conf->oth.batfile.before_audio) - 1);
    SetTXMaxLen(fcgTXBatAfterAudioPath,    sizeof(conf->oth.batfile.after_audio) - 1);
    SetTXMaxLen(fcgTXBatBeforePath,        sizeof(conf->oth.batfile.before_process) - 1);
    SetTXMaxLen(fcgTXBatAfterPath,         sizeof(conf->oth.batfile.after_process) - 1);

    fcgTSTSettingsNotes->MaxLength     =   sizeof(conf->oth.notes) - 1;
}

System::Void frmConfig::InitStgFileList() {
    RebuildStgFileDropDown(String(sys_dat->exstg->s_local.stg_dir).ToString());
    stgChanged = false;
    CheckTSSettingsDropDownItem(nullptr);
}

System::Void frmConfig::fcgChangeEnabled(System::Object^  sender, System::EventArgs^  e) {
    fcgLBX264PathSub->Visible = fcgTSBCMDOnly->Checked;
    fcgTXX264PathSub->Visible = fcgTSBCMDOnly->Checked;
    fcgBTX264PathSub->Visible = fcgTSBCMDOnly->Checked;
    fcggroupBoxDeblock->Enabled = fcgCBDeblock->Checked;
    fcgTXTCIN->Enabled = fcgCBTCIN->Checked;
    fcgBTTCIN->Enabled = fcgCBTCIN->Checked;
    fcgNUTimebaseDen->Enabled = fcgCBTimeBase->Checked;
    fcgNUTimebaseNum->Enabled = fcgCBTimeBase->Checked;
    fcgBTCmdEx->Visible = !fcgTSBCMDOnly->Checked;
    fcgCBNulOutCLI->Visible = fcgTSBCMDOnly->Checked;
}

System::Void frmConfig::fcgChangeMuxerVisible(System::Object^  sender, System::EventArgs^  e) {
    //tc2mp4のチェック
    const bool enable_tc2mp4_muxer = (0 != str_has_char(sys_dat->exstg->s_mux[MUXER_TC2MP4].base_cmd));
    fcgTXTC2MP4Path->Visible = enable_tc2mp4_muxer;
    fcgLBTC2MP4Path->Visible = enable_tc2mp4_muxer;
    fcgBTTC2MP4Path->Visible = enable_tc2mp4_muxer;
    //mp4 rawのチェック
    const bool enable_mp4raw_muxer = (0 != str_has_char(sys_dat->exstg->s_mux[MUXER_MP4_RAW].base_cmd));
    fcgTXMP4RawPath->Visible = enable_mp4raw_muxer;
    fcgLBMP4RawPath->Visible = enable_mp4raw_muxer;
    fcgBTMP4RawPath->Visible = enable_mp4raw_muxer;
    //一時フォルダのチェック
    const bool enable_mp4_tmp = (0 != str_has_char(sys_dat->exstg->s_mux[MUXER_MP4].tmp_cmd));
    fcgCXMP4BoxTempDir->Visible = enable_mp4_tmp;
    fcgLBMP4BoxTempDir->Visible = enable_mp4_tmp;
    fcgTXMP4BoxTempDir->Visible = enable_mp4_tmp;
    fcgBTMP4BoxTempDir->Visible = enable_mp4_tmp;
    //Apple Chapterのチェック
    bool enable_mp4_apple_cmdex = false;
    for (int i = 0; i < sys_dat->exstg->s_mux[MUXER_MP4].ex_count; i++)
        enable_mp4_apple_cmdex |= (0 != str_has_char(sys_dat->exstg->s_mux[MUXER_MP4].ex_cmd[i].cmd_apple));
    fcgCBMP4MuxApple->Visible = enable_mp4_apple_cmdex;

    //位置の調整
    static const int HEIGHT = 31;
    fcgLBTC2MP4Path->Location = Point(fcgLBTC2MP4Path->Location.X, fcgLBMP4MuxerPath->Location.Y + HEIGHT * enable_tc2mp4_muxer);
    fcgTXTC2MP4Path->Location = Point(fcgTXTC2MP4Path->Location.X, fcgTXMP4MuxerPath->Location.Y + HEIGHT * enable_tc2mp4_muxer);
    fcgBTTC2MP4Path->Location = Point(fcgBTTC2MP4Path->Location.X, fcgBTMP4MuxerPath->Location.Y + HEIGHT * enable_tc2mp4_muxer);
    fcgLBMP4RawPath->Location = Point(fcgLBMP4RawPath->Location.X, fcgLBTC2MP4Path->Location.Y   + HEIGHT * enable_mp4raw_muxer);
    fcgTXMP4RawPath->Location = Point(fcgTXMP4RawPath->Location.X, fcgTXTC2MP4Path->Location.Y   + HEIGHT * enable_mp4raw_muxer);
    fcgBTMP4RawPath->Location = Point(fcgBTMP4RawPath->Location.X, fcgBTTC2MP4Path->Location.Y   + HEIGHT * enable_mp4raw_muxer);
}

System::Void frmConfig::SetStgEscKey(bool Enable) {
    if (this->KeyPreview == Enable)
        return;
    this->KeyPreview = Enable;
    if (Enable)
        this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &frmConfig::frmConfig_KeyDown);
    else
        this->KeyDown -= gcnew System::Windows::Forms::KeyEventHandler(this, &frmConfig::frmConfig_KeyDown);
}

System::Void frmConfig::AdjustLocation() {
    //デスクトップ領域(タスクバー等除く)
    System::Drawing::Rectangle screen = System::Windows::Forms::Screen::GetWorkingArea(this);
    //現在のデスクトップ領域の座標
    Point CurrentDesktopLocation = this->DesktopLocation::get();
    //チェック開始
    bool ChangeLocation = false;
    if (CurrentDesktopLocation.X + this->Size.Width > screen.Width) {
        ChangeLocation = true;
        CurrentDesktopLocation.X = clamp(screen.X - this->Size.Width, 4, CurrentDesktopLocation.X);
    }
    if (CurrentDesktopLocation.Y + this->Size.Height > screen.Height) {
        ChangeLocation = true;
        CurrentDesktopLocation.Y = clamp(screen.Y - this->Size.Height, 4, CurrentDesktopLocation.Y);
    }
    if (ChangeLocation) {
        this->StartPosition = FormStartPosition::Manual;
        this->DesktopLocation::set(CurrentDesktopLocation);
    }
}

System::Void frmConfig::InitForm() {
    //UIテーマ切り替え
    CheckTheme();
    //言語設定ファイルのロード
    InitLangList();
    //設定ファイル集の初期化
    InitStgFileList();
    //言語表示
    LoadLangText();
    //スレッド数上限
    int max_threads_set = (int)(cpu_core_count() * 1.5 + 0.51);
    fcgNUThreads->Maximum = max_threads_set;
    fcgNULookaheadThreads->Maximum = max_threads_set;
    //タイマーの初期化
    InitTimer();
    //デフォルトの出力拡張子によってデフォルトのタブを変更
    switch (sys_dat->exstg->s_local.default_output_ext) {
    case 0: //mp4
        fcgtabControlMux->SelectedTab = fcgtabPageMP4;
        break;
    case 1: //mkv
    default:
        fcgtabControlMux->SelectedTab = fcgtabPageMKV;
        break;
    }
    //パラメータセット
    ConfToFrm(conf, true);
    //イベントセット
    SetTXMaxLenAll(); //テキストボックスの最大文字数
    SetAllCheckChangedEvents(this); //変更の確認,ついでにNUの
    //フォームの変更可不可を更新
    fcgChangeMuxerVisible(nullptr, nullptr);
    fcgChangeEnabled(nullptr, nullptr);
    fcgCBAFS_CheckedChanged(nullptr, nullptr);
    EnableSettingsNoteChange(false);
    ExeTXPathLeave();
    //コマンドラインの更新
    fcgRebuildCmd(nullptr, nullptr);
    //表示位置の調整
    AdjustLocation();
    AddfcgLBAMPAutoBitrate();
    AddCheckAMPAutoBitrateEvent();
    //キー設定
    SetStgEscKey(sys_dat->exstg->s_local.enable_stg_esc_key != 0);
    //フォントの設定
    if (str_has_char(sys_dat->exstg->s_local.conf_font.name))
        SetFontFamilyToForm(this, gcnew FontFamily(String(sys_dat->exstg->s_local.conf_font.name).ToString()), this->Font->FontFamily);
}



System::Void frmConfig::LoadLangText() {
    //一度ウィンドウの再描画を完全に抑止する
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 0, 0);
    //空白時にグレーで入れる文字列を言語変更のため一度空白に戻す
    ExeTXPathEnter();
    //言語更新開始
    LOAD_CLI_TEXT(fcgtabPageX264Main);
    LOAD_CLI_TEXT(fcgLBSTATUS);
    LOAD_CLI_TEXT(fcgBTStatusFile);
    LOAD_CLI_TEXT(fcgLBQuality);
    LOAD_CLI_TEXT(fcgLBQualityLeft);
    LOAD_CLI_TEXT(fcgLBQualityRight);
    LOAD_CLI_TEXT(fcgCBAMPLimitBitrateLower);
    LOAD_CLI_TEXT(fcgCBNulOut);
    LOAD_CLI_TEXT(fcgCBAMPLimitBitrateUpper);
    LOAD_CLI_TEXT(fcgCBFastFirstPass);
    LOAD_CLI_TEXT(fcgCBAMPLimitFileSize);
    LOAD_CLI_TEXT(fcgLBAutoNpass);
    LOAD_CLI_TEXT(fcgLBNalHrd);
    LOAD_CLI_TEXT(fcgCBPicStruct);
    LOAD_CLI_TEXT(fcgCBSSIM);
    LOAD_CLI_TEXT(fcgCBPSNR);
    LOAD_CLI_TEXT(fcgLBLog);
    LOAD_CLI_TEXT(fcgLBSlices);
    LOAD_CLI_TEXT(fcgLBLookaheadThreads);
    LOAD_CLI_TEXT(fcgCBSlicedThreads);
    LOAD_CLI_TEXT(fcgLBThreads);
    LOAD_CLI_TEXT(fcgCBUsehighbit);
    LOAD_CLI_TEXT(fcgCBBlurayCompat);
    LOAD_CLI_TEXT(fcgCBAud);
    LOAD_CLI_TEXT(fcgLBVideoFormat);
    LOAD_CLI_TEXT(fcgLBLevel);
    LOAD_CLI_TEXT(fcggroupBoxColorMatrix);
    LOAD_CLI_TEXT(fcgLBInputRange);
    LOAD_CLI_TEXT(fcgLBTransfer);
    LOAD_CLI_TEXT(fcgLBColorPrim);
    LOAD_CLI_TEXT(fcgLBColorMatrix);
    LOAD_CLI_TEXT(fcgGroupBoxAspectRatio);
    LOAD_CLI_TEXT(fcgLBAspectRatio);
    LOAD_CLI_TEXT(fcgBTX264Path);
    LOAD_CLI_TEXT(fcgLBX264Path);
    LOAD_CLI_TEXT(fcggroupBoxPreset);
    LOAD_CLI_TEXT(fcgBTApplyPreset);
    LOAD_CLI_TEXT(fcgLBProfile);
    LOAD_CLI_TEXT(fcgLBX264TUNE);
    LOAD_CLI_TEXT(fcgLBX264Preset);
    LOAD_CLI_TEXT(fcgLBOutputCF);
    LOAD_CLI_TEXT(fcgtabPageX264RC);
    LOAD_CLI_TEXT(fcgLBTimebase);
    LOAD_CLI_TEXT(fcgBTTCIN);
    LOAD_CLI_TEXT(fcgCBTimeBase);
    LOAD_CLI_TEXT(fcgCBTCIN);
    LOAD_CLI_TEXT(fcggroupBoxAQ);
    LOAD_CLI_TEXT(fcgLBAQStrength);
    LOAD_CLI_TEXT(fcgLBAQMode);
    LOAD_CLI_TEXT(fcggroupBoxPsyRd);
    LOAD_CLI_TEXT(fcgLBPsyTrellis);
    LOAD_CLI_TEXT(fcgLBPsyRDO);
    LOAD_CLI_TEXT(fcggroupBoxVbv);
    LOAD_CLI_TEXT(fcgLBVBVafsWarning);
    LOAD_CLI_TEXT(fcgLBVBVbuf);
    LOAD_CLI_TEXT(fcgLBVBVmax);
    LOAD_CLI_TEXT(fcgLBRCLookahead);
    LOAD_CLI_TEXT(fcgCBMBTree);
    LOAD_CLI_TEXT(fcggroupBoxQP);
    LOAD_CLI_TEXT(fcgLBQpstep);
    LOAD_CLI_TEXT(fcgLBChromaQp);
    LOAD_CLI_TEXT(fcgLBQpmax);
    LOAD_CLI_TEXT(fcgLBQpmin);
    LOAD_CLI_TEXT(fcgLBQcomp);
    LOAD_CLI_TEXT(fcgLBPBRatio);
    LOAD_CLI_TEXT(fcgLBIPRatio);
    LOAD_CLI_TEXT(fcgtabPageX264Frame);
    LOAD_CLI_TEXT(fcgCBDeblock);
    LOAD_CLI_TEXT(fcgLBInterlaced);
    LOAD_CLI_TEXT(fcggroupBoxX264Other);
    LOAD_CLI_TEXT(fcgBTMatrix);
    LOAD_CLI_TEXT(fcgLBCQM);
    LOAD_CLI_TEXT(fcgLBTrellis);
    LOAD_CLI_TEXT(fcgCBDctDecimate);
    LOAD_CLI_TEXT(fcgCBfastpskip);
    LOAD_CLI_TEXT(fcggroupBoxME);
    LOAD_CLI_TEXT(fcgLBMixedRef);
    LOAD_CLI_TEXT(fcgLBChromaME);
    LOAD_CLI_TEXT(fcgLBRef);
    LOAD_CLI_TEXT(fcgLBDirectME);
    LOAD_CLI_TEXT(fcgLBMERange);
    LOAD_CLI_TEXT(fcgLBSubME);
    LOAD_CLI_TEXT(fcgLBME);
    LOAD_CLI_TEXT(fcggroupBoxMBTypes);
    LOAD_CLI_TEXT(fcgCBi4x4);
    LOAD_CLI_TEXT(fcgCBp4x4);
    LOAD_CLI_TEXT(fcgCBi8x8);
    LOAD_CLI_TEXT(fcgCBb8x8);
    LOAD_CLI_TEXT(fcgCBp8x8);
    LOAD_CLI_TEXT(fcgCB8x8dct);
    LOAD_CLI_TEXT(fcgLBWeightP);
    LOAD_CLI_TEXT(fcggroupBoxBframes);
    LOAD_CLI_TEXT(fcgLBWeightB);
    LOAD_CLI_TEXT(fcgLBBpyramid);
    LOAD_CLI_TEXT(fcgLBBBias);
    LOAD_CLI_TEXT(fcgLBBAdapt);
    LOAD_CLI_TEXT(fcgLBBframes);
    LOAD_CLI_TEXT(fcgCBCABAC);
    LOAD_CLI_TEXT(fcggroupBoxGOP);
    LOAD_CLI_TEXT(fcgLBOpenGOP);
    LOAD_CLI_TEXT(fcgLBKeyint);
    LOAD_CLI_TEXT(fcgLBMinKeyint);
    LOAD_CLI_TEXT(fcgLBScenecut);
    LOAD_CLI_TEXT(fcgLBDeblockThreshold);
    LOAD_CLI_TEXT(fcgLBDeblockStrength);
    LOAD_CLI_TEXT(fcgtabPageExSettings);
    LOAD_CLI_TEXT(fcgBTX264PathSub);
    LOAD_CLI_TEXT(fcgLBX264PathSub);
    LOAD_CLI_TEXT(fcgLBTempDir);
    LOAD_CLI_TEXT(fcgBTCustomTempDir);
    LOAD_CLI_TEXT(fcggroupBoxCmdEx);
    LOAD_CLI_TEXT(fcgCBNulOutCLI);
    LOAD_CLI_TEXT(fcgBTCmdEx);
    LOAD_CLI_TEXT(fcgLBX264Priority);
    LOAD_CLI_TEXT(fcggroupBoxExSettings);
    LOAD_CLI_TEXT(fcgCBSetKeyframeAtChapter);
    LOAD_CLI_TEXT(fcgCBInputAsLW48);
    LOAD_CLI_TEXT(fcgCBCheckKeyframes);
    LOAD_CLI_TEXT(fcgCBAuoTcfileout);
    LOAD_CLI_TEXT(fcgCBAFSBitrateCorrection);
    LOAD_CLI_TEXT(fcgCBAFS);
    LOAD_CLI_TEXT(fcgTSExeFileshelp);
    LOAD_CLI_TEXT(fcgtoolStripSettings);
    LOAD_CLI_TEXT(fcgTSBSave);
    LOAD_CLI_TEXT(fcgTSBSaveNew);
    LOAD_CLI_TEXT(fcgTSBDelete);
    LOAD_CLI_TEXT(fcgTSSettings);
    LOAD_CLI_TEXT(fcgTSBCMDOnly);
    LOAD_CLI_TEXT(fcgTSBBitrateCalc);
    LOAD_CLI_TEXT(fcgTSBOtherSettings);
    LOAD_CLI_TEXT(fcgTSLSettingsNotes);
    LOAD_CLI_TEXT(fcgTSTSettingsNotes);
    //LOAD_CLI_TEXT(fcgTSLanguage); // 不要
    LOAD_CLI_TEXT(fcgtabPageMP4);
    LOAD_CLI_TEXT(fcgCBMP4MuxApple);
    LOAD_CLI_TEXT(fcgBTTC2MP4Path);
    LOAD_CLI_TEXT(fcgBTMP4MuxerPath);
    LOAD_CLI_TEXT(fcgLBTC2MP4Path);
    LOAD_CLI_TEXT(fcgLBMP4MuxerPath);
    LOAD_CLI_TEXT(fcgLBMP4CmdEx);
    LOAD_CLI_TEXT(fcgCBMP4MuxerExt);
    LOAD_CLI_TEXT(fcgBTMP4RawPath);
    LOAD_CLI_TEXT(fcgLBMP4RawPath);
    LOAD_CLI_TEXT(fcgBTMP4BoxTempDir);
    LOAD_CLI_TEXT(fcgLBMP4BoxTempDir);
    LOAD_CLI_TEXT(fcgtabPageMKV);
    LOAD_CLI_TEXT(fcgBTMKVMuxerPath);
    LOAD_CLI_TEXT(fcgLBMKVMuxerPath);
    LOAD_CLI_TEXT(fcgLBMKVMuxerCmdEx);
    LOAD_CLI_TEXT(fcgCBMKVMuxerExt);
    LOAD_CLI_TEXT(fcgtabPageMPG);
    LOAD_CLI_TEXT(fcgBTMPGMuxerPath);
    LOAD_CLI_TEXT(fcgLBMPGMuxerPath);
    LOAD_CLI_TEXT(fcgLBMPGMuxerCmdEx);
    LOAD_CLI_TEXT(fcgCBMPGMuxerExt);
    LOAD_CLI_TEXT(fcgtabPageMux);
    LOAD_CLI_TEXT(fcgLBMuxPriority);
    LOAD_CLI_TEXT(fcgtabPageBat);
    LOAD_CLI_TEXT(fcgLBBatAfterString);
    LOAD_CLI_TEXT(fcgLBBatBeforeString);
    LOAD_CLI_TEXT(fcgBTBatAfterPath);
    LOAD_CLI_TEXT(fcgLBBatAfterPath);
    LOAD_CLI_TEXT(fcgCBWaitForBatAfter);
    LOAD_CLI_TEXT(fcgCBRunBatAfter);
    LOAD_CLI_TEXT(fcgBTBatBeforePath);
    LOAD_CLI_TEXT(fcgLBBatBeforePath);
    LOAD_CLI_TEXT(fcgCBWaitForBatBefore);
    LOAD_CLI_TEXT(fcgCBRunBatBefore);
    LOAD_CLI_TEXT(fcgBTCancel);
    LOAD_CLI_TEXT(fcgBTOK);
    LOAD_CLI_TEXT(fcgBTDefault);
    LOAD_CLI_TEXT(fcgLBVersionDate);
    LOAD_CLI_TEXT(fcgLBVersion);
    LOAD_CLI_TEXT(fcgCSFlat);
    LOAD_CLI_TEXT(fcgCSJvt);
    LOAD_CLI_TEXT(fcgCSCqmFile);
    LOAD_CLI_TEXT(fcgLBguiExBlog);
    LOAD_CLI_TEXT(fcgtabPageAudioMain);
    LOAD_CLI_TEXT(fcgLBAudioDelayCut);
    LOAD_CLI_TEXT(fcgCBAudioEncTiming);
    LOAD_CLI_TEXT(fcgBTCustomAudioTempDir);
    LOAD_CLI_TEXT(fcgCBAudioUsePipe);
    LOAD_CLI_TEXT(fcgLBAudioBitrate);
    LOAD_CLI_TEXT(fcgCBAudio2pass);
    LOAD_CLI_TEXT(fcgLBAudioEncMode);
    LOAD_CLI_TEXT(fcgBTAudioEncoderPath);
    LOAD_CLI_TEXT(fcgLBAudioEncoderPath);
    LOAD_CLI_TEXT(fcgCBAudioOnly);
    LOAD_CLI_TEXT(fcgCBFAWCheck);
    LOAD_CLI_TEXT(fcgLBAudioEncoder);
    LOAD_CLI_TEXT(fcgLBAudioTemp);
    LOAD_CLI_TEXT(fcgtabPageAudioOther);
    LOAD_CLI_TEXT(fcgLBBatAfterAudioString);
    LOAD_CLI_TEXT(fcgLBBatBeforeAudioString);
    LOAD_CLI_TEXT(fcgBTBatAfterAudioPath);
    LOAD_CLI_TEXT(fcgLBBatAfterAudioPath);
    LOAD_CLI_TEXT(fcgCBRunBatAfterAudio);
    LOAD_CLI_TEXT(fcgBTBatBeforeAudioPath);
    LOAD_CLI_TEXT(fcgLBBatBeforeAudioPath);
    LOAD_CLI_TEXT(fcgCBRunBatBeforeAudio);
    LOAD_CLI_TEXT(fcgLBAudioPriority);
    //LOAD_CLI_MAIN_TEXT(fcgMain);

    //ローカル設定のロード(ini変更を反映)
    LoadLocalStg();
    //ローカル設定の反映
    SetLocalStg();
    //コンボボックスの値を設定
    InitComboBox();
    //ツールチップ
    SetHelpToolTips();
    SetX264VersionToolTip(LocalStg.x264Path);
    ActivateToolTip(sys_dat->exstg->s_local.disable_tooltip_help == FALSE);
    //タイムコードのappendix(後付修飾子)を反映
    fcgCBAuoTcfileout->Text = LOAD_CLI_STRING(AUO_CONFIG_TC_FILE_OUT) + L" (" + String(sys_dat->exstg->s_append.tc).ToString() + L")";
    { //タイトル表示,バージョン情報,コンパイル日時
        auto auo_full_name = g_auo_mes.get(AUO_GUIEX_FULL_NAME);
        if (auo_full_name == nullptr || wcslen(auo_full_name) == 0) auo_full_name = AUO_FULL_NAME_W;
        this->Text = String(auo_full_name).ToString();
        fcgLBVersion->Text = String(auo_full_name).ToString() + L" " + String(AUO_VERSION_STR_W).ToString();
        fcgLBVersionDate->Text = L"build " + String(__DATE__).ToString() + L" " + String(__TIME__).ToString();
    }
    //空白時にグレーで入れる文字列を言語に即して復活させる
    ExeTXPathLeave();
    //一度ウィンドウの再描画を再開し、強制的に再描画させる
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 1, 0);
    this->Refresh();
}

/////////////         データ <-> GUI     /////////////
System::Void frmConfig::ConfToFrm(CONF_GUIEX *cnf, bool all) {
    //ひたすら書くだけ。めんどい
    CONF_X264 *cx264 = &cnf->enc;
    memcpy(cnf_fcgTemp, cx264, sizeof(CONF_X264)); //一時保存用
    this->SuspendLayout();
    fcgCBUsehighbit->Checked = cx264->use_highbit_depth != 0;
    switch (cx264->rc_mode) {
        case X264_RC_QP:
            fcgCXX264Mode->SelectedIndex = 1;
            break;
        case X264_RC_BITRATE:
            if (cx264->use_auto_npass)
                fcgCXX264Mode->SelectedIndex = 5;
            else {
                switch (cx264->pass) {
                    case 0:  fcgCXX264Mode->SelectedIndex = 0; break;
                    case 1:  fcgCXX264Mode->SelectedIndex = 3; break;
                    default: fcgCXX264Mode->SelectedIndex = 4; break;
                }
            }
            break;
        case X264_RC_CRF:
        default:
            fcgCXX264Mode->SelectedIndex = (cx264->use_auto_npass) ? 6 : 2;
            break;
    }
    fcgCXX264Mode_SelectedIndexChanged(nullptr, nullptr); //こいつをやっとかないと更新されないこともある

    SetNUValue(fcgNUAutoNPass,        cx264->auto_npass);
    SetCXIndex(fcgCXPreset,           cx264->preset);
    SetCXIndex(fcgCXTune,             cx264->tune);
    SetCXIndex(fcgCXProfile,          cx264->profile);
    fcgCBAMPLimitBitrateLower->Checked   = (cnf->vid.amp_check & AMPLIMIT_BITRATE_LOWER) != 0;
    fcgCBAMPLimitBitrateUpper->Checked = (cnf->vid.amp_check & AMPLIMIT_BITRATE_UPPER) != 0;
    fcgCBAMPLimitFileSize->Checked     = (cnf->vid.amp_check & AMPLIMIT_FILE_SIZE) != 0;
    SetNUValue(fcgNUAMPLimitFileSize, cnf->vid.amp_limit_file_size);
    SetNUValue(fcgNUAMPLimitBitrateUpper, cnf->vid.amp_limit_bitrate_upper);
    SetNUValue(fcgNUAMPLimitBitrateLower, cnf->vid.amp_limit_bitrate_lower);
    SetCXIndex(fcgCXLevel,            cx264->h264_level);
    SetCXIndex(fcgCXVideoFormat,      cx264->videoformat);
    fcgCBAud->Checked               = cx264->aud != 0;
    fcgCBPicStruct->Checked         = cx264->pic_struct != 0;
    SetCXIndex(fcgCXNalHrd,           cx264->nal_hrd);
    SetCXIndex(fcgCXOutputCsp,        cx264->output_csp);
    fcgCBBlurayCompat->Checked      = cx264->bluray_compat != 0;
    SetCXIndex(fcgCXColorMatrix,      cx264->colormatrix);
    SetCXIndex(fcgCXColorPrim,        cx264->colorprim);
    SetCXIndex(fcgCXTransfer,         cx264->transfer);
    SetCXIndex(fcgCXInputRange,       cx264->input_range);

    if (cx264->sar.x * cx264->sar.y < 0)
        cx264->sar.x = cx264->sar.y = 0;
    fcgCXAspectRatio->SelectedIndex= (cx264->sar.x < 0);
    SetNUValue(fcgNUAspectRatioX, abs(cx264->sar.x));
    SetNUValue(fcgNUAspectRatioY, abs(cx264->sar.y));

    SetNUValue(fcgNUThreads,          cx264->threads);
    SetNUValue(fcgNULookaheadThreads, cx264->lookahead_threads);
    fcgCBSlicedThreads->Checked     = cx264->sliced_threading != 0;

    SetCXIndex(fcgCXLogLevel,         cx264->log_mode);
    fcgCBPSNR->Checked              = cx264->psnr != 0;
    fcgCBSSIM->Checked              = cx264->ssim != 0;

    SetNUValue(fcgNUIPRatio,   ((int)(cx264->ip_ratio * 100 + 0.5) - 100));
    SetNUValue(fcgNUPBRatio,   ((int)(cx264->pb_ratio * 100 + 0.5) - 100));
    SetNUValue(fcgNUQcomp,      (int)(cx264->qp_compress * 100 + 0.5));
    SetNUValue(fcgNUQpmin,            cx264->qp_min);
    SetNUValue(fcgNUQpmax,            cx264->qp_max);
    SetNUValue(fcgNUQpstep,           cx264->qp_step);
    SetNUValue(fcgNUChromaQp,         cx264->chroma_qp_offset);
    SetCXIndex(fcgCXAQMode,           cx264->aq_mode);
    SetNUValue(fcgNUAQStrength,       cx264->aq_strength);
    SetNUValue(fcgNUPsyRDO,           cx264->psy_rd.x);
    SetNUValue(fcgNUPsyTrellis,       cx264->psy_rd.y);
    fcgCBMBTree->Checked            = cx264->mbtree != 0;
    SetNUValue(fcgNURCLookahead,      cx264->rc_lookahead);
    SetNUValue(fcgNUVBVmax,           cx264->vbv_maxrate);
    SetNUValue(fcgNUVBVbuf,           cx264->vbv_bufsize);

    SetNUValue(fcgNUScenecut,         cx264->scenecut);
    SetNUValue(fcgNUKeyint,           cx264->keyint_max);
    SetNUValue(fcgNUMinKeyint,        cx264->keyint_min);
    fcgCBOpenGOP->Checked           = cx264->open_gop != 0;

    fcgCBDeblock->Checked           = cx264->use_deblock != 0;
    SetNUValue(fcgNUDeblockStrength,  cx264->deblock.x);
    SetNUValue(fcgNUDeblockThreshold, cx264->deblock.y);

    fcgCBCABAC->Checked = cx264->cabac != 0;
    SetNUValue(fcgNUSlices,           cx264->slice_n);
    SetCXIndex(fcgCXWeightP,          cx264->weight_p);

    fcgCXInterlaced->SelectedIndex  = (!cx264->interlaced) ? 0 : ((cx264->tff) ? 1 : 2);

    SetNUValue(fcgNUBframes,          cx264->bframes);
    SetCXIndex(fcgCXBAdpapt,          cx264->b_adapt);
    SetNUValue(fcgNUBBias,            cx264->b_bias);
    SetCXIndex(fcgCXBpyramid,         cx264->b_pyramid);
    fcgCBWeightB->Checked           = cx264->weight_b != 0;

    fcgCB8x8dct->Checked            = cx264->dct8x8 != 0;
    fcgCBp8x8->Checked             = (cx264->mb_partition & MB_PARTITION_P8x8) != 0;
    fcgCBb8x8->Checked             = (cx264->mb_partition & MB_PARTITION_B8x8) != 0;
    fcgCBp4x4->Checked             = (cx264->mb_partition & MB_PARTITION_P4x4) != 0;
    fcgCBi8x8->Checked             = (cx264->mb_partition & MB_PARTITION_I8x8) != 0;
    fcgCBi4x4->Checked             = (cx264->mb_partition & MB_PARTITION_I4x4) != 0;

    fcgCBfastpskip->Checked         = cx264->no_fast_pskip == 0;
    fcgCBDctDecimate->Checked       = cx264->no_dct_decimate == 0;
    SetCXIndex(fcgCXTrellis,          cx264->trellis);
    SetCQM(cx264->cqm,                cnf->vid.cqmfile);

    SetCXIndex(fcgCXME,               cx264->me);
    SetCXIndex(fcgCXSubME,            cx264->subme);
    SetNUValue(fcgNUMERange,          cx264->me_range);
    fcgCBChromaME->Checked          = cx264->chroma_me != 0;
    SetCXIndex(fcgCXDirectME,         cx264->direct_mv);
    SetNUValue(fcgNURef,              cx264->ref_frames);
    fcgCBMixedRef->Checked          = cx264->mixed_ref != 0;

    fcgCBTCIN->Checked              = cx264->use_tcfilein != 0;
    fcgCBTimeBase->Checked          = cx264->use_timebase != 0;
    SetNUValue(fcgNUTimebaseNum,      cx264->timebase.x);
    SetNUValue(fcgNUTimebaseDen,      cx264->timebase.y);

    if (all) {
        //動画部
        fcgTXStatusFile->Text = (str_has_char(cnf->vid.stats))     ? String(cnf->vid.stats).ToString() : String(DefaultStatusFilePath).ToString();
        fcgTXTCIN->Text       = (str_has_char(cnf->vid.tcfile_in)) ? String(cnf->vid.tcfile_in).ToString() : String(DefaultTcFilePath).ToString();

        fcgCBAFS->Checked                  = cnf->vid.afs != 0;
        fcgCBAFSBitrateCorrection->Checked = cnf->vid.afs_bitrate_correction != 0;
        fcgCBAuoTcfileout->Checked         = cnf->vid.auo_tcfile_out != 0;
        fcgCBCheckKeyframes->Checked       =(cnf->vid.check_keyframe & CHECK_KEYFRAME_AVIUTL) != 0;
        fcgCBSetKeyframeAtChapter->Checked =(cnf->vid.check_keyframe & CHECK_KEYFRAME_CHAPTER) != 0;
        fcgCBInputAsLW48->Checked          = cnf->vid.input_as_lw48 != 0;

        SetCXIndex(fcgCXX264Priority,        cnf->vid.priority);
        SetCXIndex(fcgCXTempDir,             cnf->oth.temp_dir);

        fcgTXCmdEx->Text            = String(cnf->vid.cmdex).ToString();
        if (cnf->oth.disable_guicmd)
            fcgCBNulOutCLI->Checked        = cnf->enc.nul_out != 0;

        //音声
        fcgCBAudioOnly->Checked            = cnf->oth.out_audio_only != 0;
        fcgCBFAWCheck->Checked             = cnf->aud.faw_check != 0;
        SetCXIndex(fcgCXAudioEncoder,        cnf->aud.encoder);
        fcgCBAudio2pass->Checked           = cnf->aud.use_2pass != 0;
        fcgCBAudioUsePipe->Checked = (CurrentPipeEnabled && !cnf->aud.use_wav);
        SetCXIndex(fcgCXAudioDelayCut,       cnf->aud.delay_cut);
        SetCXIndex(fcgCXAudioEncMode,        cnf->aud.enc_mode);
        SetNUValue(fcgNUAudioBitrate,       (cnf->aud.bitrate != 0) ? cnf->aud.bitrate : GetCurrentAudioDefaultBitrate());
        SetCXIndex(fcgCXAudioPriority,       cnf->aud.priority);
        SetCXIndex(fcgCXAudioTempDir,        cnf->aud.aud_temp_dir);
        SetCXIndex(fcgCXAudioEncTiming,      cnf->aud.audio_encode_timing);
        fcgCBRunBatBeforeAudio->Checked    =(cnf->oth.run_bat & RUN_BAT_BEFORE_AUDIO) != 0;
        fcgCBRunBatAfterAudio->Checked     =(cnf->oth.run_bat & RUN_BAT_AFTER_AUDIO) != 0;
        fcgTXBatBeforeAudioPath->Text      = String(cnf->oth.batfile.before_audio).ToString();
        fcgTXBatAfterAudioPath->Text       = String(cnf->oth.batfile.after_audio).ToString();

        //mux
        fcgCBMP4MuxerExt->Checked          = cnf->mux.disable_mp4ext == 0;
        fcgCBMP4MuxApple->Checked          = cnf->mux.apple_mode != 0;
        SetCXIndex(fcgCXMP4CmdEx,            cnf->mux.mp4_mode);
        SetCXIndex(fcgCXMP4BoxTempDir,       cnf->mux.mp4_temp_dir);
        fcgCBMKVMuxerExt->Checked          = cnf->mux.disable_mkvext == 0;
        SetCXIndex(fcgCXMKVCmdEx,            cnf->mux.mkv_mode);
        fcgCBMPGMuxerExt->Checked          = cnf->mux.disable_mpgext == 0;
        SetCXIndex(fcgCXMPGCmdEx,            cnf->mux.mpg_mode);
        SetCXIndex(fcgCXMuxPriority,         cnf->mux.priority);

        fcgCBRunBatBefore->Checked         =(cnf->oth.run_bat & RUN_BAT_BEFORE_PROCESS) != 0;
        fcgCBRunBatAfter->Checked          =(cnf->oth.run_bat & RUN_BAT_AFTER_PROCESS)  != 0;
        fcgCBWaitForBatBefore->Checked     =(cnf->oth.dont_wait_bat_fin & RUN_BAT_BEFORE_PROCESS) == 0;
        fcgCBWaitForBatAfter->Checked      =(cnf->oth.dont_wait_bat_fin & RUN_BAT_AFTER_PROCESS)  == 0;
        fcgTXBatBeforePath->Text           = String(cnf->oth.batfile.before_process).ToString();
        fcgTXBatAfterPath->Text            = String(cnf->oth.batfile.after_process).ToString();

        SetfcgTSLSettingsNotes(cnf->oth.notes);

        //cli mode
        fcgTSBCMDOnly->Checked             = cnf->oth.disable_guicmd != 0;
    }

    this->ResumeLayout();
    this->PerformLayout();
}

System::Void frmConfig::FrmToConf(CONF_GUIEX *cnf) {
    //これもひたすら書くだけ。めんどい
    //x264部
    cnf->enc.use_highbit_depth    = fcgCBUsehighbit->Checked;
    cnf->enc.rc_mode              = cnf_fcgTemp->rc_mode;
    cnf->enc.bitrate              = cnf_fcgTemp->bitrate;
    cnf->enc.qp                   = cnf_fcgTemp->qp;
    cnf->enc.crf                  = cnf_fcgTemp->crf;
    cnf->enc.nul_out              = fcgCBNulOut->Checked;
    cnf->enc.pass                 = cnf_fcgTemp->pass;
    cnf->enc.slow_first_pass      = cnf_fcgTemp->slow_first_pass;
    cnf->enc.use_auto_npass       = cnf_fcgTemp->use_auto_npass;
    cnf->enc.auto_npass           = (int)fcgNUAutoNPass->Value;
    cnf->vid.amp_check             = 0x00;
    cnf->vid.amp_check            |= fcgCBAMPLimitBitrateLower->Checked ? AMPLIMIT_BITRATE_LOWER : 0x00;
    cnf->vid.amp_check            |= fcgCBAMPLimitBitrateUpper->Checked ? AMPLIMIT_BITRATE_UPPER : 0x00;
    cnf->vid.amp_check            |= fcgCBAMPLimitFileSize->Checked ? AMPLIMIT_FILE_SIZE : 0x00;
    cnf->vid.amp_limit_bitrate_upper = (double)fcgNUAMPLimitBitrateUpper->Value;
    cnf->vid.amp_limit_bitrate_lower = (double)fcgNUAMPLimitBitrateLower->Value;
    cnf->vid.amp_limit_file_size     = (double)fcgNUAMPLimitFileSize->Value;
    cnf->enc.preset               = fcgCXPreset->SelectedIndex;
    cnf->enc.tune                 = fcgCXTune->SelectedIndex;
    cnf->enc.profile              = fcgCXProfile->SelectedIndex;
    cnf->enc.h264_level           = fcgCXLevel->SelectedIndex;
    cnf->enc.videoformat          = fcgCXVideoFormat->SelectedIndex;
    cnf->enc.aud                  = fcgCBAud->Checked;
    cnf->enc.pic_struct           = fcgCBPicStruct->Checked;
    cnf->enc.nal_hrd              = fcgCXNalHrd->SelectedIndex;
    cnf->enc.output_csp           = fcgCXOutputCsp->SelectedIndex;
    cnf->enc.bluray_compat        = fcgCBBlurayCompat->Checked;
    cnf->enc.colormatrix          = fcgCXColorMatrix->SelectedIndex;
    cnf->enc.colorprim            = fcgCXColorPrim->SelectedIndex;
    cnf->enc.transfer             = fcgCXTransfer->SelectedIndex;
    cnf->enc.input_range          = fcgCXInputRange->SelectedIndex;
    cnf->enc.sar.x                = (int)fcgNUAspectRatioX->Value * ((fcgCXAspectRatio->SelectedIndex != 1) ? 1 : -1);
    cnf->enc.sar.y                = (int)fcgNUAspectRatioY->Value * ((fcgCXAspectRatio->SelectedIndex != 1) ? 1 : -1);
    cnf->enc.threads              = (int)fcgNUThreads->Value;
    cnf->enc.lookahead_threads    = (int)fcgNULookaheadThreads->Value;
    cnf->enc.sliced_threading     = fcgCBSlicedThreads->Checked;
    cnf->enc.log_mode             = fcgCXLogLevel->SelectedIndex;
    cnf->enc.psnr                 = fcgCBPSNR->Checked;
    cnf->enc.ssim                 = fcgCBSSIM->Checked;

    cnf->enc.ip_ratio             = (float)((int)fcgNUIPRatio->Value + 100) / 100.0f;
    cnf->enc.pb_ratio             = (float)((int)fcgNUPBRatio->Value + 100) / 100.0f;
    cnf->enc.qp_compress          = (float)((int)fcgNUQcomp->Value / 100.0f);
    cnf->enc.qp_min               = (int)fcgNUQpmin->Value;
    cnf->enc.qp_max               = (int)fcgNUQpmax->Value;
    cnf->enc.qp_step              = (int)fcgNUQpstep->Value;
    cnf->enc.chroma_qp_offset     = (int)fcgNUChromaQp->Value;
    cnf->enc.aq_mode              = fcgCXAQMode->SelectedIndex;
    cnf->enc.aq_strength          = (float)fcgNUAQStrength->Value;
    cnf->enc.psy_rd.x             = (float)fcgNUPsyRDO->Value;
    cnf->enc.psy_rd.y             = (float)fcgNUPsyTrellis->Value;
    cnf->enc.vbv_maxrate          = (int)fcgNUVBVmax->Value;
    cnf->enc.vbv_bufsize          = (int)fcgNUVBVbuf->Value;
    cnf->enc.mbtree               = fcgCBMBTree->Checked;
    cnf->enc.rc_lookahead         = (int)fcgNURCLookahead->Value;

    cnf->enc.scenecut             = (int)fcgNUScenecut->Value;
    cnf->enc.keyint_min           = (int)fcgNUMinKeyint->Value;
    cnf->enc.keyint_max           = (int)fcgNUKeyint->Value;
    cnf->enc.open_gop             = fcgCBOpenGOP->Checked;
    cnf->enc.use_deblock          = fcgCBDeblock->Checked;
    cnf->enc.deblock.x            = (int)fcgNUDeblockStrength->Value;
    cnf->enc.deblock.y            = (int)fcgNUDeblockThreshold->Value;
    cnf->enc.cabac                = fcgCBCABAC->Checked;
    cnf->enc.slice_n              = (int)fcgNUSlices->Value;
    cnf->enc.weight_p             = fcgCXWeightP->SelectedIndex;
    cnf->enc.interlaced           = fcgCXInterlaced->SelectedIndex != 0;
    cnf->enc.tff                  = fcgCXInterlaced->SelectedIndex != 2;
    cnf->enc.bframes              = (int)fcgNUBframes->Value;
    cnf->enc.b_adapt              = fcgCXBAdpapt->SelectedIndex;
    cnf->enc.b_bias               = (int)fcgNUBBias->Value;
    cnf->enc.b_pyramid            = fcgCXBpyramid->SelectedIndex;
    cnf->enc.weight_b             = fcgCBWeightB->Checked;

    cnf->enc.dct8x8               = fcgCB8x8dct->Checked;
    cnf->enc.mb_partition         = MB_PARTITION_NONE;
    cnf->enc.mb_partition        |= (fcgCBb8x8->Checked) ? MB_PARTITION_B8x8 : MB_PARTITION_NONE;
    cnf->enc.mb_partition        |= (fcgCBp8x8->Checked) ? MB_PARTITION_P8x8 : MB_PARTITION_NONE;
    cnf->enc.mb_partition        |= (fcgCBp4x4->Checked) ? MB_PARTITION_P4x4 : MB_PARTITION_NONE;
    cnf->enc.mb_partition        |= (fcgCBi8x8->Checked) ? MB_PARTITION_I8x8 : MB_PARTITION_NONE;
    cnf->enc.mb_partition        |= (fcgCBi4x4->Checked) ? MB_PARTITION_I4x4 : MB_PARTITION_NONE;

    cnf->enc.no_fast_pskip        = !fcgCBfastpskip->Checked;
    cnf->enc.no_dct_decimate      = !fcgCBDctDecimate->Checked;
    cnf->enc.trellis              = fcgCXTrellis->SelectedIndex;
    cnf->enc.cqm                  = GetCQMIndex(cnf->vid.cqmfile, sizeof(cnf->vid.cqmfile));

    cnf->enc.me                   = fcgCXME->SelectedIndex;
    cnf->enc.subme                = fcgCXSubME->SelectedIndex;
    cnf->enc.me_range             = (int)fcgNUMERange->Value;
    cnf->enc.chroma_me            = fcgCBChromaME->Checked;
    cnf->enc.direct_mv            = fcgCXDirectME->SelectedIndex;
    cnf->enc.ref_frames           = (int)fcgNURef->Value;
    cnf->enc.mixed_ref            = fcgCBMixedRef->Checked;

    cnf->enc.use_tcfilein         = fcgCBTCIN->Checked;
    cnf->enc.use_timebase         = fcgCBTimeBase->Checked;
    cnf->enc.timebase.x           = (int)fcgNUTimebaseNum->Value;
    cnf->enc.timebase.y           = (int)fcgNUTimebaseDen->Value;

    GetCHARfromString(cnf->vid.stats,     fcgTXStatusFile->Text);
    GetCHARfromString(cnf->vid.tcfile_in, fcgTXTCIN->Text);

    //拡張部
    cnf->vid.afs                    = fcgCBAFS->Checked;
    cnf->vid.afs_bitrate_correction = fcgCBAFSBitrateCorrection->Checked;
    cnf->vid.auo_tcfile_out         = fcgCBAuoTcfileout->Checked;
    cnf->vid.check_keyframe         = CHECK_KEYFRAME_NONE;
    cnf->vid.check_keyframe        |= (fcgCBCheckKeyframes->Checked)       ? CHECK_KEYFRAME_AVIUTL  : CHECK_KEYFRAME_NONE;
    cnf->vid.check_keyframe        |= (fcgCBSetKeyframeAtChapter->Checked) ? CHECK_KEYFRAME_CHAPTER : CHECK_KEYFRAME_NONE;
    cnf->vid.priority               = fcgCXX264Priority->SelectedIndex;
    cnf->vid.input_as_lw48          = fcgCBInputAsLW48->Checked;
    cnf->oth.temp_dir               = fcgCXTempDir->SelectedIndex;
    GetCHARfromString(cnf->vid.cmdex, fcgTXCmdEx->Text);

    //音声部
    cnf->aud.encoder                = fcgCXAudioEncoder->SelectedIndex;
    cnf->oth.out_audio_only         = fcgCBAudioOnly->Checked;
    cnf->aud.faw_check              = fcgCBFAWCheck->Checked;
    cnf->aud.enc_mode               = fcgCXAudioEncMode->SelectedIndex;
    cnf->aud.bitrate                = (int)fcgNUAudioBitrate->Value;
    cnf->aud.use_2pass              = fcgCBAudio2pass->Checked;
    cnf->aud.use_wav                = !fcgCBAudioUsePipe->Checked;
    cnf->aud.delay_cut              = fcgCXAudioDelayCut->SelectedIndex;
    cnf->aud.priority               = fcgCXAudioPriority->SelectedIndex;
    cnf->aud.audio_encode_timing    = fcgCXAudioEncTiming->SelectedIndex;
    cnf->aud.aud_temp_dir           = fcgCXAudioTempDir->SelectedIndex;

    //mux部
    cnf->mux.disable_mp4ext         = !fcgCBMP4MuxerExt->Checked;
    cnf->mux.apple_mode             = fcgCBMP4MuxApple->Checked;
    cnf->mux.mp4_mode               = fcgCXMP4CmdEx->SelectedIndex;
    cnf->mux.mp4_temp_dir           = fcgCXMP4BoxTempDir->SelectedIndex;
    cnf->mux.disable_mkvext         = !fcgCBMKVMuxerExt->Checked;
    cnf->mux.mkv_mode               = fcgCXMKVCmdEx->SelectedIndex;
    cnf->mux.disable_mpgext         = !fcgCBMPGMuxerExt->Checked;
    cnf->mux.mpg_mode               = fcgCXMPGCmdEx->SelectedIndex;
    cnf->mux.priority               = fcgCXMuxPriority->SelectedIndex;

    cnf->oth.run_bat                = RUN_BAT_NONE;
    cnf->oth.run_bat               |= (fcgCBRunBatBeforeAudio->Checked) ? RUN_BAT_BEFORE_AUDIO   : NULL;
    cnf->oth.run_bat               |= (fcgCBRunBatAfterAudio->Checked)  ? RUN_BAT_AFTER_AUDIO    : NULL;
    cnf->oth.run_bat               |= (fcgCBRunBatBefore->Checked)      ? RUN_BAT_BEFORE_PROCESS : NULL;
    cnf->oth.run_bat               |= (fcgCBRunBatAfter->Checked)       ? RUN_BAT_AFTER_PROCESS  : NULL;
    cnf->oth.dont_wait_bat_fin      = RUN_BAT_NONE;
    cnf->oth.dont_wait_bat_fin     |= (!fcgCBWaitForBatBefore->Checked) ? RUN_BAT_BEFORE_PROCESS : NULL;
    cnf->oth.dont_wait_bat_fin     |= (!fcgCBWaitForBatAfter->Checked)  ? RUN_BAT_AFTER_PROCESS  : NULL;
    GetCHARfromString(cnf->oth.batfile.before_process, fcgTXBatBeforePath->Text);
    GetCHARfromString(cnf->oth.batfile.after_process,  fcgTXBatAfterPath->Text);
    GetCHARfromString(cnf->oth.batfile.before_audio,   fcgTXBatBeforeAudioPath->Text);
    GetCHARfromString(cnf->oth.batfile.after_audio,    fcgTXBatAfterAudioPath->Text);

    GetfcgTSLSettingsNotes(cnf->oth.notes, sizeof(cnf->oth.notes));

    //cli mode
    cnf->oth.disable_guicmd         = fcgTSBCMDOnly->Checked;
    if (cnf->oth.disable_guicmd)
        cnf->enc.nul_out           = fcgCBNulOutCLI->Checked;

    //制約条件として適用
    set_profile_to_conf(&cnf->enc, cnf->enc.profile);
}

System::Void frmConfig::GetfcgTSLSettingsNotes(char *notes, int nSize) {
    ZeroMemory(notes, nSize);
    if (fcgTSLSettingsNotes->Overflow != ToolStripItemOverflow::Never)
        GetCHARfromString(notes, nSize, fcgTSLSettingsNotes->Text);
}

System::Void frmConfig::SetfcgTSLSettingsNotes(const char *notes) {
    if (str_has_char(notes)) {
        fcgTSLSettingsNotes->ForeColor = Color::FromArgb(StgNotesColor[0][0], StgNotesColor[0][1], StgNotesColor[0][2]);
        fcgTSLSettingsNotes->Text = String(notes).ToString();
        fcgTSLSettingsNotes->Overflow = ToolStripItemOverflow::AsNeeded;
    } else {
        fcgTSLSettingsNotes->ForeColor = Color::FromArgb(StgNotesColor[1][0], StgNotesColor[1][1], StgNotesColor[1][2]);
        fcgTSLSettingsNotes->Text = LOAD_CLI_STRING(AuofcgTSTSettingsNotes);
        fcgTSLSettingsNotes->Overflow = ToolStripItemOverflow::Never;
    }
}

System::Void frmConfig::SetfcgTSLSettingsNotes(String^ notes) {
    if (notes->Length && fcgTSLSettingsNotes->Overflow != ToolStripItemOverflow::Never) {
        fcgTSLSettingsNotes->ForeColor = Color::FromArgb(StgNotesColor[0][0], StgNotesColor[0][1], StgNotesColor[0][2]);
        fcgTSLSettingsNotes->Text = notes;
        fcgTSLSettingsNotes->Overflow = ToolStripItemOverflow::AsNeeded;
    } else {
        fcgTSLSettingsNotes->ForeColor = Color::FromArgb(StgNotesColor[1][0], StgNotesColor[1][1], StgNotesColor[1][2]);
        fcgTSLSettingsNotes->Text = LOAD_CLI_STRING(AuofcgTSTSettingsNotes);
        fcgTSLSettingsNotes->Overflow = ToolStripItemOverflow::Never;
    }
}

System::Void frmConfig::SetChangedEvent(Control^ control, System::EventHandler^ _event) {
    System::Type^ ControlType = control->GetType();
    if (ControlType == NumericUpDown::typeid)
        ((NumericUpDown^)control)->ValueChanged += _event;
    else if (ControlType == ComboBox::typeid)
        ((ComboBox^)control)->SelectedIndexChanged += _event;
    else if (ControlType == CheckBox::typeid)
        ((CheckBox^)control)->CheckedChanged += _event;
    else if (ControlType == TextBox::typeid)
        ((TextBox^)control)->TextChanged += _event;
}

System::Void frmConfig::SetToolStripEvents(ToolStrip^ TS, System::Windows::Forms::MouseEventHandler^ _event) {
    for (int i = 0; i < TS->Items->Count; i++) {
        ToolStripButton^ TSB = dynamic_cast<ToolStripButton^>(TS->Items[i]);
        if (TSB != nullptr) TSB->MouseDown += _event;
    }
}

System::Void frmConfig::TabControl_DarkDrawItem(System::Object^ sender, DrawItemEventArgs^ e) {
    //対象のTabControlを取得
    TabControl^ tab = dynamic_cast<TabControl^>(sender);
    //タブページのテキストを取得
    System::String^ txt = tab->TabPages[e->Index]->Text;

    //タブのテキストと背景を描画するためのブラシを決定する
    SolidBrush^ foreBrush = gcnew System::Drawing::SolidBrush(ColorfromInt(DEFAULT_UI_COLOR_TEXT_DARK));
    SolidBrush^ backBrush = gcnew System::Drawing::SolidBrush(ColorfromInt(DEFAULT_UI_COLOR_BASE_DARK));

    //StringFormatを作成
    StringFormat^ sf = gcnew System::Drawing::StringFormat();
    //中央に表示する
    sf->Alignment = StringAlignment::Center;
    sf->LineAlignment = StringAlignment::Center;

    //背景の描画
    e->Graphics->FillRectangle(backBrush, e->Bounds);
    //Textの描画
    e->Graphics->DrawString(txt, e->Font, foreBrush, e->Bounds, sf);
}

System::Void frmConfig::fcgMouseEnter_SetColor(System::Object^  sender, System::EventArgs^  e) {
    fcgMouseEnterLeave_SetColor(sender, themeMode, DarkenWindowState::Hot, dwStgReader);
}
System::Void frmConfig::fcgMouseLeave_SetColor(System::Object^  sender, System::EventArgs^  e) {
    fcgMouseEnterLeave_SetColor(sender, themeMode, DarkenWindowState::Normal, dwStgReader);
}

System::Void frmConfig::SetAllMouseMove(Control ^top, const AuoTheme themeTo) {
    if (themeTo == themeMode) return;
    System::Type^ type = top->GetType();
    if (type == CheckBox::typeid /* || isToolStripItem(type)*/) {
        top->MouseEnter += gcnew System::EventHandler(this, &frmConfig::fcgMouseEnter_SetColor);
        top->MouseLeave += gcnew System::EventHandler(this, &frmConfig::fcgMouseLeave_SetColor);
    } else if (type == ToolStrip::typeid) {
        ToolStrip^ TS = dynamic_cast<ToolStrip^>(top);
        for (int i = 0; i < TS->Items->Count; i++) {
            auto item = TS->Items[i];
            item->MouseEnter += gcnew System::EventHandler(this, &frmConfig::fcgMouseEnter_SetColor);
            item->MouseLeave += gcnew System::EventHandler(this, &frmConfig::fcgMouseLeave_SetColor);
        }
    }
    for (int i = 0; i < top->Controls->Count; i++) {
        SetAllMouseMove(top->Controls[i], themeTo);
    }
}

System::Void frmConfig::CheckTheme() {
    //DarkenWindowが使用されていれば設定をロードする
    if (dwStgReader != nullptr) delete dwStgReader;
    const auto [themeTo, dwStg] = check_current_theme(sys_dat->aviutl_dir);
    dwStgReader = dwStg;

    //変更の必要がなければ終了
    if (themeTo == themeMode) return;

    //一度ウィンドウの再描画を完全に抑止する
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 0, 0);
#if 1
    //tabcontrolのborderを隠す
    SwitchComboBoxBorder(fcgtabControlVideo, fcgPNHideTabControlVideo, themeMode, themeTo, dwStgReader);
    SwitchComboBoxBorder(fcgtabControlAudio, fcgPNHideTabControlAudio, themeMode, themeTo, dwStgReader);
    SwitchComboBoxBorder(fcgtabControlMux,   fcgPNHideTabControlMux,   themeMode, themeTo, dwStgReader);
    //上部のtoolstripborderを隠すためのパネル
    fcgPNHideToolStripBorder->Visible = themeTo == AuoTheme::DarkenWindowDark;
#endif
#if 0
    //TabControlをオーナードローする
    fcgtabControlVideo->DrawMode = TabDrawMode::OwnerDrawFixed;
    fcgtabControlVideo->DrawItem += gcnew DrawItemEventHandler(this, &frmConfig::TabControl_DarkDrawItem);

    fcgtabControlAudio->DrawMode = TabDrawMode::OwnerDrawFixed;
    fcgtabControlAudio->DrawItem += gcnew DrawItemEventHandler(this, &frmConfig::TabControl_DarkDrawItem);

    fcgtabControlMux->DrawMode = TabDrawMode::OwnerDrawFixed;
    fcgtabControlMux->DrawItem += gcnew DrawItemEventHandler(this, &frmConfig::TabControl_DarkDrawItem);
#endif
    if (themeTo != themeMode) {
        SetAllColor(this, themeTo, this->GetType(), dwStgReader);
        SetAllMouseMove(this, themeTo);
    }
    //一度ウィンドウの再描画を再開し、強制的に再描画させる
    SendMessage(reinterpret_cast<HWND>(this->Handle.ToPointer()), WM_SETREDRAW, 1, 0);
    this->Refresh();
    themeMode = themeTo;
}

System::Void frmConfig::SetAllCheckChangedEvents(Control ^top) {
    //再帰を使用してすべてのコントロールのtagを調べ、イベントをセットする
    for (int i = 0; i < top->Controls->Count; i++) {
        System::Type^ type = top->Controls[i]->GetType();
        if (type == NumericUpDown::typeid)
            top->Controls[i]->Enter += gcnew System::EventHandler(this, &frmConfig::NUSelectAll);

        if (type == Label::typeid || type == Button::typeid)
            ;
        else if (type == ToolStrip::typeid)
            SetToolStripEvents((ToolStrip^)(top->Controls[i]), gcnew System::Windows::Forms::MouseEventHandler(this, &frmConfig::fcgTSItem_MouseDown));
        else if (top->Controls[i]->Tag == nullptr)
            SetAllCheckChangedEvents(top->Controls[i]);
        else if (String::Equals(top->Controls[i]->Tag->ToString(), L"reCmd"))
            SetChangedEvent(top->Controls[i], gcnew System::EventHandler(this, &frmConfig::fcgRebuildCmd));
        else if (String::Equals(top->Controls[i]->Tag->ToString(), L"chValue"))
            SetChangedEvent(top->Controls[i], gcnew System::EventHandler(this, &frmConfig::CheckOtherChanges));
        else
            SetAllCheckChangedEvents(top->Controls[i]);
    }
}

System::Void frmConfig::SetHelpToolTipsColorMatrix(Control^ control, const char *type) {
    const ENC_OPTION_STR *list = get_option_list(type);
    fcgTTX264->SetToolTip(control,      L"--" + String(type).ToString() + L"\n"
        + LOAD_CLI_STRING(AuofrmTTColorMatrix1) + L"\n"
        + LOAD_CLI_STRING(AuofrmTTColorMatrix2) + L"\n"
        + LOAD_CLI_STRING(AuofrmTTColorMatrix3) + L" " + COLOR_MATRIX_THRESHOLD + L" " + LOAD_CLI_STRING(AuofrmTTColorMatrix4) + L" … " + String(list[COLOR_MATRIX_HD].desc).ToString() + L"\n"
        + LOAD_CLI_STRING(AuofrmTTColorMatrix3) + L" " + COLOR_MATRIX_THRESHOLD + L" " + LOAD_CLI_STRING(AuofrmTTColorMatrix5) + L" … " + String(list[COLOR_MATRIX_SD].desc).ToString() + L"\n"
        + LOAD_CLI_STRING(AuofrmTTColorMatrix6)
        );
}

System::Void frmConfig::SetHelpToolTips() {
    //x264基本
#define SET_TOOL_TIP_X264(target, x) { fcgTTX264->SetToolTip(target, LOAD_CLI_STRING(AuofrmTT ## x)); }
#define SET_TOOL_TIP_EX2(target, x) { fcgTTEx->SetToolTip(target, LOAD_CLI_STRING(AuofrmTT ## x)); }
#define SET_TOOL_TIP_EX(target) { fcgTTEx->SetToolTip(target, LOAD_CLI_STRING(AuofrmTT ## target)); }

    SET_TOOL_TIP_X264(fcgCBUsehighbit,  fcgCBUsehighbit);
    SET_TOOL_TIP_X264(fcgBTX264Path,    fcgBTX264Path);
    SET_TOOL_TIP_X264(fcgBTX264PathSub, fcgBTX264Path);

    fcgTTX264->SetToolTip(fcgCXX264Mode, L""
        + L"【" + LOAD_CLI_STRING(AuofrmTTfcgCXX264ModeSinglePath) +  L"】\n"
        + L"   " + LOAD_CLI_STRING(x264_encodemode_desc[2].mes)->Replace(LOAD_CLI_STRING(AuofrmTTfcgCXX264ModeSinglePath) + L" - ", L"") + L"\t … --crf\n"
        + L"   " + LOAD_CLI_STRING(x264_encodemode_desc[1].mes)->Replace(LOAD_CLI_STRING(AuofrmTTfcgCXX264ModeSinglePath) + L" - ", L"") + L"\t\t … --qp\n"
        + L"   " + LOAD_CLI_STRING(x264_encodemode_desc[0].mes)->Replace(LOAD_CLI_STRING(AuofrmTTfcgCXX264ModeSinglePath) + L" - ", L"") + L"\t … --bitrate\n"
        + L"\n"
        + L"【" + LOAD_CLI_STRING(AuofrmTTfcgCXX264ModeMultiPath) + L"】\n"
        + L"   " + LOAD_CLI_STRING(x264_encodemode_desc[3].mes)->Replace(LOAD_CLI_STRING(AuofrmTTfcgCXX264ModeMultiPath) + L" - ", L"") + L"\t … --pass 1 --bitrate\n"
        + L"   " + LOAD_CLI_STRING(x264_encodemode_desc[4].mes)->Replace(LOAD_CLI_STRING(AuofrmTTfcgCXX264ModeMultiPath) + L" - ", L"") + L"\t … --pass 3 --bitrate\n"
        + L"\n"
        + L"【" + LOAD_CLI_STRING(x264_encodemode_desc[5].mes) + L"】\n"
        + L"    " + LOAD_CLI_STRING(AuofrmTTfcgCXX264ModeAutoMultiPath) + L"\n"
        + L"    --pass 1/3 --bitrate\n"
        + L"\n"
        + L"【" + LOAD_CLI_STRING(x264_encodemode_desc[6].mes) + L"】\n"
        + L"    " + LOAD_CLI_STRING(AuofrmTTfcgCXX264ModeCrfWithCheck) + L"\n"
        + L"    --crf"
        );
    fcgTTX264->SetToolTip(fcgCBNulOut,            L"-o nul");
    fcgTTX264->SetToolTip(fcgCBFastFirstPass,     L"--slow-firstpass (" + LOAD_CLI_STRING(AuofrmTTWhenCheckOff) + L")");
    fcgTTX264->SetToolTip(fcgTXQuality,           L"--crf / --bitrate / --qp");
    fcgTTX264->SetToolTip(fcgTXStatusFile,        L"--stats");
    fcgTTX264->SetToolTip(fcgCXProfile,           L"--profile\n"
        + L"\n"
        + LOAD_CLI_STRING(AuofrmTTfcgCXProfile)
        );

    //自動マルチパス 上限設定
    SET_TOOL_TIP_EX2(fcgCBAMPLimitBitrateUpper, fcgAMPLimitBitrateUpper);
    SET_TOOL_TIP_EX2(fcgNUAMPLimitBitrateUpper, fcgAMPLimitBitrateUpper);
    SET_TOOL_TIP_EX2(fcgCBAMPLimitBitrateLower, fcgAMPLimitBitrateLower);
    SET_TOOL_TIP_EX2(fcgNUAMPLimitBitrateLower, fcgAMPLimitBitrateLower);
    SET_TOOL_TIP_EX2(fcgCBAMPLimitFileSize,     fcgAMPLimitFileSize);
    SET_TOOL_TIP_EX2(fcgNUAMPLimitFileSize,     fcgAMPLimitFileSize);

    //プロファイルとか
    fcgTTX264->SetToolTip(fcgCXTune,              L"--tune");
    fcgTTX264->SetToolTip(fcgCXPreset,            L"--preset");
    SET_TOOL_TIP_EX(fcgBTApplyPreset);

    //出力・フォーマット
    fcgTTX264->SetToolTip(fcgCXLevel,            L"--level");
    fcgTTX264->SetToolTip(fcgCXVideoFormat,      L"--videoformat");
    fcgTTX264->SetToolTip(fcgCBAud,              L"--aud");
    fcgTTX264->SetToolTip(fcgCBPicStruct,        L"--pic-struct");
    fcgTTX264->SetToolTip(fcgCXNalHrd,           L"--nal-hrd\n"
        + LOAD_CLI_STRING(AuofrmTTfcgCXNalHrd)
        );
    fcgTTX264->SetToolTip(fcgCBBlurayCompat,     L"--bluray-compat");
    fcgTTX264->SetToolTip(fcgCXOutputCsp,        L"--output-csp\n"
        + LOAD_CLI_STRING(AuofrmTTfcgCXOutputCsp)
        );

    //色空間
    SetHelpToolTipsColorMatrix(fcgCXColorMatrix, "colormatrix");
    SetHelpToolTipsColorMatrix(fcgCXColorPrim,   "colorprim");
    SetHelpToolTipsColorMatrix(fcgCXTransfer,    "transfer");
    fcgTTX264->SetToolTip(fcgCXInputRange,      L"--input-range\n"
        + L"\n"
        + L"\"" + String(list_input_range[0].desc).ToString() + L"\"  [" + LOAD_CLI_STRING(AUO_GUIEX_DEFAULT) + L"]\n"
        + L"  output-csp i4xx … " + LOAD_CLI_STRING(AuofrmTTfcgCXInputRangeYUVLimited) + L"\n"
        + L"  output-csp rgb  … " + LOAD_CLI_STRING(AuofrmTTfcgCXInputRangeRGB) + L"\n"
        + L"\n"
        + L"\"" + String(list_input_range[1].desc).ToString() + L"\"\n"
        + LOAD_CLI_STRING(AuofrmTTfcgCXInputRangeYUVFull)
        );

    fcgTTX264->SetToolTip(fcgCXAspectRatio,      L""
        + LOAD_CLI_STRING(aspect_desc[0].mes) + L"\n"
        + L"   " + LOAD_CLI_STRING(AuofrmTTfcgCXAspectRatioSAR) + L"\n"
        + L"\n"
        + LOAD_CLI_STRING(aspect_desc[1].mes) + L"\n"
        + L"   " + LOAD_CLI_STRING(AuofrmTTfcgCXAspectRatioDAR) + L"\n"
        );
    SET_TOOL_TIP_X264(fcgNUAspectRatioX, fcgNUAspectRatioX);
    SET_TOOL_TIP_X264(fcgNUAspectRatioY, fcgNUAspectRatioY);
    fcgTTX264->SetToolTip(fcgNUThreads,          L"--threads\n"
        + LOAD_CLI_STRING(AuofrmTTZeroAsAuto)
        );
    fcgTTX264->SetToolTip(fcgNULookaheadThreads, L"--lookahead-threads\n"
        + LOAD_CLI_STRING(AuofrmTTZeroAsAuto)
        );
    fcgTTX264->SetToolTip(fcgCBSlicedThreads,    L"--sliced-threads");
    fcgTTX264->SetToolTip(fcgCXLogLevel,         L"--log-level");
    fcgTTX264->SetToolTip(fcgCBPSNR,             L"--psnr");
    fcgTTX264->SetToolTip(fcgCBSSIM,             L"--ssim");

    //量子化
    fcgTTX264->SetToolTip(fcgNUIPRatio,          L"--ipratio 1.00 + (" + LOAD_CLI_STRING(AuofrmTTValueSet) + L")%");
    fcgTTX264->SetToolTip(fcgNUPBRatio,          L"--pbratio 1.00 + (" + LOAD_CLI_STRING(AuofrmTTValueSet) + L")%");
    fcgTTX264->SetToolTip(fcgNUQcomp,            L"--qcomp (" + LOAD_CLI_STRING(AuofrmTTValueSet) + L")%");
    fcgTTX264->SetToolTip(fcgNUQpmin,            L"--qpmin");
    fcgTTX264->SetToolTip(fcgNUQpmax,            L"--qpmax");
    fcgTTX264->SetToolTip(fcgNUQpstep,           L"--qpstep");
    fcgTTX264->SetToolTip(fcgNUChromaQp,         L"--chroma-qp-offset");

    //AQ
    fcgTTX264->SetToolTip(fcgCXAQMode,           L"--aq-mode");
    fcgTTX264->SetToolTip(fcgNUAQStrength,       L"--aq-strength");

    //PsyRD
    fcgTTX264->SetToolTip(fcgNUPsyRDO,           L"--psy-rd <RDO>:<trellis>");
    fcgTTX264->SetToolTip(fcgNUPsyTrellis,       L"--psy-rd <RDO>:<trellis>");
    fcgTTX264->SetToolTip(fcgCBMBTree,           L""
        + L"--mbtree\n"
        + LOAD_CLI_STRING(AuofrmTTWhenCheckOff) + L" --no-mbtree"
        );
    fcgTTX264->SetToolTip(fcgNURCLookahead,      L"--rc-lookahead");
    fcgTTX264->SetToolTip(fcgNUVBVmax,           L""
        + L"--vbv-maxrate\n"
        + LOAD_CLI_STRING(AuofrmTTMinusOneAsAuto)
        );
    fcgTTX264->SetToolTip(fcgNUVBVbuf,           L""
        + L"--vbv-bufsize\n"
        + LOAD_CLI_STRING(AuofrmTTMinusOneAsAuto)
        );
    fcgTTX264->SetToolTip(fcgNUScenecut,         L"--scenecut");
    SET_TOOL_TIP_X264(fcgNUKeyint, fcgNUKeyint);
    fcgTTX264->SetToolTip(fcgNUMinKeyint,        L""
         + L"--min-keyint\n"
         + LOAD_CLI_STRING(AuofrmTTZeroAsAuto)
         );
    fcgTTX264->SetToolTip(fcgCBOpenGOP,          L"--open-gop");
    fcgTTX264->SetToolTip(fcgCBCABAC,            L""
        + L"--cabac\n"
        + LOAD_CLI_STRING(AuofrmTTWhenCheckOff) + L" --no-cabac");
    fcgTTX264->SetToolTip(fcgNUSlices,           L"--slices");
    fcgTTX264->SetToolTip(fcgCXWeightP,          L"--weightp");

    //インタレ
    fcgTTX264->SetToolTip(fcgCXInterlaced,       L""
        + L"[" + LOAD_CLI_STRING(interlaced_desc[0].mes) + L"]\n"
        + LOAD_CLI_STRING(AuofrmTTfcgCXInterlacedProgressive) + L"\n"
        + L"\n"
        + L"[" + LOAD_CLI_STRING(interlaced_desc[1].mes) + L"] … --tff\n"
        + LOAD_CLI_STRING(AuofrmTTfcgCXInterlacedTFF) + L"\n"
        + L"\n"
        + L"[" + LOAD_CLI_STRING(interlaced_desc[2].mes) + L"] … --bff\n"
        + LOAD_CLI_STRING(AuofrmTTfcgCXInterlacedBFF)
        );

    fcgTTX264->SetToolTip(fcgCBDeblock,          L"--deblock <Strength>:<Threshold>\n"
        + LOAD_CLI_STRING(AuofrmTTWhenCheckOff) + L" --no-deblock"
        );
    fcgTTX264->SetToolTip(fcgNUDeblockStrength,  L"--deblock <Strength>:<Threshold>"
        );
    fcgTTX264->SetToolTip(fcgNUDeblockThreshold, L"--deblock <Strength>:<Threshold>"
        );

    //Bフレーム
    fcgTTX264->SetToolTip(fcgNUBframes,          L"--bframes");
    fcgTTX264->SetToolTip(fcgCXBAdpapt,          L"--b-adapt");
    fcgTTX264->SetToolTip(fcgNUBBias,            L"--b-bias");
    fcgTTX264->SetToolTip(fcgCXBpyramid,         L"--b-pyramid");
    fcgTTX264->SetToolTip(fcgCBWeightB,          L"--weightb");

    //マクロブロックタイプ
    fcgTTX264->SetToolTip(fcgCB8x8dct,           L"--8x8dct");
    fcgTTX264->SetToolTip(fcgCBp8x8,             L"--partitions p8x8");
    fcgTTX264->SetToolTip(fcgCBb8x8,             L"--partitions b8x8");
    fcgTTX264->SetToolTip(fcgCBp4x4,             L"--partitions p4x4");
    fcgTTX264->SetToolTip(fcgCBi8x8,             L"--partitions i8x8");
    fcgTTX264->SetToolTip(fcgCBi4x4,             L"--partitions i4x4");

    //その他
    fcgTTX264->SetToolTip(fcgCBfastpskip,        LOAD_CLI_STRING(AuofrmTTWhenCheckOff) + L" --no-fast-pskip");
    fcgTTX264->SetToolTip(fcgCBDctDecimate,      LOAD_CLI_STRING(AuofrmTTWhenCheckOff) + L" --no-dct-decimate");
    fcgTTX264->SetToolTip(fcgCXTrellis,          L"--trellis");
    fcgTTX264->SetToolTip(fcgTXCQM,              L"--cqm / --cqmfile");
    fcgTTX264->SetToolTip(fcgBTMatrix,           L"--cqm / --cqmfile");

    //動き予測
    fcgTTX264->SetToolTip(fcgCXME,               L"--me");
    fcgTTX264->SetToolTip(fcgCXSubME,            L"--subme");
    fcgTTX264->SetToolTip(fcgNUMERange,          L"--merange");
    fcgTTX264->SetToolTip(fcgCBChromaME,         L""
        + L"--chroma-me\n"
        + LOAD_CLI_STRING(AuofrmTTWhenCheckOff) + L" --no-chroma-me"
        );
    fcgTTX264->SetToolTip(fcgCXDirectME,         L"--direct");
    fcgTTX264->SetToolTip(fcgNURef,              L"--ref");
    fcgTTX264->SetToolTip(fcgCBMixedRef,         L"--mixed-ref");

    //時間
    SET_TOOL_TIP_X264(fcgCBTCIN, fcgTCIN);
    SET_TOOL_TIP_X264(fcgTXTCIN, fcgTCIN);
    fcgTTX264->SetToolTip(fcgCBTimeBase,         L"--timebase");
    fcgTTX264->SetToolTip(fcgNUTimebaseDen,      L"--timebase");
    fcgTTX264->SetToolTip(fcgNUTimebaseNum,      L"--timebase");

    //拡張
    SET_TOOL_TIP_EX(fcgCBAFS);
    SET_TOOL_TIP_EX(fcgCBAFSBitrateCorrection);
    SET_TOOL_TIP_EX(fcgCBAuoTcfileout);
    SET_TOOL_TIP_EX(fcgCBCheckKeyframes);
    SET_TOOL_TIP_EX(fcgCBSetKeyframeAtChapter);
    SET_TOOL_TIP_EX(fcgCBInputAsLW48);
    SET_TOOL_TIP_EX(fcgCXTempDir);
    SET_TOOL_TIP_EX(fcgBTCustomTempDir);
    SET_TOOL_TIP_EX(fcgBTCmdEx);
    //音声
    SET_TOOL_TIP_EX(fcgCXAudioEncoder);
    SET_TOOL_TIP_EX(fcgCBAudioOnly);
    SET_TOOL_TIP_EX(fcgCBFAWCheck);
    SET_TOOL_TIP_EX(fcgBTAudioEncoderPath);
    SET_TOOL_TIP_EX(fcgCXAudioEncMode);
    SET_TOOL_TIP_EX(fcgCBAudio2pass);
    SET_TOOL_TIP_EX(fcgCBAudioUsePipe);
    SET_TOOL_TIP_EX(fcgNUAudioBitrate);
    SET_TOOL_TIP_EX(fcgCXAudioPriority);
    SET_TOOL_TIP_EX(fcgCXAudioEncTiming);
    SET_TOOL_TIP_EX(fcgCXAudioTempDir);
    SET_TOOL_TIP_EX(fcgBTCustomAudioTempDir);
    //音声バッチファイル実行
    SET_TOOL_TIP_EX(fcgCBRunBatBeforeAudio);
    SET_TOOL_TIP_EX(fcgCBRunBatAfterAudio);
    SET_TOOL_TIP_EX(fcgBTBatBeforeAudioPath);
    SET_TOOL_TIP_EX(fcgBTBatAfterAudioPath);
    //muxer
    SET_TOOL_TIP_EX(fcgCBMP4MuxerExt);
    SET_TOOL_TIP_EX(fcgCXMP4CmdEx);
    SET_TOOL_TIP_EX(fcgBTMP4MuxerPath);
    SET_TOOL_TIP_EX(fcgBTTC2MP4Path);
    SET_TOOL_TIP_EX(fcgBTMP4RawPath);
    SET_TOOL_TIP_EX(fcgCXMP4BoxTempDir);
    SET_TOOL_TIP_EX(fcgBTMP4BoxTempDir);
    SET_TOOL_TIP_EX(fcgCBMKVMuxerExt);
    SET_TOOL_TIP_EX(fcgCXMKVCmdEx);
    SET_TOOL_TIP_EX(fcgBTMKVMuxerPath);
    SET_TOOL_TIP_EX(fcgCBMPGMuxerExt);
    SET_TOOL_TIP_EX(fcgCXMPGCmdEx);
    SET_TOOL_TIP_EX(fcgBTMPGMuxerPath);
    SET_TOOL_TIP_EX(fcgCXMuxPriority);
    //バッチファイル実行
    SET_TOOL_TIP_EX(fcgCBRunBatBefore);
    SET_TOOL_TIP_EX(fcgCBRunBatAfter);
    SET_TOOL_TIP_EX(fcgCBWaitForBatBefore);
    SET_TOOL_TIP_EX(fcgCBWaitForBatAfter);
    SET_TOOL_TIP_EX(fcgBTBatBeforePath);
    SET_TOOL_TIP_EX(fcgBTBatAfterPath);
    //他
    SET_TOOL_TIP_EX(fcgTXCmd);
    SET_TOOL_TIP_EX(fcgBTDefault);
    //上部ツールストリップ
    fcgTSBCMDOnly->ToolTipText = LOAD_CLI_STRING(AuofrmTTfcgTSBCMDOnly);
    fcgTSBDelete->ToolTipText = LOAD_CLI_STRING(AuofrmTTfcgTSBDelete);
    fcgTSBOtherSettings->ToolTipText = LOAD_CLI_STRING(AuofrmTTfcgTSBOtherSettings);
    fcgTSBSave->ToolTipText = LOAD_CLI_STRING(AuofrmTTfcgTSBSave);
    fcgTSBSaveNew->ToolTipText = LOAD_CLI_STRING(AuofrmTTfcgTSBSaveNew);
}
System::Void frmConfig::SetX264VersionToolTip(String^ x264Path) {
    String^ mes;
    if (File::Exists(x264Path)) {
        char mes_buf[2560];
        char exe_path[MAX_PATH_LEN];
        GetCHARfromString(exe_path, sizeof(exe_path), x264Path);
        if (get_exe_message(exe_path, "--version", mes_buf, _countof(mes_buf), AUO_PIPE_MUXED) == RP_SUCCESS)
            mes = String(mes_buf).ToString();
        else
            mes = LOAD_CLI_STRING(AUO_CONFIG_ERR_GET_EXE_VER);
    } else {
        mes = LOAD_CLI_STRING(AUO_CONFIG_ERR_EXE_NOT_FOUND);
    }
    fcgTTX264Version->ToolTipTitle = LOAD_CLI_STRING(AuofrmTTfcgTTX264Version);
    fcgTTX264Version->SetToolTip(fcgTXX264Path, mes);
    fcgTTX264Version->SetToolTip(fcgTXX264PathSub, mes);
}
System::Void frmConfig::ShowExehelp(String^ ExePath, String^ args) {
    if (!File::Exists(ExePath)) {
        MessageBox::Show(LOAD_CLI_STRING(AUO_CONFIG_ERR_EXE_NOT_FOUND), LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OK, MessageBoxIcon::Error);
    } else {
        char exe_path[MAX_PATH_LEN];
        char file_path[MAX_PATH_LEN];
        char cmd[MAX_CMD_LEN];
        GetCHARfromString(exe_path, sizeof(exe_path), ExePath);
        apply_appendix(file_path, _countof(file_path), exe_path, "_fullhelp.txt");
        File::Delete(String(file_path).ToString());
        array<String^>^ arg_list = args->Split(L';');
        for (int i = 0; i < arg_list->Length; i++) {
            if (i) {
                StreamWriter^ sw;
                try {
                    sw = gcnew StreamWriter(String(file_path).ToString(), true, System::Text::Encoding::GetEncoding("shift_jis"));
                    sw->WriteLine();
                    sw->WriteLine();
                } catch (...) {
                    //ファイルオープンに失敗…初回のget_exe_message_to_fileでエラーとなるため、おそらく起こらない
                } finally {
                    if (sw != nullptr) { sw->Close(); }
                }
            }
            GetCHARfromString(cmd, sizeof(cmd), arg_list[i]);
            if (get_exe_message_to_file(exe_path, cmd, file_path, AUO_PIPE_MUXED, 5) != RP_SUCCESS) {
                File::Delete(String(file_path).ToString());
                MessageBox::Show(LOAD_CLI_STRING(AUO_CONFIG_ERR_GET_HELP), LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }
        }
        try {
            System::Diagnostics::Process::Start(String(file_path).ToString());
        } catch (...) {
            MessageBox::Show(LOAD_CLI_STRING(AUO_CONFIG_ERR_OPEN_HELP), LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }
}

#pragma warning( pop )
