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

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "auo.h"
#include "auo_pipe.h"
#include "auo_conf.h"
#include "auo_settings.h"
#include "auo_system.h"
#include "auo_util.h"
#include "auo_clrutil.h"

#include "h264_level.h"
#include "transparentLabel.h"

#include "frmConfig_helper.h"
#include "auo_mes.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::IO;


namespace AUO_NAME_R {

    /// <summary>
    /// frmConfig の概要
    ///
    /// 警告: このクラスの名前を変更する場合、このクラスが依存するすべての .resx ファイルに関連付けられた
    ///          マネージ リソース コンパイラ ツールに対して 'Resource File Name' プロパティを
    ///          変更する必要があります。この変更を行わないと、
    ///          デザイナと、このフォームに関連付けられたローカライズ済みリソースとが、
    ///          正しく相互に利用できなくなります。
    /// </summary>
    public ref class frmConfig : public System::Windows::Forms::Form
    {
    public:
        frmConfig(CONF_GUIEX *_conf, const SYSTEM_DATA *_sys_dat)
        {
            InitData(_conf, _sys_dat);
            list_lng = nullptr;
            dwStgReader = nullptr;
            themeMode = AuoTheme::DefaultLight;
            cnf_fcgTemp = (CONF_ENC*)calloc(1, sizeof(CONF_ENC));
            cnf_stgSelected = (CONF_GUIEX*)calloc(1, sizeof(CONF_GUIEX));
            InitializeComponent();
            //
            //TODO: ここにコンストラクタ コードを追加します
            //
            fcgLastX264ModeAsAMP = true;
            LoadLangText();
        }

    protected:
        /// <summary>
        /// 使用中のリソースをすべてクリーンアップします。
        /// </summary>
        ~frmConfig()
        {
            if (components)
            {
                delete components;
            }
            CloseBitrateCalc();
            if (cnf_fcgTemp) free(cnf_fcgTemp); cnf_fcgTemp = NULL;
            if (cnf_stgSelected) free(cnf_stgSelected); cnf_stgSelected = NULL;
            if (dwStgReader != nullptr)
                delete dwStgReader;
            if (qualityTimer != nullptr)
                delete qualityTimer;
            if (list_lng != nullptr)
                delete list_lng;
        }
private: System::Windows::Forms::Label^  fcgLBAMPAutoBitrate;


    private: System::Windows::Forms::TabControl^  fcgtabControlVideo;
    private: System::Windows::Forms::TabPage^  fcgtabPageX264Main;
    private: System::Windows::Forms::TabPage^  fcgtabPageX264RC;



    private: System::Windows::Forms::TabControl^  fcgtabControlMux;


    private: System::Windows::Forms::TabPage^  fcgtabPageMP4;
    private: System::Windows::Forms::TabPage^  fcgtabPageMKV;
    private: System::Windows::Forms::TextBox^  fcgTXCmd;

    private: System::Windows::Forms::Button^  fcgBTCancel;

    private: System::Windows::Forms::Button^  fcgBTOK;
    private: System::Windows::Forms::Button^  fcgBTDefault;



    private: System::Windows::Forms::TabPage^  fcgtabPageExSettings;










    private: System::Windows::Forms::TextBox^  fcgTXX264Path;
    private: System::Windows::Forms::Label^  fcgLBX264Path;
    private: System::Windows::Forms::GroupBox^  fcggroupBoxPreset;
    private: System::Windows::Forms::CheckBox^  fcgCBFastFirstPass;


    private: System::Windows::Forms::CheckBox^  fcgCBNulOut;
    private: System::Windows::Forms::ComboBox^  fcgCXX264Mode;
    private: System::Windows::Forms::CheckBox^  fcgCBUsehighbit;

    private: System::Windows::Forms::Button^  fcgBTX264Path;

    private: System::Windows::Forms::NumericUpDown^  fcgNUAutoNPass;
    private: System::Windows::Forms::Label^  fcgLBQualityRight;

    private: System::Windows::Forms::Label^  fcgLBQualityLeft;

    private: System::Windows::Forms::TrackBar^  fcgTBQuality;
    private: System::Windows::Forms::TextBox^  fcgTXQuality;


    private: System::Windows::Forms::Label^  fcgLBQuality;
    private: System::Windows::Forms::Label^  fcgLBX264Preset;
    private: System::Windows::Forms::Label^  fcgLBProfile;

    private: System::Windows::Forms::Label^  fcgLBX264TUNE;
    private: System::Windows::Forms::PictureBox^  fcgpictureBoxX264;
    private: System::Windows::Forms::ComboBox^  fcgCXTune;
    private: System::Windows::Forms::ComboBox^  fcgCXPreset;
    private: System::Windows::Forms::ComboBox^  fcgCXProfile;
    private: System::Windows::Forms::Button^  fcgBTApplyPreset;
private: System::Windows::Forms::GroupBox^  fcgGroupBoxAspectRatio;

    private: System::Windows::Forms::NumericUpDown^  fcgNUAspectRatioX;
    private: System::Windows::Forms::ComboBox^  fcgCXAspectRatio;


    private: System::Windows::Forms::Label^  fcgLBSTATUS;
    private: System::Windows::Forms::Button^  fcgBTStatusFile;

    private: System::Windows::Forms::TextBox^  fcgTXStatusFile;
    private: System::Windows::Forms::Label^  fcgLBAspectRatio;



    private: System::Windows::Forms::ComboBox^  fcgCXLevel;

    private: System::Windows::Forms::NumericUpDown^  fcgNUAspectRatioY;
    private: System::Windows::Forms::GroupBox^  fcggroupBoxColorMatrix;
    private: System::Windows::Forms::Label^  fcgLBColorMatrix;
    private: System::Windows::Forms::Label^  fcgLBVideoFormat;



    private: System::Windows::Forms::Label^  fcgLBLevel;
    private: System::Windows::Forms::ComboBox^  fcgCXTransfer;
    private: System::Windows::Forms::ComboBox^  fcgCXColorPrim;
    private: System::Windows::Forms::ComboBox^  fcgCXColorMatrix;


    private: System::Windows::Forms::Label^  fcgLBTransfer;

    private: System::Windows::Forms::Label^  fcgLBColorPrim;

    private: System::Windows::Forms::CheckBox^  fcgCBBlurayCompat;

    private: System::Windows::Forms::CheckBox^  fcgCBAud;
    private: System::Windows::Forms::ComboBox^  fcgCXVideoFormat;











    private: System::Windows::Forms::GroupBox^  fcggroupBoxQP;
    private: System::Windows::Forms::Label^  fcgLBQpstep;
    private: System::Windows::Forms::Label^  fcgLBChromaQp;
    private: System::Windows::Forms::Label^  fcgLBQpmax;
    private: System::Windows::Forms::Label^  fcgLBQpmin;
    private: System::Windows::Forms::Label^  fcgLBQcomp;
    private: System::Windows::Forms::Label^  fcgLBPBRatio;
    private: System::Windows::Forms::Label^  fcgLBIPRatio;
    private: System::Windows::Forms::NumericUpDown^  fcgNUChromaQp;
    private: System::Windows::Forms::NumericUpDown^  fcgNUQpstep;
    private: System::Windows::Forms::NumericUpDown^  fcgNUQpmax;
    private: System::Windows::Forms::NumericUpDown^  fcgNUQpmin;
    private: System::Windows::Forms::NumericUpDown^  fcgNUQcomp;
    private: System::Windows::Forms::NumericUpDown^  fcgNUPBRatio;
    private: System::Windows::Forms::NumericUpDown^  fcgNUIPRatio;
    private: System::Windows::Forms::NumericUpDown^  fcgNURCLookahead;

    private: System::Windows::Forms::Label^  fcgLBRCLookahead;

    private: System::Windows::Forms::CheckBox^  fcgCBMBTree;
    private: System::Windows::Forms::GroupBox^  fcggroupBoxAQ;
    private: System::Windows::Forms::GroupBox^  fcggroupBoxPsyRd;


    private: System::Windows::Forms::GroupBox^  fcggroupBoxVbv;
    private: System::Windows::Forms::Label^  fcgLBVBVbuf;

    private: System::Windows::Forms::Label^  fcgLBVBVmax;
    private: System::Windows::Forms::NumericUpDown^  fcgNUVBVbuf;
    private: System::Windows::Forms::NumericUpDown^  fcgNUVBVmax;
    private: System::Windows::Forms::Label^  fcgLBAQMode;
    private: System::Windows::Forms::NumericUpDown^  fcgNUAQStrength;
    private: System::Windows::Forms::Label^  fcgLBAQStrength;
    private: System::Windows::Forms::ComboBox^  fcgCXAQMode;

    private: System::Windows::Forms::NumericUpDown^  fcgNUPsyTrellis;

    private: System::Windows::Forms::NumericUpDown^  fcgNUPsyRDO;

    private: System::Windows::Forms::Label^  fcgLBPsyTrellis;
    private: System::Windows::Forms::Label^  fcgLBPsyRDO;






































































































    private: System::Windows::Forms::Label^  fcgLBVBVafsWarning;

    private: System::Windows::Forms::GroupBox^  fcggroupBoxExSettings;
    private: System::Windows::Forms::CheckBox^  fcgCBAFSBitrateCorrection;


    private: System::Windows::Forms::CheckBox^  fcgCBAFS;
    private: System::Windows::Forms::Label^  fcgLBX264Priority;
    private: System::Windows::Forms::CheckBox^  fcgCBCheckKeyframes;

    private: System::Windows::Forms::CheckBox^  fcgCBAuoTcfileout;

    private: System::Windows::Forms::Button^  fcgBTCustomTempDir;
    private: System::Windows::Forms::TextBox^  fcgTXCustomTempDir;
    private: System::Windows::Forms::ComboBox^  fcgCXTempDir;
    private: System::Windows::Forms::ComboBox^  fcgCXX264Priority;
    private: System::Windows::Forms::GroupBox^  fcggroupBoxCmdEx;
    private: System::Windows::Forms::Button^  fcgBTCmdEx;
    private: System::Windows::Forms::TextBox^  fcgTXCmdEx;











    private: System::Windows::Forms::TabPage^  fcgtabPageMux;












    private: System::Windows::Forms::ComboBox^  fcgCXMP4CmdEx;

    private: System::Windows::Forms::Label^  fcgLBMP4CmdEx;
    private: System::Windows::Forms::CheckBox^  fcgCBMP4MuxerExt;
    private: System::Windows::Forms::Button^  fcgBTMP4BoxTempDir;
    private: System::Windows::Forms::TextBox^  fcgTXMP4BoxTempDir;


    private: System::Windows::Forms::ComboBox^  fcgCXMP4BoxTempDir;
    private: System::Windows::Forms::Label^  fcgLBMP4BoxTempDir;
    private: System::Windows::Forms::Button^  fcgBTTC2MP4Path;
    private: System::Windows::Forms::TextBox^  fcgTXTC2MP4Path;
    private: System::Windows::Forms::Button^  fcgBTMP4MuxerPath;
    private: System::Windows::Forms::TextBox^  fcgTXMP4MuxerPath;

    private: System::Windows::Forms::Label^  fcgLBTC2MP4Path;
    private: System::Windows::Forms::Label^  fcgLBMP4MuxerPath;


    private: System::Windows::Forms::Button^  fcgBTMKVMuxerPath;

    private: System::Windows::Forms::TextBox^  fcgTXMKVMuxerPath;

    private: System::Windows::Forms::Label^  fcgLBMKVMuxerPath;
    private: System::Windows::Forms::ComboBox^  fcgCXMKVCmdEx;
    private: System::Windows::Forms::Label^  fcgLBMKVMuxerCmdEx;
    private: System::Windows::Forms::CheckBox^  fcgCBMKVMuxerExt;
    private: System::Windows::Forms::ComboBox^  fcgCXMuxPriority;
    private: System::Windows::Forms::Label^  fcgLBMuxPriority;

    private: System::Windows::Forms::Label^  fcgLBVersionDate;

    private: System::Windows::Forms::Label^  fcgLBAutoNpass;
    private: System::Windows::Forms::Label^  fcgLBVersion;
    private: System::Windows::Forms::FolderBrowserDialog^  fcgfolderBrowserTemp;
    private: System::Windows::Forms::OpenFileDialog^  fcgOpenFileDialog;
    private: System::Windows::Forms::ContextMenuStrip^  fcgCSCQM;
    private: System::Windows::Forms::ToolStripMenuItem^  fcgCSFlat;
    private: System::Windows::Forms::ToolStripMenuItem^  fcgCSJvt;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator1;
    private: System::Windows::Forms::ToolStripMenuItem^  fcgCSCqmFile;
    private: System::Windows::Forms::Button^  fcgBTTBQualitySubtract;
    private: System::Windows::Forms::Button^  fcgBTTBQualityAdd;


private: System::Windows::Forms::ToolTip^  fcgTTX264;
private: System::Windows::Forms::ToolTip^  fcgTTEx;





private: System::Windows::Forms::ToolTip^  fcgTTX264Version;


private: System::Windows::Forms::CheckBox^  fcgCBNulOutCLI;
private: System::Windows::Forms::TabPage^  fcgtabPageX264Frame;

private: System::Windows::Forms::ComboBox^  fcgCXWeightP;




private: System::Windows::Forms::Label^  fcgLBWeightP;
private: System::Windows::Forms::CheckBox^  fcgCBDeblock;
private: System::Windows::Forms::GroupBox^  fcggroupBoxBframes;
private: System::Windows::Forms::ComboBox^  fcgCXBpyramid;
private: System::Windows::Forms::ComboBox^  fcgCXBAdpapt;
private: System::Windows::Forms::CheckBox^  fcgCBWeightB;
private: System::Windows::Forms::NumericUpDown^  fcgNUBBias;
private: System::Windows::Forms::NumericUpDown^  fcgNUBframes;
private: System::Windows::Forms::Label^  fcgLBWeightB;
private: System::Windows::Forms::Label^  fcgLBBpyramid;
private: System::Windows::Forms::Label^  fcgLBBBias;
private: System::Windows::Forms::Label^  fcgLBBAdapt;
private: System::Windows::Forms::Label^  fcgLBBframes;


private: System::Windows::Forms::CheckBox^  fcgCBCABAC;
private: System::Windows::Forms::GroupBox^  fcggroupBoxGOP;
private: System::Windows::Forms::Label^  fcgLBOpenGOP;
private: System::Windows::Forms::CheckBox^  fcgCBOpenGOP;
private: System::Windows::Forms::NumericUpDown^  fcgNUKeyint;
private: System::Windows::Forms::Label^  fcgLBKeyint;
private: System::Windows::Forms::NumericUpDown^  fcgNUMinKeyint;
private: System::Windows::Forms::Label^  fcgLBMinKeyint;
private: System::Windows::Forms::NumericUpDown^  fcgNUScenecut;
private: System::Windows::Forms::Label^  fcgLBScenecut;
private: System::Windows::Forms::GroupBox^  fcggroupBoxDeblock;
private: System::Windows::Forms::NumericUpDown^  fcgNUDeblockThreshold;
private: System::Windows::Forms::NumericUpDown^  fcgNUDeblockStrength;


private: System::Windows::Forms::Label^  fcgLBDeblockThreshold;
private: System::Windows::Forms::Label^  fcgLBDeblockStrength;
private: System::Windows::Forms::GroupBox^  fcggroupBoxMBTypes;
private: System::Windows::Forms::CheckBox^  fcgCBi4x4;
private: System::Windows::Forms::CheckBox^  fcgCBp4x4;
private: System::Windows::Forms::CheckBox^  fcgCBi8x8;
private: System::Windows::Forms::CheckBox^  fcgCBb8x8;
private: System::Windows::Forms::CheckBox^  fcgCBp8x8;
private: System::Windows::Forms::CheckBox^  fcgCB8x8dct;









private: System::Windows::Forms::GroupBox^  fcggroupBoxThreads;
private: System::Windows::Forms::CheckBox^  fcgCBSlicedThreads;
private: System::Windows::Forms::Label^  fcgLBThreads;
private: System::Windows::Forms::NumericUpDown^  fcgNUThreads;
private: System::Windows::Forms::Label^  fcgLBOutputCF;
private: System::Windows::Forms::ComboBox^  fcgCXOutputCsp;


private: System::Windows::Forms::NumericUpDown^  fcgNUSlices;
private: System::Windows::Forms::Label^  fcgLBSlices;
private: System::Windows::Forms::GroupBox^  fcggroupBoxX264Out;
private: System::Windows::Forms::CheckBox^  fcgCBSSIM;
private: System::Windows::Forms::CheckBox^  fcgCBPSNR;
private: System::Windows::Forms::ComboBox^  fcgCXLogLevel;
private: System::Windows::Forms::Label^  fcgLBLog;
private: System::Windows::Forms::Label^  fcgLBTimebase;
private: System::Windows::Forms::NumericUpDown^  fcgNUTimebaseDen;
private: System::Windows::Forms::NumericUpDown^  fcgNUTimebaseNum;
private: System::Windows::Forms::Button^  fcgBTTCIN;
private: System::Windows::Forms::TextBox^  fcgTXTCIN;
private: System::Windows::Forms::CheckBox^  fcgCBTimeBase;
private: System::Windows::Forms::CheckBox^  fcgCBTCIN;
private: System::Windows::Forms::GroupBox^  fcggroupBoxX264Other;
private: System::Windows::Forms::Button^  fcgBTMatrix;
private: System::Windows::Forms::TextBox^  fcgTXCQM;
private: System::Windows::Forms::ComboBox^  fcgCXTrellis;
private: System::Windows::Forms::Label^  fcgLBCQM;
private: System::Windows::Forms::Label^  fcgLBTrellis;
private: System::Windows::Forms::CheckBox^  fcgCBDctDecimate;
private: System::Windows::Forms::CheckBox^  fcgCBfastpskip;
private: System::Windows::Forms::GroupBox^  fcggroupBoxME;
private: System::Windows::Forms::CheckBox^  fcgCBMixedRef;
private: System::Windows::Forms::CheckBox^  fcgCBChromaME;
private: System::Windows::Forms::NumericUpDown^  fcgNURef;
private: System::Windows::Forms::NumericUpDown^  fcgNUMERange;
private: System::Windows::Forms::ComboBox^  fcgCXDirectME;
private: System::Windows::Forms::ComboBox^  fcgCXSubME;
private: System::Windows::Forms::ComboBox^  fcgCXME;
private: System::Windows::Forms::Label^  fcgLBMixedRef;
private: System::Windows::Forms::Label^  fcgLBChromaME;
private: System::Windows::Forms::Label^  fcgLBRef;
private: System::Windows::Forms::Label^  fcgLBDirectME;
private: System::Windows::Forms::Label^  fcgLBMERange;
private: System::Windows::Forms::Label^  fcgLBSubME;
private: System::Windows::Forms::Label^  fcgLBME;




private: System::Windows::Forms::Label^  fcgLBTempDir;
private: System::Windows::Forms::Label^  fcgLBInterlaced;
private: System::Windows::Forms::ComboBox^  fcgCXInterlaced;
private: System::Windows::Forms::ComboBox^  fcgCXNalHrd;
private: System::Windows::Forms::Label^  fcgLBNalHrd;
private: System::Windows::Forms::CheckBox^  fcgCBPicStruct;


private: System::Windows::Forms::Button^  fcgBTX264PathSub;

private: System::Windows::Forms::TextBox^  fcgTXX264PathSub;

private: System::Windows::Forms::Label^  fcgLBX264PathSub;








private: System::Windows::Forms::CheckBox^  fcgCBMP4MuxApple;


private: System::Windows::Forms::TabPage^  fcgtabPageBat;
private: System::Windows::Forms::CheckBox^  fcgCBRunBatBefore;
private: System::Windows::Forms::Label^  fcgLBBatBeforePath;



private: System::Windows::Forms::CheckBox^  fcgCBWaitForBatBefore;
private: System::Windows::Forms::Button^  fcgBTBatBeforePath;


private: System::Windows::Forms::TextBox^  fcgTXBatBeforePath;

private: System::Windows::Forms::ComboBox^  fcgCXInputRange;

private: System::Windows::Forms::Label^  fcgLBInputRange;
private: System::Windows::Forms::ContextMenuStrip^  fcgCSReplaceStrings;
private: System::Windows::Forms::CheckBox^  fcgCBAMPLimitBitrateUpper;


private: System::Windows::Forms::CheckBox^  fcgCBAMPLimitFileSize;


private: System::Windows::Forms::Panel^  fcgPNX264Mode;
private: System::Windows::Forms::Panel^  fcgPNBitrate;
private: System::Windows::Forms::NumericUpDown^  fcgNUAMPLimitBitrateUpper;

private: System::Windows::Forms::NumericUpDown^  fcgNUAMPLimitFileSize;
private: System::Windows::Forms::Panel^  fcgPNStatusFile;










private: System::Windows::Forms::Label^  fcgLBLookaheadThreads;
private: System::Windows::Forms::NumericUpDown^  fcgNULookaheadThreads;
private: System::Windows::Forms::Button^  fcgBTMP4RawPath;

private: System::Windows::Forms::TextBox^  fcgTXMP4RawPath;
private: System::Windows::Forms::Label^  fcgLBMP4RawPath;
private: System::Windows::Forms::ContextMenuStrip^  fcgCSExeFiles;
private: System::Windows::Forms::ToolStripMenuItem^  fcgTSExeFileshelp;


private: System::Windows::Forms::ComboBox^  fcgCXCmdExInsert;
private: System::Windows::Forms::LinkLabel^  fcgLBguiExBlog;
private: System::Windows::Forms::Panel^  fcgPNSeparator;
private: System::Windows::Forms::Button^  fcgBTBatAfterPath;


private: System::Windows::Forms::TextBox^  fcgTXBatAfterPath;

private: System::Windows::Forms::Label^  fcgLBBatAfterPath;

private: System::Windows::Forms::CheckBox^  fcgCBWaitForBatAfter;

private: System::Windows::Forms::CheckBox^  fcgCBRunBatAfter;
private: System::Windows::Forms::Label^  fcgLBBatAfterString;

private: System::Windows::Forms::Label^  fcgLBBatBeforeString;
private: System::Windows::Forms::CheckBox^  fcgCBInputAsLW48;
private: System::Windows::Forms::CheckBox^  fcgCBSetKeyframeAtChapter;
private: System::Windows::Forms::TabControl^  fcgtabControlAudio;
private: System::Windows::Forms::TabPage^  fcgtabPageAudioMain;
private: System::Windows::Forms::ComboBox^  fcgCXAudioDelayCut;
private: System::Windows::Forms::Label^  fcgLBAudioDelayCut;
private: System::Windows::Forms::Label^  fcgCBAudioEncTiming;
private: System::Windows::Forms::ComboBox^  fcgCXAudioEncTiming;
private: System::Windows::Forms::ComboBox^  fcgCXAudioTempDir;
private: System::Windows::Forms::TextBox^  fcgTXCustomAudioTempDir;
private: System::Windows::Forms::Button^  fcgBTCustomAudioTempDir;
private: System::Windows::Forms::CheckBox^  fcgCBAudioUsePipe;
private: System::Windows::Forms::Label^  fcgLBAudioBitrate;
private: System::Windows::Forms::NumericUpDown^  fcgNUAudioBitrate;
private: System::Windows::Forms::CheckBox^  fcgCBAudio2pass;
private: System::Windows::Forms::ComboBox^  fcgCXAudioEncMode;
private: System::Windows::Forms::Label^  fcgLBAudioEncMode;
private: System::Windows::Forms::Button^  fcgBTAudioEncoderPath;
private: System::Windows::Forms::TextBox^  fcgTXAudioEncoderPath;
private: System::Windows::Forms::Label^  fcgLBAudioEncoderPath;
private: System::Windows::Forms::CheckBox^  fcgCBAudioOnly;
private: System::Windows::Forms::CheckBox^  fcgCBFAWCheck;
private: System::Windows::Forms::ComboBox^  fcgCXAudioEncoder;
private: System::Windows::Forms::Label^  fcgLBAudioEncoder;
private: System::Windows::Forms::Label^  fcgLBAudioTemp;
private: System::Windows::Forms::TabPage^  fcgtabPageAudioOther;
private: System::Windows::Forms::Panel^  panel2;
private: System::Windows::Forms::Label^  fcgLBBatAfterAudioString;

private: System::Windows::Forms::Label^  fcgLBBatBeforeAudioString;
private: System::Windows::Forms::Button^  fcgBTBatAfterAudioPath;


private: System::Windows::Forms::TextBox^  fcgTXBatAfterAudioPath;

private: System::Windows::Forms::Label^  fcgLBBatAfterAudioPath;

private: System::Windows::Forms::CheckBox^  fcgCBRunBatAfterAudio;

private: System::Windows::Forms::Panel^  panel1;
private: System::Windows::Forms::Button^  fcgBTBatBeforeAudioPath;

private: System::Windows::Forms::TextBox^  fcgTXBatBeforeAudioPath;

private: System::Windows::Forms::Label^  fcgLBBatBeforeAudioPath;

private: System::Windows::Forms::CheckBox^  fcgCBRunBatBeforeAudio;

private: System::Windows::Forms::ComboBox^  fcgCXAudioPriority;
private: System::Windows::Forms::Label^  fcgLBAudioPriority;
private: System::Windows::Forms::NumericUpDown^  fcgNUAMPLimitBitrateLower;
private: System::Windows::Forms::CheckBox^  fcgCBAMPLimitBitrateLower;


private: System::Windows::Forms::Panel^  fcgPNHideTabControlVideo;
private: System::Windows::Forms::Panel^  fcgPNHideTabControlAudio;
private: System::Windows::Forms::Panel^  fcgPNHideTabControlMux;




private: System::Windows::Forms::ToolStripButton^  fcgTSBSave;
private: System::Windows::Forms::ToolStripButton^  fcgTSBSaveNew;
private: System::Windows::Forms::ToolStripButton^  fcgTSBDelete;
private: System::Windows::Forms::ToolStripSeparator^  fcgtoolStripSeparator1;
private: System::Windows::Forms::ToolStripDropDownButton^  fcgTSSettings;
private: System::Windows::Forms::ToolStripButton^  fcgTSBCMDOnly;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator3;
private: System::Windows::Forms::ToolStripDropDownButton^  fcgTSLanguage;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator5;
private: System::Windows::Forms::ToolStripButton^  fcgTSBBitrateCalc;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator2;
private: System::Windows::Forms::ToolStripButton^  fcgTSBOtherSettings;
private: System::Windows::Forms::ToolStripLabel^  fcgTSLSettingsNotes;
private: System::Windows::Forms::ToolStripTextBox^  fcgTSTSettingsNotes;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator4;
private: System::Windows::Forms::ToolStrip^  fcgtoolStripSettings;
private: System::Windows::Forms::Panel^  fcgPNHideToolStripBorder;































































    private: System::ComponentModel::IContainer^  components;




    private:
        /// <summary>
        /// 必要なデザイナ変数です。
        /// </summary>


#pragma region Windows Form Designer generated code
        /// <summary>
        /// デザイナ サポートに必要なメソッドです。このメソッドの内容を
        /// コード エディタで変更しないでください。
        /// </summary>
        void InitializeComponent(void)
        {
            this->components = (gcnew System::ComponentModel::Container());
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(frmConfig::typeid));
            this->fcgtabControlVideo = (gcnew System::Windows::Forms::TabControl());
            this->fcgtabPageX264Main = (gcnew System::Windows::Forms::TabPage());
            this->fcgPNStatusFile = (gcnew System::Windows::Forms::Panel());
            this->fcgLBSTATUS = (gcnew System::Windows::Forms::Label());
            this->fcgTXStatusFile = (gcnew System::Windows::Forms::TextBox());
            this->fcgBTStatusFile = (gcnew System::Windows::Forms::Button());
            this->fcgPNBitrate = (gcnew System::Windows::Forms::Panel());
            this->fcgLBQuality = (gcnew System::Windows::Forms::Label());
            this->fcgLBQualityLeft = (gcnew System::Windows::Forms::Label());
            this->fcgLBQualityRight = (gcnew System::Windows::Forms::Label());
            this->fcgTXQuality = (gcnew System::Windows::Forms::TextBox());
            this->fcgBTTBQualitySubtract = (gcnew System::Windows::Forms::Button());
            this->fcgBTTBQualityAdd = (gcnew System::Windows::Forms::Button());
            this->fcgTBQuality = (gcnew System::Windows::Forms::TrackBar());
            this->fcgPNX264Mode = (gcnew System::Windows::Forms::Panel());
            this->fcgNUAMPLimitBitrateLower = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCBAMPLimitBitrateLower = (gcnew System::Windows::Forms::CheckBox());
            this->fcgNUAMPLimitBitrateUpper = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUAMPLimitFileSize = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCXX264Mode = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCBNulOut = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBAMPLimitBitrateUpper = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBFastFirstPass = (gcnew System::Windows::Forms::CheckBox());
            this->fcgNUAutoNPass = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCBAMPLimitFileSize = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBAutoNpass = (gcnew System::Windows::Forms::Label());
            this->fcgCXNalHrd = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBNalHrd = (gcnew System::Windows::Forms::Label());
            this->fcgCBPicStruct = (gcnew System::Windows::Forms::CheckBox());
            this->fcggroupBoxX264Out = (gcnew System::Windows::Forms::GroupBox());
            this->fcgCBSSIM = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBPSNR = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCXLogLevel = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBLog = (gcnew System::Windows::Forms::Label());
            this->fcgNUSlices = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBSlices = (gcnew System::Windows::Forms::Label());
            this->fcgCXOutputCsp = (gcnew System::Windows::Forms::ComboBox());
            this->fcggroupBoxThreads = (gcnew System::Windows::Forms::GroupBox());
            this->fcgLBLookaheadThreads = (gcnew System::Windows::Forms::Label());
            this->fcgNULookaheadThreads = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCBSlicedThreads = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBThreads = (gcnew System::Windows::Forms::Label());
            this->fcgNUThreads = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCBUsehighbit = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBBlurayCompat = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBAud = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCXVideoFormat = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVideoFormat = (gcnew System::Windows::Forms::Label());
            this->fcgLBLevel = (gcnew System::Windows::Forms::Label());
            this->fcgCXLevel = (gcnew System::Windows::Forms::ComboBox());
            this->fcggroupBoxColorMatrix = (gcnew System::Windows::Forms::GroupBox());
            this->fcgCXInputRange = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBInputRange = (gcnew System::Windows::Forms::Label());
            this->fcgCXTransfer = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXColorPrim = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXColorMatrix = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBTransfer = (gcnew System::Windows::Forms::Label());
            this->fcgLBColorPrim = (gcnew System::Windows::Forms::Label());
            this->fcgLBColorMatrix = (gcnew System::Windows::Forms::Label());
            this->fcgGroupBoxAspectRatio = (gcnew System::Windows::Forms::GroupBox());
            this->fcgNUAspectRatioY = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBAspectRatio = (gcnew System::Windows::Forms::Label());
            this->fcgNUAspectRatioX = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCXAspectRatio = (gcnew System::Windows::Forms::ComboBox());
            this->fcgpictureBoxX264 = (gcnew System::Windows::Forms::PictureBox());
            this->fcgBTX264Path = (gcnew System::Windows::Forms::Button());
            this->fcgTXX264Path = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBX264Path = (gcnew System::Windows::Forms::Label());
            this->fcggroupBoxPreset = (gcnew System::Windows::Forms::GroupBox());
            this->fcgBTApplyPreset = (gcnew System::Windows::Forms::Button());
            this->fcgCXProfile = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXTune = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXPreset = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBProfile = (gcnew System::Windows::Forms::Label());
            this->fcgLBX264TUNE = (gcnew System::Windows::Forms::Label());
            this->fcgLBX264Preset = (gcnew System::Windows::Forms::Label());
            this->fcgLBOutputCF = (gcnew System::Windows::Forms::Label());
            this->fcgtabPageX264RC = (gcnew System::Windows::Forms::TabPage());
            this->fcgLBTimebase = (gcnew System::Windows::Forms::Label());
            this->fcgNUTimebaseDen = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUTimebaseNum = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgBTTCIN = (gcnew System::Windows::Forms::Button());
            this->fcgTXTCIN = (gcnew System::Windows::Forms::TextBox());
            this->fcgCBTimeBase = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBTCIN = (gcnew System::Windows::Forms::CheckBox());
            this->fcggroupBoxAQ = (gcnew System::Windows::Forms::GroupBox());
            this->fcgNUAQStrength = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBAQStrength = (gcnew System::Windows::Forms::Label());
            this->fcgCXAQMode = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBAQMode = (gcnew System::Windows::Forms::Label());
            this->fcggroupBoxPsyRd = (gcnew System::Windows::Forms::GroupBox());
            this->fcgNUPsyTrellis = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUPsyRDO = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBPsyTrellis = (gcnew System::Windows::Forms::Label());
            this->fcgLBPsyRDO = (gcnew System::Windows::Forms::Label());
            this->fcggroupBoxVbv = (gcnew System::Windows::Forms::GroupBox());
            this->fcgLBVBVafsWarning = (gcnew System::Windows::Forms::Label());
            this->fcgNUVBVbuf = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVBVmax = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVBVbuf = (gcnew System::Windows::Forms::Label());
            this->fcgLBVBVmax = (gcnew System::Windows::Forms::Label());
            this->fcgNURCLookahead = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBRCLookahead = (gcnew System::Windows::Forms::Label());
            this->fcgCBMBTree = (gcnew System::Windows::Forms::CheckBox());
            this->fcggroupBoxQP = (gcnew System::Windows::Forms::GroupBox());
            this->fcgNUChromaQp = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUQpstep = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUQpmax = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUQpmin = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUQcomp = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUPBRatio = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUIPRatio = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBQpstep = (gcnew System::Windows::Forms::Label());
            this->fcgLBChromaQp = (gcnew System::Windows::Forms::Label());
            this->fcgLBQpmax = (gcnew System::Windows::Forms::Label());
            this->fcgLBQpmin = (gcnew System::Windows::Forms::Label());
            this->fcgLBQcomp = (gcnew System::Windows::Forms::Label());
            this->fcgLBPBRatio = (gcnew System::Windows::Forms::Label());
            this->fcgLBIPRatio = (gcnew System::Windows::Forms::Label());
            this->fcgtabPageX264Frame = (gcnew System::Windows::Forms::TabPage());
            this->fcgCBDeblock = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCXInterlaced = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBInterlaced = (gcnew System::Windows::Forms::Label());
            this->fcggroupBoxX264Other = (gcnew System::Windows::Forms::GroupBox());
            this->fcgBTMatrix = (gcnew System::Windows::Forms::Button());
            this->fcgTXCQM = (gcnew System::Windows::Forms::TextBox());
            this->fcgCXTrellis = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBCQM = (gcnew System::Windows::Forms::Label());
            this->fcgLBTrellis = (gcnew System::Windows::Forms::Label());
            this->fcgCBDctDecimate = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBfastpskip = (gcnew System::Windows::Forms::CheckBox());
            this->fcggroupBoxME = (gcnew System::Windows::Forms::GroupBox());
            this->fcgCBMixedRef = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBChromaME = (gcnew System::Windows::Forms::CheckBox());
            this->fcgNURef = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUMERange = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCXDirectME = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXSubME = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXME = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBMixedRef = (gcnew System::Windows::Forms::Label());
            this->fcgLBChromaME = (gcnew System::Windows::Forms::Label());
            this->fcgLBRef = (gcnew System::Windows::Forms::Label());
            this->fcgLBDirectME = (gcnew System::Windows::Forms::Label());
            this->fcgLBMERange = (gcnew System::Windows::Forms::Label());
            this->fcgLBSubME = (gcnew System::Windows::Forms::Label());
            this->fcgLBME = (gcnew System::Windows::Forms::Label());
            this->fcggroupBoxMBTypes = (gcnew System::Windows::Forms::GroupBox());
            this->fcgCBi4x4 = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBp4x4 = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBi8x8 = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBb8x8 = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBp8x8 = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCB8x8dct = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCXWeightP = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBWeightP = (gcnew System::Windows::Forms::Label());
            this->fcggroupBoxBframes = (gcnew System::Windows::Forms::GroupBox());
            this->fcgCXBpyramid = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXBAdpapt = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCBWeightB = (gcnew System::Windows::Forms::CheckBox());
            this->fcgNUBBias = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUBframes = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBWeightB = (gcnew System::Windows::Forms::Label());
            this->fcgLBBpyramid = (gcnew System::Windows::Forms::Label());
            this->fcgLBBBias = (gcnew System::Windows::Forms::Label());
            this->fcgLBBAdapt = (gcnew System::Windows::Forms::Label());
            this->fcgLBBframes = (gcnew System::Windows::Forms::Label());
            this->fcgCBCABAC = (gcnew System::Windows::Forms::CheckBox());
            this->fcggroupBoxGOP = (gcnew System::Windows::Forms::GroupBox());
            this->fcgLBOpenGOP = (gcnew System::Windows::Forms::Label());
            this->fcgCBOpenGOP = (gcnew System::Windows::Forms::CheckBox());
            this->fcgNUKeyint = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBKeyint = (gcnew System::Windows::Forms::Label());
            this->fcgNUMinKeyint = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBMinKeyint = (gcnew System::Windows::Forms::Label());
            this->fcgNUScenecut = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBScenecut = (gcnew System::Windows::Forms::Label());
            this->fcggroupBoxDeblock = (gcnew System::Windows::Forms::GroupBox());
            this->fcgNUDeblockThreshold = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUDeblockStrength = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBDeblockThreshold = (gcnew System::Windows::Forms::Label());
            this->fcgLBDeblockStrength = (gcnew System::Windows::Forms::Label());
            this->fcgtabPageExSettings = (gcnew System::Windows::Forms::TabPage());
            this->fcgBTX264PathSub = (gcnew System::Windows::Forms::Button());
            this->fcgTXX264PathSub = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBX264PathSub = (gcnew System::Windows::Forms::Label());
            this->fcgLBTempDir = (gcnew System::Windows::Forms::Label());
            this->fcgBTCustomTempDir = (gcnew System::Windows::Forms::Button());
            this->fcggroupBoxCmdEx = (gcnew System::Windows::Forms::GroupBox());
            this->fcgCXCmdExInsert = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCBNulOutCLI = (gcnew System::Windows::Forms::CheckBox());
            this->fcgBTCmdEx = (gcnew System::Windows::Forms::Button());
            this->fcgTXCmdEx = (gcnew System::Windows::Forms::TextBox());
            this->fcgTXCustomTempDir = (gcnew System::Windows::Forms::TextBox());
            this->fcgCXTempDir = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXX264Priority = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBX264Priority = (gcnew System::Windows::Forms::Label());
            this->fcggroupBoxExSettings = (gcnew System::Windows::Forms::GroupBox());
            this->fcgCBSetKeyframeAtChapter = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBInputAsLW48 = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBCheckKeyframes = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBAuoTcfileout = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBAFSBitrateCorrection = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBAFS = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCSExeFiles = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
            this->fcgTSExeFileshelp = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->fcgtabControlMux = (gcnew System::Windows::Forms::TabControl());
            this->fcgtabPageMP4 = (gcnew System::Windows::Forms::TabPage());
            this->fcgCBMP4MuxApple = (gcnew System::Windows::Forms::CheckBox());
            this->fcgBTTC2MP4Path = (gcnew System::Windows::Forms::Button());
            this->fcgTXTC2MP4Path = (gcnew System::Windows::Forms::TextBox());
            this->fcgBTMP4MuxerPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXMP4MuxerPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBTC2MP4Path = (gcnew System::Windows::Forms::Label());
            this->fcgLBMP4MuxerPath = (gcnew System::Windows::Forms::Label());
            this->fcgCXMP4CmdEx = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBMP4CmdEx = (gcnew System::Windows::Forms::Label());
            this->fcgCBMP4MuxerExt = (gcnew System::Windows::Forms::CheckBox());
            this->fcgBTMP4RawPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXMP4RawPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBMP4RawPath = (gcnew System::Windows::Forms::Label());
            this->fcgBTMP4BoxTempDir = (gcnew System::Windows::Forms::Button());
            this->fcgTXMP4BoxTempDir = (gcnew System::Windows::Forms::TextBox());
            this->fcgCXMP4BoxTempDir = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBMP4BoxTempDir = (gcnew System::Windows::Forms::Label());
            this->fcgtabPageMKV = (gcnew System::Windows::Forms::TabPage());
            this->fcgBTMKVMuxerPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXMKVMuxerPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBMKVMuxerPath = (gcnew System::Windows::Forms::Label());
            this->fcgCXMKVCmdEx = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBMKVMuxerCmdEx = (gcnew System::Windows::Forms::Label());
            this->fcgCBMKVMuxerExt = (gcnew System::Windows::Forms::CheckBox());
            this->fcgtabPageMux = (gcnew System::Windows::Forms::TabPage());
            this->fcgCXMuxPriority = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBMuxPriority = (gcnew System::Windows::Forms::Label());
            this->fcgtabPageBat = (gcnew System::Windows::Forms::TabPage());
            this->fcgLBBatAfterString = (gcnew System::Windows::Forms::Label());
            this->fcgLBBatBeforeString = (gcnew System::Windows::Forms::Label());
            this->fcgBTBatAfterPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXBatAfterPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBBatAfterPath = (gcnew System::Windows::Forms::Label());
            this->fcgCBWaitForBatAfter = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBRunBatAfter = (gcnew System::Windows::Forms::CheckBox());
            this->fcgPNSeparator = (gcnew System::Windows::Forms::Panel());
            this->fcgBTBatBeforePath = (gcnew System::Windows::Forms::Button());
            this->fcgTXBatBeforePath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBBatBeforePath = (gcnew System::Windows::Forms::Label());
            this->fcgCBWaitForBatBefore = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBRunBatBefore = (gcnew System::Windows::Forms::CheckBox());
            this->fcgTXCmd = (gcnew System::Windows::Forms::TextBox());
            this->fcgBTCancel = (gcnew System::Windows::Forms::Button());
            this->fcgBTOK = (gcnew System::Windows::Forms::Button());
            this->fcgBTDefault = (gcnew System::Windows::Forms::Button());
            this->fcgLBVersionDate = (gcnew System::Windows::Forms::Label());
            this->fcgLBVersion = (gcnew System::Windows::Forms::Label());
            this->fcgfolderBrowserTemp = (gcnew System::Windows::Forms::FolderBrowserDialog());
            this->fcgOpenFileDialog = (gcnew System::Windows::Forms::OpenFileDialog());
            this->fcgCSCQM = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
            this->fcgCSFlat = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->fcgCSJvt = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->toolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->fcgCSCqmFile = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->fcgTTX264 = (gcnew System::Windows::Forms::ToolTip(this->components));
            this->fcgTTEx = (gcnew System::Windows::Forms::ToolTip(this->components));
            this->fcgTTX264Version = (gcnew System::Windows::Forms::ToolTip(this->components));
            this->fcgCSReplaceStrings = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
            this->fcgLBguiExBlog = (gcnew System::Windows::Forms::LinkLabel());
            this->fcgtabControlAudio = (gcnew System::Windows::Forms::TabControl());
            this->fcgtabPageAudioMain = (gcnew System::Windows::Forms::TabPage());
            this->fcgCXAudioDelayCut = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBAudioDelayCut = (gcnew System::Windows::Forms::Label());
            this->fcgCBAudioEncTiming = (gcnew System::Windows::Forms::Label());
            this->fcgCXAudioEncTiming = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXAudioTempDir = (gcnew System::Windows::Forms::ComboBox());
            this->fcgTXCustomAudioTempDir = (gcnew System::Windows::Forms::TextBox());
            this->fcgBTCustomAudioTempDir = (gcnew System::Windows::Forms::Button());
            this->fcgCBAudioUsePipe = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBAudioBitrate = (gcnew System::Windows::Forms::Label());
            this->fcgNUAudioBitrate = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCBAudio2pass = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCXAudioEncMode = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBAudioEncMode = (gcnew System::Windows::Forms::Label());
            this->fcgBTAudioEncoderPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXAudioEncoderPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBAudioEncoderPath = (gcnew System::Windows::Forms::Label());
            this->fcgCBAudioOnly = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBFAWCheck = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCXAudioEncoder = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBAudioEncoder = (gcnew System::Windows::Forms::Label());
            this->fcgLBAudioTemp = (gcnew System::Windows::Forms::Label());
            this->fcgtabPageAudioOther = (gcnew System::Windows::Forms::TabPage());
            this->panel2 = (gcnew System::Windows::Forms::Panel());
            this->fcgLBBatAfterAudioString = (gcnew System::Windows::Forms::Label());
            this->fcgLBBatBeforeAudioString = (gcnew System::Windows::Forms::Label());
            this->fcgBTBatAfterAudioPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXBatAfterAudioPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBBatAfterAudioPath = (gcnew System::Windows::Forms::Label());
            this->fcgCBRunBatAfterAudio = (gcnew System::Windows::Forms::CheckBox());
            this->panel1 = (gcnew System::Windows::Forms::Panel());
            this->fcgBTBatBeforeAudioPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXBatBeforeAudioPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBBatBeforeAudioPath = (gcnew System::Windows::Forms::Label());
            this->fcgCBRunBatBeforeAudio = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCXAudioPriority = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBAudioPriority = (gcnew System::Windows::Forms::Label());
            this->fcgPNHideTabControlVideo = (gcnew System::Windows::Forms::Panel());
            this->fcgPNHideTabControlAudio = (gcnew System::Windows::Forms::Panel());
            this->fcgPNHideTabControlMux = (gcnew System::Windows::Forms::Panel());
            this->fcgTSBSave = (gcnew System::Windows::Forms::ToolStripButton());
            this->fcgTSBSaveNew = (gcnew System::Windows::Forms::ToolStripButton());
            this->fcgTSBDelete = (gcnew System::Windows::Forms::ToolStripButton());
            this->fcgtoolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->fcgTSSettings = (gcnew System::Windows::Forms::ToolStripDropDownButton());
            this->fcgTSBCMDOnly = (gcnew System::Windows::Forms::ToolStripButton());
            this->toolStripSeparator3 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->fcgTSLanguage = (gcnew System::Windows::Forms::ToolStripDropDownButton());
            this->toolStripSeparator5 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->fcgTSBBitrateCalc = (gcnew System::Windows::Forms::ToolStripButton());
            this->toolStripSeparator2 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->fcgTSBOtherSettings = (gcnew System::Windows::Forms::ToolStripButton());
            this->fcgTSLSettingsNotes = (gcnew System::Windows::Forms::ToolStripLabel());
            this->fcgTSTSettingsNotes = (gcnew System::Windows::Forms::ToolStripTextBox());
            this->toolStripSeparator4 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->fcgtoolStripSettings = (gcnew System::Windows::Forms::ToolStrip());
            this->fcgPNHideToolStripBorder = (gcnew System::Windows::Forms::Panel());
            this->fcgtabControlVideo->SuspendLayout();
            this->fcgtabPageX264Main->SuspendLayout();
            this->fcgPNStatusFile->SuspendLayout();
            this->fcgPNBitrate->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBQuality))->BeginInit();
            this->fcgPNX264Mode->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAMPLimitBitrateLower))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAMPLimitBitrateUpper))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAMPLimitFileSize))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAutoNPass))->BeginInit();
            this->fcggroupBoxX264Out->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUSlices))->BeginInit();
            this->fcggroupBoxThreads->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNULookaheadThreads))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUThreads))->BeginInit();
            this->fcggroupBoxColorMatrix->SuspendLayout();
            this->fcgGroupBoxAspectRatio->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAspectRatioY))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAspectRatioX))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgpictureBoxX264))->BeginInit();
            this->fcggroupBoxPreset->SuspendLayout();
            this->fcgtabPageX264RC->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUTimebaseDen))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUTimebaseNum))->BeginInit();
            this->fcggroupBoxAQ->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAQStrength))->BeginInit();
            this->fcggroupBoxPsyRd->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUPsyTrellis))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUPsyRDO))->BeginInit();
            this->fcggroupBoxVbv->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVBVbuf))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVBVmax))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNURCLookahead))->BeginInit();
            this->fcggroupBoxQP->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUChromaQp))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQpstep))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQpmax))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQpmin))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQcomp))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUPBRatio))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUIPRatio))->BeginInit();
            this->fcgtabPageX264Frame->SuspendLayout();
            this->fcggroupBoxX264Other->SuspendLayout();
            this->fcggroupBoxME->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNURef))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUMERange))->BeginInit();
            this->fcggroupBoxMBTypes->SuspendLayout();
            this->fcggroupBoxBframes->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUBBias))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUBframes))->BeginInit();
            this->fcggroupBoxGOP->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUKeyint))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUMinKeyint))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUScenecut))->BeginInit();
            this->fcggroupBoxDeblock->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUDeblockThreshold))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUDeblockStrength))->BeginInit();
            this->fcgtabPageExSettings->SuspendLayout();
            this->fcggroupBoxCmdEx->SuspendLayout();
            this->fcggroupBoxExSettings->SuspendLayout();
            this->fcgCSExeFiles->SuspendLayout();
            this->fcgtabControlMux->SuspendLayout();
            this->fcgtabPageMP4->SuspendLayout();
            this->fcgtabPageMKV->SuspendLayout();
            this->fcgtabPageMux->SuspendLayout();
            this->fcgtabPageBat->SuspendLayout();
            this->fcgCSCQM->SuspendLayout();
            this->fcgtabControlAudio->SuspendLayout();
            this->fcgtabPageAudioMain->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAudioBitrate))->BeginInit();
            this->fcgtabPageAudioOther->SuspendLayout();
            this->fcgPNHideTabControlVideo->SuspendLayout();
            this->fcgPNHideTabControlAudio->SuspendLayout();
            this->fcgPNHideTabControlMux->SuspendLayout();
            this->fcgtoolStripSettings->SuspendLayout();
            this->SuspendLayout();
            // 
            // fcgtabControlVideo
            // 
            this->fcgtabControlVideo->Controls->Add(this->fcgtabPageX264Main);
            this->fcgtabControlVideo->Controls->Add(this->fcgtabPageX264RC);
            this->fcgtabControlVideo->Controls->Add(this->fcgtabPageX264Frame);
            this->fcgtabControlVideo->Controls->Add(this->fcgtabPageExSettings);
            this->fcgtabControlVideo->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgtabControlVideo->Location = System::Drawing::Point(2, 2);
            this->fcgtabControlVideo->Name = L"fcgtabControlVideo";
            this->fcgtabControlVideo->SelectedIndex = 0;
            this->fcgtabControlVideo->Size = System::Drawing::Size(616, 518);
            this->fcgtabControlVideo->TabIndex = 0;
            // 
            // fcgtabPageX264Main
            // 
            this->fcgtabPageX264Main->Controls->Add(this->fcgPNStatusFile);
            this->fcgtabPageX264Main->Controls->Add(this->fcgPNBitrate);
            this->fcgtabPageX264Main->Controls->Add(this->fcgPNX264Mode);
            this->fcgtabPageX264Main->Controls->Add(this->fcgCXNalHrd);
            this->fcgtabPageX264Main->Controls->Add(this->fcgLBNalHrd);
            this->fcgtabPageX264Main->Controls->Add(this->fcgCBPicStruct);
            this->fcgtabPageX264Main->Controls->Add(this->fcggroupBoxX264Out);
            this->fcgtabPageX264Main->Controls->Add(this->fcgNUSlices);
            this->fcgtabPageX264Main->Controls->Add(this->fcgLBSlices);
            this->fcgtabPageX264Main->Controls->Add(this->fcgCXOutputCsp);
            this->fcgtabPageX264Main->Controls->Add(this->fcggroupBoxThreads);
            this->fcgtabPageX264Main->Controls->Add(this->fcgCBUsehighbit);
            this->fcgtabPageX264Main->Controls->Add(this->fcgCBBlurayCompat);
            this->fcgtabPageX264Main->Controls->Add(this->fcgCBAud);
            this->fcgtabPageX264Main->Controls->Add(this->fcgCXVideoFormat);
            this->fcgtabPageX264Main->Controls->Add(this->fcgLBVideoFormat);
            this->fcgtabPageX264Main->Controls->Add(this->fcgLBLevel);
            this->fcgtabPageX264Main->Controls->Add(this->fcgCXLevel);
            this->fcgtabPageX264Main->Controls->Add(this->fcggroupBoxColorMatrix);
            this->fcgtabPageX264Main->Controls->Add(this->fcgGroupBoxAspectRatio);
            this->fcgtabPageX264Main->Controls->Add(this->fcgpictureBoxX264);
            this->fcgtabPageX264Main->Controls->Add(this->fcgBTX264Path);
            this->fcgtabPageX264Main->Controls->Add(this->fcgTXX264Path);
            this->fcgtabPageX264Main->Controls->Add(this->fcgLBX264Path);
            this->fcgtabPageX264Main->Controls->Add(this->fcggroupBoxPreset);
            this->fcgtabPageX264Main->Controls->Add(this->fcgLBOutputCF);
            this->fcgtabPageX264Main->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageX264Main->Name = L"fcgtabPageX264Main";
            this->fcgtabPageX264Main->Padding = System::Windows::Forms::Padding(3);
            this->fcgtabPageX264Main->Size = System::Drawing::Size(608, 491);
            this->fcgtabPageX264Main->TabIndex = 0;
            this->fcgtabPageX264Main->Text = L" x264 ";
            this->fcgtabPageX264Main->UseVisualStyleBackColor = true;
            // 
            // fcgPNStatusFile
            // 
            this->fcgPNStatusFile->Controls->Add(this->fcgLBSTATUS);
            this->fcgPNStatusFile->Controls->Add(this->fcgTXStatusFile);
            this->fcgPNStatusFile->Controls->Add(this->fcgBTStatusFile);
            this->fcgPNStatusFile->Location = System::Drawing::Point(5, 291);
            this->fcgPNStatusFile->Name = L"fcgPNStatusFile";
            this->fcgPNStatusFile->Size = System::Drawing::Size(376, 32);
            this->fcgPNStatusFile->TabIndex = 49;
            // 
            // fcgLBSTATUS
            // 
            this->fcgLBSTATUS->AutoSize = true;
            this->fcgLBSTATUS->Location = System::Drawing::Point(-1, 10);
            this->fcgLBSTATUS->Name = L"fcgLBSTATUS";
            this->fcgLBSTATUS->Size = System::Drawing::Size(78, 14);
            this->fcgLBSTATUS->TabIndex = 19;
            this->fcgLBSTATUS->Text = L"ステータスファイル";
            // 
            // fcgTXStatusFile
            // 
            this->fcgTXStatusFile->Location = System::Drawing::Point(94, 7);
            this->fcgTXStatusFile->Name = L"fcgTXStatusFile";
            this->fcgTXStatusFile->Size = System::Drawing::Size(253, 21);
            this->fcgTXStatusFile->TabIndex = 12;
            this->fcgTXStatusFile->Tag = L"reCmd";
            // 
            // fcgBTStatusFile
            // 
            this->fcgBTStatusFile->Location = System::Drawing::Point(347, 6);
            this->fcgBTStatusFile->Name = L"fcgBTStatusFile";
            this->fcgBTStatusFile->Size = System::Drawing::Size(27, 22);
            this->fcgBTStatusFile->TabIndex = 13;
            this->fcgBTStatusFile->Text = L"...";
            this->fcgBTStatusFile->UseVisualStyleBackColor = true;
            this->fcgBTStatusFile->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTStatusFile_Click);
            // 
            // fcgPNBitrate
            // 
            this->fcgPNBitrate->Controls->Add(this->fcgLBQuality);
            this->fcgPNBitrate->Controls->Add(this->fcgLBQualityLeft);
            this->fcgPNBitrate->Controls->Add(this->fcgLBQualityRight);
            this->fcgPNBitrate->Controls->Add(this->fcgTXQuality);
            this->fcgPNBitrate->Controls->Add(this->fcgBTTBQualitySubtract);
            this->fcgPNBitrate->Controls->Add(this->fcgBTTBQualityAdd);
            this->fcgPNBitrate->Controls->Add(this->fcgTBQuality);
            this->fcgPNBitrate->Location = System::Drawing::Point(5, 211);
            this->fcgPNBitrate->Name = L"fcgPNBitrate";
            this->fcgPNBitrate->Size = System::Drawing::Size(376, 84);
            this->fcgPNBitrate->TabIndex = 48;
            // 
            // fcgLBQuality
            // 
            this->fcgLBQuality->AutoSize = true;
            this->fcgLBQuality->Location = System::Drawing::Point(3, 8);
            this->fcgLBQuality->Name = L"fcgLBQuality";
            this->fcgLBQuality->Size = System::Drawing::Size(89, 14);
            this->fcgLBQuality->TabIndex = 14;
            this->fcgLBQuality->Text = L"ビットレート(kbps)";
            // 
            // fcgLBQualityLeft
            // 
            this->fcgLBQualityLeft->AutoSize = true;
            this->fcgLBQualityLeft->Location = System::Drawing::Point(6, 62);
            this->fcgLBQualityLeft->Name = L"fcgLBQualityLeft";
            this->fcgLBQualityLeft->Size = System::Drawing::Size(40, 14);
            this->fcgLBQualityLeft->TabIndex = 12;
            this->fcgLBQualityLeft->Text = L"低画質";
            // 
            // fcgLBQualityRight
            // 
            this->fcgLBQualityRight->AutoSize = true;
            this->fcgLBQualityRight->Location = System::Drawing::Point(322, 62);
            this->fcgLBQualityRight->Name = L"fcgLBQualityRight";
            this->fcgLBQualityRight->Size = System::Drawing::Size(40, 14);
            this->fcgLBQualityRight->TabIndex = 13;
            this->fcgLBQualityRight->Text = L"高画質";
            // 
            // fcgTXQuality
            // 
            this->fcgTXQuality->Location = System::Drawing::Point(290, 8);
            this->fcgTXQuality->Name = L"fcgTXQuality";
            this->fcgTXQuality->Size = System::Drawing::Size(76, 21);
            this->fcgTXQuality->TabIndex = 8;
            this->fcgTXQuality->Tag = L"reCmd";
            this->fcgTXQuality->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgTXQuality->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXQuality_TextChanged);
            this->fcgTXQuality->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXQuality_Enter);
            this->fcgTXQuality->Validating += gcnew System::ComponentModel::CancelEventHandler(this, &frmConfig::fcgTXQuality_Validating);
            // 
            // fcgBTTBQualitySubtract
            // 
            this->fcgBTTBQualitySubtract->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgBTTBQualitySubtract.Image")));
            this->fcgBTTBQualitySubtract->Location = System::Drawing::Point(0, 33);
            this->fcgBTTBQualitySubtract->Name = L"fcgBTTBQualitySubtract";
            this->fcgBTTBQualitySubtract->Size = System::Drawing::Size(21, 23);
            this->fcgBTTBQualitySubtract->TabIndex = 9;
            this->fcgBTTBQualitySubtract->UseVisualStyleBackColor = true;
            this->fcgBTTBQualitySubtract->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTTBQualitySubtract_Click);
            this->fcgBTTBQualitySubtract->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &frmConfig::fcgBTTBQualitySubtract_MouseDown);
            this->fcgBTTBQualitySubtract->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &frmConfig::fcgBTTBQualitySubtract_MouseUp);
            // 
            // fcgBTTBQualityAdd
            // 
            this->fcgBTTBQualityAdd->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgBTTBQualityAdd.Image")));
            this->fcgBTTBQualityAdd->Location = System::Drawing::Point(352, 34);
            this->fcgBTTBQualityAdd->Name = L"fcgBTTBQualityAdd";
            this->fcgBTTBQualityAdd->Size = System::Drawing::Size(21, 23);
            this->fcgBTTBQualityAdd->TabIndex = 11;
            this->fcgBTTBQualityAdd->UseVisualStyleBackColor = true;
            this->fcgBTTBQualityAdd->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTTBQualityAdd_Click);
            this->fcgBTTBQualityAdd->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &frmConfig::fcgBTTBQualityAdd_MouseDown);
            this->fcgBTTBQualityAdd->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &frmConfig::fcgBTTBQualityAdd_MouseUp);
            // 
            // fcgTBQuality
            // 
            this->fcgTBQuality->AutoSize = false;
            this->fcgTBQuality->BackColor = System::Drawing::SystemColors::Window;
            this->fcgTBQuality->Location = System::Drawing::Point(15, 35);
            this->fcgTBQuality->Maximum = 54000;
            this->fcgTBQuality->Name = L"fcgTBQuality";
            this->fcgTBQuality->Size = System::Drawing::Size(339, 25);
            this->fcgTBQuality->TabIndex = 10;
            this->fcgTBQuality->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBQuality->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBQuality_Scroll);
            this->fcgTBQuality->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgTBQuality_ValueChanged);
            // 
            // fcgPNX264Mode
            // 
            this->fcgPNX264Mode->Controls->Add(this->fcgNUAMPLimitBitrateLower);
            this->fcgPNX264Mode->Controls->Add(this->fcgCBAMPLimitBitrateLower);
            this->fcgPNX264Mode->Controls->Add(this->fcgNUAMPLimitBitrateUpper);
            this->fcgPNX264Mode->Controls->Add(this->fcgNUAMPLimitFileSize);
            this->fcgPNX264Mode->Controls->Add(this->fcgCXX264Mode);
            this->fcgPNX264Mode->Controls->Add(this->fcgCBNulOut);
            this->fcgPNX264Mode->Controls->Add(this->fcgCBAMPLimitBitrateUpper);
            this->fcgPNX264Mode->Controls->Add(this->fcgCBFastFirstPass);
            this->fcgPNX264Mode->Controls->Add(this->fcgNUAutoNPass);
            this->fcgPNX264Mode->Controls->Add(this->fcgCBAMPLimitFileSize);
            this->fcgPNX264Mode->Controls->Add(this->fcgLBAutoNpass);
            this->fcgPNX264Mode->Location = System::Drawing::Point(5, 67);
            this->fcgPNX264Mode->Name = L"fcgPNX264Mode";
            this->fcgPNX264Mode->Size = System::Drawing::Size(376, 146);
            this->fcgPNX264Mode->TabIndex = 47;
            // 
            // fcgNUAMPLimitBitrateLower
            // 
            this->fcgNUAMPLimitBitrateLower->DecimalPlaces = 1;
            this->fcgNUAMPLimitBitrateLower->Location = System::Drawing::Point(290, 119);
            this->fcgNUAMPLimitBitrateLower->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 128000, 0, 0, 0 });
            this->fcgNUAMPLimitBitrateLower->Name = L"fcgNUAMPLimitBitrateLower";
            this->fcgNUAMPLimitBitrateLower->Size = System::Drawing::Size(78, 21);
            this->fcgNUAMPLimitBitrateLower->TabIndex = 49;
            this->fcgNUAMPLimitBitrateLower->Tag = L"chValue";
            this->fcgNUAMPLimitBitrateLower->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCBAMPLimitBitrateLower
            // 
            this->fcgCBAMPLimitBitrateLower->AutoSize = true;
            this->fcgCBAMPLimitBitrateLower->Location = System::Drawing::Point(30, 120);
            this->fcgCBAMPLimitBitrateLower->Name = L"fcgCBAMPLimitBitrateLower";
            this->fcgCBAMPLimitBitrateLower->Size = System::Drawing::Size(221, 18);
            this->fcgCBAMPLimitBitrateLower->TabIndex = 48;
            this->fcgCBAMPLimitBitrateLower->Tag = L"chValue";
            this->fcgCBAMPLimitBitrateLower->Text = L"下限ファイルビットレート(映像+音声, kbps)";
            this->fcgCBAMPLimitBitrateLower->UseVisualStyleBackColor = true;
            // 
            // fcgNUAMPLimitBitrateUpper
            // 
            this->fcgNUAMPLimitBitrateUpper->DecimalPlaces = 1;
            this->fcgNUAMPLimitBitrateUpper->Location = System::Drawing::Point(290, 94);
            this->fcgNUAMPLimitBitrateUpper->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65536000, 0, 0, 0 });
            this->fcgNUAMPLimitBitrateUpper->Name = L"fcgNUAMPLimitBitrateUpper";
            this->fcgNUAMPLimitBitrateUpper->Size = System::Drawing::Size(78, 21);
            this->fcgNUAMPLimitBitrateUpper->TabIndex = 47;
            this->fcgNUAMPLimitBitrateUpper->Tag = L"chValue";
            this->fcgNUAMPLimitBitrateUpper->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUAMPLimitFileSize
            // 
            this->fcgNUAMPLimitFileSize->DecimalPlaces = 1;
            this->fcgNUAMPLimitFileSize->Location = System::Drawing::Point(290, 69);
            this->fcgNUAMPLimitFileSize->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1048576, 0, 0, 0 });
            this->fcgNUAMPLimitFileSize->Name = L"fcgNUAMPLimitFileSize";
            this->fcgNUAMPLimitFileSize->Size = System::Drawing::Size(78, 21);
            this->fcgNUAMPLimitFileSize->TabIndex = 46;
            this->fcgNUAMPLimitFileSize->Tag = L"chValue";
            this->fcgNUAMPLimitFileSize->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCXX264Mode
            // 
            this->fcgCXX264Mode->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXX264Mode->FormattingEnabled = true;
            this->fcgCXX264Mode->Location = System::Drawing::Point(6, 7);
            this->fcgCXX264Mode->Name = L"fcgCXX264Mode";
            this->fcgCXX264Mode->Size = System::Drawing::Size(362, 22);
            this->fcgCXX264Mode->TabIndex = 4;
            this->fcgCXX264Mode->Tag = L"reCmd";
            this->fcgCXX264Mode->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXX264Mode_SelectedIndexChanged);
            // 
            // fcgCBNulOut
            // 
            this->fcgCBNulOut->AutoSize = true;
            this->fcgCBNulOut->Location = System::Drawing::Point(6, 39);
            this->fcgCBNulOut->Name = L"fcgCBNulOut";
            this->fcgCBNulOut->Size = System::Drawing::Size(65, 18);
            this->fcgCBNulOut->TabIndex = 5;
            this->fcgCBNulOut->Tag = L"reCmd";
            this->fcgCBNulOut->Text = L"nul出力";
            this->fcgCBNulOut->UseVisualStyleBackColor = true;
            this->fcgCBNulOut->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCBNulOut_CheckedChanged);
            // 
            // fcgCBAMPLimitBitrateUpper
            // 
            this->fcgCBAMPLimitBitrateUpper->AutoSize = true;
            this->fcgCBAMPLimitBitrateUpper->Location = System::Drawing::Point(30, 95);
            this->fcgCBAMPLimitBitrateUpper->Name = L"fcgCBAMPLimitBitrateUpper";
            this->fcgCBAMPLimitBitrateUpper->Size = System::Drawing::Size(221, 18);
            this->fcgCBAMPLimitBitrateUpper->TabIndex = 45;
            this->fcgCBAMPLimitBitrateUpper->Tag = L"chValue";
            this->fcgCBAMPLimitBitrateUpper->Text = L"上限ファイルビットレート(映像+音声, kbps)";
            this->fcgCBAMPLimitBitrateUpper->UseVisualStyleBackColor = true;
            // 
            // fcgCBFastFirstPass
            // 
            this->fcgCBFastFirstPass->AutoSize = true;
            this->fcgCBFastFirstPass->Location = System::Drawing::Point(86, 39);
            this->fcgCBFastFirstPass->Name = L"fcgCBFastFirstPass";
            this->fcgCBFastFirstPass->Size = System::Drawing::Size(101, 18);
            this->fcgCBFastFirstPass->TabIndex = 6;
            this->fcgCBFastFirstPass->Tag = L"reCmd";
            this->fcgCBFastFirstPass->Text = L"高速(1st pass)";
            this->fcgCBFastFirstPass->UseVisualStyleBackColor = true;
            this->fcgCBFastFirstPass->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCBSlowFirstPass_CheckedChanged);
            // 
            // fcgNUAutoNPass
            // 
            this->fcgNUAutoNPass->Location = System::Drawing::Point(313, 38);
            this->fcgNUAutoNPass->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2, 0, 0, 0 });
            this->fcgNUAutoNPass->Name = L"fcgNUAutoNPass";
            this->fcgNUAutoNPass->Size = System::Drawing::Size(55, 21);
            this->fcgNUAutoNPass->TabIndex = 7;
            this->fcgNUAutoNPass->Tag = L"reCmd";
            this->fcgNUAutoNPass->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUAutoNPass->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2, 0, 0, 0 });
            // 
            // fcgCBAMPLimitFileSize
            // 
            this->fcgCBAMPLimitFileSize->AutoSize = true;
            this->fcgCBAMPLimitFileSize->Location = System::Drawing::Point(30, 70);
            this->fcgCBAMPLimitFileSize->Name = L"fcgCBAMPLimitFileSize";
            this->fcgCBAMPLimitFileSize->Size = System::Drawing::Size(191, 18);
            this->fcgCBAMPLimitFileSize->TabIndex = 43;
            this->fcgCBAMPLimitFileSize->Tag = L"chValue";
            this->fcgCBAMPLimitFileSize->Text = L"上限ファイルサイズ(映像+音声, MB)";
            this->fcgCBAMPLimitFileSize->UseVisualStyleBackColor = true;
            // 
            // fcgLBAutoNpass
            // 
            this->fcgLBAutoNpass->AutoSize = true;
            this->fcgLBAutoNpass->Location = System::Drawing::Point(218, 40);
            this->fcgLBAutoNpass->Name = L"fcgLBAutoNpass";
            this->fcgLBAutoNpass->Size = System::Drawing::Size(83, 14);
            this->fcgLBAutoNpass->TabIndex = 34;
            this->fcgLBAutoNpass->Text = L"自動マルチパス数";
            // 
            // fcgCXNalHrd
            // 
            this->fcgCXNalHrd->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXNalHrd->FormattingEnabled = true;
            this->fcgCXNalHrd->Location = System::Drawing::Point(495, 252);
            this->fcgCXNalHrd->Name = L"fcgCXNalHrd";
            this->fcgCXNalHrd->Size = System::Drawing::Size(95, 22);
            this->fcgCXNalHrd->TabIndex = 26;
            this->fcgCXNalHrd->Tag = L"reCmd";
            // 
            // fcgLBNalHrd
            // 
            this->fcgLBNalHrd->AutoSize = true;
            this->fcgLBNalHrd->Location = System::Drawing::Point(433, 255);
            this->fcgLBNalHrd->Name = L"fcgLBNalHrd";
            this->fcgLBNalHrd->Size = System::Drawing::Size(47, 14);
            this->fcgLBNalHrd->TabIndex = 42;
            this->fcgLBNalHrd->Text = L"nal-hrd";
            // 
            // fcgCBPicStruct
            // 
            this->fcgCBPicStruct->AutoSize = true;
            this->fcgCBPicStruct->Location = System::Drawing::Point(419, 214);
            this->fcgCBPicStruct->Name = L"fcgCBPicStruct";
            this->fcgCBPicStruct->Size = System::Drawing::Size(78, 18);
            this->fcgCBPicStruct->TabIndex = 25;
            this->fcgCBPicStruct->Tag = L"reCmd";
            this->fcgCBPicStruct->Text = L"pic-struct";
            this->fcgCBPicStruct->UseVisualStyleBackColor = true;
            // 
            // fcggroupBoxX264Out
            // 
            this->fcggroupBoxX264Out->Controls->Add(this->fcgCBSSIM);
            this->fcggroupBoxX264Out->Controls->Add(this->fcgCBPSNR);
            this->fcggroupBoxX264Out->Controls->Add(this->fcgCXLogLevel);
            this->fcggroupBoxX264Out->Controls->Add(this->fcgLBLog);
            this->fcggroupBoxX264Out->Location = System::Drawing::Point(196, 423);
            this->fcggroupBoxX264Out->Name = L"fcggroupBoxX264Out";
            this->fcggroupBoxX264Out->Size = System::Drawing::Size(177, 65);
            this->fcggroupBoxX264Out->TabIndex = 16;
            this->fcggroupBoxX264Out->TabStop = false;
            // 
            // fcgCBSSIM
            // 
            this->fcgCBSSIM->AutoSize = true;
            this->fcgCBSSIM->Location = System::Drawing::Point(90, 41);
            this->fcgCBSSIM->Name = L"fcgCBSSIM";
            this->fcgCBSSIM->Size = System::Drawing::Size(53, 18);
            this->fcgCBSSIM->TabIndex = 2;
            this->fcgCBSSIM->Tag = L"reCmd";
            this->fcgCBSSIM->Text = L"SSIM";
            this->fcgCBSSIM->UseVisualStyleBackColor = true;
            // 
            // fcgCBPSNR
            // 
            this->fcgCBPSNR->AutoSize = true;
            this->fcgCBPSNR->Location = System::Drawing::Point(5, 41);
            this->fcgCBPSNR->Name = L"fcgCBPSNR";
            this->fcgCBPSNR->Size = System::Drawing::Size(56, 18);
            this->fcgCBPSNR->TabIndex = 1;
            this->fcgCBPSNR->Tag = L"reCmd";
            this->fcgCBPSNR->Text = L"PSNR";
            this->fcgCBPSNR->UseVisualStyleBackColor = true;
            // 
            // fcgCXLogLevel
            // 
            this->fcgCXLogLevel->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXLogLevel->FormattingEnabled = true;
            this->fcgCXLogLevel->Location = System::Drawing::Point(70, 14);
            this->fcgCXLogLevel->Name = L"fcgCXLogLevel";
            this->fcgCXLogLevel->Size = System::Drawing::Size(93, 22);
            this->fcgCXLogLevel->TabIndex = 0;
            this->fcgCXLogLevel->Tag = L"reCmd";
            // 
            // fcgLBLog
            // 
            this->fcgLBLog->AutoSize = true;
            this->fcgLBLog->Location = System::Drawing::Point(14, 17);
            this->fcgLBLog->Name = L"fcgLBLog";
            this->fcgLBLog->Size = System::Drawing::Size(45, 14);
            this->fcgLBLog->TabIndex = 0;
            this->fcgLBLog->Text = L"ログ表示";
            // 
            // fcgNUSlices
            // 
            this->fcgNUSlices->Location = System::Drawing::Point(534, 282);
            this->fcgNUSlices->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
            this->fcgNUSlices->Name = L"fcgNUSlices";
            this->fcgNUSlices->Size = System::Drawing::Size(54, 21);
            this->fcgNUSlices->TabIndex = 27;
            this->fcgNUSlices->Tag = L"reCmd";
            this->fcgNUSlices->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBSlices
            // 
            this->fcgLBSlices->AutoSize = true;
            this->fcgLBSlices->Location = System::Drawing::Point(431, 284);
            this->fcgLBSlices->Name = L"fcgLBSlices";
            this->fcgLBSlices->Size = System::Drawing::Size(50, 14);
            this->fcgLBSlices->TabIndex = 39;
            this->fcgLBSlices->Text = L"スライス数";
            // 
            // fcgCXOutputCsp
            // 
            this->fcgCXOutputCsp->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXOutputCsp->FormattingEnabled = true;
            this->fcgCXOutputCsp->Location = System::Drawing::Point(494, 312);
            this->fcgCXOutputCsp->Name = L"fcgCXOutputCsp";
            this->fcgCXOutputCsp->Size = System::Drawing::Size(95, 22);
            this->fcgCXOutputCsp->TabIndex = 28;
            this->fcgCXOutputCsp->Tag = L"reCmd";
            // 
            // fcggroupBoxThreads
            // 
            this->fcggroupBoxThreads->Controls->Add(this->fcgLBLookaheadThreads);
            this->fcggroupBoxThreads->Controls->Add(this->fcgNULookaheadThreads);
            this->fcggroupBoxThreads->Controls->Add(this->fcgCBSlicedThreads);
            this->fcggroupBoxThreads->Controls->Add(this->fcgLBThreads);
            this->fcggroupBoxThreads->Controls->Add(this->fcgNUThreads);
            this->fcggroupBoxThreads->Location = System::Drawing::Point(196, 325);
            this->fcggroupBoxThreads->Name = L"fcggroupBoxThreads";
            this->fcggroupBoxThreads->Size = System::Drawing::Size(177, 98);
            this->fcggroupBoxThreads->TabIndex = 15;
            this->fcggroupBoxThreads->TabStop = false;
            // 
            // fcgLBLookaheadThreads
            // 
            this->fcgLBLookaheadThreads->AutoSize = true;
            this->fcgLBLookaheadThreads->Location = System::Drawing::Point(16, 45);
            this->fcgLBLookaheadThreads->Name = L"fcgLBLookaheadThreads";
            this->fcgLBLookaheadThreads->Size = System::Drawing::Size(65, 14);
            this->fcgLBLookaheadThreads->TabIndex = 31;
            this->fcgLBLookaheadThreads->Text = L"サブスレッド数";
            // 
            // fcgNULookaheadThreads
            // 
            this->fcgNULookaheadThreads->Location = System::Drawing::Point(95, 43);
            this->fcgNULookaheadThreads->Name = L"fcgNULookaheadThreads";
            this->fcgNULookaheadThreads->Size = System::Drawing::Size(64, 21);
            this->fcgNULookaheadThreads->TabIndex = 1;
            this->fcgNULookaheadThreads->Tag = L"reCmd";
            this->fcgNULookaheadThreads->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCBSlicedThreads
            // 
            this->fcgCBSlicedThreads->AutoSize = true;
            this->fcgCBSlicedThreads->Location = System::Drawing::Point(17, 71);
            this->fcgCBSlicedThreads->Name = L"fcgCBSlicedThreads";
            this->fcgCBSlicedThreads->Size = System::Drawing::Size(140, 18);
            this->fcgCBSlicedThreads->TabIndex = 2;
            this->fcgCBSlicedThreads->Tag = L"reCmd";
            this->fcgCBSlicedThreads->Text = L"スライスベースマルチスレッド";
            this->fcgCBSlicedThreads->UseVisualStyleBackColor = true;
            // 
            // fcgLBThreads
            // 
            this->fcgLBThreads->AutoSize = true;
            this->fcgLBThreads->Location = System::Drawing::Point(33, 17);
            this->fcgLBThreads->Name = L"fcgLBThreads";
            this->fcgLBThreads->Size = System::Drawing::Size(48, 14);
            this->fcgLBThreads->TabIndex = 29;
            this->fcgLBThreads->Text = L"スレッド数";
            // 
            // fcgNUThreads
            // 
            this->fcgNUThreads->Location = System::Drawing::Point(95, 15);
            this->fcgNUThreads->Name = L"fcgNUThreads";
            this->fcgNUThreads->Size = System::Drawing::Size(64, 21);
            this->fcgNUThreads->TabIndex = 0;
            this->fcgNUThreads->Tag = L"reCmd";
            this->fcgNUThreads->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCBUsehighbit
            // 
            this->fcgCBUsehighbit->AutoSize = true;
            this->fcgCBUsehighbit->Location = System::Drawing::Point(277, 10);
            this->fcgCBUsehighbit->Name = L"fcgCBUsehighbit";
            this->fcgCBUsehighbit->Size = System::Drawing::Size(99, 18);
            this->fcgCBUsehighbit->TabIndex = 1;
            this->fcgCBUsehighbit->Tag = L"reCmd";
            this->fcgCBUsehighbit->Text = L"highbit depth";
            this->fcgCBUsehighbit->UseVisualStyleBackColor = true;
            this->fcgCBUsehighbit->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCBUsehighbit_CheckedChanged);
            // 
            // fcgCBBlurayCompat
            // 
            this->fcgCBBlurayCompat->AutoSize = true;
            this->fcgCBBlurayCompat->Location = System::Drawing::Point(504, 183);
            this->fcgCBBlurayCompat->Name = L"fcgCBBlurayCompat";
            this->fcgCBBlurayCompat->Size = System::Drawing::Size(87, 18);
            this->fcgCBBlurayCompat->TabIndex = 24;
            this->fcgCBBlurayCompat->Tag = L"reCmd";
            this->fcgCBBlurayCompat->Text = L"blu-ray互換";
            this->fcgCBBlurayCompat->UseVisualStyleBackColor = true;
            // 
            // fcgCBAud
            // 
            this->fcgCBAud->AutoSize = true;
            this->fcgCBAud->Location = System::Drawing::Point(419, 183);
            this->fcgCBAud->Name = L"fcgCBAud";
            this->fcgCBAud->Size = System::Drawing::Size(68, 18);
            this->fcgCBAud->TabIndex = 23;
            this->fcgCBAud->Tag = L"reCmd";
            this->fcgCBAud->Text = L"aud付加";
            this->fcgCBAud->UseVisualStyleBackColor = true;
            // 
            // fcgCXVideoFormat
            // 
            this->fcgCXVideoFormat->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVideoFormat->FormattingEnabled = true;
            this->fcgCXVideoFormat->Location = System::Drawing::Point(493, 149);
            this->fcgCXVideoFormat->Name = L"fcgCXVideoFormat";
            this->fcgCXVideoFormat->Size = System::Drawing::Size(95, 22);
            this->fcgCXVideoFormat->TabIndex = 22;
            this->fcgCXVideoFormat->Tag = L"reCmd";
            // 
            // fcgLBVideoFormat
            // 
            this->fcgLBVideoFormat->AutoSize = true;
            this->fcgLBVideoFormat->Location = System::Drawing::Point(411, 152);
            this->fcgLBVideoFormat->Name = L"fcgLBVideoFormat";
            this->fcgLBVideoFormat->Size = System::Drawing::Size(73, 14);
            this->fcgLBVideoFormat->TabIndex = 24;
            this->fcgLBVideoFormat->Text = L"videoformat";
            // 
            // fcgLBLevel
            // 
            this->fcgLBLevel->AutoSize = true;
            this->fcgLBLevel->Location = System::Drawing::Point(413, 120);
            this->fcgLBLevel->Name = L"fcgLBLevel";
            this->fcgLBLevel->Size = System::Drawing::Size(71, 14);
            this->fcgLBLevel->TabIndex = 23;
            this->fcgLBLevel->Text = L"H.264 Level";
            // 
            // fcgCXLevel
            // 
            this->fcgCXLevel->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXLevel->FormattingEnabled = true;
            this->fcgCXLevel->Location = System::Drawing::Point(493, 117);
            this->fcgCXLevel->Name = L"fcgCXLevel";
            this->fcgCXLevel->Size = System::Drawing::Size(95, 22);
            this->fcgCXLevel->TabIndex = 21;
            this->fcgCXLevel->Tag = L"reCmd";
            // 
            // fcggroupBoxColorMatrix
            // 
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgCXInputRange);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgLBInputRange);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgCXTransfer);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgCXColorPrim);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgCXColorMatrix);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgLBTransfer);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgLBColorPrim);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgLBColorMatrix);
            this->fcggroupBoxColorMatrix->Location = System::Drawing::Point(386, 343);
            this->fcggroupBoxColorMatrix->Name = L"fcggroupBoxColorMatrix";
            this->fcggroupBoxColorMatrix->Size = System::Drawing::Size(218, 145);
            this->fcggroupBoxColorMatrix->TabIndex = 30;
            this->fcggroupBoxColorMatrix->TabStop = false;
            this->fcggroupBoxColorMatrix->Text = L"色空間";
            // 
            // fcgCXInputRange
            // 
            this->fcgCXInputRange->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXInputRange->FormattingEnabled = true;
            this->fcgCXInputRange->Location = System::Drawing::Point(82, 114);
            this->fcgCXInputRange->Name = L"fcgCXInputRange";
            this->fcgCXInputRange->Size = System::Drawing::Size(131, 22);
            this->fcgCXInputRange->TabIndex = 4;
            this->fcgCXInputRange->Tag = L"reCmd";
            // 
            // fcgLBInputRange
            // 
            this->fcgLBInputRange->AutoSize = true;
            this->fcgLBInputRange->Location = System::Drawing::Point(8, 117);
            this->fcgLBInputRange->Name = L"fcgLBInputRange";
            this->fcgLBInputRange->Size = System::Drawing::Size(70, 14);
            this->fcgLBInputRange->TabIndex = 3;
            this->fcgLBInputRange->Text = L"input range";
            // 
            // fcgCXTransfer
            // 
            this->fcgCXTransfer->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXTransfer->FormattingEnabled = true;
            this->fcgCXTransfer->Location = System::Drawing::Point(82, 83);
            this->fcgCXTransfer->Name = L"fcgCXTransfer";
            this->fcgCXTransfer->Size = System::Drawing::Size(131, 22);
            this->fcgCXTransfer->TabIndex = 2;
            this->fcgCXTransfer->Tag = L"reCmd";
            // 
            // fcgCXColorPrim
            // 
            this->fcgCXColorPrim->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXColorPrim->FormattingEnabled = true;
            this->fcgCXColorPrim->Location = System::Drawing::Point(82, 51);
            this->fcgCXColorPrim->Name = L"fcgCXColorPrim";
            this->fcgCXColorPrim->Size = System::Drawing::Size(131, 22);
            this->fcgCXColorPrim->TabIndex = 1;
            this->fcgCXColorPrim->Tag = L"reCmd";
            // 
            // fcgCXColorMatrix
            // 
            this->fcgCXColorMatrix->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXColorMatrix->FormattingEnabled = true;
            this->fcgCXColorMatrix->Location = System::Drawing::Point(82, 20);
            this->fcgCXColorMatrix->Name = L"fcgCXColorMatrix";
            this->fcgCXColorMatrix->Size = System::Drawing::Size(131, 22);
            this->fcgCXColorMatrix->TabIndex = 0;
            this->fcgCXColorMatrix->Tag = L"reCmd";
            // 
            // fcgLBTransfer
            // 
            this->fcgLBTransfer->AutoSize = true;
            this->fcgLBTransfer->Location = System::Drawing::Point(8, 86);
            this->fcgLBTransfer->Name = L"fcgLBTransfer";
            this->fcgLBTransfer->Size = System::Drawing::Size(49, 14);
            this->fcgLBTransfer->TabIndex = 2;
            this->fcgLBTransfer->Text = L"transfer";
            // 
            // fcgLBColorPrim
            // 
            this->fcgLBColorPrim->AutoSize = true;
            this->fcgLBColorPrim->Location = System::Drawing::Point(8, 54);
            this->fcgLBColorPrim->Name = L"fcgLBColorPrim";
            this->fcgLBColorPrim->Size = System::Drawing::Size(61, 14);
            this->fcgLBColorPrim->TabIndex = 1;
            this->fcgLBColorPrim->Text = L"colorprim";
            // 
            // fcgLBColorMatrix
            // 
            this->fcgLBColorMatrix->AutoSize = true;
            this->fcgLBColorMatrix->Location = System::Drawing::Point(8, 23);
            this->fcgLBColorMatrix->Name = L"fcgLBColorMatrix";
            this->fcgLBColorMatrix->Size = System::Drawing::Size(70, 14);
            this->fcgLBColorMatrix->TabIndex = 0;
            this->fcgLBColorMatrix->Text = L"colormatrix";
            // 
            // fcgGroupBoxAspectRatio
            // 
            this->fcgGroupBoxAspectRatio->Controls->Add(this->fcgNUAspectRatioY);
            this->fcgGroupBoxAspectRatio->Controls->Add(this->fcgLBAspectRatio);
            this->fcgGroupBoxAspectRatio->Controls->Add(this->fcgNUAspectRatioX);
            this->fcgGroupBoxAspectRatio->Controls->Add(this->fcgCXAspectRatio);
            this->fcgGroupBoxAspectRatio->Location = System::Drawing::Point(396, 6);
            this->fcgGroupBoxAspectRatio->Name = L"fcgGroupBoxAspectRatio";
            this->fcgGroupBoxAspectRatio->Size = System::Drawing::Size(205, 94);
            this->fcgGroupBoxAspectRatio->TabIndex = 20;
            this->fcgGroupBoxAspectRatio->TabStop = false;
            this->fcgGroupBoxAspectRatio->Text = L"アスペクト比";
            // 
            // fcgNUAspectRatioY
            // 
            this->fcgNUAspectRatioY->Location = System::Drawing::Point(126, 57);
            this->fcgNUAspectRatioY->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000, 0, 0, 0 });
            this->fcgNUAspectRatioY->Name = L"fcgNUAspectRatioY";
            this->fcgNUAspectRatioY->Size = System::Drawing::Size(60, 21);
            this->fcgNUAspectRatioY->TabIndex = 2;
            this->fcgNUAspectRatioY->Tag = L"reCmd";
            this->fcgNUAspectRatioY->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBAspectRatio
            // 
            this->fcgLBAspectRatio->AutoSize = true;
            this->fcgLBAspectRatio->Location = System::Drawing::Point(108, 59);
            this->fcgLBAspectRatio->Name = L"fcgLBAspectRatio";
            this->fcgLBAspectRatio->Size = System::Drawing::Size(12, 14);
            this->fcgLBAspectRatio->TabIndex = 2;
            this->fcgLBAspectRatio->Text = L":";
            // 
            // fcgNUAspectRatioX
            // 
            this->fcgNUAspectRatioX->Location = System::Drawing::Point(42, 57);
            this->fcgNUAspectRatioX->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000, 0, 0, 0 });
            this->fcgNUAspectRatioX->Name = L"fcgNUAspectRatioX";
            this->fcgNUAspectRatioX->Size = System::Drawing::Size(60, 21);
            this->fcgNUAspectRatioX->TabIndex = 1;
            this->fcgNUAspectRatioX->Tag = L"reCmd";
            this->fcgNUAspectRatioX->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCXAspectRatio
            // 
            this->fcgCXAspectRatio->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAspectRatio->FormattingEnabled = true;
            this->fcgCXAspectRatio->Location = System::Drawing::Point(15, 29);
            this->fcgCXAspectRatio->Name = L"fcgCXAspectRatio";
            this->fcgCXAspectRatio->Size = System::Drawing::Size(171, 22);
            this->fcgCXAspectRatio->TabIndex = 0;
            this->fcgCXAspectRatio->Tag = L"reCmd";
            // 
            // fcgpictureBoxX264
            // 
            this->fcgpictureBoxX264->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgpictureBoxX264.Image")));
            this->fcgpictureBoxX264->Location = System::Drawing::Point(4, 12);
            this->fcgpictureBoxX264->Name = L"fcgpictureBoxX264";
            this->fcgpictureBoxX264->Size = System::Drawing::Size(110, 44);
            this->fcgpictureBoxX264->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
            this->fcgpictureBoxX264->TabIndex = 16;
            this->fcgpictureBoxX264->TabStop = false;
            // 
            // fcgBTX264Path
            // 
            this->fcgBTX264Path->Location = System::Drawing::Point(354, 34);
            this->fcgBTX264Path->Name = L"fcgBTX264Path";
            this->fcgBTX264Path->Size = System::Drawing::Size(27, 22);
            this->fcgBTX264Path->TabIndex = 3;
            this->fcgBTX264Path->Text = L"...";
            this->fcgBTX264Path->UseVisualStyleBackColor = true;
            this->fcgBTX264Path->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTX264Path_Click);
            // 
            // fcgTXX264Path
            // 
            this->fcgTXX264Path->AllowDrop = true;
            this->fcgTXX264Path->Location = System::Drawing::Point(119, 35);
            this->fcgTXX264Path->Name = L"fcgTXX264Path";
            this->fcgTXX264Path->Size = System::Drawing::Size(235, 21);
            this->fcgTXX264Path->TabIndex = 2;
            this->fcgTXX264Path->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXX264Path_TextChanged);
            this->fcgTXX264Path->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXX264Path->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            this->fcgTXX264Path->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXX264Path_Enter);
            this->fcgTXX264Path->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXX264Path_Leave);
            // 
            // fcgLBX264Path
            // 
            this->fcgLBX264Path->AutoSize = true;
            this->fcgLBX264Path->Location = System::Drawing::Point(128, 12);
            this->fcgLBX264Path->Name = L"fcgLBX264Path";
            this->fcgLBX264Path->Size = System::Drawing::Size(49, 14);
            this->fcgLBX264Path->TabIndex = 2;
            this->fcgLBX264Path->Text = L"～の指定";
            // 
            // fcggroupBoxPreset
            // 
            this->fcggroupBoxPreset->Controls->Add(this->fcgBTApplyPreset);
            this->fcggroupBoxPreset->Controls->Add(this->fcgCXProfile);
            this->fcggroupBoxPreset->Controls->Add(this->fcgCXTune);
            this->fcggroupBoxPreset->Controls->Add(this->fcgCXPreset);
            this->fcggroupBoxPreset->Controls->Add(this->fcgLBProfile);
            this->fcggroupBoxPreset->Controls->Add(this->fcgLBX264TUNE);
            this->fcggroupBoxPreset->Controls->Add(this->fcgLBX264Preset);
            this->fcggroupBoxPreset->Location = System::Drawing::Point(7, 325);
            this->fcggroupBoxPreset->Name = L"fcggroupBoxPreset";
            this->fcggroupBoxPreset->Size = System::Drawing::Size(182, 163);
            this->fcggroupBoxPreset->TabIndex = 14;
            this->fcggroupBoxPreset->TabStop = false;
            this->fcggroupBoxPreset->Text = L"プリセットのロード";
            // 
            // fcgBTApplyPreset
            // 
            this->fcgBTApplyPreset->Location = System::Drawing::Point(73, 122);
            this->fcgBTApplyPreset->Name = L"fcgBTApplyPreset";
            this->fcgBTApplyPreset->Size = System::Drawing::Size(94, 32);
            this->fcgBTApplyPreset->TabIndex = 3;
            this->fcgBTApplyPreset->Text = L"GUIにロード";
            this->fcgBTApplyPreset->UseVisualStyleBackColor = true;
            this->fcgBTApplyPreset->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTApplyPreset_Click);
            // 
            // fcgCXProfile
            // 
            this->fcgCXProfile->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXProfile->FormattingEnabled = true;
            this->fcgCXProfile->Location = System::Drawing::Point(73, 86);
            this->fcgCXProfile->Name = L"fcgCXProfile";
            this->fcgCXProfile->Size = System::Drawing::Size(94, 22);
            this->fcgCXProfile->TabIndex = 2;
            this->fcgCXProfile->Tag = L"reCmd";
            // 
            // fcgCXTune
            // 
            this->fcgCXTune->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXTune->FormattingEnabled = true;
            this->fcgCXTune->Location = System::Drawing::Point(73, 54);
            this->fcgCXTune->Name = L"fcgCXTune";
            this->fcgCXTune->Size = System::Drawing::Size(94, 22);
            this->fcgCXTune->TabIndex = 1;
            this->fcgCXTune->Tag = L"reCmd";
            // 
            // fcgCXPreset
            // 
            this->fcgCXPreset->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXPreset->FormattingEnabled = true;
            this->fcgCXPreset->Location = System::Drawing::Point(73, 22);
            this->fcgCXPreset->Name = L"fcgCXPreset";
            this->fcgCXPreset->Size = System::Drawing::Size(94, 22);
            this->fcgCXPreset->TabIndex = 0;
            this->fcgCXPreset->Tag = L"reCmd";
            // 
            // fcgLBProfile
            // 
            this->fcgLBProfile->AutoSize = true;
            this->fcgLBProfile->Location = System::Drawing::Point(9, 89);
            this->fcgLBProfile->Name = L"fcgLBProfile";
            this->fcgLBProfile->Size = System::Drawing::Size(53, 14);
            this->fcgLBProfile->TabIndex = 2;
            this->fcgLBProfile->Text = L"プロファイル";
            // 
            // fcgLBX264TUNE
            // 
            this->fcgLBX264TUNE->AutoSize = true;
            this->fcgLBX264TUNE->Location = System::Drawing::Point(9, 57);
            this->fcgLBX264TUNE->Name = L"fcgLBX264TUNE";
            this->fcgLBX264TUNE->Size = System::Drawing::Size(56, 14);
            this->fcgLBX264TUNE->TabIndex = 1;
            this->fcgLBX264TUNE->Text = L"チューニング";
            // 
            // fcgLBX264Preset
            // 
            this->fcgLBX264Preset->AutoSize = true;
            this->fcgLBX264Preset->Location = System::Drawing::Point(9, 25);
            this->fcgLBX264Preset->Name = L"fcgLBX264Preset";
            this->fcgLBX264Preset->Size = System::Drawing::Size(29, 14);
            this->fcgLBX264Preset->TabIndex = 0;
            this->fcgLBX264Preset->Text = L"速度";
            // 
            // fcgLBOutputCF
            // 
            this->fcgLBOutputCF->AutoSize = true;
            this->fcgLBOutputCF->Location = System::Drawing::Point(396, 315);
            this->fcgLBOutputCF->Name = L"fcgLBOutputCF";
            this->fcgLBOutputCF->Size = System::Drawing::Size(85, 14);
            this->fcgLBOutputCF->TabIndex = 37;
            this->fcgLBOutputCF->Text = L"出力色フォーマット";
            // 
            // fcgtabPageX264RC
            // 
            this->fcgtabPageX264RC->Controls->Add(this->fcgLBTimebase);
            this->fcgtabPageX264RC->Controls->Add(this->fcgNUTimebaseDen);
            this->fcgtabPageX264RC->Controls->Add(this->fcgNUTimebaseNum);
            this->fcgtabPageX264RC->Controls->Add(this->fcgBTTCIN);
            this->fcgtabPageX264RC->Controls->Add(this->fcgTXTCIN);
            this->fcgtabPageX264RC->Controls->Add(this->fcgCBTimeBase);
            this->fcgtabPageX264RC->Controls->Add(this->fcgCBTCIN);
            this->fcgtabPageX264RC->Controls->Add(this->fcggroupBoxAQ);
            this->fcgtabPageX264RC->Controls->Add(this->fcggroupBoxPsyRd);
            this->fcgtabPageX264RC->Controls->Add(this->fcggroupBoxVbv);
            this->fcgtabPageX264RC->Controls->Add(this->fcgNURCLookahead);
            this->fcgtabPageX264RC->Controls->Add(this->fcgLBRCLookahead);
            this->fcgtabPageX264RC->Controls->Add(this->fcgCBMBTree);
            this->fcgtabPageX264RC->Controls->Add(this->fcggroupBoxQP);
            this->fcgtabPageX264RC->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageX264RC->Name = L"fcgtabPageX264RC";
            this->fcgtabPageX264RC->Padding = System::Windows::Forms::Padding(3);
            this->fcgtabPageX264RC->Size = System::Drawing::Size(608, 491);
            this->fcgtabPageX264RC->TabIndex = 1;
            this->fcgtabPageX264RC->Text = L" レート・QP制御";
            this->fcgtabPageX264RC->UseVisualStyleBackColor = true;
            // 
            // fcgLBTimebase
            // 
            this->fcgLBTimebase->AutoSize = true;
            this->fcgLBTimebase->Location = System::Drawing::Point(235, 452);
            this->fcgLBTimebase->Name = L"fcgLBTimebase";
            this->fcgLBTimebase->Size = System::Drawing::Size(12, 14);
            this->fcgLBTimebase->TabIndex = 17;
            this->fcgLBTimebase->Text = L"/";
            // 
            // fcgNUTimebaseDen
            // 
            this->fcgNUTimebaseDen->Location = System::Drawing::Point(253, 450);
            this->fcgNUTimebaseDen->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2000000000, 0, 0, 0 });
            this->fcgNUTimebaseDen->Name = L"fcgNUTimebaseDen";
            this->fcgNUTimebaseDen->Size = System::Drawing::Size(120, 21);
            this->fcgNUTimebaseDen->TabIndex = 11;
            this->fcgNUTimebaseDen->Tag = L"reCmd";
            this->fcgNUTimebaseDen->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUTimebaseNum
            // 
            this->fcgNUTimebaseNum->Location = System::Drawing::Point(135, 450);
            this->fcgNUTimebaseNum->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2000000000, 0, 0, 0 });
            this->fcgNUTimebaseNum->Name = L"fcgNUTimebaseNum";
            this->fcgNUTimebaseNum->Size = System::Drawing::Size(94, 21);
            this->fcgNUTimebaseNum->TabIndex = 10;
            this->fcgNUTimebaseNum->Tag = L"reCmd";
            this->fcgNUTimebaseNum->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgBTTCIN
            // 
            this->fcgBTTCIN->Location = System::Drawing::Point(362, 419);
            this->fcgBTTCIN->Name = L"fcgBTTCIN";
            this->fcgBTTCIN->Size = System::Drawing::Size(34, 23);
            this->fcgBTTCIN->TabIndex = 8;
            this->fcgBTTCIN->Text = L"...";
            this->fcgBTTCIN->UseVisualStyleBackColor = true;
            this->fcgBTTCIN->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTTCIN_Click);
            // 
            // fcgTXTCIN
            // 
            this->fcgTXTCIN->Location = System::Drawing::Point(135, 420);
            this->fcgTXTCIN->Name = L"fcgTXTCIN";
            this->fcgTXTCIN->Size = System::Drawing::Size(221, 21);
            this->fcgTXTCIN->TabIndex = 7;
            this->fcgTXTCIN->Tag = L"reCmd";
            // 
            // fcgCBTimeBase
            // 
            this->fcgCBTimeBase->AutoSize = true;
            this->fcgCBTimeBase->Location = System::Drawing::Point(24, 451);
            this->fcgCBTimeBase->Name = L"fcgCBTimeBase";
            this->fcgCBTimeBase->Size = System::Drawing::Size(92, 18);
            this->fcgCBTimeBase->TabIndex = 9;
            this->fcgCBTimeBase->Tag = L"reCmd";
            this->fcgCBTimeBase->Text = L"時間精度指定";
            this->fcgCBTimeBase->UseVisualStyleBackColor = true;
            this->fcgCBTimeBase->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgChangeEnabled);
            // 
            // fcgCBTCIN
            // 
            this->fcgCBTCIN->AutoSize = true;
            this->fcgCBTCIN->Location = System::Drawing::Point(24, 422);
            this->fcgCBTCIN->Name = L"fcgCBTCIN";
            this->fcgCBTCIN->Size = System::Drawing::Size(98, 18);
            this->fcgCBTCIN->TabIndex = 6;
            this->fcgCBTCIN->Tag = L"reCmd";
            this->fcgCBTCIN->Text = L"タイムコード入力";
            this->fcgCBTCIN->UseVisualStyleBackColor = true;
            this->fcgCBTCIN->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgChangeEnabled);
            // 
            // fcggroupBoxAQ
            // 
            this->fcggroupBoxAQ->Controls->Add(this->fcgNUAQStrength);
            this->fcggroupBoxAQ->Controls->Add(this->fcgLBAQStrength);
            this->fcggroupBoxAQ->Controls->Add(this->fcgCXAQMode);
            this->fcggroupBoxAQ->Controls->Add(this->fcgLBAQMode);
            this->fcggroupBoxAQ->Location = System::Drawing::Point(8, 258);
            this->fcggroupBoxAQ->Name = L"fcggroupBoxAQ";
            this->fcggroupBoxAQ->Size = System::Drawing::Size(241, 101);
            this->fcggroupBoxAQ->TabIndex = 4;
            this->fcggroupBoxAQ->TabStop = false;
            this->fcggroupBoxAQ->Text = L"AQ (適応的QP)";
            // 
            // fcgNUAQStrength
            // 
            this->fcgNUAQStrength->DecimalPlaces = 2;
            this->fcgNUAQStrength->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 131072 });
            this->fcgNUAQStrength->Location = System::Drawing::Point(120, 67);
            this->fcgNUAQStrength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
            this->fcgNUAQStrength->Name = L"fcgNUAQStrength";
            this->fcgNUAQStrength->Size = System::Drawing::Size(75, 21);
            this->fcgNUAQStrength->TabIndex = 1;
            this->fcgNUAQStrength->Tag = L"reCmd";
            this->fcgNUAQStrength->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBAQStrength
            // 
            this->fcgLBAQStrength->AutoSize = true;
            this->fcgLBAQStrength->Location = System::Drawing::Point(58, 69);
            this->fcgLBAQStrength->Name = L"fcgLBAQStrength";
            this->fcgLBAQStrength->Size = System::Drawing::Size(41, 14);
            this->fcgLBAQStrength->TabIndex = 2;
            this->fcgLBAQStrength->Text = L"AQ強さ";
            // 
            // fcgCXAQMode
            // 
            this->fcgCXAQMode->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAQMode->FormattingEnabled = true;
            this->fcgCXAQMode->Location = System::Drawing::Point(120, 28);
            this->fcgCXAQMode->Name = L"fcgCXAQMode";
            this->fcgCXAQMode->Size = System::Drawing::Size(75, 22);
            this->fcgCXAQMode->TabIndex = 0;
            this->fcgCXAQMode->Tag = L"reCmd";
            // 
            // fcgLBAQMode
            // 
            this->fcgLBAQMode->AutoSize = true;
            this->fcgLBAQMode->Location = System::Drawing::Point(58, 31);
            this->fcgLBAQMode->Name = L"fcgLBAQMode";
            this->fcgLBAQMode->Size = System::Drawing::Size(32, 14);
            this->fcgLBAQMode->TabIndex = 0;
            this->fcgLBAQMode->Text = L"モード";
            // 
            // fcggroupBoxPsyRd
            // 
            this->fcggroupBoxPsyRd->Controls->Add(this->fcgNUPsyTrellis);
            this->fcggroupBoxPsyRd->Controls->Add(this->fcgNUPsyRDO);
            this->fcggroupBoxPsyRd->Controls->Add(this->fcgLBPsyTrellis);
            this->fcggroupBoxPsyRd->Controls->Add(this->fcgLBPsyRDO);
            this->fcggroupBoxPsyRd->Location = System::Drawing::Point(284, 258);
            this->fcggroupBoxPsyRd->Name = L"fcggroupBoxPsyRd";
            this->fcggroupBoxPsyRd->Size = System::Drawing::Size(232, 101);
            this->fcggroupBoxPsyRd->TabIndex = 5;
            this->fcggroupBoxPsyRd->TabStop = false;
            this->fcggroupBoxPsyRd->Text = L"Psy-Rd (視覚心理最適化)";
            // 
            // fcgNUPsyTrellis
            // 
            this->fcgNUPsyTrellis->DecimalPlaces = 2;
            this->fcgNUPsyTrellis->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 131072 });
            this->fcgNUPsyTrellis->Location = System::Drawing::Point(121, 64);
            this->fcgNUPsyTrellis->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
            this->fcgNUPsyTrellis->Name = L"fcgNUPsyTrellis";
            this->fcgNUPsyTrellis->Size = System::Drawing::Size(75, 21);
            this->fcgNUPsyTrellis->TabIndex = 1;
            this->fcgNUPsyTrellis->Tag = L"reCmd";
            this->fcgNUPsyTrellis->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUPsyRDO
            // 
            this->fcgNUPsyRDO->DecimalPlaces = 2;
            this->fcgNUPsyRDO->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 131072 });
            this->fcgNUPsyRDO->Location = System::Drawing::Point(121, 28);
            this->fcgNUPsyRDO->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
            this->fcgNUPsyRDO->Name = L"fcgNUPsyRDO";
            this->fcgNUPsyRDO->Size = System::Drawing::Size(75, 21);
            this->fcgNUPsyRDO->TabIndex = 0;
            this->fcgNUPsyRDO->Tag = L"reCmd";
            this->fcgNUPsyRDO->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBPsyTrellis
            // 
            this->fcgLBPsyTrellis->AutoSize = true;
            this->fcgLBPsyTrellis->Location = System::Drawing::Point(47, 67);
            this->fcgLBPsyTrellis->Name = L"fcgLBPsyTrellis";
            this->fcgLBPsyTrellis->Size = System::Drawing::Size(36, 14);
            this->fcgLBPsyTrellis->TabIndex = 1;
            this->fcgLBPsyTrellis->Text = L"trellis";
            // 
            // fcgLBPsyRDO
            // 
            this->fcgLBPsyRDO->AutoSize = true;
            this->fcgLBPsyRDO->Location = System::Drawing::Point(52, 31);
            this->fcgLBPsyRDO->Name = L"fcgLBPsyRDO";
            this->fcgLBPsyRDO->Size = System::Drawing::Size(31, 14);
            this->fcgLBPsyRDO->TabIndex = 0;
            this->fcgLBPsyRDO->Text = L"RDO";
            // 
            // fcggroupBoxVbv
            // 
            this->fcggroupBoxVbv->Controls->Add(this->fcgLBVBVafsWarning);
            this->fcggroupBoxVbv->Controls->Add(this->fcgNUVBVbuf);
            this->fcggroupBoxVbv->Controls->Add(this->fcgNUVBVmax);
            this->fcggroupBoxVbv->Controls->Add(this->fcgLBVBVbuf);
            this->fcggroupBoxVbv->Controls->Add(this->fcgLBVBVmax);
            this->fcggroupBoxVbv->Location = System::Drawing::Point(284, 114);
            this->fcggroupBoxVbv->Name = L"fcggroupBoxVbv";
            this->fcggroupBoxVbv->Size = System::Drawing::Size(232, 131);
            this->fcggroupBoxVbv->TabIndex = 3;
            this->fcggroupBoxVbv->TabStop = false;
            this->fcggroupBoxVbv->Text = L"VBV (ビデオバッファ制御, \"-1\"で自動)";
            // 
            // fcgLBVBVafsWarning
            // 
            this->fcgLBVBVafsWarning->ForeColor = System::Drawing::Color::OrangeRed;
            this->fcgLBVBVafsWarning->Location = System::Drawing::Point(25, 91);
            this->fcgLBVBVafsWarning->Name = L"fcgLBVBVafsWarning";
            this->fcgLBVBVafsWarning->Size = System::Drawing::Size(155, 28);
            this->fcgLBVBVafsWarning->TabIndex = 4;
            this->fcgLBVBVafsWarning->Text = L"※afs使用時はvbv設定は正しく反映されません。";
            // 
            // fcgNUVBVbuf
            // 
            this->fcgNUVBVbuf->Location = System::Drawing::Point(121, 57);
            this->fcgNUVBVbuf->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000000, 0, 0, 0 });
            this->fcgNUVBVbuf->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, System::Int32::MinValue });
            this->fcgNUVBVbuf->Name = L"fcgNUVBVbuf";
            this->fcgNUVBVbuf->Size = System::Drawing::Size(75, 21);
            this->fcgNUVBVbuf->TabIndex = 1;
            this->fcgNUVBVbuf->Tag = L"reCmd";
            this->fcgNUVBVbuf->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVBVmax
            // 
            this->fcgNUVBVmax->Location = System::Drawing::Point(121, 27);
            this->fcgNUVBVmax->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000000, 0, 0, 0 });
            this->fcgNUVBVmax->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, System::Int32::MinValue });
            this->fcgNUVBVmax->Name = L"fcgNUVBVmax";
            this->fcgNUVBVmax->Size = System::Drawing::Size(75, 21);
            this->fcgNUVBVmax->TabIndex = 0;
            this->fcgNUVBVmax->Tag = L"reCmd";
            this->fcgNUVBVmax->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBVBVbuf
            // 
            this->fcgLBVBVbuf->AutoSize = true;
            this->fcgLBVBVbuf->Location = System::Drawing::Point(25, 59);
            this->fcgLBVBVbuf->Name = L"fcgLBVBVbuf";
            this->fcgLBVBVbuf->Size = System::Drawing::Size(61, 14);
            this->fcgLBVBVbuf->TabIndex = 1;
            this->fcgLBVBVbuf->Text = L"バッファサイズ";
            // 
            // fcgLBVBVmax
            // 
            this->fcgLBVBVmax->AutoSize = true;
            this->fcgLBVBVmax->Location = System::Drawing::Point(10, 29);
            this->fcgLBVBVmax->Name = L"fcgLBVBVmax";
            this->fcgLBVBVmax->Size = System::Drawing::Size(76, 14);
            this->fcgLBVBVmax->TabIndex = 0;
            this->fcgLBVBVmax->Text = L"最大ビットレート";
            // 
            // fcgNURCLookahead
            // 
            this->fcgNURCLookahead->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
            this->fcgNURCLookahead->Location = System::Drawing::Point(358, 76);
            this->fcgNURCLookahead->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 250, 0, 0, 0 });
            this->fcgNURCLookahead->Name = L"fcgNURCLookahead";
            this->fcgNURCLookahead->Size = System::Drawing::Size(71, 21);
            this->fcgNURCLookahead->TabIndex = 2;
            this->fcgNURCLookahead->Tag = L"reCmd";
            this->fcgNURCLookahead->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBRCLookahead
            // 
            this->fcgLBRCLookahead->AutoSize = true;
            this->fcgLBRCLookahead->Location = System::Drawing::Point(309, 53);
            this->fcgLBRCLookahead->Name = L"fcgLBRCLookahead";
            this->fcgLBRCLookahead->Size = System::Drawing::Size(142, 14);
            this->fcgLBRCLookahead->TabIndex = 2;
            this->fcgLBRCLookahead->Text = L"レート制御先行探査フレーム数";
            // 
            // fcgCBMBTree
            // 
            this->fcgCBMBTree->AutoSize = true;
            this->fcgCBMBTree->Location = System::Drawing::Point(332, 19);
            this->fcgCBMBTree->Name = L"fcgCBMBTree";
            this->fcgCBMBTree->Size = System::Drawing::Size(116, 18);
            this->fcgCBMBTree->TabIndex = 1;
            this->fcgCBMBTree->Tag = L"reCmd";
            this->fcgCBMBTree->Text = L"mbtree レート制御";
            this->fcgCBMBTree->UseVisualStyleBackColor = true;
            // 
            // fcggroupBoxQP
            // 
            this->fcggroupBoxQP->Controls->Add(this->fcgNUChromaQp);
            this->fcggroupBoxQP->Controls->Add(this->fcgNUQpstep);
            this->fcggroupBoxQP->Controls->Add(this->fcgNUQpmax);
            this->fcggroupBoxQP->Controls->Add(this->fcgNUQpmin);
            this->fcggroupBoxQP->Controls->Add(this->fcgNUQcomp);
            this->fcggroupBoxQP->Controls->Add(this->fcgNUPBRatio);
            this->fcggroupBoxQP->Controls->Add(this->fcgNUIPRatio);
            this->fcggroupBoxQP->Controls->Add(this->fcgLBQpstep);
            this->fcggroupBoxQP->Controls->Add(this->fcgLBChromaQp);
            this->fcggroupBoxQP->Controls->Add(this->fcgLBQpmax);
            this->fcggroupBoxQP->Controls->Add(this->fcgLBQpmin);
            this->fcggroupBoxQP->Controls->Add(this->fcgLBQcomp);
            this->fcggroupBoxQP->Controls->Add(this->fcgLBPBRatio);
            this->fcggroupBoxQP->Controls->Add(this->fcgLBIPRatio);
            this->fcggroupBoxQP->Location = System::Drawing::Point(8, 6);
            this->fcggroupBoxQP->Name = L"fcggroupBoxQP";
            this->fcggroupBoxQP->Size = System::Drawing::Size(241, 239);
            this->fcggroupBoxQP->TabIndex = 0;
            this->fcggroupBoxQP->TabStop = false;
            this->fcggroupBoxQP->Text = L"量子化(QP)";
            // 
            // fcgNUChromaQp
            // 
            this->fcgNUChromaQp->Location = System::Drawing::Point(165, 205);
            this->fcgNUChromaQp->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 81, 0, 0, 0 });
            this->fcgNUChromaQp->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 81, 0, 0, System::Int32::MinValue });
            this->fcgNUChromaQp->Name = L"fcgNUChromaQp";
            this->fcgNUChromaQp->Size = System::Drawing::Size(55, 21);
            this->fcgNUChromaQp->TabIndex = 6;
            this->fcgNUChromaQp->Tag = L"reCmd";
            this->fcgNUChromaQp->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUQpstep
            // 
            this->fcgNUQpstep->Location = System::Drawing::Point(165, 175);
            this->fcgNUQpstep->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 81, 0, 0, 0 });
            this->fcgNUQpstep->Name = L"fcgNUQpstep";
            this->fcgNUQpstep->Size = System::Drawing::Size(55, 21);
            this->fcgNUQpstep->TabIndex = 5;
            this->fcgNUQpstep->Tag = L"reCmd";
            this->fcgNUQpstep->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUQpmax
            // 
            this->fcgNUQpmax->Location = System::Drawing::Point(165, 145);
            this->fcgNUQpmax->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 69, 0, 0, 0 });
            this->fcgNUQpmax->Name = L"fcgNUQpmax";
            this->fcgNUQpmax->Size = System::Drawing::Size(55, 21);
            this->fcgNUQpmax->TabIndex = 4;
            this->fcgNUQpmax->Tag = L"reCmd";
            this->fcgNUQpmax->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUQpmin
            // 
            this->fcgNUQpmin->Location = System::Drawing::Point(165, 115);
            this->fcgNUQpmin->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 81, 0, 0, 0 });
            this->fcgNUQpmin->Name = L"fcgNUQpmin";
            this->fcgNUQpmin->Size = System::Drawing::Size(55, 21);
            this->fcgNUQpmin->TabIndex = 3;
            this->fcgNUQpmin->Tag = L"reCmd";
            this->fcgNUQpmin->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUQcomp
            // 
            this->fcgNUQcomp->Location = System::Drawing::Point(165, 85);
            this->fcgNUQcomp->Name = L"fcgNUQcomp";
            this->fcgNUQcomp->Size = System::Drawing::Size(55, 21);
            this->fcgNUQcomp->TabIndex = 2;
            this->fcgNUQcomp->Tag = L"reCmd";
            this->fcgNUQcomp->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUPBRatio
            // 
            this->fcgNUPBRatio->Location = System::Drawing::Point(165, 55);
            this->fcgNUPBRatio->Name = L"fcgNUPBRatio";
            this->fcgNUPBRatio->Size = System::Drawing::Size(55, 21);
            this->fcgNUPBRatio->TabIndex = 1;
            this->fcgNUPBRatio->Tag = L"reCmd";
            this->fcgNUPBRatio->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUIPRatio
            // 
            this->fcgNUIPRatio->Location = System::Drawing::Point(165, 25);
            this->fcgNUIPRatio->Name = L"fcgNUIPRatio";
            this->fcgNUIPRatio->Size = System::Drawing::Size(55, 21);
            this->fcgNUIPRatio->TabIndex = 0;
            this->fcgNUIPRatio->Tag = L"reCmd";
            this->fcgNUIPRatio->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBQpstep
            // 
            this->fcgLBQpstep->AutoSize = true;
            this->fcgLBQpstep->Location = System::Drawing::Point(13, 177);
            this->fcgLBQpstep->Name = L"fcgLBQpstep";
            this->fcgLBQpstep->Size = System::Drawing::Size(77, 14);
            this->fcgLBQpstep->TabIndex = 6;
            this->fcgLBQpstep->Text = L"最大QP変動幅";
            // 
            // fcgLBChromaQp
            // 
            this->fcgLBChromaQp->AutoSize = true;
            this->fcgLBChromaQp->Location = System::Drawing::Point(13, 207);
            this->fcgLBChromaQp->Name = L"fcgLBChromaQp";
            this->fcgLBChromaQp->Size = System::Drawing::Size(111, 14);
            this->fcgLBChromaQp->TabIndex = 5;
            this->fcgLBChromaQp->Text = L"クロマ(色差)QP補正量";
            // 
            // fcgLBQpmax
            // 
            this->fcgLBQpmax->AutoSize = true;
            this->fcgLBQpmax->Location = System::Drawing::Point(13, 147);
            this->fcgLBQpmax->Name = L"fcgLBQpmax";
            this->fcgLBQpmax->Size = System::Drawing::Size(55, 14);
            this->fcgLBQpmax->TabIndex = 4;
            this->fcgLBQpmax->Text = L"最大QP値";
            // 
            // fcgLBQpmin
            // 
            this->fcgLBQpmin->AutoSize = true;
            this->fcgLBQpmin->Location = System::Drawing::Point(13, 117);
            this->fcgLBQpmin->Name = L"fcgLBQpmin";
            this->fcgLBQpmin->Size = System::Drawing::Size(55, 14);
            this->fcgLBQpmin->TabIndex = 3;
            this->fcgLBQpmin->Text = L"最小QP値";
            // 
            // fcgLBQcomp
            // 
            this->fcgLBQcomp->AutoSize = true;
            this->fcgLBQcomp->Location = System::Drawing::Point(13, 87);
            this->fcgLBQcomp->Name = L"fcgLBQcomp";
            this->fcgLBQcomp->Size = System::Drawing::Size(112, 14);
            this->fcgLBQcomp->TabIndex = 2;
            this->fcgLBQcomp->Text = L"ビットレート変動量 (%)";
            // 
            // fcgLBPBRatio
            // 
            this->fcgLBPBRatio->AutoSize = true;
            this->fcgLBPBRatio->Location = System::Drawing::Point(13, 57);
            this->fcgLBPBRatio->Name = L"fcgLBPBRatio";
            this->fcgLBPBRatio->Size = System::Drawing::Size(117, 14);
            this->fcgLBPBRatio->TabIndex = 1;
            this->fcgLBPBRatio->Text = L"P-Bフレーム間係数 (%)";
            // 
            // fcgLBIPRatio
            // 
            this->fcgLBIPRatio->AutoSize = true;
            this->fcgLBIPRatio->Location = System::Drawing::Point(13, 27);
            this->fcgLBIPRatio->Name = L"fcgLBIPRatio";
            this->fcgLBIPRatio->Size = System::Drawing::Size(114, 14);
            this->fcgLBIPRatio->TabIndex = 0;
            this->fcgLBIPRatio->Text = L"I-Pフレーム間係数 (%)";
            // 
            // fcgtabPageX264Frame
            // 
            this->fcgtabPageX264Frame->Controls->Add(this->fcgCBDeblock);
            this->fcgtabPageX264Frame->Controls->Add(this->fcgCXInterlaced);
            this->fcgtabPageX264Frame->Controls->Add(this->fcgLBInterlaced);
            this->fcgtabPageX264Frame->Controls->Add(this->fcggroupBoxX264Other);
            this->fcgtabPageX264Frame->Controls->Add(this->fcgCBDctDecimate);
            this->fcgtabPageX264Frame->Controls->Add(this->fcgCBfastpskip);
            this->fcgtabPageX264Frame->Controls->Add(this->fcggroupBoxME);
            this->fcgtabPageX264Frame->Controls->Add(this->fcggroupBoxMBTypes);
            this->fcgtabPageX264Frame->Controls->Add(this->fcgCXWeightP);
            this->fcgtabPageX264Frame->Controls->Add(this->fcgLBWeightP);
            this->fcgtabPageX264Frame->Controls->Add(this->fcggroupBoxBframes);
            this->fcgtabPageX264Frame->Controls->Add(this->fcgCBCABAC);
            this->fcgtabPageX264Frame->Controls->Add(this->fcggroupBoxGOP);
            this->fcgtabPageX264Frame->Controls->Add(this->fcggroupBoxDeblock);
            this->fcgtabPageX264Frame->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageX264Frame->Name = L"fcgtabPageX264Frame";
            this->fcgtabPageX264Frame->Size = System::Drawing::Size(608, 491);
            this->fcgtabPageX264Frame->TabIndex = 4;
            this->fcgtabPageX264Frame->Text = L"フレーム";
            this->fcgtabPageX264Frame->UseVisualStyleBackColor = true;
            // 
            // fcgCBDeblock
            // 
            this->fcgCBDeblock->AutoSize = true;
            this->fcgCBDeblock->Location = System::Drawing::Point(387, 144);
            this->fcgCBDeblock->Name = L"fcgCBDeblock";
            this->fcgCBDeblock->Size = System::Drawing::Size(141, 18);
            this->fcgCBDeblock->TabIndex = 7;
            this->fcgCBDeblock->Tag = L"reCmd";
            this->fcgCBDeblock->Text = L"インループ デブロックフィルタ";
            this->fcgCBDeblock->UseVisualStyleBackColor = true;
            this->fcgCBDeblock->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgChangeEnabled);
            // 
            // fcgCXInterlaced
            // 
            this->fcgCXInterlaced->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXInterlaced->FormattingEnabled = true;
            this->fcgCXInterlaced->Location = System::Drawing::Point(482, 245);
            this->fcgCXInterlaced->Name = L"fcgCXInterlaced";
            this->fcgCXInterlaced->Size = System::Drawing::Size(112, 22);
            this->fcgCXInterlaced->TabIndex = 9;
            this->fcgCXInterlaced->Tag = L"reCmd";
            // 
            // fcgLBInterlaced
            // 
            this->fcgLBInterlaced->AutoSize = true;
            this->fcgLBInterlaced->Location = System::Drawing::Point(373, 248);
            this->fcgLBInterlaced->Name = L"fcgLBInterlaced";
            this->fcgLBInterlaced->Size = System::Drawing::Size(87, 14);
            this->fcgLBInterlaced->TabIndex = 30;
            this->fcgLBInterlaced->Text = L"インターレース保持";
            // 
            // fcggroupBoxX264Other
            // 
            this->fcggroupBoxX264Other->Controls->Add(this->fcgBTMatrix);
            this->fcggroupBoxX264Other->Controls->Add(this->fcgTXCQM);
            this->fcggroupBoxX264Other->Controls->Add(this->fcgCXTrellis);
            this->fcggroupBoxX264Other->Controls->Add(this->fcgLBCQM);
            this->fcggroupBoxX264Other->Controls->Add(this->fcgLBTrellis);
            this->fcggroupBoxX264Other->Location = System::Drawing::Point(280, 368);
            this->fcggroupBoxX264Other->Name = L"fcggroupBoxX264Other";
            this->fcggroupBoxX264Other->Size = System::Drawing::Size(320, 99);
            this->fcggroupBoxX264Other->TabIndex = 14;
            this->fcggroupBoxX264Other->TabStop = false;
            this->fcggroupBoxX264Other->Text = L"その他";
            // 
            // fcgBTMatrix
            // 
            this->fcgBTMatrix->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 6, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgBTMatrix->Location = System::Drawing::Point(266, 57);
            this->fcgBTMatrix->Name = L"fcgBTMatrix";
            this->fcgBTMatrix->Size = System::Drawing::Size(18, 23);
            this->fcgBTMatrix->TabIndex = 2;
            this->fcgBTMatrix->Text = L"▼";
            this->fcgBTMatrix->UseVisualStyleBackColor = true;
            this->fcgBTMatrix->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMatrix_Click);
            // 
            // fcgTXCQM
            // 
            this->fcgTXCQM->Location = System::Drawing::Point(111, 58);
            this->fcgTXCQM->Name = L"fcgTXCQM";
            this->fcgTXCQM->Size = System::Drawing::Size(149, 21);
            this->fcgTXCQM->TabIndex = 1;
            this->fcgTXCQM->Tag = L"reCmd";
            // 
            // fcgCXTrellis
            // 
            this->fcgCXTrellis->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXTrellis->FormattingEnabled = true;
            this->fcgCXTrellis->Location = System::Drawing::Point(111, 25);
            this->fcgCXTrellis->Name = L"fcgCXTrellis";
            this->fcgCXTrellis->Size = System::Drawing::Size(172, 22);
            this->fcgCXTrellis->TabIndex = 0;
            this->fcgCXTrellis->Tag = L"reCmd";
            // 
            // fcgLBCQM
            // 
            this->fcgLBCQM->AutoSize = true;
            this->fcgLBCQM->Location = System::Drawing::Point(10, 61);
            this->fcgLBCQM->Name = L"fcgLBCQM";
            this->fcgLBCQM->Size = System::Drawing::Size(84, 14);
            this->fcgLBCQM->TabIndex = 3;
            this->fcgLBCQM->Text = L"量子化マトリックス";
            // 
            // fcgLBTrellis
            // 
            this->fcgLBTrellis->AutoSize = true;
            this->fcgLBTrellis->Location = System::Drawing::Point(10, 28);
            this->fcgLBTrellis->Name = L"fcgLBTrellis";
            this->fcgLBTrellis->Size = System::Drawing::Size(86, 14);
            this->fcgLBTrellis->TabIndex = 2;
            this->fcgLBTrellis->Text = L"レート歪み最適化";
            // 
            // fcgCBDctDecimate
            // 
            this->fcgCBDctDecimate->AutoSize = true;
            this->fcgCBDctDecimate->Location = System::Drawing::Point(376, 344);
            this->fcgCBDctDecimate->Name = L"fcgCBDctDecimate";
            this->fcgCBDctDecimate->Size = System::Drawing::Size(100, 18);
            this->fcgCBDctDecimate->TabIndex = 12;
            this->fcgCBDctDecimate->Tag = L"reCmd";
            this->fcgCBDctDecimate->Text = L"DCT係数間引き";
            this->fcgCBDctDecimate->UseVisualStyleBackColor = true;
            // 
            // fcgCBfastpskip
            // 
            this->fcgCBfastpskip->AutoSize = true;
            this->fcgCBfastpskip->Location = System::Drawing::Point(500, 344);
            this->fcgCBfastpskip->Name = L"fcgCBfastpskip";
            this->fcgCBfastpskip->Size = System::Drawing::Size(95, 18);
            this->fcgCBfastpskip->TabIndex = 13;
            this->fcgCBfastpskip->Tag = L"reCmd";
            this->fcgCBfastpskip->Text = L"スキップMB検出";
            this->fcgCBfastpskip->UseVisualStyleBackColor = true;
            // 
            // fcggroupBoxME
            // 
            this->fcggroupBoxME->Controls->Add(this->fcgCBMixedRef);
            this->fcggroupBoxME->Controls->Add(this->fcgCBChromaME);
            this->fcggroupBoxME->Controls->Add(this->fcgNURef);
            this->fcggroupBoxME->Controls->Add(this->fcgNUMERange);
            this->fcggroupBoxME->Controls->Add(this->fcgCXDirectME);
            this->fcggroupBoxME->Controls->Add(this->fcgCXSubME);
            this->fcggroupBoxME->Controls->Add(this->fcgCXME);
            this->fcggroupBoxME->Controls->Add(this->fcgLBMixedRef);
            this->fcggroupBoxME->Controls->Add(this->fcgLBChromaME);
            this->fcggroupBoxME->Controls->Add(this->fcgLBRef);
            this->fcggroupBoxME->Controls->Add(this->fcgLBDirectME);
            this->fcggroupBoxME->Controls->Add(this->fcgLBMERange);
            this->fcggroupBoxME->Controls->Add(this->fcgLBSubME);
            this->fcggroupBoxME->Controls->Add(this->fcgLBME);
            this->fcggroupBoxME->Location = System::Drawing::Point(5, 4);
            this->fcggroupBoxME->Name = L"fcggroupBoxME";
            this->fcggroupBoxME->Size = System::Drawing::Size(360, 181);
            this->fcggroupBoxME->TabIndex = 0;
            this->fcggroupBoxME->TabStop = false;
            this->fcggroupBoxME->Text = L"動き予測";
            // 
            // fcgCBMixedRef
            // 
            this->fcgCBMixedRef->AutoSize = true;
            this->fcgCBMixedRef->Location = System::Drawing::Point(274, 147);
            this->fcgCBMixedRef->Name = L"fcgCBMixedRef";
            this->fcgCBMixedRef->Size = System::Drawing::Size(15, 14);
            this->fcgCBMixedRef->TabIndex = 6;
            this->fcgCBMixedRef->Tag = L"reCmd";
            this->fcgCBMixedRef->UseVisualStyleBackColor = true;
            // 
            // fcgCBChromaME
            // 
            this->fcgCBChromaME->AutoSize = true;
            this->fcgCBChromaME->Location = System::Drawing::Point(293, 84);
            this->fcgCBChromaME->Name = L"fcgCBChromaME";
            this->fcgCBChromaME->Size = System::Drawing::Size(15, 14);
            this->fcgCBChromaME->TabIndex = 3;
            this->fcgCBChromaME->Tag = L"reCmd";
            this->fcgCBChromaME->UseVisualStyleBackColor = true;
            // 
            // fcgNURef
            // 
            this->fcgNURef->Location = System::Drawing::Point(132, 143);
            this->fcgNURef->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
            this->fcgNURef->Name = L"fcgNURef";
            this->fcgNURef->Size = System::Drawing::Size(65, 21);
            this->fcgNURef->TabIndex = 5;
            this->fcgNURef->Tag = L"reCmd";
            this->fcgNURef->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUMERange
            // 
            this->fcgNUMERange->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
            this->fcgNUMERange->Location = System::Drawing::Point(132, 81);
            this->fcgNUMERange->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 64, 0, 0, 0 });
            this->fcgNUMERange->Name = L"fcgNUMERange";
            this->fcgNUMERange->Size = System::Drawing::Size(65, 21);
            this->fcgNUMERange->TabIndex = 2;
            this->fcgNUMERange->Tag = L"reCmd";
            this->fcgNUMERange->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCXDirectME
            // 
            this->fcgCXDirectME->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXDirectME->FormattingEnabled = true;
            this->fcgCXDirectME->Location = System::Drawing::Point(132, 111);
            this->fcgCXDirectME->Name = L"fcgCXDirectME";
            this->fcgCXDirectME->Size = System::Drawing::Size(191, 22);
            this->fcgCXDirectME->TabIndex = 4;
            this->fcgCXDirectME->Tag = L"reCmd";
            // 
            // fcgCXSubME
            // 
            this->fcgCXSubME->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXSubME->FormattingEnabled = true;
            this->fcgCXSubME->Location = System::Drawing::Point(132, 49);
            this->fcgCXSubME->Name = L"fcgCXSubME";
            this->fcgCXSubME->Size = System::Drawing::Size(219, 22);
            this->fcgCXSubME->TabIndex = 1;
            this->fcgCXSubME->Tag = L"reCmd";
            // 
            // fcgCXME
            // 
            this->fcgCXME->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXME->FormattingEnabled = true;
            this->fcgCXME->Location = System::Drawing::Point(132, 18);
            this->fcgCXME->Name = L"fcgCXME";
            this->fcgCXME->Size = System::Drawing::Size(219, 22);
            this->fcgCXME->TabIndex = 0;
            this->fcgCXME->Tag = L"reCmd";
            // 
            // fcgLBMixedRef
            // 
            this->fcgLBMixedRef->AutoSize = true;
            this->fcgLBMixedRef->Location = System::Drawing::Point(217, 145);
            this->fcgLBMixedRef->Name = L"fcgLBMixedRef";
            this->fcgLBMixedRef->Size = System::Drawing::Size(51, 14);
            this->fcgLBMixedRef->TabIndex = 6;
            this->fcgLBMixedRef->Text = L"混合参照";
            // 
            // fcgLBChromaME
            // 
            this->fcgLBChromaME->AutoSize = true;
            this->fcgLBChromaME->Location = System::Drawing::Point(217, 83);
            this->fcgLBChromaME->Name = L"fcgLBChromaME";
            this->fcgLBChromaME->Size = System::Drawing::Size(70, 14);
            this->fcgLBChromaME->TabIndex = 5;
            this->fcgLBChromaME->Text = L"色差動き予測";
            // 
            // fcgLBRef
            // 
            this->fcgLBRef->AutoSize = true;
            this->fcgLBRef->Location = System::Drawing::Point(15, 145);
            this->fcgLBRef->Name = L"fcgLBRef";
            this->fcgLBRef->Size = System::Drawing::Size(51, 14);
            this->fcgLBRef->TabIndex = 4;
            this->fcgLBRef->Text = L"参照距離";
            // 
            // fcgLBDirectME
            // 
            this->fcgLBDirectME->AutoSize = true;
            this->fcgLBDirectME->Location = System::Drawing::Point(15, 114);
            this->fcgLBDirectME->Name = L"fcgLBDirectME";
            this->fcgLBDirectME->Size = System::Drawing::Size(70, 14);
            this->fcgLBDirectME->TabIndex = 3;
            this->fcgLBDirectME->Text = L"動き予測方式";
            // 
            // fcgLBMERange
            // 
            this->fcgLBMERange->AutoSize = true;
            this->fcgLBMERange->Location = System::Drawing::Point(15, 83);
            this->fcgLBMERange->Name = L"fcgLBMERange";
            this->fcgLBMERange->Size = System::Drawing::Size(70, 14);
            this->fcgLBMERange->TabIndex = 2;
            this->fcgLBMERange->Text = L"動き探索範囲";
            // 
            // fcgLBSubME
            // 
            this->fcgLBSubME->AutoSize = true;
            this->fcgLBSubME->Location = System::Drawing::Point(15, 52);
            this->fcgLBSubME->Name = L"fcgLBSubME";
            this->fcgLBSubME->Size = System::Drawing::Size(99, 14);
            this->fcgLBSubME->TabIndex = 1;
            this->fcgLBSubME->Text = L"サブピクセル動き予測";
            // 
            // fcgLBME
            // 
            this->fcgLBME->AutoSize = true;
            this->fcgLBME->Location = System::Drawing::Point(15, 21);
            this->fcgLBME->Name = L"fcgLBME";
            this->fcgLBME->Size = System::Drawing::Size(97, 14);
            this->fcgLBME->TabIndex = 0;
            this->fcgLBME->Text = L"動き予測アルゴリズム";
            // 
            // fcggroupBoxMBTypes
            // 
            this->fcggroupBoxMBTypes->Controls->Add(this->fcgCBi4x4);
            this->fcggroupBoxMBTypes->Controls->Add(this->fcgCBp4x4);
            this->fcggroupBoxMBTypes->Controls->Add(this->fcgCBi8x8);
            this->fcggroupBoxMBTypes->Controls->Add(this->fcgCBb8x8);
            this->fcggroupBoxMBTypes->Controls->Add(this->fcgCBp8x8);
            this->fcggroupBoxMBTypes->Controls->Add(this->fcgCB8x8dct);
            this->fcggroupBoxMBTypes->Location = System::Drawing::Point(5, 368);
            this->fcggroupBoxMBTypes->Name = L"fcggroupBoxMBTypes";
            this->fcggroupBoxMBTypes->Size = System::Drawing::Size(244, 99);
            this->fcggroupBoxMBTypes->TabIndex = 2;
            this->fcggroupBoxMBTypes->TabStop = false;
            this->fcggroupBoxMBTypes->Text = L"マクロブロック";
            // 
            // fcgCBi4x4
            // 
            this->fcgCBi4x4->AutoSize = true;
            this->fcgCBi4x4->Location = System::Drawing::Point(95, 71);
            this->fcgCBi4x4->Name = L"fcgCBi4x4";
            this->fcgCBi4x4->Size = System::Drawing::Size(49, 18);
            this->fcgCBi4x4->TabIndex = 5;
            this->fcgCBi4x4->Tag = L"reCmd";
            this->fcgCBi4x4->Text = L"i4x4";
            this->fcgCBi4x4->UseVisualStyleBackColor = true;
            // 
            // fcgCBp4x4
            // 
            this->fcgCBp4x4->AutoSize = true;
            this->fcgCBp4x4->Location = System::Drawing::Point(175, 46);
            this->fcgCBp4x4->Name = L"fcgCBp4x4";
            this->fcgCBp4x4->Size = System::Drawing::Size(53, 18);
            this->fcgCBp4x4->TabIndex = 3;
            this->fcgCBp4x4->Tag = L"reCmd";
            this->fcgCBp4x4->Text = L"p4x4";
            this->fcgCBp4x4->UseVisualStyleBackColor = true;
            // 
            // fcgCBi8x8
            // 
            this->fcgCBi8x8->AutoSize = true;
            this->fcgCBi8x8->Location = System::Drawing::Point(13, 71);
            this->fcgCBi8x8->Name = L"fcgCBi8x8";
            this->fcgCBi8x8->Size = System::Drawing::Size(49, 18);
            this->fcgCBi8x8->TabIndex = 4;
            this->fcgCBi8x8->Tag = L"reCmd";
            this->fcgCBi8x8->Text = L"i8x8";
            this->fcgCBi8x8->UseVisualStyleBackColor = true;
            // 
            // fcgCBb8x8
            // 
            this->fcgCBb8x8->AutoSize = true;
            this->fcgCBb8x8->Location = System::Drawing::Point(95, 46);
            this->fcgCBb8x8->Name = L"fcgCBb8x8";
            this->fcgCBb8x8->Size = System::Drawing::Size(53, 18);
            this->fcgCBb8x8->TabIndex = 2;
            this->fcgCBb8x8->Tag = L"reCmd";
            this->fcgCBb8x8->Text = L"b8x8";
            this->fcgCBb8x8->UseVisualStyleBackColor = true;
            // 
            // fcgCBp8x8
            // 
            this->fcgCBp8x8->AutoSize = true;
            this->fcgCBp8x8->Location = System::Drawing::Point(13, 46);
            this->fcgCBp8x8->Name = L"fcgCBp8x8";
            this->fcgCBp8x8->Size = System::Drawing::Size(53, 18);
            this->fcgCBp8x8->TabIndex = 1;
            this->fcgCBp8x8->Tag = L"reCmd";
            this->fcgCBp8x8->Text = L"p8x8";
            this->fcgCBp8x8->UseVisualStyleBackColor = true;
            // 
            // fcgCB8x8dct
            // 
            this->fcgCB8x8dct->AutoSize = true;
            this->fcgCB8x8dct->Location = System::Drawing::Point(13, 20);
            this->fcgCB8x8dct->Name = L"fcgCB8x8dct";
            this->fcgCB8x8dct->Size = System::Drawing::Size(127, 18);
            this->fcgCB8x8dct->TabIndex = 0;
            this->fcgCB8x8dct->Tag = L"reCmd";
            this->fcgCB8x8dct->Text = L"8x8 離散コサイン変換";
            this->fcgCB8x8dct->UseVisualStyleBackColor = true;
            // 
            // fcgCXWeightP
            // 
            this->fcgCXWeightP->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXWeightP->FormattingEnabled = true;
            this->fcgCXWeightP->Location = System::Drawing::Point(482, 275);
            this->fcgCXWeightP->Name = L"fcgCXWeightP";
            this->fcgCXWeightP->Size = System::Drawing::Size(112, 22);
            this->fcgCXWeightP->TabIndex = 10;
            this->fcgCXWeightP->Tag = L"reCmd";
            // 
            // fcgLBWeightP
            // 
            this->fcgLBWeightP->AutoSize = true;
            this->fcgLBWeightP->Location = System::Drawing::Point(373, 278);
            this->fcgLBWeightP->Name = L"fcgLBWeightP";
            this->fcgLBWeightP->Size = System::Drawing::Size(87, 14);
            this->fcgLBWeightP->TabIndex = 25;
            this->fcgLBWeightP->Text = L"重み付きPフレーム";
            // 
            // fcggroupBoxBframes
            // 
            this->fcggroupBoxBframes->Controls->Add(this->fcgCXBpyramid);
            this->fcggroupBoxBframes->Controls->Add(this->fcgCXBAdpapt);
            this->fcggroupBoxBframes->Controls->Add(this->fcgCBWeightB);
            this->fcggroupBoxBframes->Controls->Add(this->fcgNUBBias);
            this->fcggroupBoxBframes->Controls->Add(this->fcgNUBframes);
            this->fcggroupBoxBframes->Controls->Add(this->fcgLBWeightB);
            this->fcggroupBoxBframes->Controls->Add(this->fcgLBBpyramid);
            this->fcggroupBoxBframes->Controls->Add(this->fcgLBBBias);
            this->fcggroupBoxBframes->Controls->Add(this->fcgLBBAdapt);
            this->fcggroupBoxBframes->Controls->Add(this->fcgLBBframes);
            this->fcggroupBoxBframes->Location = System::Drawing::Point(5, 191);
            this->fcggroupBoxBframes->Name = L"fcggroupBoxBframes";
            this->fcggroupBoxBframes->Size = System::Drawing::Size(244, 171);
            this->fcggroupBoxBframes->TabIndex = 1;
            this->fcggroupBoxBframes->TabStop = false;
            this->fcggroupBoxBframes->Text = L"Bフレーム";
            // 
            // fcgCXBpyramid
            // 
            this->fcgCXBpyramid->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXBpyramid->FormattingEnabled = true;
            this->fcgCXBpyramid->Location = System::Drawing::Point(155, 113);
            this->fcgCXBpyramid->Name = L"fcgCXBpyramid";
            this->fcgCXBpyramid->Size = System::Drawing::Size(71, 22);
            this->fcgCXBpyramid->TabIndex = 3;
            this->fcgCXBpyramid->Tag = L"reCmd";
            // 
            // fcgCXBAdpapt
            // 
            this->fcgCXBAdpapt->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXBAdpapt->FormattingEnabled = true;
            this->fcgCXBAdpapt->Location = System::Drawing::Point(155, 54);
            this->fcgCXBAdpapt->Name = L"fcgCXBAdpapt";
            this->fcgCXBAdpapt->Size = System::Drawing::Size(71, 22);
            this->fcgCXBAdpapt->TabIndex = 1;
            this->fcgCXBAdpapt->Tag = L"reCmd";
            // 
            // fcgCBWeightB
            // 
            this->fcgCBWeightB->AutoSize = true;
            this->fcgCBWeightB->Location = System::Drawing::Point(211, 147);
            this->fcgCBWeightB->Name = L"fcgCBWeightB";
            this->fcgCBWeightB->Size = System::Drawing::Size(15, 14);
            this->fcgCBWeightB->TabIndex = 4;
            this->fcgCBWeightB->Tag = L"reCmd";
            this->fcgCBWeightB->UseVisualStyleBackColor = true;
            // 
            // fcgNUBBias
            // 
            this->fcgNUBBias->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
            this->fcgNUBBias->Location = System::Drawing::Point(172, 85);
            this->fcgNUBBias->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 100, 0, 0, System::Int32::MinValue });
            this->fcgNUBBias->Name = L"fcgNUBBias";
            this->fcgNUBBias->Size = System::Drawing::Size(54, 21);
            this->fcgNUBBias->TabIndex = 2;
            this->fcgNUBBias->Tag = L"reCmd";
            this->fcgNUBBias->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUBframes
            // 
            this->fcgNUBframes->Location = System::Drawing::Point(172, 23);
            this->fcgNUBframes->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
            this->fcgNUBframes->Name = L"fcgNUBframes";
            this->fcgNUBframes->Size = System::Drawing::Size(54, 21);
            this->fcgNUBframes->TabIndex = 0;
            this->fcgNUBframes->Tag = L"reCmd";
            this->fcgNUBframes->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBWeightB
            // 
            this->fcgLBWeightB->AutoSize = true;
            this->fcgLBWeightB->Location = System::Drawing::Point(16, 146);
            this->fcgLBWeightB->Name = L"fcgLBWeightB";
            this->fcgLBWeightB->Size = System::Drawing::Size(87, 14);
            this->fcgLBWeightB->TabIndex = 4;
            this->fcgLBWeightB->Text = L"重み付きBフレーム";
            // 
            // fcgLBBpyramid
            // 
            this->fcgLBBpyramid->AutoSize = true;
            this->fcgLBBpyramid->Location = System::Drawing::Point(16, 116);
            this->fcgLBBpyramid->Name = L"fcgLBBpyramid";
            this->fcgLBBpyramid->Size = System::Drawing::Size(66, 14);
            this->fcgLBBpyramid->TabIndex = 3;
            this->fcgLBBpyramid->Text = L"ピラミッド参照";
            // 
            // fcgLBBBias
            // 
            this->fcgLBBBias->AutoSize = true;
            this->fcgLBBBias->Location = System::Drawing::Point(16, 87);
            this->fcgLBBBias->Name = L"fcgLBBBias";
            this->fcgLBBBias->Size = System::Drawing::Size(91, 14);
            this->fcgLBBBias->TabIndex = 2;
            this->fcgLBBBias->Text = L"Bフレーム挿入傾向";
            // 
            // fcgLBBAdapt
            // 
            this->fcgLBBAdapt->AutoSize = true;
            this->fcgLBBAdapt->Location = System::Drawing::Point(16, 57);
            this->fcgLBBAdapt->Name = L"fcgLBBAdapt";
            this->fcgLBBAdapt->Size = System::Drawing::Size(102, 14);
            this->fcgLBBAdapt->TabIndex = 1;
            this->fcgLBBAdapt->Text = L"適応的Bフレーム挿入";
            // 
            // fcgLBBframes
            // 
            this->fcgLBBframes->AutoSize = true;
            this->fcgLBBframes->Location = System::Drawing::Point(16, 25);
            this->fcgLBBframes->Name = L"fcgLBBframes";
            this->fcgLBBframes->Size = System::Drawing::Size(102, 14);
            this->fcgLBBframes->TabIndex = 0;
            this->fcgLBBframes->Text = L"最大連続Bフレーム数";
            // 
            // fcgCBCABAC
            // 
            this->fcgCBCABAC->AutoSize = true;
            this->fcgCBCABAC->Location = System::Drawing::Point(376, 314);
            this->fcgCBCABAC->Name = L"fcgCBCABAC";
            this->fcgCBCABAC->Size = System::Drawing::Size(61, 18);
            this->fcgCBCABAC->TabIndex = 11;
            this->fcgCBCABAC->Tag = L"reCmd";
            this->fcgCBCABAC->Text = L"CABAC";
            this->fcgCBCABAC->UseVisualStyleBackColor = true;
            // 
            // fcggroupBoxGOP
            // 
            this->fcggroupBoxGOP->Controls->Add(this->fcgLBOpenGOP);
            this->fcggroupBoxGOP->Controls->Add(this->fcgCBOpenGOP);
            this->fcggroupBoxGOP->Controls->Add(this->fcgNUKeyint);
            this->fcggroupBoxGOP->Controls->Add(this->fcgLBKeyint);
            this->fcggroupBoxGOP->Controls->Add(this->fcgNUMinKeyint);
            this->fcggroupBoxGOP->Controls->Add(this->fcgLBMinKeyint);
            this->fcggroupBoxGOP->Controls->Add(this->fcgNUScenecut);
            this->fcggroupBoxGOP->Controls->Add(this->fcgLBScenecut);
            this->fcggroupBoxGOP->Location = System::Drawing::Point(376, 3);
            this->fcggroupBoxGOP->Name = L"fcggroupBoxGOP";
            this->fcggroupBoxGOP->Size = System::Drawing::Size(224, 133);
            this->fcggroupBoxGOP->TabIndex = 6;
            this->fcggroupBoxGOP->TabStop = false;
            this->fcggroupBoxGOP->Text = L"GOP関連設定";
            // 
            // fcgLBOpenGOP
            // 
            this->fcgLBOpenGOP->AutoSize = true;
            this->fcgLBOpenGOP->Location = System::Drawing::Point(13, 108);
            this->fcgLBOpenGOP->Name = L"fcgLBOpenGOP";
            this->fcgLBOpenGOP->Size = System::Drawing::Size(62, 14);
            this->fcgLBOpenGOP->TabIndex = 7;
            this->fcgLBOpenGOP->Text = L"Open GOP";
            // 
            // fcgCBOpenGOP
            // 
            this->fcgCBOpenGOP->AutoSize = true;
            this->fcgCBOpenGOP->Location = System::Drawing::Point(195, 109);
            this->fcgCBOpenGOP->Name = L"fcgCBOpenGOP";
            this->fcgCBOpenGOP->Size = System::Drawing::Size(15, 14);
            this->fcgCBOpenGOP->TabIndex = 3;
            this->fcgCBOpenGOP->Tag = L"reCmd";
            this->fcgCBOpenGOP->UseVisualStyleBackColor = true;
            // 
            // fcgNUKeyint
            // 
            this->fcgNUKeyint->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
            this->fcgNUKeyint->Location = System::Drawing::Point(155, 79);
            this->fcgNUKeyint->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3000, 0, 0, 0 });
            this->fcgNUKeyint->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, System::Int32::MinValue });
            this->fcgNUKeyint->Name = L"fcgNUKeyint";
            this->fcgNUKeyint->Size = System::Drawing::Size(55, 21);
            this->fcgNUKeyint->TabIndex = 2;
            this->fcgNUKeyint->Tag = L"reCmd";
            this->fcgNUKeyint->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBKeyint
            // 
            this->fcgLBKeyint->AutoSize = true;
            this->fcgLBKeyint->Location = System::Drawing::Point(13, 81);
            this->fcgLBKeyint->Name = L"fcgLBKeyint";
            this->fcgLBKeyint->Size = System::Drawing::Size(111, 14);
            this->fcgLBKeyint->TabIndex = 4;
            this->fcgLBKeyint->Text = L"キーフレーム間隔の上限";
            // 
            // fcgNUMinKeyint
            // 
            this->fcgNUMinKeyint->Location = System::Drawing::Point(155, 50);
            this->fcgNUMinKeyint->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 300, 0, 0, 0 });
            this->fcgNUMinKeyint->Name = L"fcgNUMinKeyint";
            this->fcgNUMinKeyint->Size = System::Drawing::Size(55, 21);
            this->fcgNUMinKeyint->TabIndex = 1;
            this->fcgNUMinKeyint->Tag = L"reCmd";
            this->fcgNUMinKeyint->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBMinKeyint
            // 
            this->fcgLBMinKeyint->AutoSize = true;
            this->fcgLBMinKeyint->Location = System::Drawing::Point(13, 53);
            this->fcgLBMinKeyint->Name = L"fcgLBMinKeyint";
            this->fcgLBMinKeyint->Size = System::Drawing::Size(111, 14);
            this->fcgLBMinKeyint->TabIndex = 2;
            this->fcgLBMinKeyint->Text = L"キーフレーム間隔の下限";
            // 
            // fcgNUScenecut
            // 
            this->fcgNUScenecut->Location = System::Drawing::Point(155, 22);
            this->fcgNUScenecut->Name = L"fcgNUScenecut";
            this->fcgNUScenecut->Size = System::Drawing::Size(55, 21);
            this->fcgNUScenecut->TabIndex = 0;
            this->fcgNUScenecut->Tag = L"reCmd";
            this->fcgNUScenecut->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBScenecut
            // 
            this->fcgLBScenecut->AutoSize = true;
            this->fcgLBScenecut->Location = System::Drawing::Point(13, 24);
            this->fcgLBScenecut->Name = L"fcgLBScenecut";
            this->fcgLBScenecut->Size = System::Drawing::Size(77, 14);
            this->fcgLBScenecut->TabIndex = 0;
            this->fcgLBScenecut->Text = L"シーンカット閾値";
            // 
            // fcggroupBoxDeblock
            // 
            this->fcggroupBoxDeblock->Controls->Add(this->fcgNUDeblockThreshold);
            this->fcggroupBoxDeblock->Controls->Add(this->fcgNUDeblockStrength);
            this->fcggroupBoxDeblock->Controls->Add(this->fcgLBDeblockThreshold);
            this->fcggroupBoxDeblock->Controls->Add(this->fcgLBDeblockStrength);
            this->fcggroupBoxDeblock->Location = System::Drawing::Point(376, 146);
            this->fcggroupBoxDeblock->Name = L"fcggroupBoxDeblock";
            this->fcggroupBoxDeblock->Size = System::Drawing::Size(224, 85);
            this->fcggroupBoxDeblock->TabIndex = 8;
            this->fcggroupBoxDeblock->TabStop = false;
            // 
            // fcgNUDeblockThreshold
            // 
            this->fcgNUDeblockThreshold->Location = System::Drawing::Point(155, 53);
            this->fcgNUDeblockThreshold->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 6, 0, 0, 0 });
            this->fcgNUDeblockThreshold->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 6, 0, 0, System::Int32::MinValue });
            this->fcgNUDeblockThreshold->Name = L"fcgNUDeblockThreshold";
            this->fcgNUDeblockThreshold->Size = System::Drawing::Size(54, 21);
            this->fcgNUDeblockThreshold->TabIndex = 1;
            this->fcgNUDeblockThreshold->Tag = L"reCmd";
            this->fcgNUDeblockThreshold->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUDeblockStrength
            // 
            this->fcgNUDeblockStrength->Location = System::Drawing::Point(155, 24);
            this->fcgNUDeblockStrength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 6, 0, 0, 0 });
            this->fcgNUDeblockStrength->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 6, 0, 0, System::Int32::MinValue });
            this->fcgNUDeblockStrength->Name = L"fcgNUDeblockStrength";
            this->fcgNUDeblockStrength->Size = System::Drawing::Size(54, 21);
            this->fcgNUDeblockStrength->TabIndex = 0;
            this->fcgNUDeblockStrength->Tag = L"reCmd";
            this->fcgNUDeblockStrength->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBDeblockThreshold
            // 
            this->fcgLBDeblockThreshold->AutoSize = true;
            this->fcgLBDeblockThreshold->Location = System::Drawing::Point(16, 55);
            this->fcgLBDeblockThreshold->Name = L"fcgLBDeblockThreshold";
            this->fcgLBDeblockThreshold->Size = System::Drawing::Size(68, 14);
            this->fcgLBDeblockThreshold->TabIndex = 1;
            this->fcgLBDeblockThreshold->Text = L"デブロック閾値";
            // 
            // fcgLBDeblockStrength
            // 
            this->fcgLBDeblockStrength->AutoSize = true;
            this->fcgLBDeblockStrength->Location = System::Drawing::Point(16, 26);
            this->fcgLBDeblockStrength->Name = L"fcgLBDeblockStrength";
            this->fcgLBDeblockStrength->Size = System::Drawing::Size(68, 14);
            this->fcgLBDeblockStrength->TabIndex = 0;
            this->fcgLBDeblockStrength->Text = L"デブロック強度";
            // 
            // fcgtabPageExSettings
            // 
            this->fcgtabPageExSettings->Controls->Add(this->fcgBTX264PathSub);
            this->fcgtabPageExSettings->Controls->Add(this->fcgTXX264PathSub);
            this->fcgtabPageExSettings->Controls->Add(this->fcgLBX264PathSub);
            this->fcgtabPageExSettings->Controls->Add(this->fcgLBTempDir);
            this->fcgtabPageExSettings->Controls->Add(this->fcgBTCustomTempDir);
            this->fcgtabPageExSettings->Controls->Add(this->fcggroupBoxCmdEx);
            this->fcgtabPageExSettings->Controls->Add(this->fcgTXCustomTempDir);
            this->fcgtabPageExSettings->Controls->Add(this->fcgCXTempDir);
            this->fcgtabPageExSettings->Controls->Add(this->fcgCXX264Priority);
            this->fcgtabPageExSettings->Controls->Add(this->fcgLBX264Priority);
            this->fcgtabPageExSettings->Controls->Add(this->fcggroupBoxExSettings);
            this->fcgtabPageExSettings->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageExSettings->Name = L"fcgtabPageExSettings";
            this->fcgtabPageExSettings->Size = System::Drawing::Size(608, 491);
            this->fcgtabPageExSettings->TabIndex = 3;
            this->fcgtabPageExSettings->Text = L"拡張";
            this->fcgtabPageExSettings->UseVisualStyleBackColor = true;
            // 
            // fcgBTX264PathSub
            // 
            this->fcgBTX264PathSub->Location = System::Drawing::Point(575, 33);
            this->fcgBTX264PathSub->Name = L"fcgBTX264PathSub";
            this->fcgBTX264PathSub->Size = System::Drawing::Size(27, 22);
            this->fcgBTX264PathSub->TabIndex = 2;
            this->fcgBTX264PathSub->Text = L"...";
            this->fcgBTX264PathSub->UseVisualStyleBackColor = true;
            this->fcgBTX264PathSub->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTX264PathSub_Click);
            // 
            // fcgTXX264PathSub
            // 
            this->fcgTXX264PathSub->AllowDrop = true;
            this->fcgTXX264PathSub->Location = System::Drawing::Point(377, 34);
            this->fcgTXX264PathSub->Name = L"fcgTXX264PathSub";
            this->fcgTXX264PathSub->Size = System::Drawing::Size(194, 21);
            this->fcgTXX264PathSub->TabIndex = 1;
            this->fcgTXX264PathSub->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXX264PathSub_TextChanged);
            this->fcgTXX264PathSub->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXX264PathSub_Enter);
            this->fcgTXX264PathSub->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXX264PathSub_Leave);
            // 
            // fcgLBX264PathSub
            // 
            this->fcgLBX264PathSub->AutoSize = true;
            this->fcgLBX264PathSub->Location = System::Drawing::Point(355, 13);
            this->fcgLBX264PathSub->Name = L"fcgLBX264PathSub";
            this->fcgLBX264PathSub->Size = System::Drawing::Size(49, 14);
            this->fcgLBX264PathSub->TabIndex = 5;
            this->fcgLBX264PathSub->Text = L"～の指定";
            // 
            // fcgLBTempDir
            // 
            this->fcgLBTempDir->AutoSize = true;
            this->fcgLBTempDir->Location = System::Drawing::Point(362, 154);
            this->fcgLBTempDir->Name = L"fcgLBTempDir";
            this->fcgLBTempDir->Size = System::Drawing::Size(60, 14);
            this->fcgLBTempDir->TabIndex = 4;
            this->fcgLBTempDir->Text = L"一時フォルダ";
            // 
            // fcgBTCustomTempDir
            // 
            this->fcgBTCustomTempDir->Location = System::Drawing::Point(562, 207);
            this->fcgBTCustomTempDir->Name = L"fcgBTCustomTempDir";
            this->fcgBTCustomTempDir->Size = System::Drawing::Size(29, 23);
            this->fcgBTCustomTempDir->TabIndex = 6;
            this->fcgBTCustomTempDir->Text = L"...";
            this->fcgBTCustomTempDir->UseVisualStyleBackColor = true;
            this->fcgBTCustomTempDir->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTCustomTempDir_Click);
            // 
            // fcggroupBoxCmdEx
            // 
            this->fcggroupBoxCmdEx->Controls->Add(this->fcgCXCmdExInsert);
            this->fcggroupBoxCmdEx->Controls->Add(this->fcgCBNulOutCLI);
            this->fcggroupBoxCmdEx->Controls->Add(this->fcgBTCmdEx);
            this->fcggroupBoxCmdEx->Controls->Add(this->fcgTXCmdEx);
            this->fcggroupBoxCmdEx->Location = System::Drawing::Point(8, 243);
            this->fcggroupBoxCmdEx->Name = L"fcggroupBoxCmdEx";
            this->fcggroupBoxCmdEx->Size = System::Drawing::Size(589, 233);
            this->fcggroupBoxCmdEx->TabIndex = 7;
            this->fcggroupBoxCmdEx->TabStop = false;
            this->fcggroupBoxCmdEx->Text = L"追加コマンド";
            // 
            // fcgCXCmdExInsert
            // 
            this->fcgCXCmdExInsert->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXCmdExInsert->FormattingEnabled = true;
            this->fcgCXCmdExInsert->Location = System::Drawing::Point(197, 204);
            this->fcgCXCmdExInsert->Name = L"fcgCXCmdExInsert";
            this->fcgCXCmdExInsert->Size = System::Drawing::Size(168, 22);
            this->fcgCXCmdExInsert->TabIndex = 4;
            this->fcgCXCmdExInsert->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXCmdExInsert_SelectedIndexChanged);
            this->fcgCXCmdExInsert->FontChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXCmdExInsert_FontChanged);
            // 
            // fcgCBNulOutCLI
            // 
            this->fcgCBNulOutCLI->AutoSize = true;
            this->fcgCBNulOutCLI->Location = System::Drawing::Point(18, 206);
            this->fcgCBNulOutCLI->Name = L"fcgCBNulOutCLI";
            this->fcgCBNulOutCLI->Size = System::Drawing::Size(65, 18);
            this->fcgCBNulOutCLI->TabIndex = 2;
            this->fcgCBNulOutCLI->Tag = L"chValue";
            this->fcgCBNulOutCLI->Text = L"nul出力";
            this->fcgCBNulOutCLI->UseVisualStyleBackColor = true;
            // 
            // fcgBTCmdEx
            // 
            this->fcgBTCmdEx->Location = System::Drawing::Point(482, 204);
            this->fcgBTCmdEx->Name = L"fcgBTCmdEx";
            this->fcgBTCmdEx->Size = System::Drawing::Size(97, 23);
            this->fcgBTCmdEx->TabIndex = 1;
            this->fcgBTCmdEx->Text = L"GUI部に反映";
            this->fcgBTCmdEx->UseVisualStyleBackColor = true;
            this->fcgBTCmdEx->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTCmdEx_Click);
            // 
            // fcgTXCmdEx
            // 
            this->fcgTXCmdEx->AllowDrop = true;
            this->fcgTXCmdEx->Font = (gcnew System::Drawing::Font(L"ＭＳ ゴシック", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgTXCmdEx->Location = System::Drawing::Point(6, 20);
            this->fcgTXCmdEx->Multiline = true;
            this->fcgTXCmdEx->Name = L"fcgTXCmdEx";
            this->fcgTXCmdEx->Size = System::Drawing::Size(577, 180);
            this->fcgTXCmdEx->TabIndex = 0;
            this->fcgTXCmdEx->Tag = L"chValue";
            this->fcgTXCmdEx->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgInsertDragDropFilename_DragDrop);
            this->fcgTXCmdEx->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgInsertDragDropFilename_Enter);
            this->fcgTXCmdEx->DragOver += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgInsertDragDropFilename_DragOver);
            // 
            // fcgTXCustomTempDir
            // 
            this->fcgTXCustomTempDir->Location = System::Drawing::Point(377, 208);
            this->fcgTXCustomTempDir->Name = L"fcgTXCustomTempDir";
            this->fcgTXCustomTempDir->Size = System::Drawing::Size(182, 21);
            this->fcgTXCustomTempDir->TabIndex = 5;
            this->fcgTXCustomTempDir->Tag = L"";
            this->fcgTXCustomTempDir->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXCustomTempDir_TextChanged);
            // 
            // fcgCXTempDir
            // 
            this->fcgCXTempDir->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXTempDir->FormattingEnabled = true;
            this->fcgCXTempDir->Location = System::Drawing::Point(365, 180);
            this->fcgCXTempDir->Name = L"fcgCXTempDir";
            this->fcgCXTempDir->Size = System::Drawing::Size(209, 22);
            this->fcgCXTempDir->TabIndex = 4;
            this->fcgCXTempDir->Tag = L"chValue";
            // 
            // fcgCXX264Priority
            // 
            this->fcgCXX264Priority->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXX264Priority->FormattingEnabled = true;
            this->fcgCXX264Priority->Location = System::Drawing::Point(435, 100);
            this->fcgCXX264Priority->Name = L"fcgCXX264Priority";
            this->fcgCXX264Priority->Size = System::Drawing::Size(152, 22);
            this->fcgCXX264Priority->TabIndex = 3;
            this->fcgCXX264Priority->Tag = L"chValue";
            // 
            // fcgLBX264Priority
            // 
            this->fcgLBX264Priority->AutoSize = true;
            this->fcgLBX264Priority->Location = System::Drawing::Point(362, 103);
            this->fcgLBX264Priority->Name = L"fcgLBX264Priority";
            this->fcgLBX264Priority->Size = System::Drawing::Size(67, 14);
            this->fcgLBX264Priority->TabIndex = 1;
            this->fcgLBX264Priority->Text = L"x264優先度";
            // 
            // fcggroupBoxExSettings
            // 
            this->fcggroupBoxExSettings->Controls->Add(this->fcgCBSetKeyframeAtChapter);
            this->fcggroupBoxExSettings->Controls->Add(this->fcgCBInputAsLW48);
            this->fcggroupBoxExSettings->Controls->Add(this->fcgCBCheckKeyframes);
            this->fcggroupBoxExSettings->Controls->Add(this->fcgCBAuoTcfileout);
            this->fcggroupBoxExSettings->Controls->Add(this->fcgCBAFSBitrateCorrection);
            this->fcggroupBoxExSettings->Controls->Add(this->fcgCBAFS);
            this->fcggroupBoxExSettings->Location = System::Drawing::Point(8, 3);
            this->fcggroupBoxExSettings->Name = L"fcggroupBoxExSettings";
            this->fcggroupBoxExSettings->Size = System::Drawing::Size(323, 234);
            this->fcggroupBoxExSettings->TabIndex = 0;
            this->fcggroupBoxExSettings->TabStop = false;
            this->fcggroupBoxExSettings->Text = L"拡張設定";
            // 
            // fcgCBSetKeyframeAtChapter
            // 
            this->fcgCBSetKeyframeAtChapter->AutoSize = true;
            this->fcgCBSetKeyframeAtChapter->Location = System::Drawing::Point(18, 140);
            this->fcgCBSetKeyframeAtChapter->Name = L"fcgCBSetKeyframeAtChapter";
            this->fcgCBSetKeyframeAtChapter->Size = System::Drawing::Size(267, 18);
            this->fcgCBSetKeyframeAtChapter->TabIndex = 5;
            this->fcgCBSetKeyframeAtChapter->Tag = L"chValue";
            this->fcgCBSetKeyframeAtChapter->Text = L"チャプター位置にキーフレームを設定する (mux有効時)";
            this->fcgCBSetKeyframeAtChapter->UseVisualStyleBackColor = true;
            // 
            // fcgCBInputAsLW48
            // 
            this->fcgCBInputAsLW48->AutoSize = true;
            this->fcgCBInputAsLW48->Location = System::Drawing::Point(18, 179);
            this->fcgCBInputAsLW48->Name = L"fcgCBInputAsLW48";
            this->fcgCBInputAsLW48->Size = System::Drawing::Size(81, 18);
            this->fcgCBInputAsLW48->TabIndex = 4;
            this->fcgCBInputAsLW48->Tag = L"chValue";
            this->fcgCBInputAsLW48->Text = L"LW48モード";
            this->fcgCBInputAsLW48->UseVisualStyleBackColor = true;
            // 
            // fcgCBCheckKeyframes
            // 
            this->fcgCBCheckKeyframes->AutoSize = true;
            this->fcgCBCheckKeyframes->Location = System::Drawing::Point(18, 116);
            this->fcgCBCheckKeyframes->Name = L"fcgCBCheckKeyframes";
            this->fcgCBCheckKeyframes->Size = System::Drawing::Size(186, 18);
            this->fcgCBCheckKeyframes->TabIndex = 3;
            this->fcgCBCheckKeyframes->Tag = L"chValue";
            this->fcgCBCheckKeyframes->Text = L"Aviutlのキーフレーム設定検出を行う";
            this->fcgCBCheckKeyframes->UseVisualStyleBackColor = true;
            // 
            // fcgCBAuoTcfileout
            // 
            this->fcgCBAuoTcfileout->AutoSize = true;
            this->fcgCBAuoTcfileout->Location = System::Drawing::Point(18, 92);
            this->fcgCBAuoTcfileout->Name = L"fcgCBAuoTcfileout";
            this->fcgCBAuoTcfileout->Size = System::Drawing::Size(98, 18);
            this->fcgCBAuoTcfileout->TabIndex = 2;
            this->fcgCBAuoTcfileout->Tag = L"chValue";
            this->fcgCBAuoTcfileout->Text = L"タイムコード出力";
            this->fcgCBAuoTcfileout->UseVisualStyleBackColor = true;
            // 
            // fcgCBAFSBitrateCorrection
            // 
            this->fcgCBAFSBitrateCorrection->AutoSize = true;
            this->fcgCBAFSBitrateCorrection->Location = System::Drawing::Point(34, 53);
            this->fcgCBAFSBitrateCorrection->Name = L"fcgCBAFSBitrateCorrection";
            this->fcgCBAFSBitrateCorrection->Size = System::Drawing::Size(278, 18);
            this->fcgCBAFSBitrateCorrection->TabIndex = 1;
            this->fcgCBAFSBitrateCorrection->Tag = L"chValue";
            this->fcgCBAFSBitrateCorrection->Text = L"設定したビットレートになるよう補正する(自動マルチパス時)";
            this->fcgCBAFSBitrateCorrection->UseVisualStyleBackColor = true;
            // 
            // fcgCBAFS
            // 
            this->fcgCBAFS->AutoSize = true;
            this->fcgCBAFS->Location = System::Drawing::Point(18, 29);
            this->fcgCBAFS->Name = L"fcgCBAFS";
            this->fcgCBAFS->Size = System::Drawing::Size(183, 18);
            this->fcgCBAFS->TabIndex = 0;
            this->fcgCBAFS->Tag = L"chValue";
            this->fcgCBAFS->Text = L"自動フィールドシフト(afs)を使用する";
            this->fcgCBAFS->UseVisualStyleBackColor = true;
            this->fcgCBAFS->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCBAFS_CheckedChanged);
            // 
            // fcgCSExeFiles
            // 
            this->fcgCSExeFiles->ImageScalingSize = System::Drawing::Size(18, 18);
            this->fcgCSExeFiles->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->fcgTSExeFileshelp });
            this->fcgCSExeFiles->Name = L"fcgCSx264";
            this->fcgCSExeFiles->Size = System::Drawing::Size(131, 26);
            // 
            // fcgTSExeFileshelp
            // 
            this->fcgTSExeFileshelp->Name = L"fcgTSExeFileshelp";
            this->fcgTSExeFileshelp->Size = System::Drawing::Size(130, 22);
            this->fcgTSExeFileshelp->Text = L"helpを表示";
            this->fcgTSExeFileshelp->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSExeFileshelp_Click);
            // 
            // fcgtabControlMux
            // 
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageMP4);
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageMKV);
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageMux);
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageBat);
            this->fcgtabControlMux->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgtabControlMux->Location = System::Drawing::Point(2, 5);
            this->fcgtabControlMux->Name = L"fcgtabControlMux";
            this->fcgtabControlMux->SelectedIndex = 0;
            this->fcgtabControlMux->Size = System::Drawing::Size(384, 214);
            this->fcgtabControlMux->TabIndex = 3;
            // 
            // fcgtabPageMP4
            // 
            this->fcgtabPageMP4->Controls->Add(this->fcgCBMP4MuxApple);
            this->fcgtabPageMP4->Controls->Add(this->fcgBTTC2MP4Path);
            this->fcgtabPageMP4->Controls->Add(this->fcgTXTC2MP4Path);
            this->fcgtabPageMP4->Controls->Add(this->fcgBTMP4MuxerPath);
            this->fcgtabPageMP4->Controls->Add(this->fcgTXMP4MuxerPath);
            this->fcgtabPageMP4->Controls->Add(this->fcgLBTC2MP4Path);
            this->fcgtabPageMP4->Controls->Add(this->fcgLBMP4MuxerPath);
            this->fcgtabPageMP4->Controls->Add(this->fcgCXMP4CmdEx);
            this->fcgtabPageMP4->Controls->Add(this->fcgLBMP4CmdEx);
            this->fcgtabPageMP4->Controls->Add(this->fcgCBMP4MuxerExt);
            this->fcgtabPageMP4->Controls->Add(this->fcgBTMP4RawPath);
            this->fcgtabPageMP4->Controls->Add(this->fcgTXMP4RawPath);
            this->fcgtabPageMP4->Controls->Add(this->fcgLBMP4RawPath);
            this->fcgtabPageMP4->Controls->Add(this->fcgBTMP4BoxTempDir);
            this->fcgtabPageMP4->Controls->Add(this->fcgTXMP4BoxTempDir);
            this->fcgtabPageMP4->Controls->Add(this->fcgCXMP4BoxTempDir);
            this->fcgtabPageMP4->Controls->Add(this->fcgLBMP4BoxTempDir);
            this->fcgtabPageMP4->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageMP4->Name = L"fcgtabPageMP4";
            this->fcgtabPageMP4->Padding = System::Windows::Forms::Padding(3);
            this->fcgtabPageMP4->Size = System::Drawing::Size(376, 187);
            this->fcgtabPageMP4->TabIndex = 0;
            this->fcgtabPageMP4->Text = L"mp4";
            this->fcgtabPageMP4->UseVisualStyleBackColor = true;
            // 
            // fcgCBMP4MuxApple
            // 
            this->fcgCBMP4MuxApple->AutoSize = true;
            this->fcgCBMP4MuxApple->Location = System::Drawing::Point(261, 37);
            this->fcgCBMP4MuxApple->Name = L"fcgCBMP4MuxApple";
            this->fcgCBMP4MuxApple->Size = System::Drawing::Size(109, 18);
            this->fcgCBMP4MuxApple->TabIndex = 2;
            this->fcgCBMP4MuxApple->Tag = L"chValue";
            this->fcgCBMP4MuxApple->Text = L"Apple形式に対応";
            this->fcgCBMP4MuxApple->UseVisualStyleBackColor = true;
            // 
            // fcgBTTC2MP4Path
            // 
            this->fcgBTTC2MP4Path->Location = System::Drawing::Point(342, 87);
            this->fcgBTTC2MP4Path->Name = L"fcgBTTC2MP4Path";
            this->fcgBTTC2MP4Path->Size = System::Drawing::Size(30, 23);
            this->fcgBTTC2MP4Path->TabIndex = 6;
            this->fcgBTTC2MP4Path->Text = L"...";
            this->fcgBTTC2MP4Path->UseVisualStyleBackColor = true;
            this->fcgBTTC2MP4Path->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTTC2MP4Path_Click);
            // 
            // fcgTXTC2MP4Path
            // 
            this->fcgTXTC2MP4Path->AllowDrop = true;
            this->fcgTXTC2MP4Path->Location = System::Drawing::Point(150, 88);
            this->fcgTXTC2MP4Path->Name = L"fcgTXTC2MP4Path";
            this->fcgTXTC2MP4Path->Size = System::Drawing::Size(190, 21);
            this->fcgTXTC2MP4Path->TabIndex = 5;
            this->fcgTXTC2MP4Path->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXTC2MP4Path_TextChanged);
            this->fcgTXTC2MP4Path->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXTC2MP4Path->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            this->fcgTXTC2MP4Path->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXTC2MP4Path_Enter);
            this->fcgTXTC2MP4Path->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXTC2MP4Path_Leave);
            // 
            // fcgBTMP4MuxerPath
            // 
            this->fcgBTMP4MuxerPath->Location = System::Drawing::Point(342, 65);
            this->fcgBTMP4MuxerPath->Name = L"fcgBTMP4MuxerPath";
            this->fcgBTMP4MuxerPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTMP4MuxerPath->TabIndex = 4;
            this->fcgBTMP4MuxerPath->Text = L"...";
            this->fcgBTMP4MuxerPath->UseVisualStyleBackColor = true;
            this->fcgBTMP4MuxerPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMP4MuxerPath_Click);
            // 
            // fcgTXMP4MuxerPath
            // 
            this->fcgTXMP4MuxerPath->AllowDrop = true;
            this->fcgTXMP4MuxerPath->Location = System::Drawing::Point(150, 66);
            this->fcgTXMP4MuxerPath->Name = L"fcgTXMP4MuxerPath";
            this->fcgTXMP4MuxerPath->Size = System::Drawing::Size(190, 21);
            this->fcgTXMP4MuxerPath->TabIndex = 3;
            this->fcgTXMP4MuxerPath->Tag = L"";
            this->fcgTXMP4MuxerPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4MuxerPath_TextChanged);
            this->fcgTXMP4MuxerPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXMP4MuxerPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            this->fcgTXMP4MuxerPath->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4MuxerPath_Enter);
            this->fcgTXMP4MuxerPath->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4MuxerPath_Leave);
            // 
            // fcgLBTC2MP4Path
            // 
            this->fcgLBTC2MP4Path->AutoSize = true;
            this->fcgLBTC2MP4Path->Location = System::Drawing::Point(4, 91);
            this->fcgLBTC2MP4Path->Name = L"fcgLBTC2MP4Path";
            this->fcgLBTC2MP4Path->Size = System::Drawing::Size(49, 14);
            this->fcgLBTC2MP4Path->TabIndex = 4;
            this->fcgLBTC2MP4Path->Text = L"～の指定";
            // 
            // fcgLBMP4MuxerPath
            // 
            this->fcgLBMP4MuxerPath->AutoSize = true;
            this->fcgLBMP4MuxerPath->Location = System::Drawing::Point(4, 69);
            this->fcgLBMP4MuxerPath->Name = L"fcgLBMP4MuxerPath";
            this->fcgLBMP4MuxerPath->Size = System::Drawing::Size(49, 14);
            this->fcgLBMP4MuxerPath->TabIndex = 3;
            this->fcgLBMP4MuxerPath->Text = L"～の指定";
            // 
            // fcgCXMP4CmdEx
            // 
            this->fcgCXMP4CmdEx->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMP4CmdEx->FormattingEnabled = true;
            this->fcgCXMP4CmdEx->Location = System::Drawing::Point(213, 9);
            this->fcgCXMP4CmdEx->Name = L"fcgCXMP4CmdEx";
            this->fcgCXMP4CmdEx->Size = System::Drawing::Size(157, 22);
            this->fcgCXMP4CmdEx->TabIndex = 1;
            this->fcgCXMP4CmdEx->Tag = L"chValue";
            // 
            // fcgLBMP4CmdEx
            // 
            this->fcgLBMP4CmdEx->AutoSize = true;
            this->fcgLBMP4CmdEx->Location = System::Drawing::Point(139, 12);
            this->fcgLBMP4CmdEx->Name = L"fcgLBMP4CmdEx";
            this->fcgLBMP4CmdEx->Size = System::Drawing::Size(68, 14);
            this->fcgLBMP4CmdEx->TabIndex = 1;
            this->fcgLBMP4CmdEx->Text = L"拡張オプション";
            // 
            // fcgCBMP4MuxerExt
            // 
            this->fcgCBMP4MuxerExt->AutoSize = true;
            this->fcgCBMP4MuxerExt->Location = System::Drawing::Point(10, 11);
            this->fcgCBMP4MuxerExt->Name = L"fcgCBMP4MuxerExt";
            this->fcgCBMP4MuxerExt->Size = System::Drawing::Size(113, 18);
            this->fcgCBMP4MuxerExt->TabIndex = 0;
            this->fcgCBMP4MuxerExt->Tag = L"chValue";
            this->fcgCBMP4MuxerExt->Text = L"外部muxerを使用";
            this->fcgCBMP4MuxerExt->UseVisualStyleBackColor = true;
            // 
            // fcgBTMP4RawPath
            // 
            this->fcgBTMP4RawPath->Location = System::Drawing::Point(342, 109);
            this->fcgBTMP4RawPath->Name = L"fcgBTMP4RawPath";
            this->fcgBTMP4RawPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTMP4RawPath->TabIndex = 8;
            this->fcgBTMP4RawPath->Text = L"...";
            this->fcgBTMP4RawPath->UseVisualStyleBackColor = true;
            this->fcgBTMP4RawPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMP4RawMuxerPath_Click);
            // 
            // fcgTXMP4RawPath
            // 
            this->fcgTXMP4RawPath->AllowDrop = true;
            this->fcgTXMP4RawPath->Location = System::Drawing::Point(150, 110);
            this->fcgTXMP4RawPath->Name = L"fcgTXMP4RawPath";
            this->fcgTXMP4RawPath->Size = System::Drawing::Size(190, 21);
            this->fcgTXMP4RawPath->TabIndex = 7;
            this->fcgTXMP4RawPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4RawMuxerPath_TextChanged);
            this->fcgTXMP4RawPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXMP4RawPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            this->fcgTXMP4RawPath->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4RawPath_Enter);
            this->fcgTXMP4RawPath->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4RawPath_Leave);
            // 
            // fcgLBMP4RawPath
            // 
            this->fcgLBMP4RawPath->AutoSize = true;
            this->fcgLBMP4RawPath->Location = System::Drawing::Point(4, 113);
            this->fcgLBMP4RawPath->Name = L"fcgLBMP4RawPath";
            this->fcgLBMP4RawPath->Size = System::Drawing::Size(49, 14);
            this->fcgLBMP4RawPath->TabIndex = 20;
            this->fcgLBMP4RawPath->Text = L"～の指定";
            // 
            // fcgBTMP4BoxTempDir
            // 
            this->fcgBTMP4BoxTempDir->Location = System::Drawing::Point(340, 159);
            this->fcgBTMP4BoxTempDir->Name = L"fcgBTMP4BoxTempDir";
            this->fcgBTMP4BoxTempDir->Size = System::Drawing::Size(30, 23);
            this->fcgBTMP4BoxTempDir->TabIndex = 11;
            this->fcgBTMP4BoxTempDir->Text = L"...";
            this->fcgBTMP4BoxTempDir->UseVisualStyleBackColor = true;
            this->fcgBTMP4BoxTempDir->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMP4BoxTempDir_Click);
            // 
            // fcgTXMP4BoxTempDir
            // 
            this->fcgTXMP4BoxTempDir->Location = System::Drawing::Point(107, 160);
            this->fcgTXMP4BoxTempDir->Name = L"fcgTXMP4BoxTempDir";
            this->fcgTXMP4BoxTempDir->Size = System::Drawing::Size(227, 21);
            this->fcgTXMP4BoxTempDir->TabIndex = 10;
            this->fcgTXMP4BoxTempDir->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4BoxTempDir_TextChanged);
            // 
            // fcgCXMP4BoxTempDir
            // 
            this->fcgCXMP4BoxTempDir->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMP4BoxTempDir->FormattingEnabled = true;
            this->fcgCXMP4BoxTempDir->Location = System::Drawing::Point(156, 132);
            this->fcgCXMP4BoxTempDir->Name = L"fcgCXMP4BoxTempDir";
            this->fcgCXMP4BoxTempDir->Size = System::Drawing::Size(202, 22);
            this->fcgCXMP4BoxTempDir->TabIndex = 9;
            this->fcgCXMP4BoxTempDir->Tag = L"chValue";
            // 
            // fcgLBMP4BoxTempDir
            // 
            this->fcgLBMP4BoxTempDir->AutoSize = true;
            this->fcgLBMP4BoxTempDir->Location = System::Drawing::Point(25, 135);
            this->fcgLBMP4BoxTempDir->Name = L"fcgLBMP4BoxTempDir";
            this->fcgLBMP4BoxTempDir->Size = System::Drawing::Size(105, 14);
            this->fcgLBMP4BoxTempDir->TabIndex = 18;
            this->fcgLBMP4BoxTempDir->Text = L"mp4box一時フォルダ";
            // 
            // fcgtabPageMKV
            // 
            this->fcgtabPageMKV->Controls->Add(this->fcgBTMKVMuxerPath);
            this->fcgtabPageMKV->Controls->Add(this->fcgTXMKVMuxerPath);
            this->fcgtabPageMKV->Controls->Add(this->fcgLBMKVMuxerPath);
            this->fcgtabPageMKV->Controls->Add(this->fcgCXMKVCmdEx);
            this->fcgtabPageMKV->Controls->Add(this->fcgLBMKVMuxerCmdEx);
            this->fcgtabPageMKV->Controls->Add(this->fcgCBMKVMuxerExt);
            this->fcgtabPageMKV->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageMKV->Name = L"fcgtabPageMKV";
            this->fcgtabPageMKV->Padding = System::Windows::Forms::Padding(3);
            this->fcgtabPageMKV->Size = System::Drawing::Size(376, 187);
            this->fcgtabPageMKV->TabIndex = 1;
            this->fcgtabPageMKV->Text = L"mkv";
            this->fcgtabPageMKV->UseVisualStyleBackColor = true;
            // 
            // fcgBTMKVMuxerPath
            // 
            this->fcgBTMKVMuxerPath->Location = System::Drawing::Point(340, 76);
            this->fcgBTMKVMuxerPath->Name = L"fcgBTMKVMuxerPath";
            this->fcgBTMKVMuxerPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTMKVMuxerPath->TabIndex = 3;
            this->fcgBTMKVMuxerPath->Text = L"...";
            this->fcgBTMKVMuxerPath->UseVisualStyleBackColor = true;
            this->fcgBTMKVMuxerPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMKVMuxerPath_Click);
            // 
            // fcgTXMKVMuxerPath
            // 
            this->fcgTXMKVMuxerPath->Location = System::Drawing::Point(131, 77);
            this->fcgTXMKVMuxerPath->Name = L"fcgTXMKVMuxerPath";
            this->fcgTXMKVMuxerPath->Size = System::Drawing::Size(207, 21);
            this->fcgTXMKVMuxerPath->TabIndex = 2;
            this->fcgTXMKVMuxerPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMKVMuxerPath_TextChanged);
            this->fcgTXMKVMuxerPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXMKVMuxerPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            this->fcgTXMKVMuxerPath->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXMKVMuxerPath_Enter);
            this->fcgTXMKVMuxerPath->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXMKVMuxerPath_Leave);
            // 
            // fcgLBMKVMuxerPath
            // 
            this->fcgLBMKVMuxerPath->AutoSize = true;
            this->fcgLBMKVMuxerPath->Location = System::Drawing::Point(4, 80);
            this->fcgLBMKVMuxerPath->Name = L"fcgLBMKVMuxerPath";
            this->fcgLBMKVMuxerPath->Size = System::Drawing::Size(49, 14);
            this->fcgLBMKVMuxerPath->TabIndex = 19;
            this->fcgLBMKVMuxerPath->Text = L"～の指定";
            // 
            // fcgCXMKVCmdEx
            // 
            this->fcgCXMKVCmdEx->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMKVCmdEx->FormattingEnabled = true;
            this->fcgCXMKVCmdEx->Location = System::Drawing::Point(213, 43);
            this->fcgCXMKVCmdEx->Name = L"fcgCXMKVCmdEx";
            this->fcgCXMKVCmdEx->Size = System::Drawing::Size(157, 22);
            this->fcgCXMKVCmdEx->TabIndex = 1;
            this->fcgCXMKVCmdEx->Tag = L"chValue";
            // 
            // fcgLBMKVMuxerCmdEx
            // 
            this->fcgLBMKVMuxerCmdEx->AutoSize = true;
            this->fcgLBMKVMuxerCmdEx->Location = System::Drawing::Point(139, 46);
            this->fcgLBMKVMuxerCmdEx->Name = L"fcgLBMKVMuxerCmdEx";
            this->fcgLBMKVMuxerCmdEx->Size = System::Drawing::Size(68, 14);
            this->fcgLBMKVMuxerCmdEx->TabIndex = 17;
            this->fcgLBMKVMuxerCmdEx->Text = L"拡張オプション";
            // 
            // fcgCBMKVMuxerExt
            // 
            this->fcgCBMKVMuxerExt->AutoSize = true;
            this->fcgCBMKVMuxerExt->Location = System::Drawing::Point(10, 45);
            this->fcgCBMKVMuxerExt->Name = L"fcgCBMKVMuxerExt";
            this->fcgCBMKVMuxerExt->Size = System::Drawing::Size(113, 18);
            this->fcgCBMKVMuxerExt->TabIndex = 0;
            this->fcgCBMKVMuxerExt->Tag = L"chValue";
            this->fcgCBMKVMuxerExt->Text = L"外部muxerを使用";
            this->fcgCBMKVMuxerExt->UseVisualStyleBackColor = true;
            // 
            // fcgtabPageMux
            // 
            this->fcgtabPageMux->Controls->Add(this->fcgCXMuxPriority);
            this->fcgtabPageMux->Controls->Add(this->fcgLBMuxPriority);
            this->fcgtabPageMux->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageMux->Name = L"fcgtabPageMux";
            this->fcgtabPageMux->Size = System::Drawing::Size(376, 187);
            this->fcgtabPageMux->TabIndex = 2;
            this->fcgtabPageMux->Text = L"Mux共通設定";
            this->fcgtabPageMux->UseVisualStyleBackColor = true;
            // 
            // fcgCXMuxPriority
            // 
            this->fcgCXMuxPriority->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMuxPriority->FormattingEnabled = true;
            this->fcgCXMuxPriority->Location = System::Drawing::Point(102, 64);
            this->fcgCXMuxPriority->Name = L"fcgCXMuxPriority";
            this->fcgCXMuxPriority->Size = System::Drawing::Size(198, 22);
            this->fcgCXMuxPriority->TabIndex = 1;
            this->fcgCXMuxPriority->Tag = L"chValue";
            // 
            // fcgLBMuxPriority
            // 
            this->fcgLBMuxPriority->AutoSize = true;
            this->fcgLBMuxPriority->Location = System::Drawing::Point(15, 67);
            this->fcgLBMuxPriority->Name = L"fcgLBMuxPriority";
            this->fcgLBMuxPriority->Size = System::Drawing::Size(62, 14);
            this->fcgLBMuxPriority->TabIndex = 1;
            this->fcgLBMuxPriority->Text = L"Mux優先度";
            // 
            // fcgtabPageBat
            // 
            this->fcgtabPageBat->Controls->Add(this->fcgLBBatAfterString);
            this->fcgtabPageBat->Controls->Add(this->fcgLBBatBeforeString);
            this->fcgtabPageBat->Controls->Add(this->fcgBTBatAfterPath);
            this->fcgtabPageBat->Controls->Add(this->fcgTXBatAfterPath);
            this->fcgtabPageBat->Controls->Add(this->fcgLBBatAfterPath);
            this->fcgtabPageBat->Controls->Add(this->fcgCBWaitForBatAfter);
            this->fcgtabPageBat->Controls->Add(this->fcgCBRunBatAfter);
            this->fcgtabPageBat->Controls->Add(this->fcgPNSeparator);
            this->fcgtabPageBat->Controls->Add(this->fcgBTBatBeforePath);
            this->fcgtabPageBat->Controls->Add(this->fcgTXBatBeforePath);
            this->fcgtabPageBat->Controls->Add(this->fcgLBBatBeforePath);
            this->fcgtabPageBat->Controls->Add(this->fcgCBWaitForBatBefore);
            this->fcgtabPageBat->Controls->Add(this->fcgCBRunBatBefore);
            this->fcgtabPageBat->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageBat->Name = L"fcgtabPageBat";
            this->fcgtabPageBat->Size = System::Drawing::Size(376, 187);
            this->fcgtabPageBat->TabIndex = 3;
            this->fcgtabPageBat->Text = L"エンコ前後バッチ処理";
            this->fcgtabPageBat->UseVisualStyleBackColor = true;
            // 
            // fcgLBBatAfterString
            // 
            this->fcgLBBatAfterString->AutoSize = true;
            this->fcgLBBatAfterString->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Italic | System::Drawing::FontStyle::Underline)),
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(128)));
            this->fcgLBBatAfterString->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBBatAfterString->Location = System::Drawing::Point(304, 113);
            this->fcgLBBatAfterString->Name = L"fcgLBBatAfterString";
            this->fcgLBBatAfterString->Size = System::Drawing::Size(27, 15);
            this->fcgLBBatAfterString->TabIndex = 13;
            this->fcgLBBatAfterString->Text = L" 後& ";
            this->fcgLBBatAfterString->TextAlign = System::Drawing::ContentAlignment::TopCenter;
            // 
            // fcgLBBatBeforeString
            // 
            this->fcgLBBatBeforeString->AutoSize = true;
            this->fcgLBBatBeforeString->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Italic | System::Drawing::FontStyle::Underline)),
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(128)));
            this->fcgLBBatBeforeString->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBBatBeforeString->Location = System::Drawing::Point(304, 20);
            this->fcgLBBatBeforeString->Name = L"fcgLBBatBeforeString";
            this->fcgLBBatBeforeString->Size = System::Drawing::Size(27, 15);
            this->fcgLBBatBeforeString->TabIndex = 3;
            this->fcgLBBatBeforeString->Text = L" 前& ";
            this->fcgLBBatBeforeString->TextAlign = System::Drawing::ContentAlignment::TopCenter;
            // 
            // fcgBTBatAfterPath
            // 
            this->fcgBTBatAfterPath->Location = System::Drawing::Point(330, 154);
            this->fcgBTBatAfterPath->Name = L"fcgBTBatAfterPath";
            this->fcgBTBatAfterPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTBatAfterPath->TabIndex = 11;
            this->fcgBTBatAfterPath->Tag = L"chValue";
            this->fcgBTBatAfterPath->Text = L"...";
            this->fcgBTBatAfterPath->UseVisualStyleBackColor = true;
            this->fcgBTBatAfterPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatAfterPath_Click);
            // 
            // fcgTXBatAfterPath
            // 
            this->fcgTXBatAfterPath->AllowDrop = true;
            this->fcgTXBatAfterPath->Location = System::Drawing::Point(126, 155);
            this->fcgTXBatAfterPath->Name = L"fcgTXBatAfterPath";
            this->fcgTXBatAfterPath->Size = System::Drawing::Size(202, 21);
            this->fcgTXBatAfterPath->TabIndex = 10;
            this->fcgTXBatAfterPath->Tag = L"chValue";
            this->fcgTXBatAfterPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXBatAfterPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBBatAfterPath
            // 
            this->fcgLBBatAfterPath->AutoSize = true;
            this->fcgLBBatAfterPath->Location = System::Drawing::Point(40, 158);
            this->fcgLBBatAfterPath->Name = L"fcgLBBatAfterPath";
            this->fcgLBBatAfterPath->Size = System::Drawing::Size(61, 14);
            this->fcgLBBatAfterPath->TabIndex = 9;
            this->fcgLBBatAfterPath->Text = L"バッチファイル";
            // 
            // fcgCBWaitForBatAfter
            // 
            this->fcgCBWaitForBatAfter->AutoSize = true;
            this->fcgCBWaitForBatAfter->Location = System::Drawing::Point(40, 129);
            this->fcgCBWaitForBatAfter->Name = L"fcgCBWaitForBatAfter";
            this->fcgCBWaitForBatAfter->Size = System::Drawing::Size(150, 18);
            this->fcgCBWaitForBatAfter->TabIndex = 8;
            this->fcgCBWaitForBatAfter->Tag = L"chValue";
            this->fcgCBWaitForBatAfter->Text = L"バッチ処理の終了を待機する";
            this->fcgCBWaitForBatAfter->UseVisualStyleBackColor = true;
            // 
            // fcgCBRunBatAfter
            // 
            this->fcgCBRunBatAfter->AutoSize = true;
            this->fcgCBRunBatAfter->Location = System::Drawing::Point(18, 105);
            this->fcgCBRunBatAfter->Name = L"fcgCBRunBatAfter";
            this->fcgCBRunBatAfter->Size = System::Drawing::Size(179, 18);
            this->fcgCBRunBatAfter->TabIndex = 7;
            this->fcgCBRunBatAfter->Tag = L"chValue";
            this->fcgCBRunBatAfter->Text = L"エンコード終了後、バッチ処理を行う";
            this->fcgCBRunBatAfter->UseVisualStyleBackColor = true;
            // 
            // fcgPNSeparator
            // 
            this->fcgPNSeparator->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->fcgPNSeparator->Location = System::Drawing::Point(18, 94);
            this->fcgPNSeparator->Name = L"fcgPNSeparator";
            this->fcgPNSeparator->Size = System::Drawing::Size(342, 1);
            this->fcgPNSeparator->TabIndex = 6;
            // 
            // fcgBTBatBeforePath
            // 
            this->fcgBTBatBeforePath->Location = System::Drawing::Point(330, 61);
            this->fcgBTBatBeforePath->Name = L"fcgBTBatBeforePath";
            this->fcgBTBatBeforePath->Size = System::Drawing::Size(30, 23);
            this->fcgBTBatBeforePath->TabIndex = 5;
            this->fcgBTBatBeforePath->Tag = L"chValue";
            this->fcgBTBatBeforePath->Text = L"...";
            this->fcgBTBatBeforePath->UseVisualStyleBackColor = true;
            this->fcgBTBatBeforePath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatBeforePath_Click);
            // 
            // fcgTXBatBeforePath
            // 
            this->fcgTXBatBeforePath->AllowDrop = true;
            this->fcgTXBatBeforePath->Location = System::Drawing::Point(126, 62);
            this->fcgTXBatBeforePath->Name = L"fcgTXBatBeforePath";
            this->fcgTXBatBeforePath->Size = System::Drawing::Size(202, 21);
            this->fcgTXBatBeforePath->TabIndex = 4;
            this->fcgTXBatBeforePath->Tag = L"chValue";
            this->fcgTXBatBeforePath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXBatBeforePath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBBatBeforePath
            // 
            this->fcgLBBatBeforePath->AutoSize = true;
            this->fcgLBBatBeforePath->Location = System::Drawing::Point(40, 65);
            this->fcgLBBatBeforePath->Name = L"fcgLBBatBeforePath";
            this->fcgLBBatBeforePath->Size = System::Drawing::Size(61, 14);
            this->fcgLBBatBeforePath->TabIndex = 2;
            this->fcgLBBatBeforePath->Text = L"バッチファイル";
            // 
            // fcgCBWaitForBatBefore
            // 
            this->fcgCBWaitForBatBefore->AutoSize = true;
            this->fcgCBWaitForBatBefore->Location = System::Drawing::Point(40, 36);
            this->fcgCBWaitForBatBefore->Name = L"fcgCBWaitForBatBefore";
            this->fcgCBWaitForBatBefore->Size = System::Drawing::Size(150, 18);
            this->fcgCBWaitForBatBefore->TabIndex = 1;
            this->fcgCBWaitForBatBefore->Tag = L"chValue";
            this->fcgCBWaitForBatBefore->Text = L"バッチ処理の終了を待機する";
            this->fcgCBWaitForBatBefore->UseVisualStyleBackColor = true;
            // 
            // fcgCBRunBatBefore
            // 
            this->fcgCBRunBatBefore->AutoSize = true;
            this->fcgCBRunBatBefore->Location = System::Drawing::Point(18, 12);
            this->fcgCBRunBatBefore->Name = L"fcgCBRunBatBefore";
            this->fcgCBRunBatBefore->Size = System::Drawing::Size(179, 18);
            this->fcgCBRunBatBefore->TabIndex = 0;
            this->fcgCBRunBatBefore->Tag = L"chValue";
            this->fcgCBRunBatBefore->Text = L"エンコード開始前、バッチ処理を行う";
            this->fcgCBRunBatBefore->UseVisualStyleBackColor = true;
            // 
            // fcgTXCmd
            // 
            this->fcgTXCmd->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
                | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->fcgTXCmd->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgTXCmd->Location = System::Drawing::Point(9, 547);
            this->fcgTXCmd->Name = L"fcgTXCmd";
            this->fcgTXCmd->ReadOnly = true;
            this->fcgTXCmd->Size = System::Drawing::Size(992, 21);
            this->fcgTXCmd->TabIndex = 4;
            this->fcgTXCmd->DoubleClick += gcnew System::EventHandler(this, &frmConfig::fcgTXCmd_DoubleClick);
            // 
            // fcgBTCancel
            // 
            this->fcgBTCancel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
            this->fcgBTCancel->Location = System::Drawing::Point(771, 570);
            this->fcgBTCancel->Name = L"fcgBTCancel";
            this->fcgBTCancel->Size = System::Drawing::Size(84, 28);
            this->fcgBTCancel->TabIndex = 5;
            this->fcgBTCancel->Text = L"キャンセル";
            this->fcgBTCancel->UseVisualStyleBackColor = true;
            this->fcgBTCancel->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTCancel_Click);
            // 
            // fcgBTOK
            // 
            this->fcgBTOK->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
            this->fcgBTOK->Location = System::Drawing::Point(893, 570);
            this->fcgBTOK->Name = L"fcgBTOK";
            this->fcgBTOK->Size = System::Drawing::Size(84, 28);
            this->fcgBTOK->TabIndex = 6;
            this->fcgBTOK->Text = L"OK";
            this->fcgBTOK->UseVisualStyleBackColor = true;
            this->fcgBTOK->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTOK_Click);
            // 
            // fcgBTDefault
            // 
            this->fcgBTDefault->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
            this->fcgBTDefault->Location = System::Drawing::Point(9, 572);
            this->fcgBTDefault->Name = L"fcgBTDefault";
            this->fcgBTDefault->Size = System::Drawing::Size(112, 28);
            this->fcgBTDefault->TabIndex = 7;
            this->fcgBTDefault->Text = L"デフォルト";
            this->fcgBTDefault->UseVisualStyleBackColor = true;
            this->fcgBTDefault->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTDefault_Click);
            // 
            // fcgLBVersionDate
            // 
            this->fcgLBVersionDate->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
            this->fcgLBVersionDate->AutoSize = true;
            this->fcgLBVersionDate->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, System::Drawing::FontStyle::Italic, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgLBVersionDate->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBVersionDate->Location = System::Drawing::Point(416, 579);
            this->fcgLBVersionDate->Name = L"fcgLBVersionDate";
            this->fcgLBVersionDate->Size = System::Drawing::Size(49, 15);
            this->fcgLBVersionDate->TabIndex = 8;
            this->fcgLBVersionDate->Text = L"Version";
            // 
            // fcgLBVersion
            // 
            this->fcgLBVersion->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
            this->fcgLBVersion->AutoSize = true;
            this->fcgLBVersion->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, System::Drawing::FontStyle::Italic, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgLBVersion->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBVersion->Location = System::Drawing::Point(180, 579);
            this->fcgLBVersion->Name = L"fcgLBVersion";
            this->fcgLBVersion->Size = System::Drawing::Size(49, 15);
            this->fcgLBVersion->TabIndex = 9;
            this->fcgLBVersion->Text = L"Version";
            // 
            // fcgOpenFileDialog
            // 
            this->fcgOpenFileDialog->FileName = L"openFileDialog1";
            // 
            // fcgCSCQM
            // 
            this->fcgCSCQM->ImageScalingSize = System::Drawing::Size(18, 18);
            this->fcgCSCQM->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
                this->fcgCSFlat, this->fcgCSJvt,
                    this->toolStripSeparator1, this->fcgCSCqmFile
            });
            this->fcgCSCQM->Name = L"fcgCSCQM";
            this->fcgCSCQM->Size = System::Drawing::Size(113, 76);
            // 
            // fcgCSFlat
            // 
            this->fcgCSFlat->Name = L"fcgCSFlat";
            this->fcgCSFlat->Size = System::Drawing::Size(112, 22);
            this->fcgCSFlat->Tag = L"0";
            this->fcgCSFlat->Text = L"flat";
            this->fcgCSFlat->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::CSCqmCheckedChanged);
            this->fcgCSFlat->Click += gcnew System::EventHandler(this, &frmConfig::CSCqmClick);
            // 
            // fcgCSJvt
            // 
            this->fcgCSJvt->Name = L"fcgCSJvt";
            this->fcgCSJvt->Size = System::Drawing::Size(112, 22);
            this->fcgCSJvt->Tag = L"1";
            this->fcgCSJvt->Text = L"jvt";
            this->fcgCSJvt->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::CSCqmCheckedChanged);
            this->fcgCSJvt->Click += gcnew System::EventHandler(this, &frmConfig::CSCqmClick);
            // 
            // toolStripSeparator1
            // 
            this->toolStripSeparator1->Name = L"toolStripSeparator1";
            this->toolStripSeparator1->Size = System::Drawing::Size(109, 6);
            // 
            // fcgCSCqmFile
            // 
            this->fcgCSCqmFile->Name = L"fcgCSCqmFile";
            this->fcgCSCqmFile->Size = System::Drawing::Size(112, 22);
            this->fcgCSCqmFile->Tag = L"2";
            this->fcgCSCqmFile->Text = L"cqmfile";
            this->fcgCSCqmFile->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::CSCqmCheckedChanged);
            this->fcgCSCqmFile->Click += gcnew System::EventHandler(this, &frmConfig::CSCqmClick);
            // 
            // fcgTTX264
            // 
            this->fcgTTX264->AutomaticDelay = 200;
            this->fcgTTX264->AutoPopDelay = 9999;
            this->fcgTTX264->InitialDelay = 200;
            this->fcgTTX264->IsBalloon = true;
            this->fcgTTX264->ReshowDelay = 40;
            this->fcgTTX264->ShowAlways = true;
            this->fcgTTX264->ToolTipTitle = L"x264 options...";
            this->fcgTTX264->UseAnimation = false;
            this->fcgTTX264->UseFading = false;
            // 
            // fcgTTEx
            // 
            this->fcgTTEx->AutomaticDelay = 200;
            this->fcgTTEx->AutoPopDelay = 9999;
            this->fcgTTEx->InitialDelay = 200;
            this->fcgTTEx->IsBalloon = true;
            this->fcgTTEx->ReshowDelay = 50;
            this->fcgTTEx->ShowAlways = true;
            this->fcgTTEx->UseAnimation = false;
            this->fcgTTEx->UseFading = false;
            // 
            // fcgTTX264Version
            // 
            this->fcgTTX264Version->AutomaticDelay = 200;
            this->fcgTTX264Version->AutoPopDelay = 10000;
            this->fcgTTX264Version->InitialDelay = 200;
            this->fcgTTX264Version->IsBalloon = true;
            this->fcgTTX264Version->ReshowDelay = 50;
            this->fcgTTX264Version->ToolTipTitle = L"x264 バージョン情報...";
            this->fcgTTX264Version->UseAnimation = false;
            this->fcgTTX264Version->UseFading = false;
            // 
            // fcgCSReplaceStrings
            // 
            this->fcgCSReplaceStrings->ImageScalingSize = System::Drawing::Size(18, 18);
            this->fcgCSReplaceStrings->Name = L"fcgCSReplaceStrings";
            this->fcgCSReplaceStrings->Size = System::Drawing::Size(61, 4);
            // 
            // fcgLBguiExBlog
            // 
            this->fcgLBguiExBlog->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
            this->fcgLBguiExBlog->AutoSize = true;
            this->fcgLBguiExBlog->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Italic, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgLBguiExBlog->LinkColor = System::Drawing::Color::Gray;
            this->fcgLBguiExBlog->Location = System::Drawing::Point(623, 579);
            this->fcgLBguiExBlog->Name = L"fcgLBguiExBlog";
            this->fcgLBguiExBlog->Size = System::Drawing::Size(100, 14);
            this->fcgLBguiExBlog->TabIndex = 10;
            this->fcgLBguiExBlog->TabStop = true;
            this->fcgLBguiExBlog->Text = L"x264guiExについて";
            this->fcgLBguiExBlog->VisitedLinkColor = System::Drawing::Color::Gray;
            this->fcgLBguiExBlog->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &frmConfig::fcgLBguiExBlog_LinkClicked);
            // 
            // fcgtabControlAudio
            // 
            this->fcgtabControlAudio->Controls->Add(this->fcgtabPageAudioMain);
            this->fcgtabControlAudio->Controls->Add(this->fcgtabPageAudioOther);
            this->fcgtabControlAudio->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F));
            this->fcgtabControlAudio->Location = System::Drawing::Point(2, 2);
            this->fcgtabControlAudio->Name = L"fcgtabControlAudio";
            this->fcgtabControlAudio->SelectedIndex = 0;
            this->fcgtabControlAudio->Size = System::Drawing::Size(384, 296);
            this->fcgtabControlAudio->TabIndex = 11;
            // 
            // fcgtabPageAudioMain
            // 
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCXAudioDelayCut);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgLBAudioDelayCut);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCBAudioEncTiming);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCXAudioEncTiming);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCXAudioTempDir);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgTXCustomAudioTempDir);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgBTCustomAudioTempDir);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCBAudioUsePipe);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgLBAudioBitrate);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgNUAudioBitrate);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCBAudio2pass);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCXAudioEncMode);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgLBAudioEncMode);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgBTAudioEncoderPath);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgTXAudioEncoderPath);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgLBAudioEncoderPath);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCBAudioOnly);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCBFAWCheck);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCXAudioEncoder);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgLBAudioEncoder);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgLBAudioTemp);
            this->fcgtabPageAudioMain->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageAudioMain->Name = L"fcgtabPageAudioMain";
            this->fcgtabPageAudioMain->Padding = System::Windows::Forms::Padding(3);
            this->fcgtabPageAudioMain->Size = System::Drawing::Size(376, 269);
            this->fcgtabPageAudioMain->TabIndex = 0;
            this->fcgtabPageAudioMain->Text = L"音声";
            this->fcgtabPageAudioMain->UseVisualStyleBackColor = true;
            // 
            // fcgCXAudioDelayCut
            // 
            this->fcgCXAudioDelayCut->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioDelayCut->FormattingEnabled = true;
            this->fcgCXAudioDelayCut->Location = System::Drawing::Point(291, 133);
            this->fcgCXAudioDelayCut->Name = L"fcgCXAudioDelayCut";
            this->fcgCXAudioDelayCut->Size = System::Drawing::Size(70, 22);
            this->fcgCXAudioDelayCut->TabIndex = 43;
            this->fcgCXAudioDelayCut->Tag = L"chValue";
            // 
            // fcgLBAudioDelayCut
            // 
            this->fcgLBAudioDelayCut->AutoSize = true;
            this->fcgLBAudioDelayCut->Location = System::Drawing::Point(224, 136);
            this->fcgLBAudioDelayCut->Name = L"fcgLBAudioDelayCut";
            this->fcgLBAudioDelayCut->Size = System::Drawing::Size(60, 14);
            this->fcgLBAudioDelayCut->TabIndex = 54;
            this->fcgLBAudioDelayCut->Text = L"ディレイカット";
            // 
            // fcgCBAudioEncTiming
            // 
            this->fcgCBAudioEncTiming->AutoSize = true;
            this->fcgCBAudioEncTiming->Location = System::Drawing::Point(236, 54);
            this->fcgCBAudioEncTiming->Name = L"fcgCBAudioEncTiming";
            this->fcgCBAudioEncTiming->Size = System::Drawing::Size(40, 14);
            this->fcgCBAudioEncTiming->TabIndex = 53;
            this->fcgCBAudioEncTiming->Text = L"処理順";
            // 
            // fcgCXAudioEncTiming
            // 
            this->fcgCXAudioEncTiming->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioEncTiming->FormattingEnabled = true;
            this->fcgCXAudioEncTiming->Location = System::Drawing::Point(294, 51);
            this->fcgCXAudioEncTiming->Name = L"fcgCXAudioEncTiming";
            this->fcgCXAudioEncTiming->Size = System::Drawing::Size(68, 22);
            this->fcgCXAudioEncTiming->TabIndex = 52;
            this->fcgCXAudioEncTiming->Tag = L"chValue";
            // 
            // fcgCXAudioTempDir
            // 
            this->fcgCXAudioTempDir->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioTempDir->FormattingEnabled = true;
            this->fcgCXAudioTempDir->Location = System::Drawing::Point(135, 208);
            this->fcgCXAudioTempDir->Name = L"fcgCXAudioTempDir";
            this->fcgCXAudioTempDir->Size = System::Drawing::Size(150, 22);
            this->fcgCXAudioTempDir->TabIndex = 46;
            this->fcgCXAudioTempDir->Tag = L"chValue";
            // 
            // fcgTXCustomAudioTempDir
            // 
            this->fcgTXCustomAudioTempDir->Location = System::Drawing::Point(64, 236);
            this->fcgTXCustomAudioTempDir->Name = L"fcgTXCustomAudioTempDir";
            this->fcgTXCustomAudioTempDir->Size = System::Drawing::Size(245, 21);
            this->fcgTXCustomAudioTempDir->TabIndex = 47;
            this->fcgTXCustomAudioTempDir->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXCustomAudioTempDir_TextChanged);
            // 
            // fcgBTCustomAudioTempDir
            // 
            this->fcgBTCustomAudioTempDir->Location = System::Drawing::Point(316, 234);
            this->fcgBTCustomAudioTempDir->Name = L"fcgBTCustomAudioTempDir";
            this->fcgBTCustomAudioTempDir->Size = System::Drawing::Size(29, 23);
            this->fcgBTCustomAudioTempDir->TabIndex = 49;
            this->fcgBTCustomAudioTempDir->Text = L"...";
            this->fcgBTCustomAudioTempDir->UseVisualStyleBackColor = true;
            this->fcgBTCustomAudioTempDir->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTCustomAudioTempDir_Click);
            // 
            // fcgCBAudioUsePipe
            // 
            this->fcgCBAudioUsePipe->AutoSize = true;
            this->fcgCBAudioUsePipe->Location = System::Drawing::Point(130, 134);
            this->fcgCBAudioUsePipe->Name = L"fcgCBAudioUsePipe";
            this->fcgCBAudioUsePipe->Size = System::Drawing::Size(73, 18);
            this->fcgCBAudioUsePipe->TabIndex = 42;
            this->fcgCBAudioUsePipe->Tag = L"chValue";
            this->fcgCBAudioUsePipe->Text = L"パイプ処理";
            this->fcgCBAudioUsePipe->UseVisualStyleBackColor = true;
            // 
            // fcgLBAudioBitrate
            // 
            this->fcgLBAudioBitrate->AutoSize = true;
            this->fcgLBAudioBitrate->Location = System::Drawing::Point(284, 161);
            this->fcgLBAudioBitrate->Name = L"fcgLBAudioBitrate";
            this->fcgLBAudioBitrate->Size = System::Drawing::Size(32, 14);
            this->fcgLBAudioBitrate->TabIndex = 50;
            this->fcgLBAudioBitrate->Text = L"kbps";
            // 
            // fcgNUAudioBitrate
            // 
            this->fcgNUAudioBitrate->Location = System::Drawing::Point(212, 157);
            this->fcgNUAudioBitrate->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1536, 0, 0, 0 });
            this->fcgNUAudioBitrate->Name = L"fcgNUAudioBitrate";
            this->fcgNUAudioBitrate->Size = System::Drawing::Size(65, 21);
            this->fcgNUAudioBitrate->TabIndex = 40;
            this->fcgNUAudioBitrate->Tag = L"chValue";
            this->fcgNUAudioBitrate->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCBAudio2pass
            // 
            this->fcgCBAudio2pass->AutoSize = true;
            this->fcgCBAudio2pass->Location = System::Drawing::Point(59, 134);
            this->fcgCBAudio2pass->Name = L"fcgCBAudio2pass";
            this->fcgCBAudio2pass->Size = System::Drawing::Size(56, 18);
            this->fcgCBAudio2pass->TabIndex = 41;
            this->fcgCBAudio2pass->Tag = L"chValue";
            this->fcgCBAudio2pass->Text = L"2pass";
            this->fcgCBAudio2pass->UseVisualStyleBackColor = true;
            this->fcgCBAudio2pass->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCBAudio2pass_CheckedChanged);
            // 
            // fcgCXAudioEncMode
            // 
            this->fcgCXAudioEncMode->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioEncMode->FormattingEnabled = true;
            this->fcgCXAudioEncMode->Location = System::Drawing::Point(16, 156);
            this->fcgCXAudioEncMode->Name = L"fcgCXAudioEncMode";
            this->fcgCXAudioEncMode->Size = System::Drawing::Size(189, 22);
            this->fcgCXAudioEncMode->TabIndex = 39;
            this->fcgCXAudioEncMode->Tag = L"chValue";
            this->fcgCXAudioEncMode->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXAudioEncMode_SelectedIndexChanged);
            // 
            // fcgLBAudioEncMode
            // 
            this->fcgLBAudioEncMode->AutoSize = true;
            this->fcgLBAudioEncMode->Location = System::Drawing::Point(4, 136);
            this->fcgLBAudioEncMode->Name = L"fcgLBAudioEncMode";
            this->fcgLBAudioEncMode->Size = System::Drawing::Size(32, 14);
            this->fcgLBAudioEncMode->TabIndex = 48;
            this->fcgLBAudioEncMode->Text = L"モード";
            // 
            // fcgBTAudioEncoderPath
            // 
            this->fcgBTAudioEncoderPath->Location = System::Drawing::Point(324, 90);
            this->fcgBTAudioEncoderPath->Name = L"fcgBTAudioEncoderPath";
            this->fcgBTAudioEncoderPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTAudioEncoderPath->TabIndex = 38;
            this->fcgBTAudioEncoderPath->Text = L"...";
            this->fcgBTAudioEncoderPath->UseVisualStyleBackColor = true;
            this->fcgBTAudioEncoderPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTAudioEncoderPath_Click);
            // 
            // fcgTXAudioEncoderPath
            // 
            this->fcgTXAudioEncoderPath->AllowDrop = true;
            this->fcgTXAudioEncoderPath->Location = System::Drawing::Point(16, 92);
            this->fcgTXAudioEncoderPath->Name = L"fcgTXAudioEncoderPath";
            this->fcgTXAudioEncoderPath->Size = System::Drawing::Size(303, 21);
            this->fcgTXAudioEncoderPath->TabIndex = 37;
            this->fcgTXAudioEncoderPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXAudioEncoderPath_TextChanged);
            this->fcgTXAudioEncoderPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXAudioEncoderPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            this->fcgTXAudioEncoderPath->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXAudioEncoderPath_Enter);
            this->fcgTXAudioEncoderPath->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXAudioEncoderPath_Leave);
            // 
            // fcgLBAudioEncoderPath
            // 
            this->fcgLBAudioEncoderPath->AutoSize = true;
            this->fcgLBAudioEncoderPath->Location = System::Drawing::Point(12, 75);
            this->fcgLBAudioEncoderPath->Name = L"fcgLBAudioEncoderPath";
            this->fcgLBAudioEncoderPath->Size = System::Drawing::Size(49, 14);
            this->fcgLBAudioEncoderPath->TabIndex = 44;
            this->fcgLBAudioEncoderPath->Text = L"～の指定";
            // 
            // fcgCBAudioOnly
            // 
            this->fcgCBAudioOnly->AutoSize = true;
            this->fcgCBAudioOnly->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgCBAudioOnly->Location = System::Drawing::Point(252, 5);
            this->fcgCBAudioOnly->Name = L"fcgCBAudioOnly";
            this->fcgCBAudioOnly->Size = System::Drawing::Size(89, 18);
            this->fcgCBAudioOnly->TabIndex = 34;
            this->fcgCBAudioOnly->Tag = L"chValue";
            this->fcgCBAudioOnly->Text = L"音声のみ出力";
            this->fcgCBAudioOnly->UseVisualStyleBackColor = true;
            // 
            // fcgCBFAWCheck
            // 
            this->fcgCBFAWCheck->AutoSize = true;
            this->fcgCBFAWCheck->Location = System::Drawing::Point(252, 28);
            this->fcgCBFAWCheck->Name = L"fcgCBFAWCheck";
            this->fcgCBFAWCheck->Size = System::Drawing::Size(81, 18);
            this->fcgCBFAWCheck->TabIndex = 36;
            this->fcgCBFAWCheck->Tag = L"chValue";
            this->fcgCBFAWCheck->Text = L"FAWCheck";
            this->fcgCBFAWCheck->UseVisualStyleBackColor = true;
            // 
            // fcgCXAudioEncoder
            // 
            this->fcgCXAudioEncoder->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioEncoder->FormattingEnabled = true;
            this->fcgCXAudioEncoder->Location = System::Drawing::Point(17, 34);
            this->fcgCXAudioEncoder->Name = L"fcgCXAudioEncoder";
            this->fcgCXAudioEncoder->Size = System::Drawing::Size(172, 22);
            this->fcgCXAudioEncoder->TabIndex = 32;
            this->fcgCXAudioEncoder->Tag = L"chValue";
            this->fcgCXAudioEncoder->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXAudioEncoder_SelectedIndexChanged);
            // 
            // fcgLBAudioEncoder
            // 
            this->fcgLBAudioEncoder->AutoSize = true;
            this->fcgLBAudioEncoder->Location = System::Drawing::Point(5, 14);
            this->fcgLBAudioEncoder->Name = L"fcgLBAudioEncoder";
            this->fcgLBAudioEncoder->Size = System::Drawing::Size(48, 14);
            this->fcgLBAudioEncoder->TabIndex = 33;
            this->fcgLBAudioEncoder->Text = L"エンコーダ";
            // 
            // fcgLBAudioTemp
            // 
            this->fcgLBAudioTemp->AutoSize = true;
            this->fcgLBAudioTemp->Location = System::Drawing::Point(7, 211);
            this->fcgLBAudioTemp->Name = L"fcgLBAudioTemp";
            this->fcgLBAudioTemp->Size = System::Drawing::Size(114, 14);
            this->fcgLBAudioTemp->TabIndex = 51;
            this->fcgLBAudioTemp->Text = L"音声一時ファイル出力先";
            // 
            // fcgtabPageAudioOther
            // 
            this->fcgtabPageAudioOther->Controls->Add(this->panel2);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgLBBatAfterAudioString);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgLBBatBeforeAudioString);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgBTBatAfterAudioPath);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgTXBatAfterAudioPath);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgLBBatAfterAudioPath);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgCBRunBatAfterAudio);
            this->fcgtabPageAudioOther->Controls->Add(this->panel1);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgBTBatBeforeAudioPath);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgTXBatBeforeAudioPath);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgLBBatBeforeAudioPath);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgCBRunBatBeforeAudio);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgCXAudioPriority);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgLBAudioPriority);
            this->fcgtabPageAudioOther->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageAudioOther->Name = L"fcgtabPageAudioOther";
            this->fcgtabPageAudioOther->Padding = System::Windows::Forms::Padding(3);
            this->fcgtabPageAudioOther->Size = System::Drawing::Size(376, 269);
            this->fcgtabPageAudioOther->TabIndex = 1;
            this->fcgtabPageAudioOther->Text = L"その他";
            this->fcgtabPageAudioOther->UseVisualStyleBackColor = true;
            // 
            // panel2
            // 
            this->panel2->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->panel2->Location = System::Drawing::Point(18, 126);
            this->panel2->Name = L"panel2";
            this->panel2->Size = System::Drawing::Size(342, 1);
            this->panel2->TabIndex = 61;
            // 
            // fcgLBBatAfterAudioString
            // 
            this->fcgLBBatAfterAudioString->AutoSize = true;
            this->fcgLBBatAfterAudioString->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Italic | System::Drawing::FontStyle::Underline)),
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(128)));
            this->fcgLBBatAfterAudioString->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBBatAfterAudioString->Location = System::Drawing::Point(304, 208);
            this->fcgLBBatAfterAudioString->Name = L"fcgLBBatAfterAudioString";
            this->fcgLBBatAfterAudioString->Size = System::Drawing::Size(27, 15);
            this->fcgLBBatAfterAudioString->TabIndex = 60;
            this->fcgLBBatAfterAudioString->Text = L" 後& ";
            this->fcgLBBatAfterAudioString->TextAlign = System::Drawing::ContentAlignment::TopCenter;
            // 
            // fcgLBBatBeforeAudioString
            // 
            this->fcgLBBatBeforeAudioString->AutoSize = true;
            this->fcgLBBatBeforeAudioString->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Italic | System::Drawing::FontStyle::Underline)),
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(128)));
            this->fcgLBBatBeforeAudioString->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBBatBeforeAudioString->Location = System::Drawing::Point(304, 139);
            this->fcgLBBatBeforeAudioString->Name = L"fcgLBBatBeforeAudioString";
            this->fcgLBBatBeforeAudioString->Size = System::Drawing::Size(27, 15);
            this->fcgLBBatBeforeAudioString->TabIndex = 51;
            this->fcgLBBatBeforeAudioString->Text = L" 前& ";
            this->fcgLBBatBeforeAudioString->TextAlign = System::Drawing::ContentAlignment::TopCenter;
            // 
            // fcgBTBatAfterAudioPath
            // 
            this->fcgBTBatAfterAudioPath->Location = System::Drawing::Point(330, 231);
            this->fcgBTBatAfterAudioPath->Name = L"fcgBTBatAfterAudioPath";
            this->fcgBTBatAfterAudioPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTBatAfterAudioPath->TabIndex = 59;
            this->fcgBTBatAfterAudioPath->Tag = L"chValue";
            this->fcgBTBatAfterAudioPath->Text = L"...";
            this->fcgBTBatAfterAudioPath->UseVisualStyleBackColor = true;
            this->fcgBTBatAfterAudioPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatAfterAudioPath_Click);
            // 
            // fcgTXBatAfterAudioPath
            // 
            this->fcgTXBatAfterAudioPath->AllowDrop = true;
            this->fcgTXBatAfterAudioPath->Location = System::Drawing::Point(126, 232);
            this->fcgTXBatAfterAudioPath->Name = L"fcgTXBatAfterAudioPath";
            this->fcgTXBatAfterAudioPath->Size = System::Drawing::Size(202, 21);
            this->fcgTXBatAfterAudioPath->TabIndex = 58;
            this->fcgTXBatAfterAudioPath->Tag = L"chValue";
            this->fcgTXBatAfterAudioPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXBatAfterAudioPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBBatAfterAudioPath
            // 
            this->fcgLBBatAfterAudioPath->AutoSize = true;
            this->fcgLBBatAfterAudioPath->Location = System::Drawing::Point(40, 236);
            this->fcgLBBatAfterAudioPath->Name = L"fcgLBBatAfterAudioPath";
            this->fcgLBBatAfterAudioPath->Size = System::Drawing::Size(61, 14);
            this->fcgLBBatAfterAudioPath->TabIndex = 57;
            this->fcgLBBatAfterAudioPath->Text = L"バッチファイル";
            // 
            // fcgCBRunBatAfterAudio
            // 
            this->fcgCBRunBatAfterAudio->AutoSize = true;
            this->fcgCBRunBatAfterAudio->Location = System::Drawing::Point(18, 207);
            this->fcgCBRunBatAfterAudio->Name = L"fcgCBRunBatAfterAudio";
            this->fcgCBRunBatAfterAudio->Size = System::Drawing::Size(201, 18);
            this->fcgCBRunBatAfterAudio->TabIndex = 55;
            this->fcgCBRunBatAfterAudio->Tag = L"chValue";
            this->fcgCBRunBatAfterAudio->Text = L"音声エンコード終了後、バッチ処理を行う";
            this->fcgCBRunBatAfterAudio->UseVisualStyleBackColor = true;
            // 
            // panel1
            // 
            this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->panel1->Location = System::Drawing::Point(18, 196);
            this->panel1->Name = L"panel1";
            this->panel1->Size = System::Drawing::Size(342, 1);
            this->panel1->TabIndex = 54;
            // 
            // fcgBTBatBeforeAudioPath
            // 
            this->fcgBTBatBeforeAudioPath->Location = System::Drawing::Point(330, 164);
            this->fcgBTBatBeforeAudioPath->Name = L"fcgBTBatBeforeAudioPath";
            this->fcgBTBatBeforeAudioPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTBatBeforeAudioPath->TabIndex = 53;
            this->fcgBTBatBeforeAudioPath->Tag = L"chValue";
            this->fcgBTBatBeforeAudioPath->Text = L"...";
            this->fcgBTBatBeforeAudioPath->UseVisualStyleBackColor = true;
            this->fcgBTBatBeforeAudioPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatBeforeAudioPath_Click);
            // 
            // fcgTXBatBeforeAudioPath
            // 
            this->fcgTXBatBeforeAudioPath->AllowDrop = true;
            this->fcgTXBatBeforeAudioPath->Location = System::Drawing::Point(126, 164);
            this->fcgTXBatBeforeAudioPath->Name = L"fcgTXBatBeforeAudioPath";
            this->fcgTXBatBeforeAudioPath->Size = System::Drawing::Size(202, 21);
            this->fcgTXBatBeforeAudioPath->TabIndex = 52;
            this->fcgTXBatBeforeAudioPath->Tag = L"chValue";
            this->fcgTXBatBeforeAudioPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXBatBeforeAudioPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBBatBeforeAudioPath
            // 
            this->fcgLBBatBeforeAudioPath->AutoSize = true;
            this->fcgLBBatBeforeAudioPath->Location = System::Drawing::Point(40, 167);
            this->fcgLBBatBeforeAudioPath->Name = L"fcgLBBatBeforeAudioPath";
            this->fcgLBBatBeforeAudioPath->Size = System::Drawing::Size(61, 14);
            this->fcgLBBatBeforeAudioPath->TabIndex = 50;
            this->fcgLBBatBeforeAudioPath->Text = L"バッチファイル";
            // 
            // fcgCBRunBatBeforeAudio
            // 
            this->fcgCBRunBatBeforeAudio->AutoSize = true;
            this->fcgCBRunBatBeforeAudio->Location = System::Drawing::Point(18, 139);
            this->fcgCBRunBatBeforeAudio->Name = L"fcgCBRunBatBeforeAudio";
            this->fcgCBRunBatBeforeAudio->Size = System::Drawing::Size(201, 18);
            this->fcgCBRunBatBeforeAudio->TabIndex = 48;
            this->fcgCBRunBatBeforeAudio->Tag = L"chValue";
            this->fcgCBRunBatBeforeAudio->Text = L"音声エンコード開始前、バッチ処理を行う";
            this->fcgCBRunBatBeforeAudio->UseVisualStyleBackColor = true;
            // 
            // fcgCXAudioPriority
            // 
            this->fcgCXAudioPriority->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioPriority->FormattingEnabled = true;
            this->fcgCXAudioPriority->Location = System::Drawing::Point(156, 20);
            this->fcgCXAudioPriority->Name = L"fcgCXAudioPriority";
            this->fcgCXAudioPriority->Size = System::Drawing::Size(136, 22);
            this->fcgCXAudioPriority->TabIndex = 47;
            this->fcgCXAudioPriority->Tag = L"chValue";
            // 
            // fcgLBAudioPriority
            // 
            this->fcgLBAudioPriority->AutoSize = true;
            this->fcgLBAudioPriority->Location = System::Drawing::Point(29, 23);
            this->fcgLBAudioPriority->Name = L"fcgLBAudioPriority";
            this->fcgLBAudioPriority->Size = System::Drawing::Size(62, 14);
            this->fcgLBAudioPriority->TabIndex = 46;
            this->fcgLBAudioPriority->Text = L"音声優先度";
            // 
            // fcgPNHideTabControlVideo
            // 
            this->fcgPNHideTabControlVideo->Controls->Add(this->fcgtabControlVideo);
            this->fcgPNHideTabControlVideo->Location = System::Drawing::Point(0, 26);
            this->fcgPNHideTabControlVideo->Name = L"fcgPNHideTabControlVideo";
            this->fcgPNHideTabControlVideo->Size = System::Drawing::Size(620, 522);
            this->fcgPNHideTabControlVideo->TabIndex = 12;
            // 
            // fcgPNHideTabControlAudio
            // 
            this->fcgPNHideTabControlAudio->Controls->Add(this->fcgtabControlAudio);
            this->fcgPNHideTabControlAudio->Location = System::Drawing::Point(620, 26);
            this->fcgPNHideTabControlAudio->Name = L"fcgPNHideTabControlAudio";
            this->fcgPNHideTabControlAudio->Size = System::Drawing::Size(388, 300);
            this->fcgPNHideTabControlAudio->TabIndex = 13;
            // 
            // fcgPNHideTabControlMux
            // 
            this->fcgPNHideTabControlMux->Controls->Add(this->fcgtabControlMux);
            this->fcgPNHideTabControlMux->Location = System::Drawing::Point(620, 327);
            this->fcgPNHideTabControlMux->Name = L"fcgPNHideTabControlMux";
            this->fcgPNHideTabControlMux->Size = System::Drawing::Size(388, 221);
            this->fcgPNHideTabControlMux->TabIndex = 14;
            // 
            // fcgTSBSave
            // 
            this->fcgTSBSave->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBSave.Image")));
            this->fcgTSBSave->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBSave->Name = L"fcgTSBSave";
            this->fcgTSBSave->Size = System::Drawing::Size(86, 22);
            this->fcgTSBSave->Text = L"上書き保存";
            this->fcgTSBSave->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBSave_Click);
            // 
            // fcgTSBSaveNew
            // 
            this->fcgTSBSaveNew->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBSaveNew.Image")));
            this->fcgTSBSaveNew->ImageTransparentColor = System::Drawing::Color::Black;
            this->fcgTSBSaveNew->Name = L"fcgTSBSaveNew";
            this->fcgTSBSaveNew->Size = System::Drawing::Size(77, 22);
            this->fcgTSBSaveNew->Text = L"新規保存";
            this->fcgTSBSaveNew->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBSaveNew_Click);
            // 
            // fcgTSBDelete
            // 
            this->fcgTSBDelete->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBDelete.Image")));
            this->fcgTSBDelete->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBDelete->Name = L"fcgTSBDelete";
            this->fcgTSBDelete->Size = System::Drawing::Size(53, 22);
            this->fcgTSBDelete->Text = L"削除";
            this->fcgTSBDelete->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBDelete_Click);
            // 
            // fcgtoolStripSeparator1
            // 
            this->fcgtoolStripSeparator1->Name = L"fcgtoolStripSeparator1";
            this->fcgtoolStripSeparator1->Size = System::Drawing::Size(6, 25);
            // 
            // fcgTSSettings
            // 
            this->fcgTSSettings->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSSettings.Image")));
            this->fcgTSSettings->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSSettings->Name = L"fcgTSSettings";
            this->fcgTSSettings->Size = System::Drawing::Size(79, 22);
            this->fcgTSSettings->Text = L"プリセット";
            this->fcgTSSettings->DropDownItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &frmConfig::fcgTSSettings_DropDownItemClicked);
            this->fcgTSSettings->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSSettings_Click);
            // 
            // fcgTSBCMDOnly
            // 
            this->fcgTSBCMDOnly->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->fcgTSBCMDOnly->CheckOnClick = true;
            this->fcgTSBCMDOnly->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBCMDOnly.Image")));
            this->fcgTSBCMDOnly->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBCMDOnly->Name = L"fcgTSBCMDOnly";
            this->fcgTSBCMDOnly->Size = System::Drawing::Size(76, 22);
            this->fcgTSBCMDOnly->Tag = L"chValue";
            this->fcgTSBCMDOnly->Text = L"CLIモード";
            this->fcgTSBCMDOnly->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgTSBCMDOnly_CheckedChanged);
            // 
            // toolStripSeparator3
            // 
            this->toolStripSeparator3->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->toolStripSeparator3->Name = L"toolStripSeparator3";
            this->toolStripSeparator3->Size = System::Drawing::Size(6, 25);
            // 
            // fcgTSLanguage
            // 
            this->fcgTSLanguage->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->fcgTSLanguage->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->fcgTSLanguage->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSLanguage.Image")));
            this->fcgTSLanguage->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSLanguage->Name = L"fcgTSLanguage";
            this->fcgTSLanguage->Size = System::Drawing::Size(44, 22);
            this->fcgTSLanguage->Text = L"言語";
            this->fcgTSLanguage->DropDownItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &frmConfig::fcgTSLanguage_DropDownItemClicked);
            // 
            // toolStripSeparator5
            // 
            this->toolStripSeparator5->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->toolStripSeparator5->Name = L"toolStripSeparator5";
            this->toolStripSeparator5->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->toolStripSeparator5->Size = System::Drawing::Size(6, 25);
            // 
            // fcgTSBBitrateCalc
            // 
            this->fcgTSBBitrateCalc->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->fcgTSBBitrateCalc->CheckOnClick = true;
            this->fcgTSBBitrateCalc->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->fcgTSBBitrateCalc->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBBitrateCalc.Image")));
            this->fcgTSBBitrateCalc->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBBitrateCalc->Name = L"fcgTSBBitrateCalc";
            this->fcgTSBBitrateCalc->Size = System::Drawing::Size(97, 22);
            this->fcgTSBBitrateCalc->Text = L"ビットレート計算機";
            this->fcgTSBBitrateCalc->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgTSBBitrateCalc_CheckedChanged);
            // 
            // toolStripSeparator2
            // 
            this->toolStripSeparator2->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->toolStripSeparator2->Name = L"toolStripSeparator2";
            this->toolStripSeparator2->Size = System::Drawing::Size(6, 25);
            // 
            // fcgTSBOtherSettings
            // 
            this->fcgTSBOtherSettings->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->fcgTSBOtherSettings->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->fcgTSBOtherSettings->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBOtherSettings.Image")));
            this->fcgTSBOtherSettings->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBOtherSettings->Name = L"fcgTSBOtherSettings";
            this->fcgTSBOtherSettings->Size = System::Drawing::Size(76, 22);
            this->fcgTSBOtherSettings->Text = L"その他の設定";
            this->fcgTSBOtherSettings->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBOtherSettings_Click);
            // 
            // fcgTSLSettingsNotes
            // 
            this->fcgTSLSettingsNotes->DoubleClickEnabled = true;
            this->fcgTSLSettingsNotes->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgTSLSettingsNotes->Margin = System::Windows::Forms::Padding(3, 1, 0, 2);
            this->fcgTSLSettingsNotes->Name = L"fcgTSLSettingsNotes";
            this->fcgTSLSettingsNotes->Overflow = System::Windows::Forms::ToolStripItemOverflow::Never;
            this->fcgTSLSettingsNotes->Size = System::Drawing::Size(45, 22);
            this->fcgTSLSettingsNotes->Text = L"メモ表示";
            this->fcgTSLSettingsNotes->DoubleClick += gcnew System::EventHandler(this, &frmConfig::fcgTSLSettingsNotes_DoubleClick);
            // 
            // fcgTSTSettingsNotes
            // 
            this->fcgTSTSettingsNotes->BackColor = System::Drawing::SystemColors::Window;
            this->fcgTSTSettingsNotes->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgTSTSettingsNotes->Margin = System::Windows::Forms::Padding(3, 0, 1, 0);
            this->fcgTSTSettingsNotes->Name = L"fcgTSTSettingsNotes";
            this->fcgTSTSettingsNotes->Overflow = System::Windows::Forms::ToolStripItemOverflow::Never;
            this->fcgTSTSettingsNotes->Size = System::Drawing::Size(200, 25);
            this->fcgTSTSettingsNotes->Text = L"メモ...";
            this->fcgTSTSettingsNotes->Visible = false;
            this->fcgTSTSettingsNotes->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTSTSettingsNotes_Leave);
            this->fcgTSTSettingsNotes->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &frmConfig::fcgTSTSettingsNotes_KeyDown);
            this->fcgTSTSettingsNotes->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTSTSettingsNotes_TextChanged);
            // 
            // toolStripSeparator4
            // 
            this->toolStripSeparator4->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->toolStripSeparator4->Name = L"toolStripSeparator4";
            this->toolStripSeparator4->Size = System::Drawing::Size(6, 25);
            // 
            // fcgtoolStripSettings
            // 
            this->fcgtoolStripSettings->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgtoolStripSettings->ImageScalingSize = System::Drawing::Size(18, 18);
            this->fcgtoolStripSettings->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(15) {
                this->fcgTSBSave,
                    this->fcgTSBSaveNew, this->fcgTSBDelete, this->fcgtoolStripSeparator1, this->fcgTSSettings, this->fcgTSBCMDOnly, this->toolStripSeparator3,
                    this->fcgTSLanguage, this->toolStripSeparator5, this->fcgTSBBitrateCalc, this->toolStripSeparator2, this->fcgTSBOtherSettings,
                    this->fcgTSLSettingsNotes, this->fcgTSTSettingsNotes, this->toolStripSeparator4
            });
            this->fcgtoolStripSettings->Location = System::Drawing::Point(0, 0);
            this->fcgtoolStripSettings->Name = L"fcgtoolStripSettings";
            this->fcgtoolStripSettings->Padding = System::Windows::Forms::Padding(0);
            this->fcgtoolStripSettings->Size = System::Drawing::Size(1008, 25);
            this->fcgtoolStripSettings->TabIndex = 1;
            this->fcgtoolStripSettings->Text = L"toolStrip1";
            // 
            // fcgPNHideToolStripBorder
            // 
            this->fcgPNHideToolStripBorder->Location = System::Drawing::Point(0, 22);
            this->fcgPNHideToolStripBorder->Name = L"fcgPNHideToolStripBorder";
            this->fcgPNHideToolStripBorder->Size = System::Drawing::Size(1020, 4);
            this->fcgPNHideToolStripBorder->TabIndex = 15;
            this->fcgPNHideToolStripBorder->Visible = false;
            // 
            // frmConfig
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;
            this->ClientSize = System::Drawing::Size(1008, 601);
            this->Controls->Add(this->fcgPNHideToolStripBorder);
            this->Controls->Add(this->fcgPNHideTabControlMux);
            this->Controls->Add(this->fcgPNHideTabControlAudio);
            this->Controls->Add(this->fcgPNHideTabControlVideo);
            this->Controls->Add(this->fcgLBguiExBlog);
            this->Controls->Add(this->fcgLBVersion);
            this->Controls->Add(this->fcgLBVersionDate);
            this->Controls->Add(this->fcgBTDefault);
            this->Controls->Add(this->fcgBTOK);
            this->Controls->Add(this->fcgBTCancel);
            this->Controls->Add(this->fcgTXCmd);
            this->Controls->Add(this->fcgtoolStripSettings);
            this->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;
            this->Name = L"frmConfig";
            this->ShowIcon = false;
            this->Text = L"Aviutl 出力 プラグイン";
            this->Load += gcnew System::EventHandler(this, &frmConfig::frmConfig_Load);
            this->fcgtabControlVideo->ResumeLayout(false);
            this->fcgtabPageX264Main->ResumeLayout(false);
            this->fcgtabPageX264Main->PerformLayout();
            this->fcgPNStatusFile->ResumeLayout(false);
            this->fcgPNStatusFile->PerformLayout();
            this->fcgPNBitrate->ResumeLayout(false);
            this->fcgPNBitrate->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBQuality))->EndInit();
            this->fcgPNX264Mode->ResumeLayout(false);
            this->fcgPNX264Mode->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAMPLimitBitrateLower))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAMPLimitBitrateUpper))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAMPLimitFileSize))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAutoNPass))->EndInit();
            this->fcggroupBoxX264Out->ResumeLayout(false);
            this->fcggroupBoxX264Out->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUSlices))->EndInit();
            this->fcggroupBoxThreads->ResumeLayout(false);
            this->fcggroupBoxThreads->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNULookaheadThreads))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUThreads))->EndInit();
            this->fcggroupBoxColorMatrix->ResumeLayout(false);
            this->fcggroupBoxColorMatrix->PerformLayout();
            this->fcgGroupBoxAspectRatio->ResumeLayout(false);
            this->fcgGroupBoxAspectRatio->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAspectRatioY))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAspectRatioX))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgpictureBoxX264))->EndInit();
            this->fcggroupBoxPreset->ResumeLayout(false);
            this->fcggroupBoxPreset->PerformLayout();
            this->fcgtabPageX264RC->ResumeLayout(false);
            this->fcgtabPageX264RC->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUTimebaseDen))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUTimebaseNum))->EndInit();
            this->fcggroupBoxAQ->ResumeLayout(false);
            this->fcggroupBoxAQ->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAQStrength))->EndInit();
            this->fcggroupBoxPsyRd->ResumeLayout(false);
            this->fcggroupBoxPsyRd->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUPsyTrellis))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUPsyRDO))->EndInit();
            this->fcggroupBoxVbv->ResumeLayout(false);
            this->fcggroupBoxVbv->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVBVbuf))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVBVmax))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNURCLookahead))->EndInit();
            this->fcggroupBoxQP->ResumeLayout(false);
            this->fcggroupBoxQP->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUChromaQp))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQpstep))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQpmax))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQpmin))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQcomp))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUPBRatio))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUIPRatio))->EndInit();
            this->fcgtabPageX264Frame->ResumeLayout(false);
            this->fcgtabPageX264Frame->PerformLayout();
            this->fcggroupBoxX264Other->ResumeLayout(false);
            this->fcggroupBoxX264Other->PerformLayout();
            this->fcggroupBoxME->ResumeLayout(false);
            this->fcggroupBoxME->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNURef))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUMERange))->EndInit();
            this->fcggroupBoxMBTypes->ResumeLayout(false);
            this->fcggroupBoxMBTypes->PerformLayout();
            this->fcggroupBoxBframes->ResumeLayout(false);
            this->fcggroupBoxBframes->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUBBias))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUBframes))->EndInit();
            this->fcggroupBoxGOP->ResumeLayout(false);
            this->fcggroupBoxGOP->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUKeyint))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUMinKeyint))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUScenecut))->EndInit();
            this->fcggroupBoxDeblock->ResumeLayout(false);
            this->fcggroupBoxDeblock->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUDeblockThreshold))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUDeblockStrength))->EndInit();
            this->fcgtabPageExSettings->ResumeLayout(false);
            this->fcgtabPageExSettings->PerformLayout();
            this->fcggroupBoxCmdEx->ResumeLayout(false);
            this->fcggroupBoxCmdEx->PerformLayout();
            this->fcggroupBoxExSettings->ResumeLayout(false);
            this->fcggroupBoxExSettings->PerformLayout();
            this->fcgCSExeFiles->ResumeLayout(false);
            this->fcgtabControlMux->ResumeLayout(false);
            this->fcgtabPageMP4->ResumeLayout(false);
            this->fcgtabPageMP4->PerformLayout();
            this->fcgtabPageMKV->ResumeLayout(false);
            this->fcgtabPageMKV->PerformLayout();
            this->fcgtabPageMux->ResumeLayout(false);
            this->fcgtabPageMux->PerformLayout();
            this->fcgtabPageBat->ResumeLayout(false);
            this->fcgtabPageBat->PerformLayout();
            this->fcgCSCQM->ResumeLayout(false);
            this->fcgtabControlAudio->ResumeLayout(false);
            this->fcgtabPageAudioMain->ResumeLayout(false);
            this->fcgtabPageAudioMain->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAudioBitrate))->EndInit();
            this->fcgtabPageAudioOther->ResumeLayout(false);
            this->fcgtabPageAudioOther->PerformLayout();
            this->fcgPNHideTabControlVideo->ResumeLayout(false);
            this->fcgPNHideTabControlAudio->ResumeLayout(false);
            this->fcgPNHideTabControlMux->ResumeLayout(false);
            this->fcgtoolStripSettings->ResumeLayout(false);
            this->fcgtoolStripSettings->PerformLayout();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
    private:
        CONF_ENC *cnf_fcgTemp;
        const SYSTEM_DATA *sys_dat;
        std::vector<tstring> *list_lng;
        CONF_GUIEX *conf;
        LocalSettings LocalStg;
        DarkenWindowStgReader *dwStgReader;
        AuoTheme themeMode;
        TBValueBitrateConvert TBBConvert;
        System::Threading::Timer^ qualityTimer;
        int timerChangeValue;
        bool CurrentPipeEnabled;
        bool stgChanged;
        String^ CurrentStgDir;
        ToolStripMenuItem^ CheckedStgMenuItem;
        CONF_GUIEX *cnf_stgSelected;
        String^ lastQualityStr;
    private:
        System::Void CheckTheme();
        System::Void SetAllMouseMove(Control ^top, const AuoTheme themeTo);
        System::Void fcgMouseEnter_SetColor(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgMouseLeave_SetColor(System::Object^  sender, System::EventArgs^  e);
        System::Void TabControl_DarkDrawItem(System::Object^ sender, DrawItemEventArgs^ e);

        System::Void LoadLangText();
        System::Int32 GetCurrentAudioDefaultBitrate();
        delegate System::Void qualityTimerChangeDelegate();
        System::Void qualityTimerChange(Object^ state);
        System::Void fcgTBQualityChange();
        System::Void InitTimer();
        System::Void InitComboBox();
        System::Void setAudioExtDisplay();
        System::Void AudioEncodeModeChanged();
        System::Void InitStgFileList();
        System::Void RebuildStgFileDropDown(String^ stgDir);
        System::Void RebuildStgFileDropDown(ToolStripDropDownItem^ TS, String^ dir);
        System::Void SetLocalStg();
        System::Void LoadLocalStg();
        System::Void SaveLocalStg();
        System::Boolean CheckLocalStg();
        System::Void SetTXMaxLen(TextBox^ TX, int max_len);
        System::Void SetTXMaxLenAll();
        System::Void InitForm();
        System::Void ConfToFrm(CONF_GUIEX *cnf, bool all);
        System::Void FrmToConf(CONF_GUIEX *cnf);
        System::Void SetChangedEvent(Control^ control, System::EventHandler^ _event);
        System::Void SetToolStripEvents(ToolStrip^ TS, System::Windows::Forms::MouseEventHandler^ _event);
        System::Void SetAllCheckChangedEvents(Control ^top);
        System::Void SaveToStgFile(String^ stgName);
        System::Void DeleteStgFile(ToolStripMenuItem^ mItem);
        System::Boolean EnableSettingsNoteChange(bool Enable);
        System::Void fcgTSLSettingsNotes_DoubleClick(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSTSettingsNotes_Leave(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSTSettingsNotes_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e);
        System::Void fcgTSTSettingsNotes_TextChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void GetfcgTSLSettingsNotes(TCHAR *notes, int nSize);
        System::Void SetfcgTSLSettingsNotes(const TCHAR *notes);
        System::Void SetfcgTSLSettingsNotes(String^ notes);
        System::Void fcgTSBSave_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSBSaveNew_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSBDelete_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSSettings_DropDownItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e);
        System::Void UncheckAllDropDownItem(ToolStripItem^ mItem);
        ToolStripMenuItem^ fcgTSSettingsSearchItem(String^ stgPath, ToolStripItem^ mItem);
        ToolStripMenuItem^ fcgTSSettingsSearchItem(String^ stgPath);
        System::Void CheckTSSettingsDropDownItem(ToolStripMenuItem^ mItem);
        System::Void CheckTSItemsEnabled(CONF_GUIEX *current_conf);

        System::Void InitLangList();
        System::Void SaveSelectedLanguage(const TCHAR *language_text);
        System::Void SetSelectedLanguage(const TCHAR *language_text);
        System::Void CheckTSLanguageDropDownItem(ToolStripMenuItem^ mItem);
        System::Void fcgTSLanguage_DropDownItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e);

        System::Void SetHelpToolTips();
        System::Void SetHelpToolTipsColorMatrix(Control^ control, const TCHAR *type);
        System::Void SetX264VersionToolTip(String^ x264Path);
        System::Void ShowExehelp(String^ ExePath, String^ args);
        System::Void fcgTSBOtherSettings_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgChangeEnabled(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgChangeMuxerVisible(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSBBitrateCalc_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void CloseBitrateCalc();
        System::Void SetfbcBTABEnable(bool enable, int max);
        System::Void SetfbcBTVBEnable(bool enable);
        System::Void fcgCBUsehighbit_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSBCMDOnly_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgCBAudio2pass_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgCBAFS_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgCXAudioEncoder_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgCXAudioEncMode_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgCXX264Mode_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTXQuality_TextChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTXQuality_Validating(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e);
        System::Void SetTBValueToTextBox();
        System::Void AdjustLocation();
        System::Void ActivateToolTip(bool Enable);
        System::Void SetStgEscKey(bool Enable);
        System::Void fcgArrangeForAutoMultiPass(bool enable);
        System::Boolean fcgLastX264ModeAsAMP;
        System::Void AdjustCXDropDownWidth(ComboBox^ CX);
        System::Void InitCXCmdExInsert();
        System::Void fcgCXCmdExInsert_FontChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgCXCmdExInsert_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTXQuality_Enter(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgCheckAMPAutoBitrateEvent(System::Object^  sender, System::EventArgs^  e);
        System::Void AddCheckAMPAutoBitrateEvent();
    public:
        System::Void InitData(CONF_GUIEX *set_config, const SYSTEM_DATA *system_data);
        System::Void SetVideoBitrate(int bitrate);
        System::Void SetAudioBitrate(int bitrate);
        System::Void InformfbcClosed();
    private:
        System::Boolean useAudioExt() {
            #if ENCODER_X264 || ENCODER_X265 || ENCODER_SVTAV1
                return true;
            #else
                return fcgCBAudioUseExt->Checked;
            #endif
        }
        System::Void AddfcgLBAMPAutoBitrate() {
            //fcgLBAMPAutoBitrateには拡張した簡易透過ラベルを使用する(背景透過&マウスイベント透過)
            //普通に作成しておくと、フォームデザイナが使用できなくなり厄介なので、ここで作っておく
            fcgLBAMPAutoBitrate = gcnew TransparentLabel();
            fcgPNBitrate->Controls->Add(fcgLBAMPAutoBitrate);
            fcgLBAMPAutoBitrate->ForeColor = System::Drawing::Color::OrangeRed;
            fcgLBAMPAutoBitrate->Location = System::Drawing::Point(90, 30);
            fcgLBAMPAutoBitrate->Name = L"fcgLBAMPAutoBitrate";
            fcgLBAMPAutoBitrate->Size = System::Drawing::Size(181, 30);
            fcgLBAMPAutoBitrate->TabIndex = 15;
            fcgLBAMPAutoBitrate->Visible = false;
            fcgLBAMPAutoBitrate->Text = LOAD_CLI_STRING(AuofcgLBAMPAutoBitrate);
            fcgLBAMPAutoBitrate->BringToFront();
        }
    private:
        System::Void fcgTSItem_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
            EnableSettingsNoteChange(false);
        }
    private:
        System::Void frmConfig_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
            if (e->KeyCode == Keys::Escape)
                this->Close();
            if ((e->KeyData & (Keys::Control | Keys::Shift | Keys::Enter)) == (Keys::Control | Keys::Shift | Keys::Enter))
                fcgBTOK_Click(sender, nullptr);
        }
    private:
        System::Void NUSelectAll(System::Object^  sender, System::EventArgs^  e) {
            NumericUpDown^ NU = dynamic_cast<NumericUpDown^>(sender);
            NU->Select(0, NU->Text->Length);
        }
    private:
        System::Void setComboBox(ComboBox^ CX, const ENC_OPTION_STR * list) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            for (int i = 0; list[i].desc; i++) {
                String^ string = nullptr;
                if (list[i].mes != AUO_MES_UNKNOWN) {
                    string = LOAD_CLI_STRING(list[i].mes);
                } 
                if (string == nullptr || string->Length == 0) {
                    string = String(list[i].desc).ToString();
                }
                CX->Items->Add(string);
            }
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox^ CX, const char * const * list) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            for (int i = 0; list[i]; i++)
                CX->Items->Add(String(list[i]).ToString());
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox^ CX, const WCHAR * const * list) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            for (int i = 0; list[i]; i++)
                CX->Items->Add(String(list[i]).ToString());
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setPriorityList(ComboBox^ CX) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            for (int i = 0; priority_table[i].text; i++) {
                String^ string = nullptr;
                if (priority_table[i].mes != AUO_MES_UNKNOWN) {
                    string = LOAD_CLI_STRING(priority_table[i].mes);
                }
                if (string == nullptr || string->Length == 0) {
                    string = String(priority_table[i].text).ToString();
                }
                CX->Items->Add(string);
            }
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setMuxerCmdExNames(ComboBox^ CX, int muxer_index) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            MUXER_SETTINGS *mstg = &sys_dat->exstg->s_mux[muxer_index];
            for (int i = 0; i < mstg->ex_count; i++)
                CX->Items->Add(String(mstg->ex_cmd[i].name).ToString());
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setAudioEncoderNames() {
            fcgCXAudioEncoder->BeginUpdate();
            const int prevIdx = fcgCXAudioEncoder->SelectedIndex;
            fcgCXAudioEncoder->Items->Clear();
            //fcgCXAudioEncoder->Items->AddRange(reinterpret_cast<array<String^>^>(LocalStg.audEncName->ToArray(String::typeid)));
            fcgCXAudioEncoder->Items->AddRange(LocalStg.audEncName->ToArray());
            SetCXIndex(fcgCXAudioEncoder, prevIdx);
            fcgCXAudioEncoder->EndUpdate();
        }
    private:
        System::Void TX_LimitbyBytes(System::Object^  sender, System::ComponentModel::CancelEventArgs^ e) {
            int maxLength = 0;
            int stringBytes = 0;
            TextBox^ TX = nullptr;
            if ((TX = dynamic_cast<TextBox^>(sender)) == nullptr)
                return;
            stringBytes = CountStringBytes(TX->Text);
            maxLength = TX->MaxLength;
            if (stringBytes > maxLength - 1) {
                e->Cancel = true;
                MessageBox::Show(this, LOAD_CLI_STRING(AUO_CONFIG_TEXT_LIMIT_LENGTH), LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }
    private:
        System::Boolean openExeFile(TextBox^ TX, String^ ExeName) {
            //WinXPにおいて、OpenFileDialogはCurrentDirctoryを勝手に変更しやがるので、
            //一度保存し、あとから再適用する
            String^ CurrentDir = Directory::GetCurrentDirectory();
            OpenFileDialog ofd;
            ofd.Multiselect = false;
            ofd.FileName = ExeName;
            ofd.Filter = MakeExeFilter(ExeName);
            if (Directory::Exists(LocalStg.LastAppDir))
                ofd.InitialDirectory = Path::GetFullPath(LocalStg.LastAppDir);
            else if (File::Exists(TX->Text))
                ofd.InitialDirectory = Path::GetFullPath(Path::GetDirectoryName(TX->Text));
            else
                ofd.InitialDirectory = String(sys_dat->aviutl_dir).ToString();
            bool ret = (ofd.ShowDialog() == System::Windows::Forms::DialogResult::OK);
            if (ret) {
                if (sys_dat->exstg->s_local.get_relative_path)
                    ofd.FileName = GetRelativePath(ofd.FileName, CurrentDir);
                LocalStg.LastAppDir = Path::GetDirectoryName(Path::GetFullPath(ofd.FileName));
                TX->Text = ofd.FileName;
                TX->SelectionStart = TX->Text->Length;
            }
            Directory::SetCurrentDirectory(CurrentDir);
            return ret;
        }
    private:
        System::Void fcgBTX264Path_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openExeFile(fcgTXX264Path, LocalStg.x264ExeName))
                SetX264VersionToolTip(fcgTXX264Path->Text);
        }
    private:
        System::Void fcgBTX264PathSub_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openExeFile(fcgTXX264PathSub, LocalStg.x264ExeName))
                SetX264VersionToolTip(fcgTXX264PathSub->Text);
        }
    private:
        System::Void frmConfig::ExeTXPathEnter() {
            fcgTXX264Path_Enter(nullptr, nullptr);
            fcgTXX264PathSub_Enter(nullptr, nullptr);
            fcgTXAudioEncoderPath_Enter(nullptr, nullptr);
            fcgTXMP4MuxerPath_Enter(nullptr, nullptr);
            fcgTXTC2MP4Path_Enter(nullptr, nullptr);
            fcgTXMP4RawPath_Enter(nullptr, nullptr);
            fcgTXMKVMuxerPath_Enter(nullptr, nullptr);
        }
    private:
        System::Void frmConfig::ExeTXPathLeave() {
            fcgTXX264Path_Leave(nullptr, nullptr);
            fcgTXX264PathSub_Leave(nullptr, nullptr);
            fcgTXAudioEncoderPath_Leave(nullptr, nullptr);
            fcgTXMP4MuxerPath_Leave(nullptr, nullptr);
            fcgTXTC2MP4Path_Leave(nullptr, nullptr);
            fcgTXMP4RawPath_Leave(nullptr, nullptr);
            fcgTXMKVMuxerPath_Leave(nullptr, nullptr);
        }
    private:
        System::Void fcgTXX264Path_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXX264Path->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXX264Path->Text = L"";
            }
        }
    private:
        System::Void fcgTXX264Path_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXX264Path->Text->Length == 0) {
                fcgTXX264Path->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
            SetX264VersionToolTip(fcgTXX264Path->Text);
        }
    private:
        System::Void fcgTXX264PathSub_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXX264PathSub->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXX264PathSub->Text = L"";
            }
        }
    private:
        System::Void fcgTXX264PathSub_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXX264PathSub->Text->Length == 0) {
                fcgTXX264PathSub->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
            SetX264VersionToolTip(fcgTXX264PathSub->Text);
        }
    private:
        System::Void fcgTXAudioEncoderPath_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXAudioEncoderPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXAudioEncoderPath->Text = L"";
            }
        }
    private:
        System::Void fcgTXAudioEncoderPath_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXAudioEncoderPath->Text->Length == 0) {
                fcgTXAudioEncoderPath->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
        }
    private:
        System::Void fcgTXMP4MuxerPath_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMP4MuxerPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXMP4MuxerPath->Text = L"";
            }
        }
    private:
        System::Void fcgTXMP4MuxerPath_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMP4MuxerPath->Text->Length == 0) {
                fcgTXMP4MuxerPath->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
        }
    private:
        System::Void fcgTXTC2MP4Path_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXTC2MP4Path->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXTC2MP4Path->Text = L"";
            }
        }
    private:
        System::Void fcgTXTC2MP4Path_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXTC2MP4Path->Text->Length == 0) {
                fcgTXTC2MP4Path->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
        }
    private:
        System::Void fcgTXMP4RawPath_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMP4RawPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXMP4RawPath->Text = L"";
            }
        }
    private:
        System::Void fcgTXMP4RawPath_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMP4RawPath->Text->Length == 0) {
                fcgTXMP4RawPath->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
        }
    private:
        System::Void fcgTXMKVMuxerPath_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMKVMuxerPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXMKVMuxerPath->Text = L"";
            }
        }
    private:
        System::Void fcgTXMKVMuxerPath_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMKVMuxerPath->Text->Length == 0) {
                fcgTXMKVMuxerPath->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
        }
    private:
        System::Void fcgBTMP4MuxerPath_Click(System::Object^  sender, System::EventArgs^  e) {
            openExeFile(fcgTXMP4MuxerPath, LocalStg.MP4MuxerExeName);
        }
    private:
        System::Void fcgBTTC2MP4Path_Click(System::Object^  sender, System::EventArgs^  e) {
            openExeFile(fcgTXTC2MP4Path, LocalStg.TC2MP4ExeName);
        }
    private:
        System::Void fcgBTMP4RawMuxerPath_Click(System::Object^  sender, System::EventArgs^  e) {
            openExeFile(fcgTXMP4RawPath, LocalStg.MP4RawExeName);
        }
    private:
        System::Void fcgBTAudioEncoderPath_Click(System::Object^  sender, System::EventArgs^  e) {
            int index = fcgCXAudioEncoder->SelectedIndex;
            openExeFile(fcgTXAudioEncoderPath, LocalStg.audEncExeName[index]);
        }
    private:
        System::Void fcgBTMKVMuxerPath_Click(System::Object^  sender, System::EventArgs^  e) {
            openExeFile(fcgTXMKVMuxerPath, LocalStg.MKVMuxerExeName);
        }
    private:
        System::Void openTempFolder(TextBox^ TX) {
            FolderBrowserDialog^ fbd = fcgfolderBrowserTemp;
            if (Directory::Exists(TX->Text))
                fbd->SelectedPath = TX->Text;
            if (fbd->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
                if (sys_dat->exstg->s_local.get_relative_path)
                    fbd->SelectedPath = GetRelativePath(fbd->SelectedPath);
                TX->Text = fbd->SelectedPath;
                TX->SelectionStart = TX->Text->Length;
            }
        }
    private:
        System::Void fcgBTCustomAudioTempDir_Click(System::Object^  sender, System::EventArgs^  e) {
            openTempFolder(fcgTXCustomAudioTempDir);
        }
    private:
        System::Void fcgBTMP4BoxTempDir_Click(System::Object^  sender, System::EventArgs^  e) {
            openTempFolder(fcgTXMP4BoxTempDir);
        }
    private:
        System::Void fcgBTCustomTempDir_Click(System::Object^  sender, System::EventArgs^  e) {
            openTempFolder(fcgTXCustomTempDir);
        }
    private:
        System::Boolean openAndSetFilePath(TextBox^ TX, String^ fileTypeName) {
            return openAndSetFilePath(TX, fileTypeName, nullptr, nullptr);
        }
    private:
        System::Boolean openAndSetFilePath(TextBox^ TX, String^ fileTypeName, String^ ext) {
            return openAndSetFilePath(TX, fileTypeName, ext, nullptr);
        }
    private:
        System::Boolean openAndSetFilePath(TextBox^ TX, String^ fileTypeName, String^ ext, String^ dir) {
            //WinXPにおいて、OpenFileDialogはCurrentDirctoryを勝手に変更しやがるので、
            //一度保存し、あとから再適用する
            String^ CurrentDir = Directory::GetCurrentDirectory();
            //設定
            if (ext == nullptr)
                ext = L".*";
            OpenFileDialog^ ofd = fcgOpenFileDialog;
            ofd->FileName = L"";
            if (dir != nullptr && Directory::Exists(dir))
                ofd->InitialDirectory = dir;
            if (TX->Text->Length) {
                String^ fileName = nullptr;
                try {
                    fileName = Path::GetFileName(TX->Text);
                } catch (...) {
                    //invalid charによる例外は破棄
                }
                if (fileName != nullptr)
                    ofd->FileName = fileName;
            }
            ofd->Multiselect = false;
            ofd->Filter = fileTypeName + L"(*" + ext + L")|*" + ext;
            bool ret = (ofd->ShowDialog() == System::Windows::Forms::DialogResult::OK);
            if (ret) {
                if (sys_dat->exstg->s_local.get_relative_path)
                    ofd->FileName = GetRelativePath(ofd->FileName, CurrentDir);
                TX->Text = ofd->FileName;
                TX->SelectionStart = TX->Text->Length;
            }
            Directory::SetCurrentDirectory(CurrentDir);
            return ret;
        }
    private:
        System::Void fcgBTStatusFile_Click(System::Object^  sender, System::EventArgs^  e) {
            openAndSetFilePath(fcgTXStatusFile, LOAD_CLI_STRING(AUO_CONFIG_STATUS_FILE));
        }
    private:
        System::Void fcgBTTCIN_Click(System::Object^  sender, System::EventArgs^  e) {
            openAndSetFilePath(fcgTXTCIN, LOAD_CLI_STRING(AUO_CONFIG_TC_FILE));
        }
    private:
        System::Void fcgBTBatBeforePath_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openAndSetFilePath(fcgTXBatBeforePath, LOAD_CLI_STRING(AUO_CONFIG_BAT_FILE), ".bat", LocalStg.LastBatDir))
                LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatBeforePath->Text);
        }
    private:
        System::Void fcgBTBatAfterPath_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openAndSetFilePath(fcgTXBatAfterPath, LOAD_CLI_STRING(AUO_CONFIG_BAT_FILE), ".bat", LocalStg.LastBatDir))
                LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatAfterPath->Text);
        }
    private:
        System::Void fcgBTBatBeforeAudioPath_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openAndSetFilePath(fcgTXBatBeforeAudioPath, LOAD_CLI_STRING(AUO_CONFIG_BAT_FILE), ".bat", LocalStg.LastBatDir))
                LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatBeforeAudioPath->Text);
        }
    private:
        System::Void fcgBTBatAfterAudioPath_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openAndSetFilePath(fcgTXBatAfterAudioPath, LOAD_CLI_STRING(AUO_CONFIG_BAT_FILE), ".bat", LocalStg.LastBatDir))
                LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatAfterAudioPath->Text);
        }
    private:
        System::Void SetCXIndex(ComboBox^ CX, int index) {
            if (CX->Items->Count > 0) {
                CX->SelectedIndex = clamp(index, 0, CX->Items->Count - 1);
            }
        }
    private:
        System::Void SetNUValue(NumericUpDown^ NU, Decimal d) {
            NU->Value = clamp(d, NU->Minimum, NU->Maximum);
        }
    private:
        System::Void SetNUValue(NumericUpDown^ NU, int i) {
            NU->Value = clamp(Convert::ToDecimal(i), NU->Minimum, NU->Maximum);
        }
    private:
        System::Void SetNUValue(NumericUpDown^ NU, unsigned int i) {
            NU->Value = clamp(Convert::ToDecimal(i), NU->Minimum, NU->Maximum);
        }
    private:
        System::Void SetNUValue(NumericUpDown^ NU, float f) {
            NU->Value = clamp(Convert::ToDecimal(f), NU->Minimum, NU->Maximum);
        }
    private:
        System::Void SetNUValue(NumericUpDown^ NU, double d) {
            NU->Value = clamp(Convert::ToDecimal(d), NU->Minimum, NU->Maximum);
        }
    private:
        System::Void CSCqmCheckedChanged(System::Object^  sender, System::EventArgs^  e) {
            ToolStripMenuItem^ item = (ToolStripMenuItem^)sender;
            if (item->Checked) {
                fcgTXCQM->ReadOnly = !(String::Equals(item->Text, fcgCSCqmFile->Text));
                if (fcgTXCQM->ReadOnly)
                    fcgTXCQM->Text = item->Text;
            }
        }
    private:
        System::Void CSCqmClick(System::Object^  sender, System::EventArgs^  e) {
            ToolStripMenuItem^ item = (ToolStripMenuItem^)sender;
            bool cqmfile_clicked = String::Equals(item->Text, fcgCSCqmFile->Text);
            if (cqmfile_clicked)
                if (!(openAndSetFilePath(fcgTXCQM, LOAD_CLI_STRING(AUO_CONFIG_MATRIX_FILE))))
                    return;
            fcgTXCQM->SelectionStart = fcgTXCQM->Text->Length;
            fcgCSFlat->Checked = false;
            fcgCSJvt->Checked = false;
            fcgCSCqmFile->Checked = false;
            item->Checked = true;
            fcgRebuildCmd(sender, e);
        }
    private:
        System::Void SetCQM(int index, const TCHAR *cqmfile) {
            for (int i = 0; i < fcgCSCQM->Items->Count; i++) {
                ToolStripMenuItem^ TSItem = dynamic_cast<ToolStripMenuItem^>(fcgCSCQM->Items[i]);
                if (TSItem != nullptr)
                    TSItem->Checked = (index == Convert::ToInt32(TSItem->Tag));
            }
            if (index == 2)
                fcgTXCQM->Text = String(cqmfile).ToString();
        }
    private:
        System::Int32 GetCQMIndex(TCHAR *cqmfile, int nSize) {
            cqmfile[0] = _T('\0');
            for (int i = 0; i < fcgCSCQM->Items->Count; i++) {
                ToolStripMenuItem^ TSItem = dynamic_cast<ToolStripMenuItem^>(fcgCSCQM->Items[i]);
                if (TSItem != nullptr && TSItem->Checked) {
                    int index = Convert::ToInt32(TSItem->Tag);
                    if (index == 2)
                        GetWCHARfromString(cqmfile, nSize, fcgTXCQM->Text);
                    return index;
                }
            }
            return 0;
        }
    private:
        System::Void fcgBTMatrix_Click(System::Object^  sender, System::EventArgs^  e) {
            fcgCSCQM->Show(fcggroupBoxX264Other, fcgBTMatrix->Location.X, fcgBTMatrix->Location.Y + fcgBTMatrix->Height);
        }
    private:
        System::Int32 SetTBValue(TrackBar^ TB, int value) {
            int i = clamp(value, TB->Minimum, TB->Maximum);
            TB->Value = i;
            return i;
        }
    private:
        System::Void fcgTBQuality_Scroll(System::Object^  sender, System::EventArgs^  e) {
            SetTBValueToTextBox();
        }
    private:
        System::Void fcgTBQuality_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgLBAMPAutoBitrate != nullptr)
                fcgLBAMPAutoBitrate->Invalidate(); //上に乗っかっているので再描画をかける
        }
    private:
        System::Void fcgBTTBQualitySubtract_Click(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTBQuality->Value > fcgTBQuality->Minimum) {
                fcgTBQuality->Value--;
                SetTBValueToTextBox();
            }
        }
    private:
        System::Void fcgBTTBQualityAdd_Click(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTBQuality->Value < fcgTBQuality->Maximum) {
                fcgTBQuality->Value++;
                SetTBValueToTextBox();
            }
        }
    private:
        System::Void fcgCBNulOut_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgCBNulOut->Enabled)
                cnf_fcgTemp->nul_out = fcgCBNulOut->Checked;
        }
    private:
        System::Void fcgCBSlowFirstPass_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgCBFastFirstPass->Enabled)
                cnf_fcgTemp->slow_first_pass = !fcgCBFastFirstPass->Checked;
        }
    private:
        System::Void fcgBTApplyPreset_Click(System::Object^  sender, System::EventArgs^  e) {
            CONF_GUIEX cnf;
            init_CONF_GUIEX(&cnf, fcgCBUsehighbit->Checked);
            cnf.enc.preset = fcgCXPreset->SelectedIndex;
            cnf.enc.tune = fcgCXTune->SelectedIndex;
            cnf.enc.profile = fcgCXProfile->SelectedIndex;
            apply_presets(&cnf.enc);
            ConfToFrm(&cnf, false);
        }
    private:
        System::Void fcgBTCmdEx_Click(System::Object^  sender, System::EventArgs^  e) {
            CONF_GUIEX cnf;
            init_CONF_GUIEX(&cnf, fcgCBUsehighbit->Checked);
            FrmToConf(&cnf);
            TCHAR cmdex[2048] = { 0 };
            GetWCHARfromString(cmdex, _countof(cmdex), fcgTXCmdEx->Text);
            set_cmd_to_conf(cmdex, &cnf.enc);
            ConfToFrm(&cnf, false);
        }
    private:
        System::Void frmConfig_Load(System::Object^  sender, System::EventArgs^  e) {
            InitForm();
        }
    private:
        System::Void fcgBTOK_Click(System::Object^  sender, System::EventArgs^  e) {
            if (CheckLocalStg())
                return;
            init_CONF_GUIEX(conf, fcgCBUsehighbit->Checked);
            FrmToConf(conf);
            SaveLocalStg();
            ZeroMemory(conf->oth.notes, sizeof(conf->oth.notes));
            this->Close();
        }
    private:
        System::Void fcgBTCancel_Click(System::Object^  sender, System::EventArgs^  e) {
            this->Close();
        }
    private:
        System::Void fcgBTDefault_Click(System::Object^  sender, System::EventArgs^  e) {
            CONF_GUIEX confDefault;
            init_CONF_GUIEX(&confDefault, FALSE);
            ConfToFrm(&confDefault, true);
        }
    private:
        System::Void ChangePresetNameDisplay(bool changed) {
            if (CheckedStgMenuItem != nullptr) {
                fcgTSSettings->Text = (changed) ? L"[" + CheckedStgMenuItem->Text + L"]*" : CheckedStgMenuItem->Text;
                fcgTSBSave->Enabled = changed;
            }
        }
    private:
        System::Void fcgRebuildCmd(System::Object^  sender, System::EventArgs^  e) {
            TCHAR re_cmd[MAX_CMD_LEN] = { 0 };
            CONF_GUIEX rebuild;
            init_CONF_GUIEX(&rebuild, fcgCBUsehighbit->Checked);
            FrmToConf(&rebuild);
            if (!rebuild.oth.disable_guicmd)
                build_cmd_from_conf(re_cmd, _countof(re_cmd), &rebuild.enc, &rebuild.vid, FALSE);
            fcgTXCmd->Text = String(re_cmd).ToString();
            if (CheckedStgMenuItem != nullptr)
                ChangePresetNameDisplay(memcmp(&rebuild, cnf_stgSelected, sizeof(CONF_GUIEX)) != 0);
        }
    private:
        System::Void CheckOtherChanges(System::Object^  sender, System::EventArgs^  e) {
            if (CheckedStgMenuItem == nullptr)
                return;
            CONF_GUIEX check_change;
            init_CONF_GUIEX(&check_change, fcgCBUsehighbit->Checked);
            FrmToConf(&check_change);
            ChangePresetNameDisplay(memcmp(&check_change, cnf_stgSelected, sizeof(CONF_GUIEX)) != 0);
        }
    private:
        System::Void fcgTXCmd_DoubleClick(System::Object^  sender, System::EventArgs^  e) {
            int offset = (fcgTXCmd->Multiline) ? -fcgTXCmdfulloffset : fcgTXCmdfulloffset;
            fcgTXCmd->Height += offset;
            this->Height += offset;
            fcgTXCmd->Multiline = !fcgTXCmd->Multiline;
        }
    private:
        System::Void fcgTSSettings_Click(System::Object^  sender, System::EventArgs^  e) {
            if (EnableSettingsNoteChange(false))
                fcgTSSettings->ShowDropDown();
        }
    private:
        System::Void fcgBTTBQualityAdd_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
            timerChangeValue = 1;
            qualityTimer->Change(fcgTBQualityTimerLatency, fcgTBQualityTimerPeriod);
        }
    private:
        System::Void fcgBTTBQualityAdd_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
            qualityTimer->Change(System::Threading::Timeout::Infinite, fcgTBQualityTimerPeriod);
        }
    private:
        System::Void fcgBTTBQualitySubtract_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
            timerChangeValue = -1;
            qualityTimer->Change(fcgTBQualityTimerLatency, fcgTBQualityTimerPeriod);
        }
    private:
        System::Void fcgBTTBQualitySubtract_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
            qualityTimer->Change(System::Threading::Timeout::Infinite, fcgTBQualityTimerPeriod);
        }
    private:
        System::Void fcgTXX264Path_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXX264Path->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXX264PathSub->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
                LocalStg.x264Path = L"";
                fcgTXX264Path->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                int c = fcgTXX264Path->SelectionStart;
                LocalStg.x264Path = fcgTXX264Path->Text;
                fcgTXX264Path->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                fcgTXX264PathSub->Text = LocalStg.x264Path;
                fcgTXX264PathSub->SelectionStart = fcgTXX264PathSub->Text->Length;
                fcgTXX264Path->SelectionStart = c;
                fcgBTX264Path->ContextMenuStrip = (File::Exists(fcgTXX264Path->Text)) ? fcgCSExeFiles : nullptr;
            }
        }
    private:
        System::Void fcgTXX264PathSub_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXX264PathSub->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXX264Path->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
                LocalStg.x264Path = L"";
                fcgTXX264PathSub->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                LocalStg.x264Path = fcgTXX264PathSub->Text;
                fcgTXX264PathSub->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                int c = fcgTXX264PathSub->SelectionStart;
                fcgTXX264Path->Text = LocalStg.x264Path;
                fcgTXX264Path->SelectionStart = fcgTXX264Path->Text->Length;
                fcgTXX264PathSub->SelectionStart = c;
                fcgBTX264PathSub->ContextMenuStrip = (File::Exists(fcgTXX264PathSub->Text)) ? fcgCSExeFiles : nullptr;
            }
        }
    private:
        System::Void fcgTXAudioEncoderPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgCXAudioEncoder->SelectedIndex < 0) return;
            if (fcgTXAudioEncoderPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                LocalStg.audEncPath[fcgCXAudioEncoder->SelectedIndex] = L"";
                fcgTXAudioEncoderPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                fcgTXAudioEncoderPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                LocalStg.audEncPath[fcgCXAudioEncoder->SelectedIndex] = fcgTXAudioEncoderPath->Text;
                fcgBTAudioEncoderPath->ContextMenuStrip = (File::Exists(fcgTXAudioEncoderPath->Text)) ? fcgCSExeFiles : nullptr;
            }
        }
    private:
        System::Void fcgTXMP4MuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMP4MuxerPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                LocalStg.MP4MuxerPath = L"";
                fcgTXMP4MuxerPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                fcgTXMP4MuxerPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                LocalStg.MP4MuxerPath = fcgTXMP4MuxerPath->Text;
                fcgBTMP4MuxerPath->ContextMenuStrip = (File::Exists(fcgTXMP4MuxerPath->Text)) ? fcgCSExeFiles : nullptr;
            }
        }
    private:
        System::Void fcgTXTC2MP4Path_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXTC2MP4Path->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                LocalStg.TC2MP4Path = L"";
                fcgTXTC2MP4Path->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                fcgTXTC2MP4Path->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                LocalStg.TC2MP4Path = fcgTXTC2MP4Path->Text;
                fcgBTTC2MP4Path->ContextMenuStrip = (File::Exists(fcgTXTC2MP4Path->Text)) ? fcgCSExeFiles : nullptr;
            }
        }
    private:
        System::Void fcgTXMP4RawMuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMP4RawPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                LocalStg.MP4RawPath = L"";
                fcgTXMP4RawPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                fcgTXMP4RawPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                LocalStg.MP4RawPath = fcgTXMP4RawPath->Text;
                fcgBTMP4RawPath->ContextMenuStrip = (File::Exists(fcgTXMP4RawPath->Text)) ? fcgCSExeFiles : nullptr;
            }
        }
    private:
        System::Void fcgTXMKVMuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMKVMuxerPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                LocalStg.MKVMuxerPath = L"";
                fcgTXMKVMuxerPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                fcgTXMKVMuxerPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                LocalStg.MKVMuxerPath = fcgTXMKVMuxerPath->Text;
                fcgBTMKVMuxerPath->ContextMenuStrip = (File::Exists(fcgTXMKVMuxerPath->Text)) ? fcgCSExeFiles : nullptr;
            }
        }
    private:
        System::Void fcgTXMP4BoxTempDir_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            LocalStg.CustomMP4TmpDir = fcgTXMP4BoxTempDir->Text;
        }
    private:
        System::Void fcgTXCustomAudioTempDir_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            LocalStg.CustomAudTmpDir = fcgTXCustomAudioTempDir->Text;
        }
    private:
        System::Void fcgTXCustomTempDir_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            LocalStg.CustomTmpDir = fcgTXCustomTempDir->Text;
        }
    private:
        System::Void fcgSetDragDropFilename_Enter(System::Object^  sender, DragEventArgs^  e) {
            e->Effect = (e->Data->GetDataPresent(DataFormats::FileDrop)) ? DragDropEffects::Copy : DragDropEffects::None;
        }
    private:
        System::Void fcgSetDragDropFilename_DragDrop(System::Object^  sender, DragEventArgs^  e) {
            TextBox^ TX = dynamic_cast<TextBox^>(sender);
            array<System::String ^>^ filelist = dynamic_cast<array<System::String ^>^>(e->Data->GetData(DataFormats::FileDrop, false));
            if (filelist == nullptr || TX == nullptr)
                return;
            String^ filePath = filelist[0]; //複数だった場合は先頭のものを使用
            if (sys_dat->exstg->s_local.get_relative_path)
                filePath = GetRelativePath(filePath);
            TX->Text = filePath;
        }
    private:
        System::Void fcgInsertDragDropFilename_Enter(System::Object^  sender, DragEventArgs^  e) {
            e->Effect = (   e->Data->GetDataPresent(DataFormats::FileDrop)
                         || e->Data->GetDataPresent(DataFormats::Text)
                         || e->Data->GetDataPresent(DataFormats::UnicodeText)
                         || e->Data->GetDataPresent(DataFormats::Rtf)
                         || e->Data->GetDataPresent(DataFormats::Html)) ? DragDropEffects::Copy : DragDropEffects::None;
        }
    private:
        System::Void fcgInsertDragDropFilename_DragOver(System::Object^  sender, DragEventArgs^  e) {
            TextBox^ TX = dynamic_cast<TextBox^>(sender);
            if (TX == nullptr)
                return;
            TX->Focus();
            this->Activate();
            //文字列がない部分の座標を指定した時に文字列の長さを返さない(一つ前の文字)になるため厄介
            //int index = TX->GetCharIndexFromPosition(TX->PointToClient(Point(e->X, e->Y)));
            //そこでEM_CHARFROMPOSメッセージを送る方法を使用する
            Point pint = TX->PointToClient(Point(e->X, e->Y));
            int p = ((pint.X & 0xffff) | (pint.Y << 16));
            int index = ::SendMessage((HWND)TX->Handle.ToPointer(), EM_CHARFROMPOS, 0, p) & 0xffff;
            TX->SelectionStart = index;
            //行・桁位置を求めるには以下の方法があるが、まあ今回は使わない
            //int line = TX->GetLineFromCharIndex(index);
            //int column = index - TX->GetFirstCharIndexFromLine(line);
        }
    private:
        System::Void fcgInsertDragDropFilename_DragDrop(System::Object^  sender, DragEventArgs^  e) {
            TextBox^ TX = dynamic_cast<TextBox^>(sender);
            if (TX == nullptr)
                return;
            String^ insertStr;
            if (e->Data->GetDataPresent(DataFormats::FileDrop)) {
                array<System::String ^>^ filelist = dynamic_cast<array<System::String ^>^>(e->Data->GetData(DataFormats::FileDrop, false));
                if (filelist == nullptr)
                    return;
                for (int i = 0; i < filelist->Length; i++) {
                    if (i) insertStr += L" ";
                    insertStr += L"\"" + ((sys_dat->exstg->s_local.get_relative_path) ? GetRelativePath(filelist[i]) : filelist[i]) + L"\"";
                }
            } else {
                insertStr = dynamic_cast<System::String^>(e->Data->GetData(DataFormats::Text));
                if (insertStr == nullptr)
                    return;
            }
            int current_selection = TX->SelectionStart;
            TX->Text = TX->Text->Insert(TX->SelectionStart, insertStr);
            TX->SelectionStart = current_selection + insertStr->Length; //たまに変なところへ行くので念のため必要
        }
    private:
        System::Void fcgTSExeFileshelp_Click(System::Object^  sender, System::EventArgs^  e) {
            System::Windows::Forms::ToolStripMenuItem^ TS = dynamic_cast<System::Windows::Forms::ToolStripMenuItem^>(sender);
            if (TS == nullptr) return;
            System::Windows::Forms::ContextMenuStrip^ CS = dynamic_cast<System::Windows::Forms::ContextMenuStrip^>(TS->Owner);
            if (CS == nullptr) return;

            //Name, args, Path の順番
            array<ExeControls>^ ControlList = {
                { fcgBTX264Path->Name,           fcgTXX264Path->Text,           sys_dat->exstg->s_enc.help_cmd },
                { fcgBTX264PathSub->Name,        fcgTXX264PathSub->Text,        sys_dat->exstg->s_enc.help_cmd },
                { fcgBTAudioEncoderPath->Name,   fcgTXAudioEncoderPath->Text,   sys_dat->exstg->s_aud_ext[fcgCXAudioEncoder->SelectedIndex].cmd_help },
                { fcgBTMP4MuxerPath->Name,       fcgTXMP4MuxerPath->Text,       sys_dat->exstg->s_mux[MUXER_MP4].help_cmd },
                { fcgBTTC2MP4Path->Name,         fcgTXTC2MP4Path->Text,         sys_dat->exstg->s_mux[MUXER_TC2MP4].help_cmd },
                { fcgBTMP4RawPath->Name,         fcgTXMP4RawPath->Text,         sys_dat->exstg->s_mux[MUXER_MP4_RAW].help_cmd },
                { fcgBTMKVMuxerPath->Name,       fcgTXMKVMuxerPath->Text,       sys_dat->exstg->s_mux[MUXER_MKV].help_cmd },
            };
            for (int i = 0; i < ControlList->Length; i++) {
                if (NULL == String::Compare(CS->SourceControl->Name, ControlList[i].Name)) {
                    ShowExehelp(ControlList[i].Path, String(ControlList[i].args).ToString());
                    return;
                }
            }
            MessageBox::Show(LOAD_CLI_STRING(AUO_CONFIG_HELP_CMD_UNSET), LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    private:
        System::Void fcgLBguiExBlog_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) {
            fcgLBguiExBlog->LinkVisited = true;
            try {
                System::Diagnostics::Process::Start(String(sys_dat->exstg->blog_url).ToString());
            } catch (...) {
                //まあ放置
            };
        }
    };
}
