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
        MessageBox::Show(L"ファイル名に使用できない文字が含まれています。\n保存できません。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
        return false;
    }
    if (String::Compare(Path::GetExtension(stgName), L".stg", true))
        stgName += L".stg";
    if (File::Exists(fileName = Path::Combine(fsnCXFolderBrowser->GetSelectedFolder(), stgName)))
        if (MessageBox::Show(stgName + L" はすでに存在します。上書きしますか?", L"上書き確認", MessageBoxButtons::YesNo, MessageBoxIcon::Question)
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
System::Void frmBitrateCalculator::Init(int VideoBitrate, int AudioBitrate, bool BTVBEnable, bool BTABEnable, int ab_max) {
    guiEx_settings exStg(true);
    exStg.load_fbc();
    enable_events = false;
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
            (int)fcgNUAudioBitrate->Maximum
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
///     frmUpdate 関数
/// -------------------------------------------------
System::Void frmConfig::fcgTSBUpdate_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
#if ENABLE_AUOSETUP
    if (fcgTSBUpdate->Checked)
        frmExeUpdate->Show();
    else
        frmExeUpdate->Visible = false; //Close()してしまうとfrmExeUpdateがDisposeされてしまう
#endif
}

System::Void frmConfig::fcgTSBUpdate_CheckFinished(String^ mes) {
#if ENABLE_AUOSETUP
    //更新をチェックした時間を保存する
    if (this->InvokeRequired) {
        this->Invoke(gcnew x264guiEx::DelegateProcessFin(this, &frmConfig::fcgTSBUpdate_CheckFinished), mes);
        return;
    }

    guiEx_settings stg;
    stg.load_encode_stg();
    GetCHARfromString(stg.s_local.last_update_check, _countof(stg.s_local.last_update_check), DateTime::Now.ToString());
    stg.save_local();

    if (auoSetupControl::checkIfUpdateAvailable(mes)) {
        fcgTSBUpdate->Text = L"更新*";
        fcgTSBUpdate->ForeColor = Color::MediumSlateBlue;
    }
#endif
}

System::Void frmConfig::fcgTSBUpdate_UpdateFinished(String^ mes) {
#if ENABLE_AUOSETUP
    if (this->InvokeRequired) {
        this->Invoke(gcnew x264guiEx::DelegateProcessFin(this, &frmConfig::fcgTSBUpdate_UpdateFinished), mes);
        return;
    }

    fcgTSBUpdate->Text = L"更新";
    fcgTSBUpdate->ForeColor = SystemColors::ControlText;

    //更新にしたがって表示を更新する
    guiEx_settings stg;
    stg.load_encode_stg();
    LocalStg.x264Path        = String(stg.s_x264.fullpath).ToString();
    LocalStg.MP4MuxerPath    = String(stg.s_mux[MUXER_MP4].fullpath).ToString();
    LocalStg.TC2MP4Path      = String(stg.s_mux[MUXER_TC2MP4].fullpath).ToString();
    LocalStg.MP4RawPath      = String(stg.s_mux[MUXER_MP4_RAW].fullpath).ToString();
    LocalStg.audEncPath[DEFAULT_AUDIO_ENCODER] = String(stg.s_aud[DEFAULT_AUDIO_ENCODER].fullpath).ToString();
    LocalStg.audEncPath[DEFAULT_AUDIO_ENCODER+1] = String(stg.s_aud[DEFAULT_AUDIO_ENCODER+1].fullpath).ToString();

    SetLocalStg();
    if (fcgCXAudioEncoder->SelectedIndex == DEFAULT_AUDIO_ENCODER
        || fcgCXAudioEncoder->SelectedIndex == (DEFAULT_AUDIO_ENCODER+1)) {
        fcgTXAudioEncoderPath->Text = LocalStg.audEncPath[fcgCXAudioEncoder->SelectedIndex];
    }
#endif
}

System::Void frmConfig::InformfruClosed() {
    fcgTSBUpdate->Checked = false;
}

#if ENABLE_AUOSETUP
System::Void frmUpdate::frmUpdate_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
    e->Cancel = true;
    this->Visible = false; //Close()してしまうとfrmExeUpdateがDisposeされてしまう
    frmConfig^ fcg = dynamic_cast<frmConfig^>(this->Owner);
    if (fcg != nullptr)
        fcg->InformfruClosed();
}
#endif

/// -------------------------------------------------
///     frmConfig 関数
/// -------------------------------------------------
/////////////   LocalStg関連  //////////////////////
System::Void frmConfig::LoadLocalStg() {
    guiEx_settings *_ex_stg = sys_dat->exstg;
    _ex_stg->load_encode_stg();
    LocalStg.x264ExeName     = String(_ex_stg->s_x264.filename).ToString();
    LocalStg.x264Path        = String(_ex_stg->s_x264.fullpath).ToString();
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
    if (!File::Exists(LocalStg.x264Path)) {
        error = true;
        err += L"指定された x264 は存在しません。\n [ " + LocalStg.x264Path + L" ]\n";
    }
    //音声エンコーダのチェック (実行ファイル名がない場合はチェックしない)
    if (LocalStg.audEncExeName[fcgCXAudioEncoder->SelectedIndex]->Length) {
        String^ AudioEncoderPath = LocalStg.audEncPath[fcgCXAudioEncoder->SelectedIndex];
        if (!File::Exists(AudioEncoderPath) 
            && (fcgCXAudioEncoder->SelectedIndex != sys_dat->exstg->s_aud_faw_index 
                || !check_if_faw2aac_exists()) ) {
            //音声実行ファイルがない かつ
            //選択された音声がfawでない または fawであってもfaw2aacがない
            if (error) err += L"\n\n";
            error = true;
            err += L"指定された 音声エンコーダ は存在しません。\n [ " + AudioEncoderPath + L" ]\n";
        }
    }
    //FAWのチェック
    if (fcgCBFAWCheck->Checked) {
        if (sys_dat->exstg->s_aud_faw_index == FAW_INDEX_ERROR) {
            if (error) err += L"\n\n";
            error = true;
            err += L"FAWCheckが選択されましたが、x264guiEx.ini から\n"
                + L"FAW の設定を読み込めませんでした。\n"
                + L"x264guiEx.ini を確認してください。\n";
        } else if (!File::Exists(LocalStg.audEncPath[sys_dat->exstg->s_aud_faw_index])
                   && !check_if_faw2aac_exists()) {
            //fawの実行ファイルが存在しない かつ faw2aacも存在しない
            if (error) err += L"\n\n";
            error = true;
            err += L"FAWCheckが選択されましたが、FAW(fawcl)へのパスが正しく指定されていません。\n"
                +  L"一度設定画面でFAW(fawcl)へのパスを指定してください。\n";
        }
    }
    //自動マルチパスの自動ビットレート設定のチェック
    if (fcgLBAMPAutoBitrate != nullptr && fcgLBAMPAutoBitrate->Visible) {
        if (error) err += L"\n\n";
        error = true;
        err += L"目標映像ビットレートを自動に設定するには、\n"
            + L"上限ビットレート、上限ファイルサイズの少なくとも片方を\n"
            + L"適切に設定する必要があります。\n"
            + L"上限ビットレート、上限ファイルサイズの設定を見なおしてください。";
    }
    if (error) 
        MessageBox::Show(this, err, L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
    return error;
}

System::Void frmConfig::SaveLocalStg() {
    guiEx_settings *_ex_stg = sys_dat->exstg;
    _ex_stg->load_encode_stg();
    _ex_stg->s_local.large_cmdbox = fcgTXCmd->Multiline;
    GetCHARfromString(_ex_stg->s_x264.fullpath,               sizeof(_ex_stg->s_x264.fullpath),               LocalStg.x264Path);
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
    fcgLBX264Path->Text           = L"x264.exe の指定";
    fcgLBX264PathSub->Text        = L"x264.exe の指定";
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
    fcgLBMP4MuxerPath->Text       = LocalStg.MP4MuxerExeName + L" の指定";
    fcgLBMKVMuxerPath->Text       = LocalStg.MKVMuxerExeName + L" の指定";
    fcgLBTC2MP4Path->Text         = LocalStg.TC2MP4ExeName   + L" の指定";
    fcgLBMPGMuxerPath->Text       = LocalStg.MPGMuxerExeName + L" の指定";
    fcgLBMP4RawPath->Text         = LocalStg.MP4RawExeName + L" の指定";

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
        fcgtabPageExSettings->Text = L"映像";
        fcggroupBoxCmdEx->Text = L"コマンドライン";
    } else {
        fcgtabControlVideo->TabPages->Insert(0, fcgtabPageX264Main);
        fcgtabControlVideo->TabPages->Insert(1, fcgtabPageX264RC);
        fcgtabControlVideo->TabPages->Insert(2, fcgtabPageX264Frame);
        fcgtabPageExSettings->Text = L"拡張";
        fcggroupBoxCmdEx->Text = L"追加コマンド";
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
        if (   0 == String::Compare(fcgTXQuality->Text, STR_BITRATE_AUTO)
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
            fcgLBQuality->Text = (fcgCXX264Mode->SelectedIndex == 5) ? L"目標映像ビットレート(kbps, \"-1\"で自動)" : L"ビットレート(kbps)";
            fcgLBQualityLeft->Text = L"低品質";
            fcgLBQualityRight->Text = L"高品質";
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
            fcgLBQuality->Text = L"量子化量(Quantizer)";
            fcgLBQualityLeft->Text = L"高品質";
            fcgLBQualityRight->Text = L"低品質";
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
            fcgLBQuality->Text = L"品質(Quality)";
            fcgLBQualityLeft->Text = L"高品質";
            fcgLBQualityRight->Text = L"低品質";
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
    if (0 == String::Compare(fcgTXQuality->Text, STR_BITRATE_AUTO)) {
        fcgTXQuality->Text = L"-1";
        fcgTXQuality->Select(0, fcgTXQuality->Text->Length);
    }
}

System::Void frmConfig::fcgTXQuality_TextChanged(System::Object^  sender, System::EventArgs^  e) {
    if (fcgTXQuality->Text->Length == 0 || String::Compare(fcgTXQuality->Text, L"-") == 0)
        return;
    //自動モードの文字列に変更されたときの処理
    if (0 == String::Compare(fcgTXQuality->Text, STR_BITRATE_AUTO)) {
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
        MessageBox::Show(this, L"入力された文字数が多すぎます。減らしてください。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
        fcgTSTSettingsNotes->Focus();
        fcgTSTSettingsNotes->SelectionStart = fcgTSTSettingsNotes->Text->Length;
        return false;
    }
    fcgTSTSettingsNotes->Visible = Enable;
    fcgTSLSettingsNotes->Visible = !Enable;
    if (Enable) {
        fcgTSTSettingsNotes->Text = fcgTSLSettingsNotes->Text;
        fcgTSTSettingsNotes->Focus();
        bool isDefaultNote = String::Compare(fcgTSTSettingsNotes->Text, String(DefaultStgNotes).ToString()) == 0;
        fcgTSTSettingsNotes->Select((isDefaultNote) ? 0 : fcgTSTSettingsNotes->Text->Length, fcgTSTSettingsNotes->Text->Length);
    } else {
        SetfcgTSLSettingsNotes(fcgTSTSettingsNotes->Text);
        CheckOtherChanges(nullptr, nullptr);
    }
    return true;
}
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
        maxwidth = max(maxwidth, ds->MeasureString(CX->Items[i]->ToString(), CX->Font).Width);
    CX->DropDownWidth = (int)(maxwidth + 0.5);
    delete ds;
}

System::Void frmConfig::InitCXCmdExInsert() {
    fcgCXCmdExInsert->BeginUpdate();
    fcgCXCmdExInsert->Items->Clear();
    fcgCXCmdExInsert->Items->Add(L"文字列を挿入...");
    fcgCXCmdExInsert->Items->Add(L"ファイル フルパス...");
    System::Drawing::Graphics^ ds = fcgCXCmdExInsert->CreateGraphics();
    float max_width_of_string = 0;
    for (int i = 0; REPLACE_STRINGS_LIST[i].desc; i++)
        max_width_of_string = max(max_width_of_string, ds->MeasureString(String(REPLACE_STRINGS_LIST[i].string).ToString() + L" … ", fcgCXCmdExInsert->Font).Width);
    for (int i = 0; REPLACE_STRINGS_LIST[i].desc; i++) {
        String^ AppenStr = String(REPLACE_STRINGS_LIST[i].string).ToString();
        const int length_of_string = AppenStr->Length;
        AppenStr += L" … ";
        for (float current_width = 0.0; current_width < max_width_of_string; AppenStr = AppenStr->Insert(length_of_string, L" "))
            current_width = ds->MeasureString(AppenStr, fcgCXCmdExInsert->Font).Width;
        AppenStr += String(REPLACE_STRINGS_LIST[i].desc).ToString();
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
        fcgLBAudioEncoderPath->Text = String(astg->filename).ToString() + L" の指定";
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
        for (int i = 0; i < items_to_set; i++)
            fcgCXAudioDelayCut->Items->Add(String(AUDIO_DELAY_CUT_MODE[i]).ToString());
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
    fcgTSSettings->Text = (mItem == nullptr) ? L"プロファイル" : mItem->Text;
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
    guiEx_config exCnf;
    int result = exCnf.save_x264guiEx_conf(cnf_stgSelected, stg_name);
    free(stg_name);
    switch (result) {
        case CONF_ERROR_FILE_OPEN:
            MessageBox::Show(L"設定ファイルオープンに失敗しました。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
            return;
        case CONF_ERROR_INVALID_FILENAME:
            MessageBox::Show(L"ファイル名に使用できない文字が含まれています。\n保存できません。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
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
        MessageBox::Show(L"設定ファイル " + mItem->Text + L" を削除してよろしいですか?",
        L"エラー", MessageBoxButtons::OKCancel, MessageBoxIcon::Exclamation)) 
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
    guiEx_config exCnf;
    char stg_path[MAX_PATH_LEN];
    GetCHARfromString(stg_path, sizeof(stg_path), ClickedMenuItem->Tag->ToString());
    if (exCnf.load_x264guiEx_conf(&load_stg, stg_path) == CONF_ERROR_FILE_OPEN) {
        if (MessageBox::Show(L"設定ファイルオープンに失敗しました。\n"
                           + L"このファイルを削除しますか?",
                           L"エラー", MessageBoxButtons::YesNo, MessageBoxIcon::Error)
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
    setComboBox(fcgCXPreset,         sys_dat->exstg->s_x264.preset.name);
    setComboBox(fcgCXProfile,        sys_dat->exstg->s_x264.profile.name);
    setComboBox(fcgCXTune,             sys_dat->exstg->s_x264.tune.name);
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
    SetTXMaxLen(fcgTXX264Path,             sizeof(sys_dat->exstg->s_x264.fullpath) - 1);
    SetTXMaxLen(fcgTXX264PathSub,          sizeof(sys_dat->exstg->s_x264.fullpath) - 1);
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

System::Void frmConfig::initUpdater() {
#if ENABLE_AUOSETUP
    frmExeUpdate = gcnew frmUpdate();
    frmExeUpdate->Owner = this;
    frmExeUpdate->init(gcnew x264guiEx::DelegateProcessFin(this, &frmConfig::fcgTSBUpdate_CheckFinished),
        gcnew x264guiEx::DelegateProcessFin(this, &frmConfig::fcgTSBUpdate_UpdateFinished));
    if (!sys_dat->exstg->s_local.update_check_auto) {
        return;
    }
    if (str_has_char(sys_dat->exstg->s_local.last_update_check)) {
        try {
            DateTime dtLastUpdate = DateTime::Parse(String(sys_dat->exstg->s_local.last_update_check).ToString());
            TimeSpan tsFromLastUpdate = DateTime::Now - dtLastUpdate;
            //一週間以内に確認していれば自動的に更新チェック
            if (tsFromLastUpdate < TimeSpan(7, 0, 0, 0)) {
                return;
            }
        } catch (...) {
            //そのまま下へ
        }
    }
    frmExeUpdate->startCheck();
#else
    fcgTSBUpdate->Enabled = false;
#endif
}

System::Void frmConfig::InitForm() {
    initUpdater();
    //ローカル設定のロード
    LoadLocalStg();
    //ローカル設定の反映
    SetLocalStg();
    //設定ファイル集の初期化
    InitStgFileList();
    //コンボボックスの値を設定
    InitComboBox();
    //タイムコードのappendix(後付修飾子)を反映
    fcgCBAuoTcfileout->Text = L"タイムコード出力 (" + String(sys_dat->exstg->s_append.tc).ToString() + L")";
    //タイトル表示
    this->Text = String(AUO_FULL_NAME).ToString();
    //バージョン情報,コンパイル日時
    fcgLBVersion->Text     = String(AUO_VERSION_NAME).ToString();
    fcgLBVersionDate->Text = L"build " + String(__DATE__).ToString() + L" " + String(__TIME__).ToString();
    //スレッド数上限
    int max_threads_set = (int)(cpu_core_count() * 1.5 + 0.51);
    fcgNUThreads->Maximum = max_threads_set;
    fcgNULookaheadThreads->Maximum = max_threads_set;
    //タイマーの初期化
    InitTimer();
    //ツールチップ
    SetHelpToolTips();
    SetX264VersionToolTip(LocalStg.x264Path);
    ActivateToolTip(sys_dat->exstg->s_local.disable_tooltip_help == FALSE);
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

/////////////         データ <-> GUI     /////////////
System::Void frmConfig::ConfToFrm(CONF_GUIEX *cnf, bool all) {
    //ひたすら書くだけ。めんどい
    CONF_X264 *cx264 = &cnf->x264;
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
        fcgCBRyzenDisableAVX2->Checked     = cnf->vid.ryzen_disable_avx2 != 0;

        SetCXIndex(fcgCXX264Priority,        cnf->vid.priority);
        SetCXIndex(fcgCXTempDir,             cnf->oth.temp_dir);

        fcgTXCmdEx->Text            = String(cnf->vid.cmdex).ToString();
        if (cnf->oth.disable_guicmd)
            fcgCBNulOutCLI->Checked        = cnf->x264.nul_out != 0;

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
    cnf->x264.use_highbit_depth    = fcgCBUsehighbit->Checked;
    cnf->x264.rc_mode              = cnf_fcgTemp->rc_mode;
    cnf->x264.bitrate              = cnf_fcgTemp->bitrate;
    cnf->x264.qp                   = cnf_fcgTemp->qp;
    cnf->x264.crf                  = cnf_fcgTemp->crf;
    cnf->x264.nul_out              = fcgCBNulOut->Checked;
    cnf->x264.pass                 = cnf_fcgTemp->pass;
    cnf->x264.slow_first_pass      = cnf_fcgTemp->slow_first_pass;
    cnf->x264.use_auto_npass       = cnf_fcgTemp->use_auto_npass;
    cnf->x264.auto_npass           = (int)fcgNUAutoNPass->Value;
    cnf->vid.amp_check             = 0x00;
    cnf->vid.amp_check            |= fcgCBAMPLimitBitrateLower->Checked ? AMPLIMIT_BITRATE_LOWER : 0x00;
    cnf->vid.amp_check            |= fcgCBAMPLimitBitrateUpper->Checked ? AMPLIMIT_BITRATE_UPPER : 0x00;
    cnf->vid.amp_check            |= fcgCBAMPLimitFileSize->Checked ? AMPLIMIT_FILE_SIZE : 0x00;
    cnf->vid.amp_limit_bitrate_upper = (double)fcgNUAMPLimitBitrateUpper->Value;
    cnf->vid.amp_limit_bitrate_lower = (double)fcgNUAMPLimitBitrateLower->Value;
    cnf->vid.amp_limit_file_size     = (double)fcgNUAMPLimitFileSize->Value;
    cnf->x264.preset               = fcgCXPreset->SelectedIndex;
    cnf->x264.tune                 = fcgCXTune->SelectedIndex;
    cnf->x264.profile              = fcgCXProfile->SelectedIndex;
    cnf->x264.h264_level           = fcgCXLevel->SelectedIndex;
    cnf->x264.videoformat          = fcgCXVideoFormat->SelectedIndex;
    cnf->x264.aud                  = fcgCBAud->Checked;
    cnf->x264.pic_struct           = fcgCBPicStruct->Checked;
    cnf->x264.nal_hrd              = fcgCXNalHrd->SelectedIndex;
    cnf->x264.output_csp           = fcgCXOutputCsp->SelectedIndex;
    cnf->x264.bluray_compat        = fcgCBBlurayCompat->Checked;
    cnf->x264.colormatrix          = fcgCXColorMatrix->SelectedIndex;
    cnf->x264.colorprim            = fcgCXColorPrim->SelectedIndex;
    cnf->x264.transfer             = fcgCXTransfer->SelectedIndex;
    cnf->x264.input_range          = fcgCXInputRange->SelectedIndex;
    cnf->x264.sar.x                = (int)fcgNUAspectRatioX->Value * ((fcgCXAspectRatio->SelectedIndex != 1) ? 1 : -1);
    cnf->x264.sar.y                = (int)fcgNUAspectRatioY->Value * ((fcgCXAspectRatio->SelectedIndex != 1) ? 1 : -1);
    cnf->x264.threads              = (int)fcgNUThreads->Value;
    cnf->x264.lookahead_threads    = (int)fcgNULookaheadThreads->Value;
    cnf->x264.sliced_threading     = fcgCBSlicedThreads->Checked;
    cnf->x264.log_mode             = fcgCXLogLevel->SelectedIndex;
    cnf->x264.psnr                 = fcgCBPSNR->Checked;
    cnf->x264.ssim                 = fcgCBSSIM->Checked;

    cnf->x264.ip_ratio             = (float)((int)fcgNUIPRatio->Value + 100) / 100.0f;
    cnf->x264.pb_ratio             = (float)((int)fcgNUPBRatio->Value + 100) / 100.0f;
    cnf->x264.qp_compress          = (float)((int)fcgNUQcomp->Value / 100.0f);
    cnf->x264.qp_min               = (int)fcgNUQpmin->Value;
    cnf->x264.qp_max               = (int)fcgNUQpmax->Value;
    cnf->x264.qp_step              = (int)fcgNUQpstep->Value;
    cnf->x264.chroma_qp_offset     = (int)fcgNUChromaQp->Value;
    cnf->x264.aq_mode              = fcgCXAQMode->SelectedIndex;
    cnf->x264.aq_strength          = (float)fcgNUAQStrength->Value;
    cnf->x264.psy_rd.x             = (float)fcgNUPsyRDO->Value;
    cnf->x264.psy_rd.y             = (float)fcgNUPsyTrellis->Value;
    cnf->x264.vbv_maxrate          = (int)fcgNUVBVmax->Value;
    cnf->x264.vbv_bufsize          = (int)fcgNUVBVbuf->Value;
    cnf->x264.mbtree               = fcgCBMBTree->Checked;
    cnf->x264.rc_lookahead         = (int)fcgNURCLookahead->Value;

    cnf->x264.scenecut             = (int)fcgNUScenecut->Value;
    cnf->x264.keyint_min           = (int)fcgNUMinKeyint->Value;
    cnf->x264.keyint_max           = (int)fcgNUKeyint->Value;
    cnf->x264.open_gop             = fcgCBOpenGOP->Checked;
    cnf->x264.use_deblock          = fcgCBDeblock->Checked;
    cnf->x264.deblock.x            = (int)fcgNUDeblockStrength->Value;
    cnf->x264.deblock.y            = (int)fcgNUDeblockThreshold->Value;
    cnf->x264.cabac                = fcgCBCABAC->Checked;
    cnf->x264.slice_n              = (int)fcgNUSlices->Value;
    cnf->x264.weight_p             = fcgCXWeightP->SelectedIndex;
    cnf->x264.interlaced           = fcgCXInterlaced->SelectedIndex != 0;
    cnf->x264.tff                  = fcgCXInterlaced->SelectedIndex != 2;
    cnf->x264.bframes              = (int)fcgNUBframes->Value;
    cnf->x264.b_adapt              = fcgCXBAdpapt->SelectedIndex;
    cnf->x264.b_bias               = (int)fcgNUBBias->Value;
    cnf->x264.b_pyramid            = fcgCXBpyramid->SelectedIndex;
    cnf->x264.weight_b             = fcgCBWeightB->Checked;

    cnf->x264.dct8x8               = fcgCB8x8dct->Checked;
    cnf->x264.mb_partition         = MB_PARTITION_NONE;
    cnf->x264.mb_partition        |= (fcgCBb8x8->Checked) ? MB_PARTITION_B8x8 : MB_PARTITION_NONE;
    cnf->x264.mb_partition        |= (fcgCBp8x8->Checked) ? MB_PARTITION_P8x8 : MB_PARTITION_NONE;
    cnf->x264.mb_partition        |= (fcgCBp4x4->Checked) ? MB_PARTITION_P4x4 : MB_PARTITION_NONE;
    cnf->x264.mb_partition        |= (fcgCBi8x8->Checked) ? MB_PARTITION_I8x8 : MB_PARTITION_NONE;
    cnf->x264.mb_partition        |= (fcgCBi4x4->Checked) ? MB_PARTITION_I4x4 : MB_PARTITION_NONE;

    cnf->x264.no_fast_pskip        = !fcgCBfastpskip->Checked;
    cnf->x264.no_dct_decimate      = !fcgCBDctDecimate->Checked;
    cnf->x264.trellis              = fcgCXTrellis->SelectedIndex;
    cnf->x264.cqm                  = GetCQMIndex(cnf->vid.cqmfile, sizeof(cnf->vid.cqmfile));

    cnf->x264.me                   = fcgCXME->SelectedIndex;
    cnf->x264.subme                = fcgCXSubME->SelectedIndex;
    cnf->x264.me_range             = (int)fcgNUMERange->Value;
    cnf->x264.chroma_me            = fcgCBChromaME->Checked;
    cnf->x264.direct_mv            = fcgCXDirectME->SelectedIndex;
    cnf->x264.ref_frames           = (int)fcgNURef->Value;
    cnf->x264.mixed_ref            = fcgCBMixedRef->Checked;

    cnf->x264.use_tcfilein         = fcgCBTCIN->Checked;
    cnf->x264.use_timebase         = fcgCBTimeBase->Checked;
    cnf->x264.timebase.x           = (int)fcgNUTimebaseNum->Value;
    cnf->x264.timebase.y           = (int)fcgNUTimebaseDen->Value;

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
    cnf->vid.ryzen_disable_avx2     = fcgCBRyzenDisableAVX2->Checked;
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
        cnf->x264.nul_out           = fcgCBNulOutCLI->Checked;

    //制約条件として適用
    set_profile_to_conf(&cnf->x264, cnf->x264.profile);
}

System::Void frmConfig::GetfcgTSLSettingsNotes(char *notes, int nSize) {
    ZeroMemory(notes, nSize);
    if (fcgTSLSettingsNotes->ForeColor == Color::FromArgb(StgNotesColor[0][0], StgNotesColor[0][1], StgNotesColor[0][2]))
        GetCHARfromString(notes, nSize, fcgTSLSettingsNotes->Text);
}

System::Void frmConfig::SetfcgTSLSettingsNotes(const char *notes) {
    if (str_has_char(notes)) {
        fcgTSLSettingsNotes->ForeColor = Color::FromArgb(StgNotesColor[0][0], StgNotesColor[0][1], StgNotesColor[0][2]);
        fcgTSLSettingsNotes->Text = String(notes).ToString();
    } else {
        fcgTSLSettingsNotes->ForeColor = Color::FromArgb(StgNotesColor[1][0], StgNotesColor[1][1], StgNotesColor[1][2]);
        fcgTSLSettingsNotes->Text = String(DefaultStgNotes).ToString();
    }
}

System::Void frmConfig::SetfcgTSLSettingsNotes(String^ notes) {
    if (notes->Length && String::Compare(notes, String(DefaultStgNotes).ToString()) != 0) {
        fcgTSLSettingsNotes->ForeColor = Color::FromArgb(StgNotesColor[0][0], StgNotesColor[0][1], StgNotesColor[0][2]);
        fcgTSLSettingsNotes->Text = notes;
    } else {
        fcgTSLSettingsNotes->ForeColor = Color::FromArgb(StgNotesColor[1][0], StgNotesColor[1][1], StgNotesColor[1][2]);
        fcgTSLSettingsNotes->Text = String(DefaultStgNotes).ToString();
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
    const X264_OPTION_STR *list = get_option_list(type);
    fcgTTX264->SetToolTip(control,      L"--" + String(type).ToString() + L"\n"
        + L"auto とするとAviutlの色空間「自動」に合わせ\n"
        + L"以下のように設定します。\n"
        + L"縦解像度" + COLOR_MATRIX_THRESHOLD + L"以上 … " + String(list[COLOR_MATRIX_HD].desc).ToString() + L"\n"
        + L"縦解像度" + COLOR_MATRIX_THRESHOLD + L"未満 … " + String(list[COLOR_MATRIX_SD].desc).ToString() + L"\n"
        + L"よくわからない場合は 指定なし が無難です。"
        );
}

System::Void frmConfig::SetHelpToolTips() {
    //x264基本
    fcgTTX264->SetToolTip(fcgCBUsehighbit, L"" 
        + L"--input-depth 16\n"
        + L"\n"
        + L"high bit-depthでエンコードを行います。\n"
        + L"x264も10bit版など、high bit depthのものを使用してください。\n"
        + L"通常のプレーヤーでは再生できないこともあるため、\n"
        + L"high bit depthエンコードがなにかを理解している場合にのみ、\n"
        + L"使用してください。\n"
        + L"\n"
        + L"8bit用x264.exeとhigh bit depth用x264.exeは別々に設定でき、\n"
        + L"このチェックボックスによって切り替わります。"
        );
    fcgTTX264->SetToolTip(fcgBTX264Path, L""
        + L"x264.exeの場所を指定します。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTX264->SetToolTip(fcgBTX264PathSub, L""
        + L"x264.exeの場所を指定します。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTX264->SetToolTip(fcgCXX264Mode, L""
        + L"【シングルパス】\n"
        + L"   " + String(x264_encodemode_desc[2]).ToString()->Replace(L"シングルパス - ", L"") + L"\t … --crf\n"
        + L"   " + String(x264_encodemode_desc[1]).ToString()->Replace(L"シングルパス - ", L"") + L"\t\t … --qp\n"
        + L"   " + String(x264_encodemode_desc[0]).ToString()->Replace(L"シングルパス - ", L"") + L"\t … --bitrate\n"
        + L"\n"
        + L"【マルチパス】\n"
        + L"   " + String(x264_encodemode_desc[3]).ToString()->Replace(L"マルチパス - ", L"") + L"\t … --pass 1 --bitrate\n"
        + L"   " + String(x264_encodemode_desc[4]).ToString()->Replace(L"マルチパス - ", L"") + L"\t … --pass 3 --bitrate\n"
        + L"\n"
        + L"【" + String(x264_encodemode_desc[5]).ToString() + L"】\n"
        + L"    マルチパス出力(1pass → npass)を自動で行います。\n"
        + L"    --pass 1/3 --bitrate\n"
        + L"\n"
        + L"【" + String(x264_encodemode_desc[6]).ToString() + L"】\n"
        + L"    品質基準VBR (crf)でのエンコード後、ファイルサイズ・ビットレートを確認します。\n"
        + L"    --crf"
        );
    fcgTTX264->SetToolTip(fcgCBNulOut,            L"-o nul");
    fcgTTX264->SetToolTip(fcgCBFastFirstPass,     L"--slow-firstpass (チェックオフ時)");
    fcgTTX264->SetToolTip(fcgTXQuality,           L"--crf / --bitrate / --qp");
    fcgTTX264->SetToolTip(fcgTXStatusFile,        L"--stats");
    fcgTTX264->SetToolTip(fcgCXProfile,           L"--profile\n"
        + L"\n"
        + L"最終的にこの設定による制約が課されます。"
        );

    //自動マルチパス 上限設定
    String^ AMP_LimitBitrateUpper = L""
        + L"出力する動画ファイル(映像＋音声)のビットレートが、\n"
        + L"ここで設定した上限ビットレートを超えないようエンコードを行います。\n"
        + L"\n"
        + L"エンコード終了後にも確認を行い、ファイルビットレートが上限設定を\n"
        + L"上回ってしまった場合には、再エンコードを行います。\n"
        + L"\n"
        + L"上限設定はチェックボックスによりオン/オフできます。";
    String^ AMP_LimitBitrateLower = L""
        + L"出力する動画ファイル(映像＋音声)のビットレートが、\n"
        + L"ここで設定した下限ビットレートを下回らないようエンコードを行います。\n"
        + L"\n"
        + L"エンコード終了後にも確認を行い、ファイルビットレートが下限設定を\n"
        + L"下回ってしまった場合には、再エンコードを行います。\n"
        + L"\n"
        + L"下限設定はチェックボックスによりオン/オフできます。";
    fcgTTEx->SetToolTip(fcgCBAMPLimitBitrateUpper, AMP_LimitBitrateUpper);
    fcgTTEx->SetToolTip(fcgNUAMPLimitBitrateUpper, AMP_LimitBitrateUpper);
    fcgTTEx->SetToolTip(fcgCBAMPLimitBitrateLower, AMP_LimitBitrateLower);
    fcgTTEx->SetToolTip(fcgNUAMPLimitBitrateLower, AMP_LimitBitrateLower);
    String^ AMP_LimitFileSize = L""
        + L"出力する動画ファイル(映像＋音声)のサイズが、\n"
        + L"ここで設定した上限ファイルサイズを超えないようエンコードを行います。\n"
        + L"\n"
        + L"エンコード終了後にも確認を行い、ファイルサイズが上限設定を\n"
        + L"上回ってしまった場合には、再エンコードを行います。\n"
        + L"\n"
        + L"上限設定はチェックボックスによりオン/オフできます。";
    fcgTTEx->SetToolTip(fcgCBAMPLimitFileSize,     AMP_LimitFileSize);
    fcgTTEx->SetToolTip(fcgNUAMPLimitFileSize,     AMP_LimitFileSize);

    //プロファイルとか
    fcgTTX264->SetToolTip(fcgCXTune,              L"--tune");
    fcgTTX264->SetToolTip(fcgCXPreset,            L"--preset");
    fcgTTEx->SetToolTip(fcgBTApplyPreset,         L""
        + L"ボックス内で指定した\n"
        + L"\n"
        + L"・速度 (Preset)\n"
        + L"・チューニング (tune)\n"
        + L"・プロファイル (Profile)\n"
        + L"\n"
        + L"をGUIに適用します。"
        );

    //出力・フォーマット
    fcgTTX264->SetToolTip(fcgCXLevel,            L"--level");
    fcgTTX264->SetToolTip(fcgCXVideoFormat,      L"--videoformat");
    fcgTTX264->SetToolTip(fcgCBAud,              L"--aud");
    fcgTTX264->SetToolTip(fcgCBPicStruct,        L"--pic-struct");
    fcgTTX264->SetToolTip(fcgCXNalHrd,           L"--nal-hrd\n"
        + L" vbr 時は ビデオバッファ制御(VBV)の設定を行う必要があります。"
        );
    fcgTTX264->SetToolTip(fcgCBBlurayCompat,     L"--bluray-compat");
    fcgTTX264->SetToolTip(fcgCXOutputCsp,        L"--output-csp\n"
        + L"通常は i420 を使用します。"
        );

    //色空間
    SetHelpToolTipsColorMatrix(fcgCXColorMatrix, "colormatrix");
    SetHelpToolTipsColorMatrix(fcgCXColorPrim,   "colorprim");
    SetHelpToolTipsColorMatrix(fcgCXTransfer,    "transfer");
    fcgTTX264->SetToolTip(fcgCXInputRange,      L"--input-range\n"
        + L"\n"
        + L"\"" + String(list_input_range[0].desc).ToString() + L"\"  [デフォルト]\n"
        + L"  output-csp yuv系 … tv色調 (圧縮レンジ)\n"
        + L"  output-csp rgb系 … pc色調\n"
        + L"\n"
        + L"\"" + String(list_input_range[1].desc).ToString() + L"\"\n"
        + L"  pc色調 (フルレンジ)"
        );

    fcgTTX264->SetToolTip(fcgCXAspectRatio,      L""
        + String(aspect_desc[0]).ToString() + L"\n"
        + L"   --sar を直接指定します。\n"
        + L"\n"
        + String(aspect_desc[1]).ToString() + L"\n"
        + L"   エンコード時に 解像度から --sarを自動計算します。"
        );
    fcgTTX264->SetToolTip(fcgNUAspectRatioX,     L"アスペクト比 横 (幅)");
    fcgTTX264->SetToolTip(fcgNUAspectRatioY,     L"アスペクト比 縦 (高さ)");
    fcgTTX264->SetToolTip(fcgNUThreads,          L"--threads\n"
        + L"\"0\" で自動です。"
        );
    fcgTTX264->SetToolTip(fcgNULookaheadThreads, L"--lookahead-threads\n"
        + L"\"0\" で自動です。"
        );
    fcgTTX264->SetToolTip(fcgCBSlicedThreads,    L"--sliced-threads");
    fcgTTX264->SetToolTip(fcgCXLogLevel,         L"--log-level");
    fcgTTX264->SetToolTip(fcgCBPSNR,             L"--psnr");
    fcgTTX264->SetToolTip(fcgCBSSIM,             L"--ssim");

    //量子化
    fcgTTX264->SetToolTip(fcgNUIPRatio,          L"--ipratio 1.00 + (設定値)%");
    fcgTTX264->SetToolTip(fcgNUPBRatio,          L"--pbratio 1.00 + (設定値)%");
    fcgTTX264->SetToolTip(fcgNUQcomp,            L"--qcomp (設定値)%");
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
        + L"チェックオフ時 --no-mbtree"
        );
    fcgTTX264->SetToolTip(fcgNURCLookahead,      L"--rc-lookahead");
    fcgTTX264->SetToolTip(fcgNUVBVmax,           L""
        + L"--vbv-maxrate\n"
        + L"\"-1\" とするとエンコード時に自動で設定します。"
        );
    fcgTTX264->SetToolTip(fcgNUVBVbuf,           L""
        + L"--vbv-bufsize\n"
        + L"\"-1\" とするとエンコード時に自動で設定します。"
        );
    fcgTTX264->SetToolTip(fcgNUScenecut,         L"--scenecut");
    fcgTTX264->SetToolTip(fcgNUKeyint,           L""
        + L"--keyint\n"
        + L"\"0\" で inifinite(無限大) を指定します。\n"
        + L"\"-1\"で エンコ時に自動的にfps×10を設定します。");
    fcgTTX264->SetToolTip(fcgNUMinKeyint,        L""
         + L"--min-keyint\n"
         + L"\"0\" で 自動となります。"
         );
    fcgTTX264->SetToolTip(fcgCBOpenGOP,          L"--open-gop");
    fcgTTX264->SetToolTip(fcgCBCABAC,            L""
        + L"--cabac\n"
        + L"チェックオフ時 --no-cabac");
    fcgTTX264->SetToolTip(fcgNUSlices,           L"--slices");
    fcgTTX264->SetToolTip(fcgCXWeightP,          L"--weightp");

    //インタレ
    String^ InterlacedEncodingToolTip = L"   インターレース保持エンコードを行います。";
    fcgTTX264->SetToolTip(fcgCXInterlaced,       L""
        + L"[" + String(interlaced_desc[0]).ToString() + L"]\n"
        + L"   プログレッシブ(非インタレ)としてエンコードします。\n"
        + L"\n"
        + L"[" + String(interlaced_desc[1]).ToString() + L"] … --tff\n"
        + L"   TFF(トップフィールド -> ボトムフィールド) で\n" + InterlacedEncodingToolTip + L"\n"
        + L"\n"
        + L"[" + String(interlaced_desc[2]).ToString() + L"] … --bff\n"
        + L"   BFF(ボトムフィールド -> トップフィールド) で\n" + InterlacedEncodingToolTip
        );

    fcgTTX264->SetToolTip(fcgCBDeblock,          L"--deblock <Strength>:<Threshold>\n"
        + L"チェックオフ時 --no-deblock"
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
    fcgTTX264->SetToolTip(fcgCBfastpskip,        L"チェックオフ時 --no-fast-pskip");
    fcgTTX264->SetToolTip(fcgCBDctDecimate,      L"チェックオフ時 --no-dct-decimate");
    fcgTTX264->SetToolTip(fcgCXTrellis,          L"--trellis");
    fcgTTX264->SetToolTip(fcgTXCQM,              L"--cqm / --cqmfile");
    fcgTTX264->SetToolTip(fcgBTMatrix,           L"--cqm / --cqmfile");

    //動き予測
    fcgTTX264->SetToolTip(fcgCXME,               L"--me");
    fcgTTX264->SetToolTip(fcgCXSubME,            L"--subme");
    fcgTTX264->SetToolTip(fcgNUMERange,          L"--merange");
    fcgTTX264->SetToolTip(fcgCBChromaME,         L""
        + L"--chroma-me\n"
        + L"チェックオフ時 --no-chroma-me"
        );
    fcgTTX264->SetToolTip(fcgCXDirectME,         L"--direct");
    fcgTTX264->SetToolTip(fcgNURef,              L"--ref");
    fcgTTX264->SetToolTip(fcgCBMixedRef,         L"--mixed-ref");

    //時間
    String^ TCINToolTip = L""
        + L"--tcfile-in\n"
        + L"x264guiExによる --fpsの自動付加が無効になり、\n"
        + L"入力したタイムコードファイルに基づいてフレーム速度が決定されます。\n"
        + L"\n"
        + L"--tcfile-in 指定時は同時に時間精度を指定する\n"
        + L"(--timebaseを使用する)ことをお勧めします。";
    fcgTTX264->SetToolTip(fcgCBTCIN,             TCINToolTip);
    fcgTTX264->SetToolTip(fcgTXTCIN,             TCINToolTip);
    fcgTTX264->SetToolTip(fcgCBTimeBase,         L"--timebase");
    fcgTTX264->SetToolTip(fcgNUTimebaseDen,      L"--timebase");
    fcgTTX264->SetToolTip(fcgNUTimebaseNum,      L"--timebase");

    //拡張
    fcgTTEx->SetToolTip(fcgCBAFS,                L""
        + L"自動フィールドシフト(afs)を使用してVFR化を行います。\n"
        + L"エンコード時にタイムコードを作成し、mux時に埋め込んで\n"
        + L"フレームレートを変更します。\n"
        + L"\n"
        + L"外部muxerの使用が必須となり、mp4出力時は\n"
        + L"mp4boxとともにtc2mp4modが必要になりますので、\n"
        + L"忘れずに指定してください。\n"
        + L"\n"
        + L"また、あとからフレームレートを変更するため、\n"
        + L"x264のVBV設定が正確に反映されなくなる点に注意してください。"
        );
    fcgTTEx->SetToolTip(fcgCBAFSBitrateCorrection, L""
        + L"自動フィールドシフト(afs)使用時にはmux時のフレームレート変更により\n"
        + L"実レートが指定したビットレートからずれてしまいます。\n"
        + L"\n"
        + L"そこで、1pass目のdrop数から計算して\n"
        + L"ビットレート指定値の補正を行います。\n"
        + L"これにより最終的な実レートが設定通りの動画を作成できます。"
        );
    fcgTTEx->SetToolTip(fcgCBAuoTcfileout, L""
        + L"タイムコードを出力します。このタイムコードは\n"
        + L"自動フィールドシフト(afs)を反映したものになります。"
        );
    fcgTTEx->SetToolTip(fcgCBCheckKeyframes, L""
        + L"Aviutlのキーフレーム設定をx264に伝えるため、\n"
        + L"キーフレーム検出を行います。\n"
        + L"\n"
        + L"キーフレーム検出は、自動フィールドシフトと同時に使用できません。\n"
        + L"自動フィールドシフト使用時は無効となります。"
        );
    fcgTTEx->SetToolTip(fcgCBSetKeyframeAtChapter, L""
        + L"チャプターの位置にキーフレームを設定します。\n"
        + L"\n"
        + L"キーフレーム検出は、自動フィールドシフトと同時に使用できません。\n"
        + L"自動フィールドシフト使用時は無効となります。"
        );
    fcgTTEx->SetToolTip(fcgCBInputAsLW48, L""
        + L"L-SMASH WorksのLW48機能を使用します。\n"
        + L"lwinput.auiおよびlwcolor.aucと組み合わせて使用します。\n"
        + L"チェックを入れることで、通常のYC48ではなく、\n"
        + L"LW48として処理するようになります。"
        );
    fcgTTEx->SetToolTip(fcgCXTempDir,      L""
        + L"一時ファイル群\n"
        + L"・音声一時ファイル(wav / エンコード後音声)\n"
        + L"・動画一時ファイル\n"
        + L"・タイムコードファイル\n"
        + L"・qpファイル\n"
        + L"・mux後ファイル\n"
        + L"の作成場所を指定します。"
        );
    fcgTTEx->SetToolTip(fcgBTCustomTempDir, L""
        + L"一時ファイルの場所を「カスタム」に設定した際に\n"
        + L"使用される一時ファイルの場所を指定します。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgBTCmdEx,       L""
        + L"入力したコマンド(オプション)をGUIに反映させます。"
        );

    //音声
    fcgTTEx->SetToolTip(fcgCXAudioEncoder, L""
        + L"使用する音声エンコーダを指定します。\n"
        + L"これらの設定はx264guiEx.iniに記述されています。"
        );
    fcgTTEx->SetToolTip(fcgCBAudioOnly,    L""
        + L"動画の出力を行わず、音声エンコードのみ行います。\n"
        + L"音声エンコードに失敗した場合などに使用してください。"
        );
    fcgTTEx->SetToolTip(fcgCBFAWCheck,     L""
        + L"音声エンコード時に音声がFakeAACWav(FAW)かどうかの判定を行い、\n"
        + L"FAWだと判定された場合、設定を無視して、\n"
        + L"自動的にFAWを使用するよう切り替えます。\n"
        + L"\n"
        + L"一度音声エンコーダからFAW(fawcl)を選択し、\n"
        + L"実行ファイルの場所を指定しておく必要があります。"
        );
    fcgTTEx->SetToolTip(fcgBTAudioEncoderPath, L""
        + L"音声エンコーダの場所を指定します。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgCXAudioEncMode, L""
        + L"音声エンコーダのエンコードモードを切り替えます。\n"
        + L"これらの設定はx264guiEx.iniに記述されています。"
        );
    fcgTTEx->SetToolTip(fcgCBAudio2pass,   L""
        + L"音声エンコードを2passで行います。\n"
        + L"2pass時はパイプ処理は行えません。"
        );
    fcgTTEx->SetToolTip(fcgCBAudioUsePipe, L""
        + L"パイプを通して、音声データをエンコーダに渡します。\n"
        + L"パイプと2passは同時に指定できません。"
        );
    fcgTTEx->SetToolTip(fcgNUAudioBitrate, L""
        + L"音声ビットレートを指定します。"
        );
    fcgTTEx->SetToolTip(fcgCXAudioPriority, L""
        + L"音声エンコーダのCPU優先度を設定します。\n"
        + L"AviutlSync で Aviutlの優先度と同じになります。"
        );
    fcgTTEx->SetToolTip(fcgCXAudioEncTiming, L""
        + L"音声を処理するタイミングを設定します。\n"
        + L" 後　 … 映像→音声の順で処理します。\n"
        + L" 前　 … 音声→映像の順で処理します。\n"
        + L" 同時 … 映像と音声を同時に処理します。"
        );
    fcgTTEx->SetToolTip(fcgCXAudioTempDir, L""
        + L"音声一時ファイル(エンコード後のファイル)\n"
        + L"の出力先を変更します。"
        );
    fcgTTEx->SetToolTip(fcgBTCustomAudioTempDir, L""
        + L"音声一時ファイルの場所を「カスタム」にした時に\n"
        + L"使用される音声一時ファイルの場所を指定します。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    //音声バッチファイル実行
    fcgTTEx->SetToolTip(fcgCBRunBatBeforeAudio, L""
        + L"音声エンコード開始前にバッチファイルを実行します。"
        );
    fcgTTEx->SetToolTip(fcgCBRunBatAfterAudio, L""
        + L"音声エンコード終了後、バッチファイルを実行します。"
        );
    fcgTTEx->SetToolTip(fcgBTBatBeforeAudioPath, L""
        + L"音声エンコード終了後実行するバッチファイルを指定します。\n"
        + L"実際のバッチ実行時には新たに\"<バッチファイル名>_tmp.bat\"を作成、\n"
        + L"指定したバッチファイルの内容をコピーし、\n"
        + L"さらに特定文字列を置換して実行します。\n"
        + L"使用できる置換文字列はreadmeをご覧下さい。"
        );
    fcgTTEx->SetToolTip(fcgBTBatAfterAudioPath, L""
        + L"音声エンコード終了後実行するバッチファイルを指定します。\n"
        + L"実際のバッチ実行時には新たに\"<バッチファイル名>_tmp.bat\"を作成、\n"
        + L"指定したバッチファイルの内容をコピーし、\n"
        + L"さらに特定文字列を置換して実行します。\n"
        + L"使用できる置換文字列はreadmeをご覧下さい。"
        );

    //muxer
    fcgTTEx->SetToolTip(fcgCBMP4MuxerExt, L""
        + L"指定したmuxerでmuxを行います。\n"
        + L"チェックを外すとmuxを行いません。"
        );
    fcgTTEx->SetToolTip(fcgCXMP4CmdEx,    L""
        + L"muxerに渡す追加オプションを選択します。\n"
        + L"これらの設定はx264guiEx.iniに記述されています。"
        );
    fcgTTEx->SetToolTip(fcgBTMP4MuxerPath, L""
        + L"mp4用muxerの場所を指定します。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgBTTC2MP4Path, L""
        + L"tc2mp4modの場所を指定します。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgBTMP4RawPath, L""
        + L"raw用mp4muxerの場所を指定します。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgCXMP4BoxTempDir, L""
        + L"mp4box用の一時フォルダの場所を指定します。"
        );
    fcgTTEx->SetToolTip(fcgBTMP4BoxTempDir, L""
        + L"mp4box用一時フォルダの場所を「カスタム」に設定した際に\n"
        + L"使用される一時フォルダの場所です。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgCBMKVMuxerExt, L""
        + L"指定したmuxerでmuxを行います。\n"
        + L"チェックを外すとmuxを行いません。"
        );
    fcgTTEx->SetToolTip(fcgCXMKVCmdEx,    L""
        + L"muxerに渡す追加オプションを選択します。\n"
        + L"これらの設定はx264guiEx.iniに記述されています。"
        );
    fcgTTEx->SetToolTip(fcgBTMKVMuxerPath, L""
        + L"mkv用muxerの場所を指定します。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgCBMPGMuxerExt, L""
        + L"指定したmuxerでmuxを行います。\n"
        + L"チェックを外すとmuxを行いません。"
        );
    fcgTTEx->SetToolTip(fcgCXMPGCmdEx,    L""
        + L"muxerに渡す追加オプションを選択します。\n"
        + L"これらの設定はx264guiEx.iniに記述されています。"
        );
    fcgTTEx->SetToolTip(fcgBTMPGMuxerPath, L""
        + L"mpg用muxerの場所を指定します。\n"
        + L"\n"
        + L"この設定はx264guiEx.confに保存され、\n"
        + L"バッチ処理ごとの変更はできません。"
        );
    fcgTTEx->SetToolTip(fcgCXMuxPriority, L""
        + L"muxerのCPU優先度を指定します。\n"
        + L"AviutlSync で Aviutlの優先度と同じになります。"
        );
    //バッチファイル実行
    fcgTTEx->SetToolTip(fcgCBRunBatBefore, L""
        + L"エンコード開始前にバッチファイルを実行します。"
        );
    fcgTTEx->SetToolTip(fcgCBRunBatAfter, L""
        + L"エンコード終了後、バッチファイルを実行します。"
        );
    fcgTTEx->SetToolTip(fcgCBWaitForBatBefore, L""
        + L"バッチ処理開始後、バッチ処理が終了するまで待機します。"
        );
    fcgTTEx->SetToolTip(fcgCBWaitForBatAfter, L""
        + L"バッチ処理開始後、バッチ処理が終了するまで待機します。"
        );
    fcgTTEx->SetToolTip(fcgBTBatBeforePath, L""
        + L"エンコード終了後実行するバッチファイルを指定します。\n"
        + L"実際のバッチ実行時には新たに\"<バッチファイル名>_tmp.bat\"を作成、\n"
        + L"指定したバッチファイルの内容をコピーし、\n"
        + L"さらに特定文字列を置換して実行します。\n"
        + L"使用できる置換文字列はreadmeをご覧下さい。"
        );
    fcgTTEx->SetToolTip(fcgBTBatAfterPath, L""
        + L"エンコード終了後実行するバッチファイルを指定します。\n"
        + L"実際のバッチ実行時には新たに\"<バッチファイル名>_tmp.bat\"を作成、\n"
        + L"指定したバッチファイルの内容をコピーし、\n"
        + L"さらに特定文字列を置換して実行します。\n"
        + L"使用できる置換文字列はreadmeをご覧下さい。"
        );
    //上部ツールストリップ
    fcgTSBCMDOnly->ToolTipText = L""
        + L"GUIモード と CLIモードを切り替えます。\n"
        + L"CLIモードはコマンドライン入力のみで\n"
        + L"オプションの指定を行います。";

    fcgTSBDelete->ToolTipText = L""
        + L"現在選択中のプロファイルを削除します。";

    fcgTSBOtherSettings->ToolTipText = L""
        + L"プロファイルの保存フォルダを変更します。";

    fcgTSBSave->ToolTipText = L""
        + L"現在の設定をプロファイルに上書き保存します。";

    fcgTSBSaveNew->ToolTipText = L""
        + L"現在の設定を新たなプロファイルに保存します。";

    //他
    fcgTTEx->SetToolTip(fcgTXCmd,         L""
        + L"x264に渡される予定のコマンドラインです。\n"
        + L"エンコード時には更に\n"
        + L"・「追加コマンド」の付加\n"
        + L"・\"auto\"な設定項目の反映\n"
        + L"・必要な情報の付加(--fps/-o/--input-res/--input-csp/--frames等)\n"
        + L"が行われます。\n"
        + L"\n"
        + L"このウィンドウはダブルクリックで拡大縮小できます。"
        );
    fcgTTEx->SetToolTip(fcgBTDefault,     L""
        + L"デフォルト設定をロードします。"
        );
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
            mes = L"バージョン情報の取得に失敗しました。";
    } else {
        mes = L"指定されたx264が存在しません。";
    }
    fcgTTX264Version->SetToolTip(fcgTXX264PathSub, mes);
}
System::Void frmConfig::ShowExehelp(String^ ExePath, String^ args) {
    if (!File::Exists(ExePath)) {
        MessageBox::Show(L"指定された実行ファイルが存在しません。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
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
                } finally {
                    if (sw != nullptr) { sw->Close(); }
                }
            }
            GetCHARfromString(cmd, sizeof(cmd), arg_list[i]);
            if (get_exe_message_to_file(exe_path, cmd, file_path, AUO_PIPE_MUXED, 5) != RP_SUCCESS) {
                File::Delete(String(file_path).ToString());
                MessageBox::Show(L"helpの取得に失敗しました。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }
        }
        try {
            System::Diagnostics::Process::Start(String(file_path).ToString());
        } catch (...) {
            MessageBox::Show(L"helpを開く際に不明なエラーが発生しました。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    }
}

#pragma warning( pop )
