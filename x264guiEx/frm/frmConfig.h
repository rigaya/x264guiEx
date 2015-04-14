//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx/2.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#pragma once

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

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::IO;


namespace x264guiEx {

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
			cnf_fcgTemp = (CONF_X264*)calloc(1, sizeof(CONF_X264));
			cnf_stgSelected = (CONF_GUIEX*)calloc(1, sizeof(CONF_GUIEX));
			InitializeComponent();
			//
			//TODO: ここにコンストラクタ コードを追加します
			//
			fcgLastX264ModeAsAMP = true;
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
			if (qualityTimer != nullptr)
				delete qualityTimer;
		}
private: System::Windows::Forms::Label^  fcgLBAMPAutoBitrate;


	private: System::Windows::Forms::TabControl^  fcgtabControlVideo;
	private: System::Windows::Forms::TabPage^  fcgtabPageX264Main;
	private: System::Windows::Forms::TabPage^  fcgtabPageX264RC;

	private: System::Windows::Forms::ToolStrip^  fcgtoolStripSettings;

	private: System::Windows::Forms::TabControl^  fcgtabControlMux;


	private: System::Windows::Forms::TabPage^  fcgtabPageMP4;
	private: System::Windows::Forms::TabPage^  fcgtabPageMKV;
	private: System::Windows::Forms::TextBox^  fcgTXCmd;

	private: System::Windows::Forms::Button^  fcgBTCancel;

	private: System::Windows::Forms::Button^  fcgBTOK;
	private: System::Windows::Forms::Button^  fcgBTDefault;



	private: System::Windows::Forms::TabPage^  fcgtabPageExSettings;
	private: System::Windows::Forms::ToolStripButton^  fcgTSBSave;

	private: System::Windows::Forms::ToolStripButton^  fcgTSBSaveNew;

	private: System::Windows::Forms::ToolStripButton^  fcgTSBDelete;

	private: System::Windows::Forms::ToolStripSeparator^  fcgtoolStripSeparator1;
	private: System::Windows::Forms::ToolStripDropDownButton^  fcgTSSettings;

	private: System::Windows::Forms::ToolStripButton^  fcgTSBCMDOnly;
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
	private: System::Windows::Forms::GroupBox^  fcggroupBoxAepectRatio;
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
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator2;
private: System::Windows::Forms::ToolStripButton^  fcgTSBOtherSettings;
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
private: System::Windows::Forms::ToolStripButton^  fcgTSBBitrateCalc;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator3;
private: System::Windows::Forms::Button^  fcgBTX264PathSub;

private: System::Windows::Forms::TextBox^  fcgTXX264PathSub;

private: System::Windows::Forms::Label^  fcgLBX264PathSub;
private: System::Windows::Forms::Label^  fcgLBX264PathSubhighbit;
private: System::Windows::Forms::Label^  fcgLBX264PathSub8bit;
private: System::Windows::Forms::Button^  fcgBTX264PathSubhighbit;
private: System::Windows::Forms::TextBox^  fcgTXX264PathSubhighbit;
private: System::Windows::Forms::ToolStripTextBox^  fcgTSTSettingsNotes;

private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator4;
private: System::Windows::Forms::ToolStripLabel^  fcgTSLSettingsNotes;
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
private: System::Windows::Forms::CheckBox^  fcgCBAMPLimitBitrate;


private: System::Windows::Forms::CheckBox^  fcgCBAMPLimitFileSize;


private: System::Windows::Forms::Panel^  fcgPNX264Mode;
private: System::Windows::Forms::Panel^  fcgPNBitrate;
private: System::Windows::Forms::NumericUpDown^  fcgNUAMPLimitBitrate;

private: System::Windows::Forms::NumericUpDown^  fcgNUAMPLimitFileSize;
private: System::Windows::Forms::Panel^  fcgPNStatusFile;
private: System::Windows::Forms::TabPage^  fcgtabPageMPG;
private: System::Windows::Forms::Button^  fcgBTMPGMuxerPath;

private: System::Windows::Forms::TextBox^  fcgTXMPGMuxerPath;

private: System::Windows::Forms::Label^  fcgLBMPGMuxerPath;

private: System::Windows::Forms::ComboBox^  fcgCXMPGCmdEx;
private: System::Windows::Forms::Label^  fcgLBMPGMuxerCmdEx;
private: System::Windows::Forms::CheckBox^  fcgCBMPGMuxerExt;
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
			this->fcgNUAMPLimitBitrate = (gcnew System::Windows::Forms::NumericUpDown());
			this->fcgNUAMPLimitFileSize = (gcnew System::Windows::Forms::NumericUpDown());
			this->fcgCXX264Mode = (gcnew System::Windows::Forms::ComboBox());
			this->fcgCBNulOut = (gcnew System::Windows::Forms::CheckBox());
			this->fcgCBAMPLimitBitrate = (gcnew System::Windows::Forms::CheckBox());
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
			this->fcggroupBoxAepectRatio = (gcnew System::Windows::Forms::GroupBox());
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
			this->fcgLBX264PathSubhighbit = (gcnew System::Windows::Forms::Label());
			this->fcgLBX264PathSub8bit = (gcnew System::Windows::Forms::Label());
			this->fcgBTX264PathSubhighbit = (gcnew System::Windows::Forms::Button());
			this->fcgTXX264PathSubhighbit = (gcnew System::Windows::Forms::TextBox());
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
			this->fcgtoolStripSettings = (gcnew System::Windows::Forms::ToolStrip());
			this->fcgTSBSave = (gcnew System::Windows::Forms::ToolStripButton());
			this->fcgTSBSaveNew = (gcnew System::Windows::Forms::ToolStripButton());
			this->fcgTSBDelete = (gcnew System::Windows::Forms::ToolStripButton());
			this->fcgtoolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->fcgTSSettings = (gcnew System::Windows::Forms::ToolStripDropDownButton());
			this->fcgTSBCMDOnly = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripSeparator3 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->fcgTSBBitrateCalc = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripSeparator2 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->fcgTSBOtherSettings = (gcnew System::Windows::Forms::ToolStripButton());
			this->fcgTSLSettingsNotes = (gcnew System::Windows::Forms::ToolStripLabel());
			this->fcgTSTSettingsNotes = (gcnew System::Windows::Forms::ToolStripTextBox());
			this->toolStripSeparator4 = (gcnew System::Windows::Forms::ToolStripSeparator());
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
			this->fcgtabPageMPG = (gcnew System::Windows::Forms::TabPage());
			this->fcgBTMPGMuxerPath = (gcnew System::Windows::Forms::Button());
			this->fcgTXMPGMuxerPath = (gcnew System::Windows::Forms::TextBox());
			this->fcgLBMPGMuxerPath = (gcnew System::Windows::Forms::Label());
			this->fcgCXMPGCmdEx = (gcnew System::Windows::Forms::ComboBox());
			this->fcgLBMPGMuxerCmdEx = (gcnew System::Windows::Forms::Label());
			this->fcgCBMPGMuxerExt = (gcnew System::Windows::Forms::CheckBox());
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
			this->fcgtabControlVideo->SuspendLayout();
			this->fcgtabPageX264Main->SuspendLayout();
			this->fcgPNStatusFile->SuspendLayout();
			this->fcgPNBitrate->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBQuality))->BeginInit();
			this->fcgPNX264Mode->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAMPLimitBitrate))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAMPLimitFileSize))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAutoNPass))->BeginInit();
			this->fcggroupBoxX264Out->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUSlices))->BeginInit();
			this->fcggroupBoxThreads->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNULookaheadThreads))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUThreads))->BeginInit();
			this->fcggroupBoxColorMatrix->SuspendLayout();
			this->fcggroupBoxAepectRatio->SuspendLayout();
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
			this->fcgtoolStripSettings->SuspendLayout();
			this->fcgtabControlMux->SuspendLayout();
			this->fcgtabPageMP4->SuspendLayout();
			this->fcgtabPageMKV->SuspendLayout();
			this->fcgtabPageMPG->SuspendLayout();
			this->fcgtabPageMux->SuspendLayout();
			this->fcgtabPageBat->SuspendLayout();
			this->fcgCSCQM->SuspendLayout();
			this->fcgtabControlAudio->SuspendLayout();
			this->fcgtabPageAudioMain->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAudioBitrate))->BeginInit();
			this->fcgtabPageAudioOther->SuspendLayout();
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
			this->fcgtabControlVideo->Location = System::Drawing::Point(0, 28);
			this->fcgtabControlVideo->Name = L"fcgtabControlVideo";
			this->fcgtabControlVideo->SelectedIndex = 0;
			this->fcgtabControlVideo->Size = System::Drawing::Size(693, 585);
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
			this->fcgtabPageX264Main->Controls->Add(this->fcggroupBoxAepectRatio);
			this->fcgtabPageX264Main->Controls->Add(this->fcgpictureBoxX264);
			this->fcgtabPageX264Main->Controls->Add(this->fcgBTX264Path);
			this->fcgtabPageX264Main->Controls->Add(this->fcgTXX264Path);
			this->fcgtabPageX264Main->Controls->Add(this->fcgLBX264Path);
			this->fcgtabPageX264Main->Controls->Add(this->fcggroupBoxPreset);
			this->fcgtabPageX264Main->Controls->Add(this->fcgLBOutputCF);
			this->fcgtabPageX264Main->Location = System::Drawing::Point(4, 24);
			this->fcgtabPageX264Main->Name = L"fcgtabPageX264Main";
			this->fcgtabPageX264Main->Padding = System::Windows::Forms::Padding(3);
			this->fcgtabPageX264Main->Size = System::Drawing::Size(685, 557);
			this->fcgtabPageX264Main->TabIndex = 0;
			this->fcgtabPageX264Main->Text = L" x264 ";
			this->fcgtabPageX264Main->UseVisualStyleBackColor = true;
			// 
			// fcgPNStatusFile
			// 
			this->fcgPNStatusFile->Controls->Add(this->fcgLBSTATUS);
			this->fcgPNStatusFile->Controls->Add(this->fcgTXStatusFile);
			this->fcgPNStatusFile->Controls->Add(this->fcgBTStatusFile);
			this->fcgPNStatusFile->Location = System::Drawing::Point(6, 305);
			this->fcgPNStatusFile->Name = L"fcgPNStatusFile";
			this->fcgPNStatusFile->Size = System::Drawing::Size(423, 36);
			this->fcgPNStatusFile->TabIndex = 49;
			// 
			// fcgLBSTATUS
			// 
			this->fcgLBSTATUS->AutoSize = true;
			this->fcgLBSTATUS->Location = System::Drawing::Point(-1, 11);
			this->fcgLBSTATUS->Name = L"fcgLBSTATUS";
			this->fcgLBSTATUS->Size = System::Drawing::Size(95, 17);
			this->fcgLBSTATUS->TabIndex = 19;
			this->fcgLBSTATUS->Text = L"ステータスファイル";
			// 
			// fcgTXStatusFile
			// 
			this->fcgTXStatusFile->Location = System::Drawing::Point(106, 8);
			this->fcgTXStatusFile->Name = L"fcgTXStatusFile";
			this->fcgTXStatusFile->Size = System::Drawing::Size(284, 23);
			this->fcgTXStatusFile->TabIndex = 12;
			this->fcgTXStatusFile->Tag = L"reCmd";
			// 
			// fcgBTStatusFile
			// 
			this->fcgBTStatusFile->Location = System::Drawing::Point(390, 7);
			this->fcgBTStatusFile->Name = L"fcgBTStatusFile";
			this->fcgBTStatusFile->Size = System::Drawing::Size(30, 25);
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
			this->fcgPNBitrate->Location = System::Drawing::Point(6, 215);
			this->fcgPNBitrate->Name = L"fcgPNBitrate";
			this->fcgPNBitrate->Size = System::Drawing::Size(423, 94);
			this->fcgPNBitrate->TabIndex = 48;
			// 
			// fcgLBQuality
			// 
			this->fcgLBQuality->AutoSize = true;
			this->fcgLBQuality->Location = System::Drawing::Point(3, 9);
			this->fcgLBQuality->Name = L"fcgLBQuality";
			this->fcgLBQuality->Size = System::Drawing::Size(105, 17);
			this->fcgLBQuality->TabIndex = 14;
			this->fcgLBQuality->Text = L"ビットレート(kbps)";
			// 
			// fcgLBQualityLeft
			// 
			this->fcgLBQualityLeft->AutoSize = true;
			this->fcgLBQualityLeft->Location = System::Drawing::Point(7, 70);
			this->fcgLBQualityLeft->Name = L"fcgLBQualityLeft";
			this->fcgLBQualityLeft->Size = System::Drawing::Size(47, 17);
			this->fcgLBQualityLeft->TabIndex = 12;
			this->fcgLBQualityLeft->Text = L"低画質";
			// 
			// fcgLBQualityRight
			// 
			this->fcgLBQualityRight->AutoSize = true;
			this->fcgLBQualityRight->Location = System::Drawing::Point(362, 70);
			this->fcgLBQualityRight->Name = L"fcgLBQualityRight";
			this->fcgLBQualityRight->Size = System::Drawing::Size(47, 17);
			this->fcgLBQualityRight->TabIndex = 13;
			this->fcgLBQualityRight->Text = L"高画質";
			// 
			// fcgTXQuality
			// 
			this->fcgTXQuality->Location = System::Drawing::Point(326, 9);
			this->fcgTXQuality->Name = L"fcgTXQuality";
			this->fcgTXQuality->Size = System::Drawing::Size(85, 23);
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
			this->fcgBTTBQualitySubtract->Location = System::Drawing::Point(0, 37);
			this->fcgBTTBQualitySubtract->Name = L"fcgBTTBQualitySubtract";
			this->fcgBTTBQualitySubtract->Size = System::Drawing::Size(24, 26);
			this->fcgBTTBQualitySubtract->TabIndex = 9;
			this->fcgBTTBQualitySubtract->UseVisualStyleBackColor = true;
			this->fcgBTTBQualitySubtract->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTTBQualitySubtract_Click);
			this->fcgBTTBQualitySubtract->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &frmConfig::fcgBTTBQualitySubtract_MouseDown);
			this->fcgBTTBQualitySubtract->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &frmConfig::fcgBTTBQualitySubtract_MouseUp);
			// 
			// fcgBTTBQualityAdd
			// 
			this->fcgBTTBQualityAdd->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgBTTBQualityAdd.Image")));
			this->fcgBTTBQualityAdd->Location = System::Drawing::Point(396, 38);
			this->fcgBTTBQualityAdd->Name = L"fcgBTTBQualityAdd";
			this->fcgBTTBQualityAdd->Size = System::Drawing::Size(24, 26);
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
			this->fcgTBQuality->Location = System::Drawing::Point(17, 39);
			this->fcgTBQuality->Maximum = 54000;
			this->fcgTBQuality->Name = L"fcgTBQuality";
			this->fcgTBQuality->Size = System::Drawing::Size(381, 28);
			this->fcgTBQuality->TabIndex = 10;
			this->fcgTBQuality->TickStyle = System::Windows::Forms::TickStyle::None;
			this->fcgTBQuality->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBQuality_Scroll);
			this->fcgTBQuality->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgTBQuality_ValueChanged);
			// 
			// fcgPNX264Mode
			// 
			this->fcgPNX264Mode->Controls->Add(this->fcgNUAMPLimitBitrate);
			this->fcgPNX264Mode->Controls->Add(this->fcgNUAMPLimitFileSize);
			this->fcgPNX264Mode->Controls->Add(this->fcgCXX264Mode);
			this->fcgPNX264Mode->Controls->Add(this->fcgCBNulOut);
			this->fcgPNX264Mode->Controls->Add(this->fcgCBAMPLimitBitrate);
			this->fcgPNX264Mode->Controls->Add(this->fcgCBFastFirstPass);
			this->fcgPNX264Mode->Controls->Add(this->fcgNUAutoNPass);
			this->fcgPNX264Mode->Controls->Add(this->fcgCBAMPLimitFileSize);
			this->fcgPNX264Mode->Controls->Add(this->fcgLBAutoNpass);
			this->fcgPNX264Mode->Location = System::Drawing::Point(6, 75);
			this->fcgPNX264Mode->Name = L"fcgPNX264Mode";
			this->fcgPNX264Mode->Size = System::Drawing::Size(423, 142);
			this->fcgPNX264Mode->TabIndex = 47;
			// 
			// fcgNUAMPLimitBitrate
			// 
			this->fcgNUAMPLimitBitrate->DecimalPlaces = 1;
			this->fcgNUAMPLimitBitrate->Location = System::Drawing::Point(326, 112);
			this->fcgNUAMPLimitBitrate->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 128000, 0, 0, 0 });
			this->fcgNUAMPLimitBitrate->Name = L"fcgNUAMPLimitBitrate";
			this->fcgNUAMPLimitBitrate->Size = System::Drawing::Size(88, 23);
			this->fcgNUAMPLimitBitrate->TabIndex = 47;
			this->fcgNUAMPLimitBitrate->Tag = L"chValue";
			this->fcgNUAMPLimitBitrate->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUAMPLimitFileSize
			// 
			this->fcgNUAMPLimitFileSize->DecimalPlaces = 1;
			this->fcgNUAMPLimitFileSize->Location = System::Drawing::Point(326, 84);
			this->fcgNUAMPLimitFileSize->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16384, 0, 0, 0 });
			this->fcgNUAMPLimitFileSize->Name = L"fcgNUAMPLimitFileSize";
			this->fcgNUAMPLimitFileSize->Size = System::Drawing::Size(88, 23);
			this->fcgNUAMPLimitFileSize->TabIndex = 46;
			this->fcgNUAMPLimitFileSize->Tag = L"chValue";
			this->fcgNUAMPLimitFileSize->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgCXX264Mode
			// 
			this->fcgCXX264Mode->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXX264Mode->FormattingEnabled = true;
			this->fcgCXX264Mode->Location = System::Drawing::Point(7, 8);
			this->fcgCXX264Mode->Name = L"fcgCXX264Mode";
			this->fcgCXX264Mode->Size = System::Drawing::Size(407, 23);
			this->fcgCXX264Mode->TabIndex = 4;
			this->fcgCXX264Mode->Tag = L"reCmd";
			this->fcgCXX264Mode->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXX264Mode_SelectedIndexChanged);
			// 
			// fcgCBNulOut
			// 
			this->fcgCBNulOut->AutoSize = true;
			this->fcgCBNulOut->Location = System::Drawing::Point(7, 51);
			this->fcgCBNulOut->Name = L"fcgCBNulOut";
			this->fcgCBNulOut->Size = System::Drawing::Size(72, 21);
			this->fcgCBNulOut->TabIndex = 5;
			this->fcgCBNulOut->Tag = L"reCmd";
			this->fcgCBNulOut->Text = L"nul出力";
			this->fcgCBNulOut->UseVisualStyleBackColor = true;
			this->fcgCBNulOut->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCBNulOut_CheckedChanged);
			// 
			// fcgCBAMPLimitBitrate
			// 
			this->fcgCBAMPLimitBitrate->AutoSize = true;
			this->fcgCBAMPLimitBitrate->Location = System::Drawing::Point(34, 114);
			this->fcgCBAMPLimitBitrate->Name = L"fcgCBAMPLimitBitrate";
			this->fcgCBAMPLimitBitrate->Size = System::Drawing::Size(258, 21);
			this->fcgCBAMPLimitBitrate->TabIndex = 45;
			this->fcgCBAMPLimitBitrate->Tag = L"chValue";
			this->fcgCBAMPLimitBitrate->Text = L"上限ファイルビットレート(映像+音声, kbps)";
			this->fcgCBAMPLimitBitrate->UseVisualStyleBackColor = true;
			// 
			// fcgCBFastFirstPass
			// 
			this->fcgCBFastFirstPass->AutoSize = true;
			this->fcgCBFastFirstPass->Location = System::Drawing::Point(97, 51);
			this->fcgCBFastFirstPass->Name = L"fcgCBFastFirstPass";
			this->fcgCBFastFirstPass->Size = System::Drawing::Size(116, 21);
			this->fcgCBFastFirstPass->TabIndex = 6;
			this->fcgCBFastFirstPass->Tag = L"reCmd";
			this->fcgCBFastFirstPass->Text = L"高速(1st pass)";
			this->fcgCBFastFirstPass->UseVisualStyleBackColor = true;
			this->fcgCBFastFirstPass->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCBSlowFirstPass_CheckedChanged);
			// 
			// fcgNUAutoNPass
			// 
			this->fcgNUAutoNPass->Location = System::Drawing::Point(352, 50);
			this->fcgNUAutoNPass->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2, 0, 0, 0 });
			this->fcgNUAutoNPass->Name = L"fcgNUAutoNPass";
			this->fcgNUAutoNPass->Size = System::Drawing::Size(62, 23);
			this->fcgNUAutoNPass->TabIndex = 7;
			this->fcgNUAutoNPass->Tag = L"reCmd";
			this->fcgNUAutoNPass->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->fcgNUAutoNPass->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2, 0, 0, 0 });
			// 
			// fcgCBAMPLimitFileSize
			// 
			this->fcgCBAMPLimitFileSize->AutoSize = true;
			this->fcgCBAMPLimitFileSize->Location = System::Drawing::Point(34, 86);
			this->fcgCBAMPLimitFileSize->Name = L"fcgCBAMPLimitFileSize";
			this->fcgCBAMPLimitFileSize->Size = System::Drawing::Size(224, 21);
			this->fcgCBAMPLimitFileSize->TabIndex = 43;
			this->fcgCBAMPLimitFileSize->Tag = L"chValue";
			this->fcgCBAMPLimitFileSize->Text = L"上限ファイルサイズ(映像+音声, MB)";
			this->fcgCBAMPLimitFileSize->UseVisualStyleBackColor = true;
			// 
			// fcgLBAutoNpass
			// 
			this->fcgLBAutoNpass->AutoSize = true;
			this->fcgLBAutoNpass->Location = System::Drawing::Point(245, 52);
			this->fcgLBAutoNpass->Name = L"fcgLBAutoNpass";
			this->fcgLBAutoNpass->Size = System::Drawing::Size(99, 17);
			this->fcgLBAutoNpass->TabIndex = 34;
			this->fcgLBAutoNpass->Text = L"自動マルチパス数";
			// 
			// fcgCXNalHrd
			// 
			this->fcgCXNalHrd->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXNalHrd->FormattingEnabled = true;
			this->fcgCXNalHrd->Location = System::Drawing::Point(557, 284);
			this->fcgCXNalHrd->Name = L"fcgCXNalHrd";
			this->fcgCXNalHrd->Size = System::Drawing::Size(106, 23);
			this->fcgCXNalHrd->TabIndex = 26;
			this->fcgCXNalHrd->Tag = L"reCmd";
			// 
			// fcgLBNalHrd
			// 
			this->fcgLBNalHrd->AutoSize = true;
			this->fcgLBNalHrd->Location = System::Drawing::Point(487, 287);
			this->fcgLBNalHrd->Name = L"fcgLBNalHrd";
			this->fcgLBNalHrd->Size = System::Drawing::Size(54, 17);
			this->fcgLBNalHrd->TabIndex = 42;
			this->fcgLBNalHrd->Text = L"nal-hrd";
			// 
			// fcgCBPicStruct
			// 
			this->fcgCBPicStruct->AutoSize = true;
			this->fcgCBPicStruct->Location = System::Drawing::Point(471, 241);
			this->fcgCBPicStruct->Name = L"fcgCBPicStruct";
			this->fcgCBPicStruct->Size = System::Drawing::Size(87, 21);
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
			this->fcggroupBoxX264Out->Location = System::Drawing::Point(220, 462);
			this->fcggroupBoxX264Out->Name = L"fcggroupBoxX264Out";
			this->fcggroupBoxX264Out->Size = System::Drawing::Size(199, 87);
			this->fcggroupBoxX264Out->TabIndex = 16;
			this->fcggroupBoxX264Out->TabStop = false;
			// 
			// fcgCBSSIM
			// 
			this->fcgCBSSIM->AutoSize = true;
			this->fcgCBSSIM->Location = System::Drawing::Point(119, 57);
			this->fcgCBSSIM->Name = L"fcgCBSSIM";
			this->fcgCBSSIM->Size = System::Drawing::Size(59, 21);
			this->fcgCBSSIM->TabIndex = 2;
			this->fcgCBSSIM->Tag = L"reCmd";
			this->fcgCBSSIM->Text = L"SSIM";
			this->fcgCBSSIM->UseVisualStyleBackColor = true;
			// 
			// fcgCBPSNR
			// 
			this->fcgCBPSNR->AutoSize = true;
			this->fcgCBPSNR->Location = System::Drawing::Point(19, 57);
			this->fcgCBPSNR->Name = L"fcgCBPSNR";
			this->fcgCBPSNR->Size = System::Drawing::Size(62, 21);
			this->fcgCBPSNR->TabIndex = 1;
			this->fcgCBPSNR->Tag = L"reCmd";
			this->fcgCBPSNR->Text = L"PSNR";
			this->fcgCBPSNR->UseVisualStyleBackColor = true;
			// 
			// fcgCXLogLevel
			// 
			this->fcgCXLogLevel->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXLogLevel->FormattingEnabled = true;
			this->fcgCXLogLevel->Location = System::Drawing::Point(79, 20);
			this->fcgCXLogLevel->Name = L"fcgCXLogLevel";
			this->fcgCXLogLevel->Size = System::Drawing::Size(104, 23);
			this->fcgCXLogLevel->TabIndex = 0;
			this->fcgCXLogLevel->Tag = L"reCmd";
			// 
			// fcgLBLog
			// 
			this->fcgLBLog->AutoSize = true;
			this->fcgLBLog->Location = System::Drawing::Point(16, 24);
			this->fcgLBLog->Name = L"fcgLBLog";
			this->fcgLBLog->Size = System::Drawing::Size(53, 17);
			this->fcgLBLog->TabIndex = 0;
			this->fcgLBLog->Text = L"ログ表示";
			// 
			// fcgNUSlices
			// 
			this->fcgNUSlices->Location = System::Drawing::Point(601, 317);
			this->fcgNUSlices->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
			this->fcgNUSlices->Name = L"fcgNUSlices";
			this->fcgNUSlices->Size = System::Drawing::Size(61, 23);
			this->fcgNUSlices->TabIndex = 27;
			this->fcgNUSlices->Tag = L"reCmd";
			this->fcgNUSlices->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgLBSlices
			// 
			this->fcgLBSlices->AutoSize = true;
			this->fcgLBSlices->Location = System::Drawing::Point(485, 320);
			this->fcgLBSlices->Name = L"fcgLBSlices";
			this->fcgLBSlices->Size = System::Drawing::Size(59, 17);
			this->fcgLBSlices->TabIndex = 39;
			this->fcgLBSlices->Text = L"スライス数";
			// 
			// fcgCXOutputCsp
			// 
			this->fcgCXOutputCsp->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXOutputCsp->FormattingEnabled = true;
			this->fcgCXOutputCsp->Location = System::Drawing::Point(556, 351);
			this->fcgCXOutputCsp->Name = L"fcgCXOutputCsp";
			this->fcgCXOutputCsp->Size = System::Drawing::Size(106, 23);
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
			this->fcggroupBoxThreads->Location = System::Drawing::Point(220, 343);
			this->fcggroupBoxThreads->Name = L"fcggroupBoxThreads";
			this->fcggroupBoxThreads->Size = System::Drawing::Size(199, 119);
			this->fcggroupBoxThreads->TabIndex = 15;
			this->fcggroupBoxThreads->TabStop = false;
			// 
			// fcgLBLookaheadThreads
			// 
			this->fcgLBLookaheadThreads->AutoSize = true;
			this->fcgLBLookaheadThreads->Location = System::Drawing::Point(18, 57);
			this->fcgLBLookaheadThreads->Name = L"fcgLBLookaheadThreads";
			this->fcgLBLookaheadThreads->Size = System::Drawing::Size(79, 17);
			this->fcgLBLookaheadThreads->TabIndex = 31;
			this->fcgLBLookaheadThreads->Text = L"サブスレッド数";
			// 
			// fcgNULookaheadThreads
			// 
			this->fcgNULookaheadThreads->Location = System::Drawing::Point(107, 55);
			this->fcgNULookaheadThreads->Name = L"fcgNULookaheadThreads";
			this->fcgNULookaheadThreads->Size = System::Drawing::Size(72, 23);
			this->fcgNULookaheadThreads->TabIndex = 1;
			this->fcgNULookaheadThreads->Tag = L"reCmd";
			this->fcgNULookaheadThreads->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgCBSlicedThreads
			// 
			this->fcgCBSlicedThreads->AutoSize = true;
			this->fcgCBSlicedThreads->Location = System::Drawing::Point(19, 89);
			this->fcgCBSlicedThreads->Name = L"fcgCBSlicedThreads";
			this->fcgCBSlicedThreads->Size = System::Drawing::Size(165, 21);
			this->fcgCBSlicedThreads->TabIndex = 2;
			this->fcgCBSlicedThreads->Tag = L"reCmd";
			this->fcgCBSlicedThreads->Text = L"スライスベースマルチスレッド";
			this->fcgCBSlicedThreads->UseVisualStyleBackColor = true;
			// 
			// fcgLBThreads
			// 
			this->fcgLBThreads->AutoSize = true;
			this->fcgLBThreads->Location = System::Drawing::Point(37, 24);
			this->fcgLBThreads->Name = L"fcgLBThreads";
			this->fcgLBThreads->Size = System::Drawing::Size(58, 17);
			this->fcgLBThreads->TabIndex = 29;
			this->fcgLBThreads->Text = L"スレッド数";
			// 
			// fcgNUThreads
			// 
			this->fcgNUThreads->Location = System::Drawing::Point(107, 21);
			this->fcgNUThreads->Name = L"fcgNUThreads";
			this->fcgNUThreads->Size = System::Drawing::Size(72, 23);
			this->fcgNUThreads->TabIndex = 0;
			this->fcgNUThreads->Tag = L"reCmd";
			this->fcgNUThreads->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgCBUsehighbit
			// 
			this->fcgCBUsehighbit->AutoSize = true;
			this->fcgCBUsehighbit->Location = System::Drawing::Point(312, 11);
			this->fcgCBUsehighbit->Name = L"fcgCBUsehighbit";
			this->fcgCBUsehighbit->Size = System::Drawing::Size(111, 21);
			this->fcgCBUsehighbit->TabIndex = 1;
			this->fcgCBUsehighbit->Tag = L"reCmd";
			this->fcgCBUsehighbit->Text = L"highbit depth";
			this->fcgCBUsehighbit->UseVisualStyleBackColor = true;
			this->fcgCBUsehighbit->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCBUsehighbit_CheckedChanged);
			// 
			// fcgCBBlurayCompat
			// 
			this->fcgCBBlurayCompat->AutoSize = true;
			this->fcgCBBlurayCompat->Location = System::Drawing::Point(567, 206);
			this->fcgCBBlurayCompat->Name = L"fcgCBBlurayCompat";
			this->fcgCBBlurayCompat->Size = System::Drawing::Size(98, 21);
			this->fcgCBBlurayCompat->TabIndex = 24;
			this->fcgCBBlurayCompat->Tag = L"reCmd";
			this->fcgCBBlurayCompat->Text = L"blu-ray互換";
			this->fcgCBBlurayCompat->UseVisualStyleBackColor = true;
			// 
			// fcgCBAud
			// 
			this->fcgCBAud->AutoSize = true;
			this->fcgCBAud->Location = System::Drawing::Point(471, 206);
			this->fcgCBAud->Name = L"fcgCBAud";
			this->fcgCBAud->Size = System::Drawing::Size(77, 21);
			this->fcgCBAud->TabIndex = 23;
			this->fcgCBAud->Tag = L"reCmd";
			this->fcgCBAud->Text = L"aud付加";
			this->fcgCBAud->UseVisualStyleBackColor = true;
			// 
			// fcgCXVideoFormat
			// 
			this->fcgCXVideoFormat->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXVideoFormat->FormattingEnabled = true;
			this->fcgCXVideoFormat->Location = System::Drawing::Point(555, 168);
			this->fcgCXVideoFormat->Name = L"fcgCXVideoFormat";
			this->fcgCXVideoFormat->Size = System::Drawing::Size(106, 23);
			this->fcgCXVideoFormat->TabIndex = 22;
			this->fcgCXVideoFormat->Tag = L"reCmd";
			// 
			// fcgLBVideoFormat
			// 
			this->fcgLBVideoFormat->AutoSize = true;
			this->fcgLBVideoFormat->Location = System::Drawing::Point(462, 171);
			this->fcgLBVideoFormat->Name = L"fcgLBVideoFormat";
			this->fcgLBVideoFormat->Size = System::Drawing::Size(84, 17);
			this->fcgLBVideoFormat->TabIndex = 24;
			this->fcgLBVideoFormat->Text = L"videoformat";
			// 
			// fcgLBLevel
			// 
			this->fcgLBLevel->AutoSize = true;
			this->fcgLBLevel->Location = System::Drawing::Point(465, 135);
			this->fcgLBLevel->Name = L"fcgLBLevel";
			this->fcgLBLevel->Size = System::Drawing::Size(84, 17);
			this->fcgLBLevel->TabIndex = 23;
			this->fcgLBLevel->Text = L"H.264 Level";
			// 
			// fcgCXLevel
			// 
			this->fcgCXLevel->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXLevel->FormattingEnabled = true;
			this->fcgCXLevel->Location = System::Drawing::Point(555, 132);
			this->fcgCXLevel->Name = L"fcgCXLevel";
			this->fcgCXLevel->Size = System::Drawing::Size(106, 23);
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
			this->fcggroupBoxColorMatrix->Location = System::Drawing::Point(441, 386);
			this->fcggroupBoxColorMatrix->Name = L"fcggroupBoxColorMatrix";
			this->fcggroupBoxColorMatrix->Size = System::Drawing::Size(235, 163);
			this->fcggroupBoxColorMatrix->TabIndex = 30;
			this->fcggroupBoxColorMatrix->TabStop = false;
			this->fcggroupBoxColorMatrix->Text = L"色空間";
			// 
			// fcgCXInputRange
			// 
			this->fcgCXInputRange->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXInputRange->FormattingEnabled = true;
			this->fcgCXInputRange->Location = System::Drawing::Point(118, 128);
			this->fcgCXInputRange->Name = L"fcgCXInputRange";
			this->fcgCXInputRange->Size = System::Drawing::Size(103, 23);
			this->fcgCXInputRange->TabIndex = 4;
			this->fcgCXInputRange->Tag = L"reCmd";
			// 
			// fcgLBInputRange
			// 
			this->fcgLBInputRange->AutoSize = true;
			this->fcgLBInputRange->Location = System::Drawing::Point(20, 132);
			this->fcgLBInputRange->Name = L"fcgLBInputRange";
			this->fcgLBInputRange->Size = System::Drawing::Size(81, 17);
			this->fcgLBInputRange->TabIndex = 3;
			this->fcgLBInputRange->Text = L"input range";
			// 
			// fcgCXTransfer
			// 
			this->fcgCXTransfer->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXTransfer->FormattingEnabled = true;
			this->fcgCXTransfer->Location = System::Drawing::Point(118, 93);
			this->fcgCXTransfer->Name = L"fcgCXTransfer";
			this->fcgCXTransfer->Size = System::Drawing::Size(103, 23);
			this->fcgCXTransfer->TabIndex = 2;
			this->fcgCXTransfer->Tag = L"reCmd";
			// 
			// fcgCXColorPrim
			// 
			this->fcgCXColorPrim->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXColorPrim->FormattingEnabled = true;
			this->fcgCXColorPrim->Location = System::Drawing::Point(118, 57);
			this->fcgCXColorPrim->Name = L"fcgCXColorPrim";
			this->fcgCXColorPrim->Size = System::Drawing::Size(103, 23);
			this->fcgCXColorPrim->TabIndex = 1;
			this->fcgCXColorPrim->Tag = L"reCmd";
			// 
			// fcgCXColorMatrix
			// 
			this->fcgCXColorMatrix->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXColorMatrix->FormattingEnabled = true;
			this->fcgCXColorMatrix->Location = System::Drawing::Point(118, 22);
			this->fcgCXColorMatrix->Name = L"fcgCXColorMatrix";
			this->fcgCXColorMatrix->Size = System::Drawing::Size(103, 23);
			this->fcgCXColorMatrix->TabIndex = 0;
			this->fcgCXColorMatrix->Tag = L"reCmd";
			// 
			// fcgLBTransfer
			// 
			this->fcgLBTransfer->AutoSize = true;
			this->fcgLBTransfer->Location = System::Drawing::Point(20, 97);
			this->fcgLBTransfer->Name = L"fcgLBTransfer";
			this->fcgLBTransfer->Size = System::Drawing::Size(57, 17);
			this->fcgLBTransfer->TabIndex = 2;
			this->fcgLBTransfer->Text = L"transfer";
			// 
			// fcgLBColorPrim
			// 
			this->fcgLBColorPrim->AutoSize = true;
			this->fcgLBColorPrim->Location = System::Drawing::Point(20, 61);
			this->fcgLBColorPrim->Name = L"fcgLBColorPrim";
			this->fcgLBColorPrim->Size = System::Drawing::Size(67, 17);
			this->fcgLBColorPrim->TabIndex = 1;
			this->fcgLBColorPrim->Text = L"colorprim";
			// 
			// fcgLBColorMatrix
			// 
			this->fcgLBColorMatrix->AutoSize = true;
			this->fcgLBColorMatrix->Location = System::Drawing::Point(20, 26);
			this->fcgLBColorMatrix->Name = L"fcgLBColorMatrix";
			this->fcgLBColorMatrix->Size = System::Drawing::Size(79, 17);
			this->fcgLBColorMatrix->TabIndex = 0;
			this->fcgLBColorMatrix->Text = L"colormatrix";
			// 
			// fcggroupBoxAepectRatio
			// 
			this->fcggroupBoxAepectRatio->Controls->Add(this->fcgNUAspectRatioY);
			this->fcggroupBoxAepectRatio->Controls->Add(this->fcgLBAspectRatio);
			this->fcggroupBoxAepectRatio->Controls->Add(this->fcgNUAspectRatioX);
			this->fcggroupBoxAepectRatio->Controls->Add(this->fcgCXAspectRatio);
			this->fcggroupBoxAepectRatio->Location = System::Drawing::Point(446, 7);
			this->fcggroupBoxAepectRatio->Name = L"fcggroupBoxAepectRatio";
			this->fcggroupBoxAepectRatio->Size = System::Drawing::Size(231, 106);
			this->fcggroupBoxAepectRatio->TabIndex = 20;
			this->fcggroupBoxAepectRatio->TabStop = false;
			this->fcggroupBoxAepectRatio->Text = L"アスペクト比";
			// 
			// fcgNUAspectRatioY
			// 
			this->fcgNUAspectRatioY->Location = System::Drawing::Point(142, 64);
			this->fcgNUAspectRatioY->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000, 0, 0, 0 });
			this->fcgNUAspectRatioY->Name = L"fcgNUAspectRatioY";
			this->fcgNUAspectRatioY->Size = System::Drawing::Size(68, 23);
			this->fcgNUAspectRatioY->TabIndex = 2;
			this->fcgNUAspectRatioY->Tag = L"reCmd";
			this->fcgNUAspectRatioY->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgLBAspectRatio
			// 
			this->fcgLBAspectRatio->AutoSize = true;
			this->fcgLBAspectRatio->Location = System::Drawing::Point(122, 66);
			this->fcgLBAspectRatio->Name = L"fcgLBAspectRatio";
			this->fcgLBAspectRatio->Size = System::Drawing::Size(14, 17);
			this->fcgLBAspectRatio->TabIndex = 2;
			this->fcgLBAspectRatio->Text = L":";
			// 
			// fcgNUAspectRatioX
			// 
			this->fcgNUAspectRatioX->Location = System::Drawing::Point(47, 64);
			this->fcgNUAspectRatioX->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000, 0, 0, 0 });
			this->fcgNUAspectRatioX->Name = L"fcgNUAspectRatioX";
			this->fcgNUAspectRatioX->Size = System::Drawing::Size(68, 23);
			this->fcgNUAspectRatioX->TabIndex = 1;
			this->fcgNUAspectRatioX->Tag = L"reCmd";
			this->fcgNUAspectRatioX->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgCXAspectRatio
			// 
			this->fcgCXAspectRatio->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXAspectRatio->FormattingEnabled = true;
			this->fcgCXAspectRatio->Location = System::Drawing::Point(17, 33);
			this->fcgCXAspectRatio->Name = L"fcgCXAspectRatio";
			this->fcgCXAspectRatio->Size = System::Drawing::Size(192, 23);
			this->fcgCXAspectRatio->TabIndex = 0;
			this->fcgCXAspectRatio->Tag = L"reCmd";
			// 
			// fcgpictureBoxX264
			// 
			this->fcgpictureBoxX264->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgpictureBoxX264.Image")));
			this->fcgpictureBoxX264->Location = System::Drawing::Point(4, 14);
			this->fcgpictureBoxX264->Name = L"fcgpictureBoxX264";
			this->fcgpictureBoxX264->Size = System::Drawing::Size(124, 50);
			this->fcgpictureBoxX264->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->fcgpictureBoxX264->TabIndex = 16;
			this->fcgpictureBoxX264->TabStop = false;
			// 
			// fcgBTX264Path
			// 
			this->fcgBTX264Path->Location = System::Drawing::Point(398, 38);
			this->fcgBTX264Path->Name = L"fcgBTX264Path";
			this->fcgBTX264Path->Size = System::Drawing::Size(30, 25);
			this->fcgBTX264Path->TabIndex = 3;
			this->fcgBTX264Path->Text = L"...";
			this->fcgBTX264Path->UseVisualStyleBackColor = true;
			this->fcgBTX264Path->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTX264Path_Click);
			// 
			// fcgTXX264Path
			// 
			this->fcgTXX264Path->AllowDrop = true;
			this->fcgTXX264Path->Location = System::Drawing::Point(134, 39);
			this->fcgTXX264Path->Name = L"fcgTXX264Path";
			this->fcgTXX264Path->Size = System::Drawing::Size(264, 23);
			this->fcgTXX264Path->TabIndex = 2;
			this->fcgTXX264Path->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXX264Path_TextChanged);
			this->fcgTXX264Path->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
			this->fcgTXX264Path->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
			this->fcgTXX264Path->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXX264Path_Leave);
			// 
			// fcgLBX264Path
			// 
			this->fcgLBX264Path->AutoSize = true;
			this->fcgLBX264Path->Location = System::Drawing::Point(144, 14);
			this->fcgLBX264Path->Name = L"fcgLBX264Path";
			this->fcgLBX264Path->Size = System::Drawing::Size(58, 17);
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
			this->fcggroupBoxPreset->Location = System::Drawing::Point(8, 343);
			this->fcggroupBoxPreset->Name = L"fcggroupBoxPreset";
			this->fcggroupBoxPreset->Size = System::Drawing::Size(205, 206);
			this->fcggroupBoxPreset->TabIndex = 14;
			this->fcggroupBoxPreset->TabStop = false;
			this->fcggroupBoxPreset->Text = L"プリセットのロード";
			// 
			// fcgBTApplyPreset
			// 
			this->fcgBTApplyPreset->Location = System::Drawing::Point(82, 155);
			this->fcgBTApplyPreset->Name = L"fcgBTApplyPreset";
			this->fcgBTApplyPreset->Size = System::Drawing::Size(106, 36);
			this->fcgBTApplyPreset->TabIndex = 3;
			this->fcgBTApplyPreset->Text = L"GUIにロード";
			this->fcgBTApplyPreset->UseVisualStyleBackColor = true;
			this->fcgBTApplyPreset->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTApplyPreset_Click);
			// 
			// fcgCXProfile
			// 
			this->fcgCXProfile->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXProfile->FormattingEnabled = true;
			this->fcgCXProfile->Location = System::Drawing::Point(82, 110);
			this->fcgCXProfile->Name = L"fcgCXProfile";
			this->fcgCXProfile->Size = System::Drawing::Size(105, 23);
			this->fcgCXProfile->TabIndex = 2;
			this->fcgCXProfile->Tag = L"reCmd";
			// 
			// fcgCXTune
			// 
			this->fcgCXTune->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXTune->FormattingEnabled = true;
			this->fcgCXTune->Location = System::Drawing::Point(82, 70);
			this->fcgCXTune->Name = L"fcgCXTune";
			this->fcgCXTune->Size = System::Drawing::Size(105, 23);
			this->fcgCXTune->TabIndex = 1;
			this->fcgCXTune->Tag = L"reCmd";
			// 
			// fcgCXPreset
			// 
			this->fcgCXPreset->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXPreset->FormattingEnabled = true;
			this->fcgCXPreset->Location = System::Drawing::Point(82, 29);
			this->fcgCXPreset->Name = L"fcgCXPreset";
			this->fcgCXPreset->Size = System::Drawing::Size(105, 23);
			this->fcgCXPreset->TabIndex = 0;
			this->fcgCXPreset->Tag = L"reCmd";
			// 
			// fcgLBProfile
			// 
			this->fcgLBProfile->AutoSize = true;
			this->fcgLBProfile->Location = System::Drawing::Point(10, 114);
			this->fcgLBProfile->Name = L"fcgLBProfile";
			this->fcgLBProfile->Size = System::Drawing::Size(64, 17);
			this->fcgLBProfile->TabIndex = 2;
			this->fcgLBProfile->Text = L"プロファイル";
			// 
			// fcgLBX264TUNE
			// 
			this->fcgLBX264TUNE->AutoSize = true;
			this->fcgLBX264TUNE->Location = System::Drawing::Point(10, 73);
			this->fcgLBX264TUNE->Name = L"fcgLBX264TUNE";
			this->fcgLBX264TUNE->Size = System::Drawing::Size(68, 17);
			this->fcgLBX264TUNE->TabIndex = 1;
			this->fcgLBX264TUNE->Text = L"チューニング";
			// 
			// fcgLBX264Preset
			// 
			this->fcgLBX264Preset->AutoSize = true;
			this->fcgLBX264Preset->Location = System::Drawing::Point(10, 33);
			this->fcgLBX264Preset->Name = L"fcgLBX264Preset";
			this->fcgLBX264Preset->Size = System::Drawing::Size(34, 17);
			this->fcgLBX264Preset->TabIndex = 0;
			this->fcgLBX264Preset->Text = L"速度";
			// 
			// fcgLBOutputCF
			// 
			this->fcgLBOutputCF->AutoSize = true;
			this->fcgLBOutputCF->Location = System::Drawing::Point(446, 354);
			this->fcgLBOutputCF->Name = L"fcgLBOutputCF";
			this->fcgLBOutputCF->Size = System::Drawing::Size(103, 17);
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
			this->fcgtabPageX264RC->Location = System::Drawing::Point(4, 24);
			this->fcgtabPageX264RC->Name = L"fcgtabPageX264RC";
			this->fcgtabPageX264RC->Padding = System::Windows::Forms::Padding(3);
			this->fcgtabPageX264RC->Size = System::Drawing::Size(685, 557);
			this->fcgtabPageX264RC->TabIndex = 1;
			this->fcgtabPageX264RC->Text = L" レート・QP制御";
			this->fcgtabPageX264RC->UseVisualStyleBackColor = true;
			// 
			// fcgLBTimebase
			// 
			this->fcgLBTimebase->AutoSize = true;
			this->fcgLBTimebase->Location = System::Drawing::Point(264, 508);
			this->fcgLBTimebase->Name = L"fcgLBTimebase";
			this->fcgLBTimebase->Size = System::Drawing::Size(14, 17);
			this->fcgLBTimebase->TabIndex = 17;
			this->fcgLBTimebase->Text = L"/";
			// 
			// fcgNUTimebaseDen
			// 
			this->fcgNUTimebaseDen->Location = System::Drawing::Point(285, 506);
			this->fcgNUTimebaseDen->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2000000000, 0, 0, 0 });
			this->fcgNUTimebaseDen->Name = L"fcgNUTimebaseDen";
			this->fcgNUTimebaseDen->Size = System::Drawing::Size(135, 23);
			this->fcgNUTimebaseDen->TabIndex = 11;
			this->fcgNUTimebaseDen->Tag = L"reCmd";
			this->fcgNUTimebaseDen->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUTimebaseNum
			// 
			this->fcgNUTimebaseNum->Location = System::Drawing::Point(152, 506);
			this->fcgNUTimebaseNum->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2000000000, 0, 0, 0 });
			this->fcgNUTimebaseNum->Name = L"fcgNUTimebaseNum";
			this->fcgNUTimebaseNum->Size = System::Drawing::Size(106, 23);
			this->fcgNUTimebaseNum->TabIndex = 10;
			this->fcgNUTimebaseNum->Tag = L"reCmd";
			this->fcgNUTimebaseNum->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgBTTCIN
			// 
			this->fcgBTTCIN->Location = System::Drawing::Point(407, 471);
			this->fcgBTTCIN->Name = L"fcgBTTCIN";
			this->fcgBTTCIN->Size = System::Drawing::Size(38, 26);
			this->fcgBTTCIN->TabIndex = 8;
			this->fcgBTTCIN->Text = L"...";
			this->fcgBTTCIN->UseVisualStyleBackColor = true;
			this->fcgBTTCIN->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTTCIN_Click);
			// 
			// fcgTXTCIN
			// 
			this->fcgTXTCIN->Location = System::Drawing::Point(152, 472);
			this->fcgTXTCIN->Name = L"fcgTXTCIN";
			this->fcgTXTCIN->Size = System::Drawing::Size(248, 23);
			this->fcgTXTCIN->TabIndex = 7;
			this->fcgTXTCIN->Tag = L"reCmd";
			// 
			// fcgCBTimeBase
			// 
			this->fcgCBTimeBase->AutoSize = true;
			this->fcgCBTimeBase->Location = System::Drawing::Point(27, 507);
			this->fcgCBTimeBase->Name = L"fcgCBTimeBase";
			this->fcgCBTimeBase->Size = System::Drawing::Size(105, 21);
			this->fcgCBTimeBase->TabIndex = 9;
			this->fcgCBTimeBase->Tag = L"reCmd";
			this->fcgCBTimeBase->Text = L"時間精度指定";
			this->fcgCBTimeBase->UseVisualStyleBackColor = true;
			this->fcgCBTimeBase->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgChangeEnabled);
			// 
			// fcgCBTCIN
			// 
			this->fcgCBTCIN->AutoSize = true;
			this->fcgCBTCIN->Location = System::Drawing::Point(27, 475);
			this->fcgCBTCIN->Name = L"fcgCBTCIN";
			this->fcgCBTCIN->Size = System::Drawing::Size(111, 21);
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
			this->fcggroupBoxAQ->Location = System::Drawing::Point(9, 290);
			this->fcggroupBoxAQ->Name = L"fcggroupBoxAQ";
			this->fcggroupBoxAQ->Size = System::Drawing::Size(271, 114);
			this->fcggroupBoxAQ->TabIndex = 4;
			this->fcggroupBoxAQ->TabStop = false;
			this->fcggroupBoxAQ->Text = L"AQ (適応的QP)";
			// 
			// fcgNUAQStrength
			// 
			this->fcgNUAQStrength->DecimalPlaces = 2;
			this->fcgNUAQStrength->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 131072 });
			this->fcgNUAQStrength->Location = System::Drawing::Point(135, 75);
			this->fcgNUAQStrength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
			this->fcgNUAQStrength->Name = L"fcgNUAQStrength";
			this->fcgNUAQStrength->Size = System::Drawing::Size(84, 23);
			this->fcgNUAQStrength->TabIndex = 1;
			this->fcgNUAQStrength->Tag = L"reCmd";
			this->fcgNUAQStrength->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgLBAQStrength
			// 
			this->fcgLBAQStrength->AutoSize = true;
			this->fcgLBAQStrength->Location = System::Drawing::Point(65, 78);
			this->fcgLBAQStrength->Name = L"fcgLBAQStrength";
			this->fcgLBAQStrength->Size = System::Drawing::Size(49, 17);
			this->fcgLBAQStrength->TabIndex = 2;
			this->fcgLBAQStrength->Text = L"AQ強さ";
			// 
			// fcgCXAQMode
			// 
			this->fcgCXAQMode->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXAQMode->FormattingEnabled = true;
			this->fcgCXAQMode->Location = System::Drawing::Point(135, 32);
			this->fcgCXAQMode->Name = L"fcgCXAQMode";
			this->fcgCXAQMode->Size = System::Drawing::Size(84, 23);
			this->fcgCXAQMode->TabIndex = 0;
			this->fcgCXAQMode->Tag = L"reCmd";
			// 
			// fcgLBAQMode
			// 
			this->fcgLBAQMode->AutoSize = true;
			this->fcgLBAQMode->Location = System::Drawing::Point(65, 35);
			this->fcgLBAQMode->Name = L"fcgLBAQMode";
			this->fcgLBAQMode->Size = System::Drawing::Size(38, 17);
			this->fcgLBAQMode->TabIndex = 0;
			this->fcgLBAQMode->Text = L"モード";
			// 
			// fcggroupBoxPsyRd
			// 
			this->fcggroupBoxPsyRd->Controls->Add(this->fcgNUPsyTrellis);
			this->fcggroupBoxPsyRd->Controls->Add(this->fcgNUPsyRDO);
			this->fcggroupBoxPsyRd->Controls->Add(this->fcgLBPsyTrellis);
			this->fcggroupBoxPsyRd->Controls->Add(this->fcgLBPsyRDO);
			this->fcggroupBoxPsyRd->Location = System::Drawing::Point(320, 290);
			this->fcggroupBoxPsyRd->Name = L"fcggroupBoxPsyRd";
			this->fcggroupBoxPsyRd->Size = System::Drawing::Size(261, 114);
			this->fcggroupBoxPsyRd->TabIndex = 5;
			this->fcggroupBoxPsyRd->TabStop = false;
			this->fcggroupBoxPsyRd->Text = L"Psy-Rd (視覚心理最適化)";
			// 
			// fcgNUPsyTrellis
			// 
			this->fcgNUPsyTrellis->DecimalPlaces = 2;
			this->fcgNUPsyTrellis->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 131072 });
			this->fcgNUPsyTrellis->Location = System::Drawing::Point(136, 72);
			this->fcgNUPsyTrellis->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
			this->fcgNUPsyTrellis->Name = L"fcgNUPsyTrellis";
			this->fcgNUPsyTrellis->Size = System::Drawing::Size(84, 23);
			this->fcgNUPsyTrellis->TabIndex = 1;
			this->fcgNUPsyTrellis->Tag = L"reCmd";
			this->fcgNUPsyTrellis->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUPsyRDO
			// 
			this->fcgNUPsyRDO->DecimalPlaces = 2;
			this->fcgNUPsyRDO->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 131072 });
			this->fcgNUPsyRDO->Location = System::Drawing::Point(136, 32);
			this->fcgNUPsyRDO->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
			this->fcgNUPsyRDO->Name = L"fcgNUPsyRDO";
			this->fcgNUPsyRDO->Size = System::Drawing::Size(84, 23);
			this->fcgNUPsyRDO->TabIndex = 0;
			this->fcgNUPsyRDO->Tag = L"reCmd";
			this->fcgNUPsyRDO->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgLBPsyTrellis
			// 
			this->fcgLBPsyTrellis->AutoSize = true;
			this->fcgLBPsyTrellis->Location = System::Drawing::Point(53, 75);
			this->fcgLBPsyTrellis->Name = L"fcgLBPsyTrellis";
			this->fcgLBPsyTrellis->Size = System::Drawing::Size(41, 17);
			this->fcgLBPsyTrellis->TabIndex = 1;
			this->fcgLBPsyTrellis->Text = L"trellis";
			// 
			// fcgLBPsyRDO
			// 
			this->fcgLBPsyRDO->AutoSize = true;
			this->fcgLBPsyRDO->Location = System::Drawing::Point(58, 35);
			this->fcgLBPsyRDO->Name = L"fcgLBPsyRDO";
			this->fcgLBPsyRDO->Size = System::Drawing::Size(37, 17);
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
			this->fcggroupBoxVbv->Location = System::Drawing::Point(320, 128);
			this->fcggroupBoxVbv->Name = L"fcggroupBoxVbv";
			this->fcggroupBoxVbv->Size = System::Drawing::Size(261, 147);
			this->fcggroupBoxVbv->TabIndex = 3;
			this->fcggroupBoxVbv->TabStop = false;
			this->fcggroupBoxVbv->Text = L"VBV (ビデオバッファ制御, \"-1\"で自動)";
			// 
			// fcgLBVBVafsWarning
			// 
			this->fcgLBVBVafsWarning->ForeColor = System::Drawing::Color::OrangeRed;
			this->fcgLBVBVafsWarning->Location = System::Drawing::Point(28, 102);
			this->fcgLBVBVafsWarning->Name = L"fcgLBVBVafsWarning";
			this->fcgLBVBVafsWarning->Size = System::Drawing::Size(174, 32);
			this->fcgLBVBVafsWarning->TabIndex = 4;
			this->fcgLBVBVafsWarning->Text = L"※afs使用時はvbv設定は正しく反映されません。";
			// 
			// fcgNUVBVbuf
			// 
			this->fcgNUVBVbuf->Location = System::Drawing::Point(136, 64);
			this->fcgNUVBVbuf->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000000, 0, 0, 0 });
			this->fcgNUVBVbuf->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, System::Int32::MinValue });
			this->fcgNUVBVbuf->Name = L"fcgNUVBVbuf";
			this->fcgNUVBVbuf->Size = System::Drawing::Size(84, 23);
			this->fcgNUVBVbuf->TabIndex = 1;
			this->fcgNUVBVbuf->Tag = L"reCmd";
			this->fcgNUVBVbuf->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUVBVmax
			// 
			this->fcgNUVBVmax->Location = System::Drawing::Point(136, 30);
			this->fcgNUVBVmax->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000000, 0, 0, 0 });
			this->fcgNUVBVmax->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, System::Int32::MinValue });
			this->fcgNUVBVmax->Name = L"fcgNUVBVmax";
			this->fcgNUVBVmax->Size = System::Drawing::Size(84, 23);
			this->fcgNUVBVmax->TabIndex = 0;
			this->fcgNUVBVmax->Tag = L"reCmd";
			this->fcgNUVBVmax->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgLBVBVbuf
			// 
			this->fcgLBVBVbuf->AutoSize = true;
			this->fcgLBVBVbuf->Location = System::Drawing::Point(28, 66);
			this->fcgLBVBVbuf->Name = L"fcgLBVBVbuf";
			this->fcgLBVBVbuf->Size = System::Drawing::Size(75, 17);
			this->fcgLBVBVbuf->TabIndex = 1;
			this->fcgLBVBVbuf->Text = L"バッファサイズ";
			// 
			// fcgLBVBVmax
			// 
			this->fcgLBVBVmax->AutoSize = true;
			this->fcgLBVBVmax->Location = System::Drawing::Point(11, 33);
			this->fcgLBVBVmax->Name = L"fcgLBVBVmax";
			this->fcgLBVBVmax->Size = System::Drawing::Size(90, 17);
			this->fcgLBVBVmax->TabIndex = 0;
			this->fcgLBVBVmax->Text = L"最大ビットレート";
			// 
			// fcgNURCLookahead
			// 
			this->fcgNURCLookahead->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
			this->fcgNURCLookahead->Location = System::Drawing::Point(403, 86);
			this->fcgNURCLookahead->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 250, 0, 0, 0 });
			this->fcgNURCLookahead->Name = L"fcgNURCLookahead";
			this->fcgNURCLookahead->Size = System::Drawing::Size(80, 23);
			this->fcgNURCLookahead->TabIndex = 2;
			this->fcgNURCLookahead->Tag = L"reCmd";
			this->fcgNURCLookahead->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgLBRCLookahead
			// 
			this->fcgLBRCLookahead->AutoSize = true;
			this->fcgLBRCLookahead->Location = System::Drawing::Point(348, 60);
			this->fcgLBRCLookahead->Name = L"fcgLBRCLookahead";
			this->fcgLBRCLookahead->Size = System::Drawing::Size(170, 17);
			this->fcgLBRCLookahead->TabIndex = 2;
			this->fcgLBRCLookahead->Text = L"レート制御先行探査フレーム数";
			// 
			// fcgCBMBTree
			// 
			this->fcgCBMBTree->AutoSize = true;
			this->fcgCBMBTree->Location = System::Drawing::Point(374, 21);
			this->fcgCBMBTree->Name = L"fcgCBMBTree";
			this->fcgCBMBTree->Size = System::Drawing::Size(133, 21);
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
			this->fcggroupBoxQP->Location = System::Drawing::Point(9, 7);
			this->fcggroupBoxQP->Name = L"fcggroupBoxQP";
			this->fcggroupBoxQP->Size = System::Drawing::Size(271, 269);
			this->fcggroupBoxQP->TabIndex = 0;
			this->fcggroupBoxQP->TabStop = false;
			this->fcggroupBoxQP->Text = L"量子化(QP)";
			// 
			// fcgNUChromaQp
			// 
			this->fcgNUChromaQp->Location = System::Drawing::Point(186, 231);
			this->fcgNUChromaQp->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 81, 0, 0, 0 });
			this->fcgNUChromaQp->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 81, 0, 0, System::Int32::MinValue });
			this->fcgNUChromaQp->Name = L"fcgNUChromaQp";
			this->fcgNUChromaQp->Size = System::Drawing::Size(62, 23);
			this->fcgNUChromaQp->TabIndex = 6;
			this->fcgNUChromaQp->Tag = L"reCmd";
			this->fcgNUChromaQp->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUQpstep
			// 
			this->fcgNUQpstep->Location = System::Drawing::Point(186, 197);
			this->fcgNUQpstep->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 81, 0, 0, 0 });
			this->fcgNUQpstep->Name = L"fcgNUQpstep";
			this->fcgNUQpstep->Size = System::Drawing::Size(62, 23);
			this->fcgNUQpstep->TabIndex = 5;
			this->fcgNUQpstep->Tag = L"reCmd";
			this->fcgNUQpstep->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUQpmax
			// 
			this->fcgNUQpmax->Location = System::Drawing::Point(186, 163);
			this->fcgNUQpmax->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 69, 0, 0, 0 });
			this->fcgNUQpmax->Name = L"fcgNUQpmax";
			this->fcgNUQpmax->Size = System::Drawing::Size(62, 23);
			this->fcgNUQpmax->TabIndex = 4;
			this->fcgNUQpmax->Tag = L"reCmd";
			this->fcgNUQpmax->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUQpmin
			// 
			this->fcgNUQpmin->Location = System::Drawing::Point(186, 129);
			this->fcgNUQpmin->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 81, 0, 0, 0 });
			this->fcgNUQpmin->Name = L"fcgNUQpmin";
			this->fcgNUQpmin->Size = System::Drawing::Size(62, 23);
			this->fcgNUQpmin->TabIndex = 3;
			this->fcgNUQpmin->Tag = L"reCmd";
			this->fcgNUQpmin->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUQcomp
			// 
			this->fcgNUQcomp->Location = System::Drawing::Point(186, 96);
			this->fcgNUQcomp->Name = L"fcgNUQcomp";
			this->fcgNUQcomp->Size = System::Drawing::Size(62, 23);
			this->fcgNUQcomp->TabIndex = 2;
			this->fcgNUQcomp->Tag = L"reCmd";
			this->fcgNUQcomp->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUPBRatio
			// 
			this->fcgNUPBRatio->Location = System::Drawing::Point(186, 62);
			this->fcgNUPBRatio->Name = L"fcgNUPBRatio";
			this->fcgNUPBRatio->Size = System::Drawing::Size(62, 23);
			this->fcgNUPBRatio->TabIndex = 1;
			this->fcgNUPBRatio->Tag = L"reCmd";
			this->fcgNUPBRatio->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUIPRatio
			// 
			this->fcgNUIPRatio->Location = System::Drawing::Point(186, 28);
			this->fcgNUIPRatio->Name = L"fcgNUIPRatio";
			this->fcgNUIPRatio->Size = System::Drawing::Size(62, 23);
			this->fcgNUIPRatio->TabIndex = 0;
			this->fcgNUIPRatio->Tag = L"reCmd";
			this->fcgNUIPRatio->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgLBQpstep
			// 
			this->fcgLBQpstep->AutoSize = true;
			this->fcgLBQpstep->Location = System::Drawing::Point(15, 199);
			this->fcgLBQpstep->Name = L"fcgLBQpstep";
			this->fcgLBQpstep->Size = System::Drawing::Size(91, 17);
			this->fcgLBQpstep->TabIndex = 6;
			this->fcgLBQpstep->Text = L"最大QP変動幅";
			// 
			// fcgLBChromaQp
			// 
			this->fcgLBChromaQp->AutoSize = true;
			this->fcgLBChromaQp->Location = System::Drawing::Point(15, 233);
			this->fcgLBChromaQp->Name = L"fcgLBChromaQp";
			this->fcgLBChromaQp->Size = System::Drawing::Size(131, 17);
			this->fcgLBChromaQp->TabIndex = 5;
			this->fcgLBChromaQp->Text = L"クロマ(色差)QP補正量";
			// 
			// fcgLBQpmax
			// 
			this->fcgLBQpmax->AutoSize = true;
			this->fcgLBQpmax->Location = System::Drawing::Point(15, 165);
			this->fcgLBQpmax->Name = L"fcgLBQpmax";
			this->fcgLBQpmax->Size = System::Drawing::Size(65, 17);
			this->fcgLBQpmax->TabIndex = 4;
			this->fcgLBQpmax->Text = L"最大QP値";
			// 
			// fcgLBQpmin
			// 
			this->fcgLBQpmin->AutoSize = true;
			this->fcgLBQpmin->Location = System::Drawing::Point(15, 132);
			this->fcgLBQpmin->Name = L"fcgLBQpmin";
			this->fcgLBQpmin->Size = System::Drawing::Size(65, 17);
			this->fcgLBQpmin->TabIndex = 3;
			this->fcgLBQpmin->Text = L"最小QP値";
			// 
			// fcgLBQcomp
			// 
			this->fcgLBQcomp->AutoSize = true;
			this->fcgLBQcomp->Location = System::Drawing::Point(15, 98);
			this->fcgLBQcomp->Name = L"fcgLBQcomp";
			this->fcgLBQcomp->Size = System::Drawing::Size(132, 17);
			this->fcgLBQcomp->TabIndex = 2;
			this->fcgLBQcomp->Text = L"ビットレート変動量 (%)";
			// 
			// fcgLBPBRatio
			// 
			this->fcgLBPBRatio->AutoSize = true;
			this->fcgLBPBRatio->Location = System::Drawing::Point(15, 64);
			this->fcgLBPBRatio->Name = L"fcgLBPBRatio";
			this->fcgLBPBRatio->Size = System::Drawing::Size(140, 17);
			this->fcgLBPBRatio->TabIndex = 1;
			this->fcgLBPBRatio->Text = L"P-Bフレーム間係数 (%)";
			// 
			// fcgLBIPRatio
			// 
			this->fcgLBIPRatio->AutoSize = true;
			this->fcgLBIPRatio->Location = System::Drawing::Point(15, 30);
			this->fcgLBIPRatio->Name = L"fcgLBIPRatio";
			this->fcgLBIPRatio->Size = System::Drawing::Size(136, 17);
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
			this->fcgtabPageX264Frame->Location = System::Drawing::Point(4, 24);
			this->fcgtabPageX264Frame->Name = L"fcgtabPageX264Frame";
			this->fcgtabPageX264Frame->Size = System::Drawing::Size(685, 557);
			this->fcgtabPageX264Frame->TabIndex = 4;
			this->fcgtabPageX264Frame->Text = L"フレーム";
			this->fcgtabPageX264Frame->UseVisualStyleBackColor = true;
			// 
			// fcgCBDeblock
			// 
			this->fcgCBDeblock->AutoSize = true;
			this->fcgCBDeblock->Location = System::Drawing::Point(435, 162);
			this->fcgCBDeblock->Name = L"fcgCBDeblock";
			this->fcgCBDeblock->Size = System::Drawing::Size(165, 21);
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
			this->fcgCXInterlaced->Location = System::Drawing::Point(542, 276);
			this->fcgCXInterlaced->Name = L"fcgCXInterlaced";
			this->fcgCXInterlaced->Size = System::Drawing::Size(126, 23);
			this->fcgCXInterlaced->TabIndex = 9;
			this->fcgCXInterlaced->Tag = L"reCmd";
			// 
			// fcgLBInterlaced
			// 
			this->fcgLBInterlaced->AutoSize = true;
			this->fcgLBInterlaced->Location = System::Drawing::Point(420, 279);
			this->fcgLBInterlaced->Name = L"fcgLBInterlaced";
			this->fcgLBInterlaced->Size = System::Drawing::Size(104, 17);
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
			this->fcggroupBoxX264Other->Location = System::Drawing::Point(315, 414);
			this->fcggroupBoxX264Other->Name = L"fcggroupBoxX264Other";
			this->fcggroupBoxX264Other->Size = System::Drawing::Size(360, 111);
			this->fcggroupBoxX264Other->TabIndex = 14;
			this->fcggroupBoxX264Other->TabStop = false;
			this->fcggroupBoxX264Other->Text = L"その他";
			// 
			// fcgBTMatrix
			// 
			this->fcgBTMatrix->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 6, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(128)));
			this->fcgBTMatrix->Location = System::Drawing::Point(299, 64);
			this->fcgBTMatrix->Name = L"fcgBTMatrix";
			this->fcgBTMatrix->Size = System::Drawing::Size(20, 26);
			this->fcgBTMatrix->TabIndex = 2;
			this->fcgBTMatrix->Text = L"▼";
			this->fcgBTMatrix->UseVisualStyleBackColor = true;
			this->fcgBTMatrix->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMatrix_Click);
			// 
			// fcgTXCQM
			// 
			this->fcgTXCQM->Location = System::Drawing::Point(125, 65);
			this->fcgTXCQM->Name = L"fcgTXCQM";
			this->fcgTXCQM->Size = System::Drawing::Size(167, 23);
			this->fcgTXCQM->TabIndex = 1;
			this->fcgTXCQM->Tag = L"reCmd";
			// 
			// fcgCXTrellis
			// 
			this->fcgCXTrellis->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXTrellis->FormattingEnabled = true;
			this->fcgCXTrellis->Location = System::Drawing::Point(125, 28);
			this->fcgCXTrellis->Name = L"fcgCXTrellis";
			this->fcgCXTrellis->Size = System::Drawing::Size(193, 23);
			this->fcgCXTrellis->TabIndex = 0;
			this->fcgCXTrellis->Tag = L"reCmd";
			// 
			// fcgLBCQM
			// 
			this->fcgLBCQM->AutoSize = true;
			this->fcgLBCQM->Location = System::Drawing::Point(11, 69);
			this->fcgLBCQM->Name = L"fcgLBCQM";
			this->fcgLBCQM->Size = System::Drawing::Size(101, 17);
			this->fcgLBCQM->TabIndex = 3;
			this->fcgLBCQM->Text = L"量子化マトリックス";
			// 
			// fcgLBTrellis
			// 
			this->fcgLBTrellis->AutoSize = true;
			this->fcgLBTrellis->Location = System::Drawing::Point(11, 32);
			this->fcgLBTrellis->Name = L"fcgLBTrellis";
			this->fcgLBTrellis->Size = System::Drawing::Size(102, 17);
			this->fcgLBTrellis->TabIndex = 2;
			this->fcgLBTrellis->Text = L"レート歪み最適化";
			// 
			// fcgCBDctDecimate
			// 
			this->fcgCBDctDecimate->AutoSize = true;
			this->fcgCBDctDecimate->Location = System::Drawing::Point(423, 387);
			this->fcgCBDctDecimate->Name = L"fcgCBDctDecimate";
			this->fcgCBDctDecimate->Size = System::Drawing::Size(116, 21);
			this->fcgCBDctDecimate->TabIndex = 12;
			this->fcgCBDctDecimate->Tag = L"reCmd";
			this->fcgCBDctDecimate->Text = L"DCT係数間引き";
			this->fcgCBDctDecimate->UseVisualStyleBackColor = true;
			// 
			// fcgCBfastpskip
			// 
			this->fcgCBfastpskip->AutoSize = true;
			this->fcgCBfastpskip->Location = System::Drawing::Point(562, 387);
			this->fcgCBfastpskip->Name = L"fcgCBfastpskip";
			this->fcgCBfastpskip->Size = System::Drawing::Size(112, 21);
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
			this->fcggroupBoxME->Location = System::Drawing::Point(6, 4);
			this->fcggroupBoxME->Name = L"fcggroupBoxME";
			this->fcggroupBoxME->Size = System::Drawing::Size(405, 204);
			this->fcggroupBoxME->TabIndex = 0;
			this->fcggroupBoxME->TabStop = false;
			this->fcggroupBoxME->Text = L"動き予測";
			// 
			// fcgCBMixedRef
			// 
			this->fcgCBMixedRef->AutoSize = true;
			this->fcgCBMixedRef->Location = System::Drawing::Point(308, 165);
			this->fcgCBMixedRef->Name = L"fcgCBMixedRef";
			this->fcgCBMixedRef->Size = System::Drawing::Size(15, 14);
			this->fcgCBMixedRef->TabIndex = 6;
			this->fcgCBMixedRef->Tag = L"reCmd";
			this->fcgCBMixedRef->UseVisualStyleBackColor = true;
			// 
			// fcgCBChromaME
			// 
			this->fcgCBChromaME->AutoSize = true;
			this->fcgCBChromaME->Location = System::Drawing::Point(330, 94);
			this->fcgCBChromaME->Name = L"fcgCBChromaME";
			this->fcgCBChromaME->Size = System::Drawing::Size(15, 14);
			this->fcgCBChromaME->TabIndex = 3;
			this->fcgCBChromaME->Tag = L"reCmd";
			this->fcgCBChromaME->UseVisualStyleBackColor = true;
			// 
			// fcgNURef
			// 
			this->fcgNURef->Location = System::Drawing::Point(148, 161);
			this->fcgNURef->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
			this->fcgNURef->Name = L"fcgNURef";
			this->fcgNURef->Size = System::Drawing::Size(73, 23);
			this->fcgNURef->TabIndex = 5;
			this->fcgNURef->Tag = L"reCmd";
			this->fcgNURef->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUMERange
			// 
			this->fcgNUMERange->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
			this->fcgNUMERange->Location = System::Drawing::Point(148, 91);
			this->fcgNUMERange->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 64, 0, 0, 0 });
			this->fcgNUMERange->Name = L"fcgNUMERange";
			this->fcgNUMERange->Size = System::Drawing::Size(73, 23);
			this->fcgNUMERange->TabIndex = 2;
			this->fcgNUMERange->Tag = L"reCmd";
			this->fcgNUMERange->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgCXDirectME
			// 
			this->fcgCXDirectME->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXDirectME->FormattingEnabled = true;
			this->fcgCXDirectME->Location = System::Drawing::Point(148, 125);
			this->fcgCXDirectME->Name = L"fcgCXDirectME";
			this->fcgCXDirectME->Size = System::Drawing::Size(214, 23);
			this->fcgCXDirectME->TabIndex = 4;
			this->fcgCXDirectME->Tag = L"reCmd";
			// 
			// fcgCXSubME
			// 
			this->fcgCXSubME->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXSubME->FormattingEnabled = true;
			this->fcgCXSubME->Location = System::Drawing::Point(148, 55);
			this->fcgCXSubME->Name = L"fcgCXSubME";
			this->fcgCXSubME->Size = System::Drawing::Size(246, 23);
			this->fcgCXSubME->TabIndex = 1;
			this->fcgCXSubME->Tag = L"reCmd";
			// 
			// fcgCXME
			// 
			this->fcgCXME->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXME->FormattingEnabled = true;
			this->fcgCXME->Location = System::Drawing::Point(148, 20);
			this->fcgCXME->Name = L"fcgCXME";
			this->fcgCXME->Size = System::Drawing::Size(246, 23);
			this->fcgCXME->TabIndex = 0;
			this->fcgCXME->Tag = L"reCmd";
			// 
			// fcgLBMixedRef
			// 
			this->fcgLBMixedRef->AutoSize = true;
			this->fcgLBMixedRef->Location = System::Drawing::Point(244, 163);
			this->fcgLBMixedRef->Name = L"fcgLBMixedRef";
			this->fcgLBMixedRef->Size = System::Drawing::Size(60, 17);
			this->fcgLBMixedRef->TabIndex = 6;
			this->fcgLBMixedRef->Text = L"混合参照";
			// 
			// fcgLBChromaME
			// 
			this->fcgLBChromaME->AutoSize = true;
			this->fcgLBChromaME->Location = System::Drawing::Point(244, 93);
			this->fcgLBChromaME->Name = L"fcgLBChromaME";
			this->fcgLBChromaME->Size = System::Drawing::Size(83, 17);
			this->fcgLBChromaME->TabIndex = 5;
			this->fcgLBChromaME->Text = L"色差動き予測";
			// 
			// fcgLBRef
			// 
			this->fcgLBRef->AutoSize = true;
			this->fcgLBRef->Location = System::Drawing::Point(17, 163);
			this->fcgLBRef->Name = L"fcgLBRef";
			this->fcgLBRef->Size = System::Drawing::Size(60, 17);
			this->fcgLBRef->TabIndex = 4;
			this->fcgLBRef->Text = L"参照距離";
			// 
			// fcgLBDirectME
			// 
			this->fcgLBDirectME->AutoSize = true;
			this->fcgLBDirectME->Location = System::Drawing::Point(17, 128);
			this->fcgLBDirectME->Name = L"fcgLBDirectME";
			this->fcgLBDirectME->Size = System::Drawing::Size(83, 17);
			this->fcgLBDirectME->TabIndex = 3;
			this->fcgLBDirectME->Text = L"動き予測方式";
			// 
			// fcgLBMERange
			// 
			this->fcgLBMERange->AutoSize = true;
			this->fcgLBMERange->Location = System::Drawing::Point(17, 93);
			this->fcgLBMERange->Name = L"fcgLBMERange";
			this->fcgLBMERange->Size = System::Drawing::Size(83, 17);
			this->fcgLBMERange->TabIndex = 2;
			this->fcgLBMERange->Text = L"動き探索範囲";
			// 
			// fcgLBSubME
			// 
			this->fcgLBSubME->AutoSize = true;
			this->fcgLBSubME->Location = System::Drawing::Point(17, 58);
			this->fcgLBSubME->Name = L"fcgLBSubME";
			this->fcgLBSubME->Size = System::Drawing::Size(119, 17);
			this->fcgLBSubME->TabIndex = 1;
			this->fcgLBSubME->Text = L"サブピクセル動き予測";
			// 
			// fcgLBME
			// 
			this->fcgLBME->AutoSize = true;
			this->fcgLBME->Location = System::Drawing::Point(17, 24);
			this->fcgLBME->Name = L"fcgLBME";
			this->fcgLBME->Size = System::Drawing::Size(118, 17);
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
			this->fcggroupBoxMBTypes->Location = System::Drawing::Point(6, 414);
			this->fcggroupBoxMBTypes->Name = L"fcggroupBoxMBTypes";
			this->fcggroupBoxMBTypes->Size = System::Drawing::Size(274, 111);
			this->fcggroupBoxMBTypes->TabIndex = 2;
			this->fcggroupBoxMBTypes->TabStop = false;
			this->fcggroupBoxMBTypes->Text = L"マクロブロック";
			// 
			// fcgCBi4x4
			// 
			this->fcgCBi4x4->AutoSize = true;
			this->fcgCBi4x4->Location = System::Drawing::Point(107, 80);
			this->fcgCBi4x4->Name = L"fcgCBi4x4";
			this->fcgCBi4x4->Size = System::Drawing::Size(53, 21);
			this->fcgCBi4x4->TabIndex = 5;
			this->fcgCBi4x4->Tag = L"reCmd";
			this->fcgCBi4x4->Text = L"i4x4";
			this->fcgCBi4x4->UseVisualStyleBackColor = true;
			// 
			// fcgCBp4x4
			// 
			this->fcgCBp4x4->AutoSize = true;
			this->fcgCBp4x4->Location = System::Drawing::Point(197, 52);
			this->fcgCBp4x4->Name = L"fcgCBp4x4";
			this->fcgCBp4x4->Size = System::Drawing::Size(58, 21);
			this->fcgCBp4x4->TabIndex = 3;
			this->fcgCBp4x4->Tag = L"reCmd";
			this->fcgCBp4x4->Text = L"p4x4";
			this->fcgCBp4x4->UseVisualStyleBackColor = true;
			// 
			// fcgCBi8x8
			// 
			this->fcgCBi8x8->AutoSize = true;
			this->fcgCBi8x8->Location = System::Drawing::Point(15, 80);
			this->fcgCBi8x8->Name = L"fcgCBi8x8";
			this->fcgCBi8x8->Size = System::Drawing::Size(53, 21);
			this->fcgCBi8x8->TabIndex = 4;
			this->fcgCBi8x8->Tag = L"reCmd";
			this->fcgCBi8x8->Text = L"i8x8";
			this->fcgCBi8x8->UseVisualStyleBackColor = true;
			// 
			// fcgCBb8x8
			// 
			this->fcgCBb8x8->AutoSize = true;
			this->fcgCBb8x8->Location = System::Drawing::Point(107, 52);
			this->fcgCBb8x8->Name = L"fcgCBb8x8";
			this->fcgCBb8x8->Size = System::Drawing::Size(58, 21);
			this->fcgCBb8x8->TabIndex = 2;
			this->fcgCBb8x8->Tag = L"reCmd";
			this->fcgCBb8x8->Text = L"b8x8";
			this->fcgCBb8x8->UseVisualStyleBackColor = true;
			// 
			// fcgCBp8x8
			// 
			this->fcgCBp8x8->AutoSize = true;
			this->fcgCBp8x8->Location = System::Drawing::Point(15, 52);
			this->fcgCBp8x8->Name = L"fcgCBp8x8";
			this->fcgCBp8x8->Size = System::Drawing::Size(58, 21);
			this->fcgCBp8x8->TabIndex = 1;
			this->fcgCBp8x8->Tag = L"reCmd";
			this->fcgCBp8x8->Text = L"p8x8";
			this->fcgCBp8x8->UseVisualStyleBackColor = true;
			// 
			// fcgCB8x8dct
			// 
			this->fcgCB8x8dct->AutoSize = true;
			this->fcgCB8x8dct->Location = System::Drawing::Point(15, 22);
			this->fcgCB8x8dct->Name = L"fcgCB8x8dct";
			this->fcgCB8x8dct->Size = System::Drawing::Size(145, 21);
			this->fcgCB8x8dct->TabIndex = 0;
			this->fcgCB8x8dct->Tag = L"reCmd";
			this->fcgCB8x8dct->Text = L"8x8 離散コサイン変換";
			this->fcgCB8x8dct->UseVisualStyleBackColor = true;
			// 
			// fcgCXWeightP
			// 
			this->fcgCXWeightP->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXWeightP->FormattingEnabled = true;
			this->fcgCXWeightP->Location = System::Drawing::Point(542, 309);
			this->fcgCXWeightP->Name = L"fcgCXWeightP";
			this->fcgCXWeightP->Size = System::Drawing::Size(126, 23);
			this->fcgCXWeightP->TabIndex = 10;
			this->fcgCXWeightP->Tag = L"reCmd";
			// 
			// fcgLBWeightP
			// 
			this->fcgLBWeightP->AutoSize = true;
			this->fcgLBWeightP->Location = System::Drawing::Point(420, 313);
			this->fcgLBWeightP->Name = L"fcgLBWeightP";
			this->fcgLBWeightP->Size = System::Drawing::Size(105, 17);
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
			this->fcggroupBoxBframes->Location = System::Drawing::Point(6, 215);
			this->fcggroupBoxBframes->Name = L"fcggroupBoxBframes";
			this->fcggroupBoxBframes->Size = System::Drawing::Size(274, 192);
			this->fcggroupBoxBframes->TabIndex = 1;
			this->fcggroupBoxBframes->TabStop = false;
			this->fcggroupBoxBframes->Text = L"Bフレーム";
			// 
			// fcgCXBpyramid
			// 
			this->fcgCXBpyramid->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXBpyramid->FormattingEnabled = true;
			this->fcgCXBpyramid->Location = System::Drawing::Point(174, 127);
			this->fcgCXBpyramid->Name = L"fcgCXBpyramid";
			this->fcgCXBpyramid->Size = System::Drawing::Size(79, 23);
			this->fcgCXBpyramid->TabIndex = 3;
			this->fcgCXBpyramid->Tag = L"reCmd";
			// 
			// fcgCXBAdpapt
			// 
			this->fcgCXBAdpapt->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXBAdpapt->FormattingEnabled = true;
			this->fcgCXBAdpapt->Location = System::Drawing::Point(174, 61);
			this->fcgCXBAdpapt->Name = L"fcgCXBAdpapt";
			this->fcgCXBAdpapt->Size = System::Drawing::Size(79, 23);
			this->fcgCXBAdpapt->TabIndex = 1;
			this->fcgCXBAdpapt->Tag = L"reCmd";
			// 
			// fcgCBWeightB
			// 
			this->fcgCBWeightB->AutoSize = true;
			this->fcgCBWeightB->Location = System::Drawing::Point(237, 165);
			this->fcgCBWeightB->Name = L"fcgCBWeightB";
			this->fcgCBWeightB->Size = System::Drawing::Size(15, 14);
			this->fcgCBWeightB->TabIndex = 4;
			this->fcgCBWeightB->Tag = L"reCmd";
			this->fcgCBWeightB->UseVisualStyleBackColor = true;
			// 
			// fcgNUBBias
			// 
			this->fcgNUBBias->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
			this->fcgNUBBias->Location = System::Drawing::Point(194, 96);
			this->fcgNUBBias->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 100, 0, 0, System::Int32::MinValue });
			this->fcgNUBBias->Name = L"fcgNUBBias";
			this->fcgNUBBias->Size = System::Drawing::Size(61, 23);
			this->fcgNUBBias->TabIndex = 2;
			this->fcgNUBBias->Tag = L"reCmd";
			this->fcgNUBBias->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUBframes
			// 
			this->fcgNUBframes->Location = System::Drawing::Point(194, 26);
			this->fcgNUBframes->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
			this->fcgNUBframes->Name = L"fcgNUBframes";
			this->fcgNUBframes->Size = System::Drawing::Size(61, 23);
			this->fcgNUBframes->TabIndex = 0;
			this->fcgNUBframes->Tag = L"reCmd";
			this->fcgNUBframes->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgLBWeightB
			// 
			this->fcgLBWeightB->AutoSize = true;
			this->fcgLBWeightB->Location = System::Drawing::Point(18, 164);
			this->fcgLBWeightB->Name = L"fcgLBWeightB";
			this->fcgLBWeightB->Size = System::Drawing::Size(106, 17);
			this->fcgLBWeightB->TabIndex = 4;
			this->fcgLBWeightB->Text = L"重み付きBフレーム";
			// 
			// fcgLBBpyramid
			// 
			this->fcgLBBpyramid->AutoSize = true;
			this->fcgLBBpyramid->Location = System::Drawing::Point(18, 130);
			this->fcgLBBpyramid->Name = L"fcgLBBpyramid";
			this->fcgLBBpyramid->Size = System::Drawing::Size(78, 17);
			this->fcgLBBpyramid->TabIndex = 3;
			this->fcgLBBpyramid->Text = L"ピラミッド参照";
			// 
			// fcgLBBBias
			// 
			this->fcgLBBBias->AutoSize = true;
			this->fcgLBBBias->Location = System::Drawing::Point(18, 98);
			this->fcgLBBBias->Name = L"fcgLBBBias";
			this->fcgLBBBias->Size = System::Drawing::Size(110, 17);
			this->fcgLBBBias->TabIndex = 2;
			this->fcgLBBBias->Text = L"Bフレーム挿入傾向";
			// 
			// fcgLBBAdapt
			// 
			this->fcgLBBAdapt->AutoSize = true;
			this->fcgLBBAdapt->Location = System::Drawing::Point(18, 64);
			this->fcgLBBAdapt->Name = L"fcgLBBAdapt";
			this->fcgLBBAdapt->Size = System::Drawing::Size(123, 17);
			this->fcgLBBAdapt->TabIndex = 1;
			this->fcgLBBAdapt->Text = L"適応的Bフレーム挿入";
			// 
			// fcgLBBframes
			// 
			this->fcgLBBframes->AutoSize = true;
			this->fcgLBBframes->Location = System::Drawing::Point(18, 28);
			this->fcgLBBframes->Name = L"fcgLBBframes";
			this->fcgLBBframes->Size = System::Drawing::Size(123, 17);
			this->fcgLBBframes->TabIndex = 0;
			this->fcgLBBframes->Text = L"最大連続Bフレーム数";
			// 
			// fcgCBCABAC
			// 
			this->fcgCBCABAC->AutoSize = true;
			this->fcgCBCABAC->Location = System::Drawing::Point(423, 353);
			this->fcgCBCABAC->Name = L"fcgCBCABAC";
			this->fcgCBCABAC->Size = System::Drawing::Size(72, 21);
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
			this->fcggroupBoxGOP->Location = System::Drawing::Point(423, 3);
			this->fcggroupBoxGOP->Name = L"fcggroupBoxGOP";
			this->fcggroupBoxGOP->Size = System::Drawing::Size(252, 150);
			this->fcggroupBoxGOP->TabIndex = 6;
			this->fcggroupBoxGOP->TabStop = false;
			this->fcggroupBoxGOP->Text = L"GOP関連設定";
			// 
			// fcgLBOpenGOP
			// 
			this->fcgLBOpenGOP->AutoSize = true;
			this->fcgLBOpenGOP->Location = System::Drawing::Point(15, 122);
			this->fcgLBOpenGOP->Name = L"fcgLBOpenGOP";
			this->fcgLBOpenGOP->Size = System::Drawing::Size(73, 17);
			this->fcgLBOpenGOP->TabIndex = 7;
			this->fcgLBOpenGOP->Text = L"Open GOP";
			// 
			// fcgCBOpenGOP
			// 
			this->fcgCBOpenGOP->AutoSize = true;
			this->fcgCBOpenGOP->Location = System::Drawing::Point(219, 123);
			this->fcgCBOpenGOP->Name = L"fcgCBOpenGOP";
			this->fcgCBOpenGOP->Size = System::Drawing::Size(15, 14);
			this->fcgCBOpenGOP->TabIndex = 3;
			this->fcgCBOpenGOP->Tag = L"reCmd";
			this->fcgCBOpenGOP->UseVisualStyleBackColor = true;
			// 
			// fcgNUKeyint
			// 
			this->fcgNUKeyint->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
			this->fcgNUKeyint->Location = System::Drawing::Point(174, 89);
			this->fcgNUKeyint->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3000, 0, 0, 0 });
			this->fcgNUKeyint->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, System::Int32::MinValue });
			this->fcgNUKeyint->Name = L"fcgNUKeyint";
			this->fcgNUKeyint->Size = System::Drawing::Size(62, 23);
			this->fcgNUKeyint->TabIndex = 2;
			this->fcgNUKeyint->Tag = L"reCmd";
			this->fcgNUKeyint->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgLBKeyint
			// 
			this->fcgLBKeyint->AutoSize = true;
			this->fcgLBKeyint->Location = System::Drawing::Point(15, 91);
			this->fcgLBKeyint->Name = L"fcgLBKeyint";
			this->fcgLBKeyint->Size = System::Drawing::Size(134, 17);
			this->fcgLBKeyint->TabIndex = 4;
			this->fcgLBKeyint->Text = L"キーフレーム間隔の上限";
			// 
			// fcgNUMinKeyint
			// 
			this->fcgNUMinKeyint->Location = System::Drawing::Point(174, 56);
			this->fcgNUMinKeyint->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 300, 0, 0, 0 });
			this->fcgNUMinKeyint->Name = L"fcgNUMinKeyint";
			this->fcgNUMinKeyint->Size = System::Drawing::Size(62, 23);
			this->fcgNUMinKeyint->TabIndex = 1;
			this->fcgNUMinKeyint->Tag = L"reCmd";
			this->fcgNUMinKeyint->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgLBMinKeyint
			// 
			this->fcgLBMinKeyint->AutoSize = true;
			this->fcgLBMinKeyint->Location = System::Drawing::Point(15, 60);
			this->fcgLBMinKeyint->Name = L"fcgLBMinKeyint";
			this->fcgLBMinKeyint->Size = System::Drawing::Size(134, 17);
			this->fcgLBMinKeyint->TabIndex = 2;
			this->fcgLBMinKeyint->Text = L"キーフレーム間隔の下限";
			// 
			// fcgNUScenecut
			// 
			this->fcgNUScenecut->Location = System::Drawing::Point(174, 25);
			this->fcgNUScenecut->Name = L"fcgNUScenecut";
			this->fcgNUScenecut->Size = System::Drawing::Size(62, 23);
			this->fcgNUScenecut->TabIndex = 0;
			this->fcgNUScenecut->Tag = L"reCmd";
			this->fcgNUScenecut->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgLBScenecut
			// 
			this->fcgLBScenecut->AutoSize = true;
			this->fcgLBScenecut->Location = System::Drawing::Point(15, 27);
			this->fcgLBScenecut->Name = L"fcgLBScenecut";
			this->fcgLBScenecut->Size = System::Drawing::Size(92, 17);
			this->fcgLBScenecut->TabIndex = 0;
			this->fcgLBScenecut->Text = L"シーンカット閾値";
			// 
			// fcggroupBoxDeblock
			// 
			this->fcggroupBoxDeblock->Controls->Add(this->fcgNUDeblockThreshold);
			this->fcggroupBoxDeblock->Controls->Add(this->fcgNUDeblockStrength);
			this->fcggroupBoxDeblock->Controls->Add(this->fcgLBDeblockThreshold);
			this->fcggroupBoxDeblock->Controls->Add(this->fcgLBDeblockStrength);
			this->fcggroupBoxDeblock->Location = System::Drawing::Point(423, 164);
			this->fcggroupBoxDeblock->Name = L"fcggroupBoxDeblock";
			this->fcggroupBoxDeblock->Size = System::Drawing::Size(252, 96);
			this->fcggroupBoxDeblock->TabIndex = 8;
			this->fcggroupBoxDeblock->TabStop = false;
			// 
			// fcgNUDeblockThreshold
			// 
			this->fcgNUDeblockThreshold->Location = System::Drawing::Point(174, 60);
			this->fcgNUDeblockThreshold->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 6, 0, 0, 0 });
			this->fcgNUDeblockThreshold->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 6, 0, 0, System::Int32::MinValue });
			this->fcgNUDeblockThreshold->Name = L"fcgNUDeblockThreshold";
			this->fcgNUDeblockThreshold->Size = System::Drawing::Size(61, 23);
			this->fcgNUDeblockThreshold->TabIndex = 1;
			this->fcgNUDeblockThreshold->Tag = L"reCmd";
			this->fcgNUDeblockThreshold->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgNUDeblockStrength
			// 
			this->fcgNUDeblockStrength->Location = System::Drawing::Point(174, 27);
			this->fcgNUDeblockStrength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 6, 0, 0, 0 });
			this->fcgNUDeblockStrength->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 6, 0, 0, System::Int32::MinValue });
			this->fcgNUDeblockStrength->Name = L"fcgNUDeblockStrength";
			this->fcgNUDeblockStrength->Size = System::Drawing::Size(61, 23);
			this->fcgNUDeblockStrength->TabIndex = 0;
			this->fcgNUDeblockStrength->Tag = L"reCmd";
			this->fcgNUDeblockStrength->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgLBDeblockThreshold
			// 
			this->fcgLBDeblockThreshold->AutoSize = true;
			this->fcgLBDeblockThreshold->Location = System::Drawing::Point(18, 62);
			this->fcgLBDeblockThreshold->Name = L"fcgLBDeblockThreshold";
			this->fcgLBDeblockThreshold->Size = System::Drawing::Size(81, 17);
			this->fcgLBDeblockThreshold->TabIndex = 1;
			this->fcgLBDeblockThreshold->Text = L"デブロック閾値";
			// 
			// fcgLBDeblockStrength
			// 
			this->fcgLBDeblockStrength->AutoSize = true;
			this->fcgLBDeblockStrength->Location = System::Drawing::Point(18, 29);
			this->fcgLBDeblockStrength->Name = L"fcgLBDeblockStrength";
			this->fcgLBDeblockStrength->Size = System::Drawing::Size(81, 17);
			this->fcgLBDeblockStrength->TabIndex = 0;
			this->fcgLBDeblockStrength->Text = L"デブロック強度";
			// 
			// fcgtabPageExSettings
			// 
			this->fcgtabPageExSettings->Controls->Add(this->fcgLBX264PathSubhighbit);
			this->fcgtabPageExSettings->Controls->Add(this->fcgLBX264PathSub8bit);
			this->fcgtabPageExSettings->Controls->Add(this->fcgBTX264PathSubhighbit);
			this->fcgtabPageExSettings->Controls->Add(this->fcgTXX264PathSubhighbit);
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
			this->fcgtabPageExSettings->Location = System::Drawing::Point(4, 24);
			this->fcgtabPageExSettings->Name = L"fcgtabPageExSettings";
			this->fcgtabPageExSettings->Size = System::Drawing::Size(685, 557);
			this->fcgtabPageExSettings->TabIndex = 3;
			this->fcgtabPageExSettings->Text = L"拡張";
			this->fcgtabPageExSettings->UseVisualStyleBackColor = true;
			// 
			// fcgLBX264PathSubhighbit
			// 
			this->fcgLBX264PathSubhighbit->AutoSize = true;
			this->fcgLBX264PathSubhighbit->Location = System::Drawing::Point(387, 72);
			this->fcgLBX264PathSubhighbit->Name = L"fcgLBX264PathSubhighbit";
			this->fcgLBX264PathSubhighbit->Size = System::Drawing::Size(64, 17);
			this->fcgLBX264PathSubhighbit->TabIndex = 11;
			this->fcgLBX264PathSubhighbit->Text = L"highbit用";
			// 
			// fcgLBX264PathSub8bit
			// 
			this->fcgLBX264PathSub8bit->AutoSize = true;
			this->fcgLBX264PathSub8bit->Location = System::Drawing::Point(395, 42);
			this->fcgLBX264PathSub8bit->Name = L"fcgLBX264PathSub8bit";
			this->fcgLBX264PathSub8bit->Size = System::Drawing::Size(45, 17);
			this->fcgLBX264PathSub8bit->TabIndex = 10;
			this->fcgLBX264PathSub8bit->Text = L"8bit用";
			// 
			// fcgBTX264PathSubhighbit
			// 
			this->fcgBTX264PathSubhighbit->Location = System::Drawing::Point(647, 68);
			this->fcgBTX264PathSubhighbit->Name = L"fcgBTX264PathSubhighbit";
			this->fcgBTX264PathSubhighbit->Size = System::Drawing::Size(30, 25);
			this->fcgBTX264PathSubhighbit->TabIndex = 9;
			this->fcgBTX264PathSubhighbit->Text = L"...";
			this->fcgBTX264PathSubhighbit->UseVisualStyleBackColor = true;
			this->fcgBTX264PathSubhighbit->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTX264PathSubhighbit_Click);
			// 
			// fcgTXX264PathSubhighbit
			// 
			this->fcgTXX264PathSubhighbit->AllowDrop = true;
			this->fcgTXX264PathSubhighbit->Location = System::Drawing::Point(444, 69);
			this->fcgTXX264PathSubhighbit->Name = L"fcgTXX264PathSubhighbit";
			this->fcgTXX264PathSubhighbit->Size = System::Drawing::Size(198, 23);
			this->fcgTXX264PathSubhighbit->TabIndex = 8;
			this->fcgTXX264PathSubhighbit->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXX264PathSubhighbit_TextChanged);
			this->fcgTXX264PathSubhighbit->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXX264PathSubhighbit_Leave);
			// 
			// fcgBTX264PathSub
			// 
			this->fcgBTX264PathSub->Location = System::Drawing::Point(647, 37);
			this->fcgBTX264PathSub->Name = L"fcgBTX264PathSub";
			this->fcgBTX264PathSub->Size = System::Drawing::Size(30, 25);
			this->fcgBTX264PathSub->TabIndex = 2;
			this->fcgBTX264PathSub->Text = L"...";
			this->fcgBTX264PathSub->UseVisualStyleBackColor = true;
			this->fcgBTX264PathSub->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTX264PathSub_Click);
			// 
			// fcgTXX264PathSub
			// 
			this->fcgTXX264PathSub->AllowDrop = true;
			this->fcgTXX264PathSub->Location = System::Drawing::Point(444, 38);
			this->fcgTXX264PathSub->Name = L"fcgTXX264PathSub";
			this->fcgTXX264PathSub->Size = System::Drawing::Size(198, 23);
			this->fcgTXX264PathSub->TabIndex = 1;
			this->fcgTXX264PathSub->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXX264PathSub_TextChanged);
			this->fcgTXX264PathSub->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXX264PathSub_Leave);
			// 
			// fcgLBX264PathSub
			// 
			this->fcgLBX264PathSub->AutoSize = true;
			this->fcgLBX264PathSub->Location = System::Drawing::Point(399, 15);
			this->fcgLBX264PathSub->Name = L"fcgLBX264PathSub";
			this->fcgLBX264PathSub->Size = System::Drawing::Size(58, 17);
			this->fcgLBX264PathSub->TabIndex = 5;
			this->fcgLBX264PathSub->Text = L"～の指定";
			// 
			// fcgLBTempDir
			// 
			this->fcgLBTempDir->AutoSize = true;
			this->fcgLBTempDir->Location = System::Drawing::Point(407, 173);
			this->fcgLBTempDir->Name = L"fcgLBTempDir";
			this->fcgLBTempDir->Size = System::Drawing::Size(72, 17);
			this->fcgLBTempDir->TabIndex = 4;
			this->fcgLBTempDir->Text = L"一時フォルダ";
			// 
			// fcgBTCustomTempDir
			// 
			this->fcgBTCustomTempDir->Location = System::Drawing::Point(632, 233);
			this->fcgBTCustomTempDir->Name = L"fcgBTCustomTempDir";
			this->fcgBTCustomTempDir->Size = System::Drawing::Size(33, 26);
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
			this->fcggroupBoxCmdEx->Location = System::Drawing::Point(9, 273);
			this->fcggroupBoxCmdEx->Name = L"fcggroupBoxCmdEx";
			this->fcggroupBoxCmdEx->Size = System::Drawing::Size(663, 262);
			this->fcggroupBoxCmdEx->TabIndex = 7;
			this->fcggroupBoxCmdEx->TabStop = false;
			this->fcggroupBoxCmdEx->Text = L"追加コマンド";
			// 
			// fcgCXCmdExInsert
			// 
			this->fcgCXCmdExInsert->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXCmdExInsert->FormattingEnabled = true;
			this->fcgCXCmdExInsert->Location = System::Drawing::Point(222, 230);
			this->fcgCXCmdExInsert->Name = L"fcgCXCmdExInsert";
			this->fcgCXCmdExInsert->Size = System::Drawing::Size(188, 23);
			this->fcgCXCmdExInsert->TabIndex = 4;
			this->fcgCXCmdExInsert->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXCmdExInsert_SelectedIndexChanged);
			this->fcgCXCmdExInsert->FontChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXCmdExInsert_FontChanged);
			// 
			// fcgCBNulOutCLI
			// 
			this->fcgCBNulOutCLI->AutoSize = true;
			this->fcgCBNulOutCLI->Location = System::Drawing::Point(20, 232);
			this->fcgCBNulOutCLI->Name = L"fcgCBNulOutCLI";
			this->fcgCBNulOutCLI->Size = System::Drawing::Size(72, 21);
			this->fcgCBNulOutCLI->TabIndex = 2;
			this->fcgCBNulOutCLI->Tag = L"chValue";
			this->fcgCBNulOutCLI->Text = L"nul出力";
			this->fcgCBNulOutCLI->UseVisualStyleBackColor = true;
			// 
			// fcgBTCmdEx
			// 
			this->fcgBTCmdEx->Location = System::Drawing::Point(542, 230);
			this->fcgBTCmdEx->Name = L"fcgBTCmdEx";
			this->fcgBTCmdEx->Size = System::Drawing::Size(109, 26);
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
			this->fcgTXCmdEx->Location = System::Drawing::Point(7, 22);
			this->fcgTXCmdEx->Multiline = true;
			this->fcgTXCmdEx->Name = L"fcgTXCmdEx";
			this->fcgTXCmdEx->Size = System::Drawing::Size(649, 202);
			this->fcgTXCmdEx->TabIndex = 0;
			this->fcgTXCmdEx->Tag = L"chValue";
			this->fcgTXCmdEx->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgInsertDragDropFilename_DragDrop);
			this->fcgTXCmdEx->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgInsertDragDropFilename_Enter);
			this->fcgTXCmdEx->DragOver += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgInsertDragDropFilename_DragOver);
			// 
			// fcgTXCustomTempDir
			// 
			this->fcgTXCustomTempDir->Location = System::Drawing::Point(424, 234);
			this->fcgTXCustomTempDir->Name = L"fcgTXCustomTempDir";
			this->fcgTXCustomTempDir->Size = System::Drawing::Size(204, 23);
			this->fcgTXCustomTempDir->TabIndex = 5;
			this->fcgTXCustomTempDir->Tag = L"";
			this->fcgTXCustomTempDir->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXCustomTempDir_TextChanged);
			// 
			// fcgCXTempDir
			// 
			this->fcgCXTempDir->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXTempDir->FormattingEnabled = true;
			this->fcgCXTempDir->Location = System::Drawing::Point(411, 202);
			this->fcgCXTempDir->Name = L"fcgCXTempDir";
			this->fcgCXTempDir->Size = System::Drawing::Size(235, 23);
			this->fcgCXTempDir->TabIndex = 4;
			this->fcgCXTempDir->Tag = L"chValue";
			// 
			// fcgCXX264Priority
			// 
			this->fcgCXX264Priority->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXX264Priority->FormattingEnabled = true;
			this->fcgCXX264Priority->Location = System::Drawing::Point(489, 112);
			this->fcgCXX264Priority->Name = L"fcgCXX264Priority";
			this->fcgCXX264Priority->Size = System::Drawing::Size(170, 23);
			this->fcgCXX264Priority->TabIndex = 3;
			this->fcgCXX264Priority->Tag = L"chValue";
			// 
			// fcgLBX264Priority
			// 
			this->fcgLBX264Priority->AutoSize = true;
			this->fcgLBX264Priority->Location = System::Drawing::Point(407, 116);
			this->fcgLBX264Priority->Name = L"fcgLBX264Priority";
			this->fcgLBX264Priority->Size = System::Drawing::Size(78, 17);
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
			this->fcggroupBoxExSettings->Location = System::Drawing::Point(9, 3);
			this->fcggroupBoxExSettings->Name = L"fcggroupBoxExSettings";
			this->fcggroupBoxExSettings->Size = System::Drawing::Size(363, 263);
			this->fcggroupBoxExSettings->TabIndex = 0;
			this->fcggroupBoxExSettings->TabStop = false;
			this->fcggroupBoxExSettings->Text = L"拡張設定";
			// 
			// fcgCBSetKeyframeAtChapter
			// 
			this->fcgCBSetKeyframeAtChapter->AutoSize = true;
			this->fcgCBSetKeyframeAtChapter->Location = System::Drawing::Point(20, 158);
			this->fcgCBSetKeyframeAtChapter->Name = L"fcgCBSetKeyframeAtChapter";
			this->fcgCBSetKeyframeAtChapter->Size = System::Drawing::Size(314, 21);
			this->fcgCBSetKeyframeAtChapter->TabIndex = 5;
			this->fcgCBSetKeyframeAtChapter->Tag = L"chValue";
			this->fcgCBSetKeyframeAtChapter->Text = L"チャプター位置にキーフレームを設定する (mux有効時)";
			this->fcgCBSetKeyframeAtChapter->UseVisualStyleBackColor = true;
			// 
			// fcgCBInputAsLW48
			// 
			this->fcgCBInputAsLW48->AutoSize = true;
			this->fcgCBInputAsLW48->Location = System::Drawing::Point(20, 201);
			this->fcgCBInputAsLW48->Name = L"fcgCBInputAsLW48";
			this->fcgCBInputAsLW48->Size = System::Drawing::Size(92, 21);
			this->fcgCBInputAsLW48->TabIndex = 4;
			this->fcgCBInputAsLW48->Tag = L"chValue";
			this->fcgCBInputAsLW48->Text = L"LW48モード";
			this->fcgCBInputAsLW48->UseVisualStyleBackColor = true;
			// 
			// fcgCBCheckKeyframes
			// 
			this->fcgCBCheckKeyframes->AutoSize = true;
			this->fcgCBCheckKeyframes->Location = System::Drawing::Point(20, 130);
			this->fcgCBCheckKeyframes->Name = L"fcgCBCheckKeyframes";
			this->fcgCBCheckKeyframes->Size = System::Drawing::Size(219, 21);
			this->fcgCBCheckKeyframes->TabIndex = 3;
			this->fcgCBCheckKeyframes->Tag = L"chValue";
			this->fcgCBCheckKeyframes->Text = L"Aviutlのキーフレーム設定検出を行う";
			this->fcgCBCheckKeyframes->UseVisualStyleBackColor = true;
			// 
			// fcgCBAuoTcfileout
			// 
			this->fcgCBAuoTcfileout->AutoSize = true;
			this->fcgCBAuoTcfileout->Location = System::Drawing::Point(20, 104);
			this->fcgCBAuoTcfileout->Name = L"fcgCBAuoTcfileout";
			this->fcgCBAuoTcfileout->Size = System::Drawing::Size(111, 21);
			this->fcgCBAuoTcfileout->TabIndex = 2;
			this->fcgCBAuoTcfileout->Tag = L"chValue";
			this->fcgCBAuoTcfileout->Text = L"タイムコード出力";
			this->fcgCBAuoTcfileout->UseVisualStyleBackColor = true;
			// 
			// fcgCBAFSBitrateCorrection
			// 
			this->fcgCBAFSBitrateCorrection->AutoSize = true;
			this->fcgCBAFSBitrateCorrection->Location = System::Drawing::Point(38, 60);
			this->fcgCBAFSBitrateCorrection->Name = L"fcgCBAFSBitrateCorrection";
			this->fcgCBAFSBitrateCorrection->Size = System::Drawing::Size(327, 21);
			this->fcgCBAFSBitrateCorrection->TabIndex = 1;
			this->fcgCBAFSBitrateCorrection->Tag = L"chValue";
			this->fcgCBAFSBitrateCorrection->Text = L"設定したビットレートになるよう補正する(自動マルチパス時)";
			this->fcgCBAFSBitrateCorrection->UseVisualStyleBackColor = true;
			// 
			// fcgCBAFS
			// 
			this->fcgCBAFS->AutoSize = true;
			this->fcgCBAFS->Location = System::Drawing::Point(20, 33);
			this->fcgCBAFS->Name = L"fcgCBAFS";
			this->fcgCBAFS->Size = System::Drawing::Size(216, 21);
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
			this->fcgCSExeFiles->Size = System::Drawing::Size(146, 26);
			// 
			// fcgTSExeFileshelp
			// 
			this->fcgTSExeFileshelp->Name = L"fcgTSExeFileshelp";
			this->fcgTSExeFileshelp->Size = System::Drawing::Size(145, 22);
			this->fcgTSExeFileshelp->Text = L"helpを表示";
			this->fcgTSExeFileshelp->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSExeFileshelp_Click);
			// 
			// fcgtoolStripSettings
			// 
			this->fcgtoolStripSettings->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(128)));
			this->fcgtoolStripSettings->ImageScalingSize = System::Drawing::Size(18, 18);
			this->fcgtoolStripSettings->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(13) {
				this->fcgTSBSave,
					this->fcgTSBSaveNew, this->fcgTSBDelete, this->fcgtoolStripSeparator1, this->fcgTSSettings, this->fcgTSBCMDOnly, this->toolStripSeparator3,
					this->fcgTSBBitrateCalc, this->toolStripSeparator2, this->fcgTSBOtherSettings, this->fcgTSLSettingsNotes, this->fcgTSTSettingsNotes,
					this->toolStripSeparator4
			});
			this->fcgtoolStripSettings->Location = System::Drawing::Point(0, 0);
			this->fcgtoolStripSettings->Name = L"fcgtoolStripSettings";
			this->fcgtoolStripSettings->Padding = System::Windows::Forms::Padding(0);
			this->fcgtoolStripSettings->Size = System::Drawing::Size(1134, 25);
			this->fcgtoolStripSettings->TabIndex = 1;
			this->fcgtoolStripSettings->Text = L"toolStrip1";
			// 
			// fcgTSBSave
			// 
			this->fcgTSBSave->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBSave.Image")));
			this->fcgTSBSave->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->fcgTSBSave->Name = L"fcgTSBSave";
			this->fcgTSBSave->Size = System::Drawing::Size(97, 22);
			this->fcgTSBSave->Text = L"上書き保存";
			this->fcgTSBSave->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBSave_Click);
			// 
			// fcgTSBSaveNew
			// 
			this->fcgTSBSaveNew->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBSaveNew.Image")));
			this->fcgTSBSaveNew->ImageTransparentColor = System::Drawing::Color::Black;
			this->fcgTSBSaveNew->Name = L"fcgTSBSaveNew";
			this->fcgTSBSaveNew->Size = System::Drawing::Size(86, 22);
			this->fcgTSBSaveNew->Text = L"新規保存";
			this->fcgTSBSaveNew->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBSaveNew_Click);
			// 
			// fcgTSBDelete
			// 
			this->fcgTSBDelete->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBDelete.Image")));
			this->fcgTSBDelete->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->fcgTSBDelete->Name = L"fcgTSBDelete";
			this->fcgTSBDelete->Size = System::Drawing::Size(58, 22);
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
			this->fcgTSSettings->Size = System::Drawing::Size(88, 22);
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
			this->fcgTSBCMDOnly->Size = System::Drawing::Size(85, 22);
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
			// fcgTSBBitrateCalc
			// 
			this->fcgTSBBitrateCalc->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
			this->fcgTSBBitrateCalc->CheckOnClick = true;
			this->fcgTSBBitrateCalc->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->fcgTSBBitrateCalc->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBBitrateCalc.Image")));
			this->fcgTSBBitrateCalc->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->fcgTSBBitrateCalc->Name = L"fcgTSBBitrateCalc";
			this->fcgTSBBitrateCalc->Size = System::Drawing::Size(114, 22);
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
			this->fcgTSBOtherSettings->Size = System::Drawing::Size(86, 22);
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
			this->fcgTSLSettingsNotes->Size = System::Drawing::Size(53, 22);
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
			this->fcgTSTSettingsNotes->Size = System::Drawing::Size(224, 25);
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
			// fcgtabControlMux
			// 
			this->fcgtabControlMux->Controls->Add(this->fcgtabPageMP4);
			this->fcgtabControlMux->Controls->Add(this->fcgtabPageMKV);
			this->fcgtabControlMux->Controls->Add(this->fcgtabPageMPG);
			this->fcgtabControlMux->Controls->Add(this->fcgtabPageMux);
			this->fcgtabControlMux->Controls->Add(this->fcgtabPageBat);
			this->fcgtabControlMux->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(128)));
			this->fcgtabControlMux->Location = System::Drawing::Point(700, 372);
			this->fcgtabControlMux->Name = L"fcgtabControlMux";
			this->fcgtabControlMux->SelectedIndex = 0;
			this->fcgtabControlMux->Size = System::Drawing::Size(432, 241);
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
			this->fcgtabPageMP4->Location = System::Drawing::Point(4, 24);
			this->fcgtabPageMP4->Name = L"fcgtabPageMP4";
			this->fcgtabPageMP4->Padding = System::Windows::Forms::Padding(3);
			this->fcgtabPageMP4->Size = System::Drawing::Size(424, 213);
			this->fcgtabPageMP4->TabIndex = 0;
			this->fcgtabPageMP4->Text = L"mp4";
			this->fcgtabPageMP4->UseVisualStyleBackColor = true;
			// 
			// fcgCBMP4MuxApple
			// 
			this->fcgCBMP4MuxApple->AutoSize = true;
			this->fcgCBMP4MuxApple->Location = System::Drawing::Point(294, 42);
			this->fcgCBMP4MuxApple->Name = L"fcgCBMP4MuxApple";
			this->fcgCBMP4MuxApple->Size = System::Drawing::Size(125, 21);
			this->fcgCBMP4MuxApple->TabIndex = 2;
			this->fcgCBMP4MuxApple->Tag = L"chValue";
			this->fcgCBMP4MuxApple->Text = L"Apple形式に対応";
			this->fcgCBMP4MuxApple->UseVisualStyleBackColor = true;
			// 
			// fcgBTTC2MP4Path
			// 
			this->fcgBTTC2MP4Path->Location = System::Drawing::Point(382, 98);
			this->fcgBTTC2MP4Path->Name = L"fcgBTTC2MP4Path";
			this->fcgBTTC2MP4Path->Size = System::Drawing::Size(34, 26);
			this->fcgBTTC2MP4Path->TabIndex = 6;
			this->fcgBTTC2MP4Path->Text = L"...";
			this->fcgBTTC2MP4Path->UseVisualStyleBackColor = true;
			this->fcgBTTC2MP4Path->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTTC2MP4Path_Click);
			// 
			// fcgTXTC2MP4Path
			// 
			this->fcgTXTC2MP4Path->AllowDrop = true;
			this->fcgTXTC2MP4Path->Location = System::Drawing::Point(153, 99);
			this->fcgTXTC2MP4Path->Name = L"fcgTXTC2MP4Path";
			this->fcgTXTC2MP4Path->Size = System::Drawing::Size(227, 23);
			this->fcgTXTC2MP4Path->TabIndex = 5;
			this->fcgTXTC2MP4Path->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXTC2MP4Path_TextChanged);
			this->fcgTXTC2MP4Path->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
			this->fcgTXTC2MP4Path->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
			// 
			// fcgBTMP4MuxerPath
			// 
			this->fcgBTMP4MuxerPath->Location = System::Drawing::Point(382, 73);
			this->fcgBTMP4MuxerPath->Name = L"fcgBTMP4MuxerPath";
			this->fcgBTMP4MuxerPath->Size = System::Drawing::Size(34, 26);
			this->fcgBTMP4MuxerPath->TabIndex = 4;
			this->fcgBTMP4MuxerPath->Text = L"...";
			this->fcgBTMP4MuxerPath->UseVisualStyleBackColor = true;
			this->fcgBTMP4MuxerPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMP4MuxerPath_Click);
			// 
			// fcgTXMP4MuxerPath
			// 
			this->fcgTXMP4MuxerPath->AllowDrop = true;
			this->fcgTXMP4MuxerPath->Location = System::Drawing::Point(153, 74);
			this->fcgTXMP4MuxerPath->Name = L"fcgTXMP4MuxerPath";
			this->fcgTXMP4MuxerPath->Size = System::Drawing::Size(227, 23);
			this->fcgTXMP4MuxerPath->TabIndex = 3;
			this->fcgTXMP4MuxerPath->Tag = L"";
			this->fcgTXMP4MuxerPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4MuxerPath_TextChanged);
			this->fcgTXMP4MuxerPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
			this->fcgTXMP4MuxerPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
			// 
			// fcgLBTC2MP4Path
			// 
			this->fcgLBTC2MP4Path->AutoSize = true;
			this->fcgLBTC2MP4Path->Location = System::Drawing::Point(4, 102);
			this->fcgLBTC2MP4Path->Name = L"fcgLBTC2MP4Path";
			this->fcgLBTC2MP4Path->Size = System::Drawing::Size(58, 17);
			this->fcgLBTC2MP4Path->TabIndex = 4;
			this->fcgLBTC2MP4Path->Text = L"～の指定";
			// 
			// fcgLBMP4MuxerPath
			// 
			this->fcgLBMP4MuxerPath->AutoSize = true;
			this->fcgLBMP4MuxerPath->Location = System::Drawing::Point(4, 78);
			this->fcgLBMP4MuxerPath->Name = L"fcgLBMP4MuxerPath";
			this->fcgLBMP4MuxerPath->Size = System::Drawing::Size(58, 17);
			this->fcgLBMP4MuxerPath->TabIndex = 3;
			this->fcgLBMP4MuxerPath->Text = L"～の指定";
			// 
			// fcgCXMP4CmdEx
			// 
			this->fcgCXMP4CmdEx->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXMP4CmdEx->FormattingEnabled = true;
			this->fcgCXMP4CmdEx->Location = System::Drawing::Point(240, 10);
			this->fcgCXMP4CmdEx->Name = L"fcgCXMP4CmdEx";
			this->fcgCXMP4CmdEx->Size = System::Drawing::Size(176, 23);
			this->fcgCXMP4CmdEx->TabIndex = 1;
			this->fcgCXMP4CmdEx->Tag = L"chValue";
			// 
			// fcgLBMP4CmdEx
			// 
			this->fcgLBMP4CmdEx->AutoSize = true;
			this->fcgLBMP4CmdEx->Location = System::Drawing::Point(156, 14);
			this->fcgLBMP4CmdEx->Name = L"fcgLBMP4CmdEx";
			this->fcgLBMP4CmdEx->Size = System::Drawing::Size(82, 17);
			this->fcgLBMP4CmdEx->TabIndex = 1;
			this->fcgLBMP4CmdEx->Text = L"拡張オプション";
			// 
			// fcgCBMP4MuxerExt
			// 
			this->fcgCBMP4MuxerExt->AutoSize = true;
			this->fcgCBMP4MuxerExt->Location = System::Drawing::Point(11, 12);
			this->fcgCBMP4MuxerExt->Name = L"fcgCBMP4MuxerExt";
			this->fcgCBMP4MuxerExt->Size = System::Drawing::Size(129, 21);
			this->fcgCBMP4MuxerExt->TabIndex = 0;
			this->fcgCBMP4MuxerExt->Tag = L"chValue";
			this->fcgCBMP4MuxerExt->Text = L"外部muxerを使用";
			this->fcgCBMP4MuxerExt->UseVisualStyleBackColor = true;
			// 
			// fcgBTMP4RawPath
			// 
			this->fcgBTMP4RawPath->Location = System::Drawing::Point(382, 123);
			this->fcgBTMP4RawPath->Name = L"fcgBTMP4RawPath";
			this->fcgBTMP4RawPath->Size = System::Drawing::Size(34, 26);
			this->fcgBTMP4RawPath->TabIndex = 8;
			this->fcgBTMP4RawPath->Text = L"...";
			this->fcgBTMP4RawPath->UseVisualStyleBackColor = true;
			this->fcgBTMP4RawPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMP4RawMuxerPath_Click);
			// 
			// fcgTXMP4RawPath
			// 
			this->fcgTXMP4RawPath->AllowDrop = true;
			this->fcgTXMP4RawPath->Location = System::Drawing::Point(153, 124);
			this->fcgTXMP4RawPath->Name = L"fcgTXMP4RawPath";
			this->fcgTXMP4RawPath->Size = System::Drawing::Size(227, 23);
			this->fcgTXMP4RawPath->TabIndex = 7;
			this->fcgTXMP4RawPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4RawMuxerPath_TextChanged);
			this->fcgTXMP4RawPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
			this->fcgTXMP4RawPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
			// 
			// fcgLBMP4RawPath
			// 
			this->fcgLBMP4RawPath->AutoSize = true;
			this->fcgLBMP4RawPath->Location = System::Drawing::Point(4, 127);
			this->fcgLBMP4RawPath->Name = L"fcgLBMP4RawPath";
			this->fcgLBMP4RawPath->Size = System::Drawing::Size(58, 17);
			this->fcgLBMP4RawPath->TabIndex = 20;
			this->fcgLBMP4RawPath->Text = L"～の指定";
			// 
			// fcgBTMP4BoxTempDir
			// 
			this->fcgBTMP4BoxTempDir->Location = System::Drawing::Point(382, 179);
			this->fcgBTMP4BoxTempDir->Name = L"fcgBTMP4BoxTempDir";
			this->fcgBTMP4BoxTempDir->Size = System::Drawing::Size(34, 26);
			this->fcgBTMP4BoxTempDir->TabIndex = 11;
			this->fcgBTMP4BoxTempDir->Text = L"...";
			this->fcgBTMP4BoxTempDir->UseVisualStyleBackColor = true;
			this->fcgBTMP4BoxTempDir->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMP4BoxTempDir_Click);
			// 
			// fcgTXMP4BoxTempDir
			// 
			this->fcgTXMP4BoxTempDir->Location = System::Drawing::Point(120, 180);
			this->fcgTXMP4BoxTempDir->Name = L"fcgTXMP4BoxTempDir";
			this->fcgTXMP4BoxTempDir->Size = System::Drawing::Size(255, 23);
			this->fcgTXMP4BoxTempDir->TabIndex = 10;
			this->fcgTXMP4BoxTempDir->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4BoxTempDir_TextChanged);
			// 
			// fcgCXMP4BoxTempDir
			// 
			this->fcgCXMP4BoxTempDir->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXMP4BoxTempDir->FormattingEnabled = true;
			this->fcgCXMP4BoxTempDir->Location = System::Drawing::Point(163, 148);
			this->fcgCXMP4BoxTempDir->Name = L"fcgCXMP4BoxTempDir";
			this->fcgCXMP4BoxTempDir->Size = System::Drawing::Size(231, 23);
			this->fcgCXMP4BoxTempDir->TabIndex = 9;
			this->fcgCXMP4BoxTempDir->Tag = L"chValue";
			// 
			// fcgLBMP4BoxTempDir
			// 
			this->fcgLBMP4BoxTempDir->AutoSize = true;
			this->fcgLBMP4BoxTempDir->Location = System::Drawing::Point(28, 152);
			this->fcgLBMP4BoxTempDir->Name = L"fcgLBMP4BoxTempDir";
			this->fcgLBMP4BoxTempDir->Size = System::Drawing::Size(123, 17);
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
			this->fcgtabPageMKV->Location = System::Drawing::Point(4, 24);
			this->fcgtabPageMKV->Name = L"fcgtabPageMKV";
			this->fcgtabPageMKV->Padding = System::Windows::Forms::Padding(3);
			this->fcgtabPageMKV->Size = System::Drawing::Size(424, 213);
			this->fcgtabPageMKV->TabIndex = 1;
			this->fcgtabPageMKV->Text = L"mkv";
			this->fcgtabPageMKV->UseVisualStyleBackColor = true;
			// 
			// fcgBTMKVMuxerPath
			// 
			this->fcgBTMKVMuxerPath->Location = System::Drawing::Point(382, 86);
			this->fcgBTMKVMuxerPath->Name = L"fcgBTMKVMuxerPath";
			this->fcgBTMKVMuxerPath->Size = System::Drawing::Size(34, 26);
			this->fcgBTMKVMuxerPath->TabIndex = 3;
			this->fcgBTMKVMuxerPath->Text = L"...";
			this->fcgBTMKVMuxerPath->UseVisualStyleBackColor = true;
			this->fcgBTMKVMuxerPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMKVMuxerPath_Click);
			// 
			// fcgTXMKVMuxerPath
			// 
			this->fcgTXMKVMuxerPath->Location = System::Drawing::Point(147, 87);
			this->fcgTXMKVMuxerPath->Name = L"fcgTXMKVMuxerPath";
			this->fcgTXMKVMuxerPath->Size = System::Drawing::Size(232, 23);
			this->fcgTXMKVMuxerPath->TabIndex = 2;
			this->fcgTXMKVMuxerPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMKVMuxerPath_TextChanged);
			this->fcgTXMKVMuxerPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
			this->fcgTXMKVMuxerPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
			// 
			// fcgLBMKVMuxerPath
			// 
			this->fcgLBMKVMuxerPath->AutoSize = true;
			this->fcgLBMKVMuxerPath->Location = System::Drawing::Point(4, 90);
			this->fcgLBMKVMuxerPath->Name = L"fcgLBMKVMuxerPath";
			this->fcgLBMKVMuxerPath->Size = System::Drawing::Size(58, 17);
			this->fcgLBMKVMuxerPath->TabIndex = 19;
			this->fcgLBMKVMuxerPath->Text = L"～の指定";
			// 
			// fcgCXMKVCmdEx
			// 
			this->fcgCXMKVCmdEx->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXMKVCmdEx->FormattingEnabled = true;
			this->fcgCXMKVCmdEx->Location = System::Drawing::Point(240, 48);
			this->fcgCXMKVCmdEx->Name = L"fcgCXMKVCmdEx";
			this->fcgCXMKVCmdEx->Size = System::Drawing::Size(176, 23);
			this->fcgCXMKVCmdEx->TabIndex = 1;
			this->fcgCXMKVCmdEx->Tag = L"chValue";
			// 
			// fcgLBMKVMuxerCmdEx
			// 
			this->fcgLBMKVMuxerCmdEx->AutoSize = true;
			this->fcgLBMKVMuxerCmdEx->Location = System::Drawing::Point(156, 52);
			this->fcgLBMKVMuxerCmdEx->Name = L"fcgLBMKVMuxerCmdEx";
			this->fcgLBMKVMuxerCmdEx->Size = System::Drawing::Size(82, 17);
			this->fcgLBMKVMuxerCmdEx->TabIndex = 17;
			this->fcgLBMKVMuxerCmdEx->Text = L"拡張オプション";
			// 
			// fcgCBMKVMuxerExt
			// 
			this->fcgCBMKVMuxerExt->AutoSize = true;
			this->fcgCBMKVMuxerExt->Location = System::Drawing::Point(11, 51);
			this->fcgCBMKVMuxerExt->Name = L"fcgCBMKVMuxerExt";
			this->fcgCBMKVMuxerExt->Size = System::Drawing::Size(129, 21);
			this->fcgCBMKVMuxerExt->TabIndex = 0;
			this->fcgCBMKVMuxerExt->Tag = L"chValue";
			this->fcgCBMKVMuxerExt->Text = L"外部muxerを使用";
			this->fcgCBMKVMuxerExt->UseVisualStyleBackColor = true;
			// 
			// fcgtabPageMPG
			// 
			this->fcgtabPageMPG->Controls->Add(this->fcgBTMPGMuxerPath);
			this->fcgtabPageMPG->Controls->Add(this->fcgTXMPGMuxerPath);
			this->fcgtabPageMPG->Controls->Add(this->fcgLBMPGMuxerPath);
			this->fcgtabPageMPG->Controls->Add(this->fcgCXMPGCmdEx);
			this->fcgtabPageMPG->Controls->Add(this->fcgLBMPGMuxerCmdEx);
			this->fcgtabPageMPG->Controls->Add(this->fcgCBMPGMuxerExt);
			this->fcgtabPageMPG->Location = System::Drawing::Point(4, 24);
			this->fcgtabPageMPG->Name = L"fcgtabPageMPG";
			this->fcgtabPageMPG->Size = System::Drawing::Size(424, 213);
			this->fcgtabPageMPG->TabIndex = 4;
			this->fcgtabPageMPG->Text = L"mpg";
			this->fcgtabPageMPG->UseVisualStyleBackColor = true;
			// 
			// fcgBTMPGMuxerPath
			// 
			this->fcgBTMPGMuxerPath->Location = System::Drawing::Point(384, 110);
			this->fcgBTMPGMuxerPath->Name = L"fcgBTMPGMuxerPath";
			this->fcgBTMPGMuxerPath->Size = System::Drawing::Size(34, 26);
			this->fcgBTMPGMuxerPath->TabIndex = 23;
			this->fcgBTMPGMuxerPath->Text = L"...";
			this->fcgBTMPGMuxerPath->UseVisualStyleBackColor = true;
			this->fcgBTMPGMuxerPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMPGMuxerPath_Click);
			// 
			// fcgTXMPGMuxerPath
			// 
			this->fcgTXMPGMuxerPath->Location = System::Drawing::Point(148, 111);
			this->fcgTXMPGMuxerPath->Name = L"fcgTXMPGMuxerPath";
			this->fcgTXMPGMuxerPath->Size = System::Drawing::Size(232, 23);
			this->fcgTXMPGMuxerPath->TabIndex = 22;
			this->fcgTXMPGMuxerPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMPGMuxerPath_TextChanged);
			this->fcgTXMPGMuxerPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
			this->fcgTXMPGMuxerPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
			// 
			// fcgLBMPGMuxerPath
			// 
			this->fcgLBMPGMuxerPath->AutoSize = true;
			this->fcgLBMPGMuxerPath->Location = System::Drawing::Point(6, 115);
			this->fcgLBMPGMuxerPath->Name = L"fcgLBMPGMuxerPath";
			this->fcgLBMPGMuxerPath->Size = System::Drawing::Size(58, 17);
			this->fcgLBMPGMuxerPath->TabIndex = 25;
			this->fcgLBMPGMuxerPath->Text = L"～の指定";
			// 
			// fcgCXMPGCmdEx
			// 
			this->fcgCXMPGCmdEx->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXMPGCmdEx->FormattingEnabled = true;
			this->fcgCXMPGCmdEx->Location = System::Drawing::Point(241, 73);
			this->fcgCXMPGCmdEx->Name = L"fcgCXMPGCmdEx";
			this->fcgCXMPGCmdEx->Size = System::Drawing::Size(176, 23);
			this->fcgCXMPGCmdEx->TabIndex = 21;
			this->fcgCXMPGCmdEx->Tag = L"chValue";
			// 
			// fcgLBMPGMuxerCmdEx
			// 
			this->fcgLBMPGMuxerCmdEx->AutoSize = true;
			this->fcgLBMPGMuxerCmdEx->Location = System::Drawing::Point(158, 76);
			this->fcgLBMPGMuxerCmdEx->Name = L"fcgLBMPGMuxerCmdEx";
			this->fcgLBMPGMuxerCmdEx->Size = System::Drawing::Size(82, 17);
			this->fcgLBMPGMuxerCmdEx->TabIndex = 24;
			this->fcgLBMPGMuxerCmdEx->Text = L"拡張オプション";
			// 
			// fcgCBMPGMuxerExt
			// 
			this->fcgCBMPGMuxerExt->AutoSize = true;
			this->fcgCBMPGMuxerExt->Location = System::Drawing::Point(12, 75);
			this->fcgCBMPGMuxerExt->Name = L"fcgCBMPGMuxerExt";
			this->fcgCBMPGMuxerExt->Size = System::Drawing::Size(129, 21);
			this->fcgCBMPGMuxerExt->TabIndex = 20;
			this->fcgCBMPGMuxerExt->Tag = L"chValue";
			this->fcgCBMPGMuxerExt->Text = L"外部muxerを使用";
			this->fcgCBMPGMuxerExt->UseVisualStyleBackColor = true;
			// 
			// fcgtabPageMux
			// 
			this->fcgtabPageMux->Controls->Add(this->fcgCXMuxPriority);
			this->fcgtabPageMux->Controls->Add(this->fcgLBMuxPriority);
			this->fcgtabPageMux->Location = System::Drawing::Point(4, 24);
			this->fcgtabPageMux->Name = L"fcgtabPageMux";
			this->fcgtabPageMux->Size = System::Drawing::Size(424, 213);
			this->fcgtabPageMux->TabIndex = 2;
			this->fcgtabPageMux->Text = L"Mux共通設定";
			this->fcgtabPageMux->UseVisualStyleBackColor = true;
			// 
			// fcgCXMuxPriority
			// 
			this->fcgCXMuxPriority->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXMuxPriority->FormattingEnabled = true;
			this->fcgCXMuxPriority->Location = System::Drawing::Point(115, 72);
			this->fcgCXMuxPriority->Name = L"fcgCXMuxPriority";
			this->fcgCXMuxPriority->Size = System::Drawing::Size(222, 23);
			this->fcgCXMuxPriority->TabIndex = 1;
			this->fcgCXMuxPriority->Tag = L"chValue";
			// 
			// fcgLBMuxPriority
			// 
			this->fcgLBMuxPriority->AutoSize = true;
			this->fcgLBMuxPriority->Location = System::Drawing::Point(17, 75);
			this->fcgLBMuxPriority->Name = L"fcgLBMuxPriority";
			this->fcgLBMuxPriority->Size = System::Drawing::Size(73, 17);
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
			this->fcgtabPageBat->Location = System::Drawing::Point(4, 24);
			this->fcgtabPageBat->Name = L"fcgtabPageBat";
			this->fcgtabPageBat->Size = System::Drawing::Size(424, 213);
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
			this->fcgLBBatAfterString->Location = System::Drawing::Point(342, 127);
			this->fcgLBBatAfterString->Name = L"fcgLBBatAfterString";
			this->fcgLBBatAfterString->Size = System::Drawing::Size(32, 18);
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
			this->fcgLBBatBeforeString->Location = System::Drawing::Point(342, 22);
			this->fcgLBBatBeforeString->Name = L"fcgLBBatBeforeString";
			this->fcgLBBatBeforeString->Size = System::Drawing::Size(32, 18);
			this->fcgLBBatBeforeString->TabIndex = 3;
			this->fcgLBBatBeforeString->Text = L" 前& ";
			this->fcgLBBatBeforeString->TextAlign = System::Drawing::ContentAlignment::TopCenter;
			// 
			// fcgBTBatAfterPath
			// 
			this->fcgBTBatAfterPath->Location = System::Drawing::Point(371, 173);
			this->fcgBTBatAfterPath->Name = L"fcgBTBatAfterPath";
			this->fcgBTBatAfterPath->Size = System::Drawing::Size(34, 26);
			this->fcgBTBatAfterPath->TabIndex = 11;
			this->fcgBTBatAfterPath->Tag = L"chValue";
			this->fcgBTBatAfterPath->Text = L"...";
			this->fcgBTBatAfterPath->UseVisualStyleBackColor = true;
			this->fcgBTBatAfterPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatAfterPath_Click);
			// 
			// fcgTXBatAfterPath
			// 
			this->fcgTXBatAfterPath->AllowDrop = true;
			this->fcgTXBatAfterPath->Location = System::Drawing::Point(142, 174);
			this->fcgTXBatAfterPath->Name = L"fcgTXBatAfterPath";
			this->fcgTXBatAfterPath->Size = System::Drawing::Size(227, 23);
			this->fcgTXBatAfterPath->TabIndex = 10;
			this->fcgTXBatAfterPath->Tag = L"chValue";
			this->fcgTXBatAfterPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
			this->fcgTXBatAfterPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
			// 
			// fcgLBBatAfterPath
			// 
			this->fcgLBBatAfterPath->AutoSize = true;
			this->fcgLBBatAfterPath->Location = System::Drawing::Point(45, 178);
			this->fcgLBBatAfterPath->Name = L"fcgLBBatAfterPath";
			this->fcgLBBatAfterPath->Size = System::Drawing::Size(74, 17);
			this->fcgLBBatAfterPath->TabIndex = 9;
			this->fcgLBBatAfterPath->Text = L"バッチファイル";
			// 
			// fcgCBWaitForBatAfter
			// 
			this->fcgCBWaitForBatAfter->AutoSize = true;
			this->fcgCBWaitForBatAfter->Location = System::Drawing::Point(45, 145);
			this->fcgCBWaitForBatAfter->Name = L"fcgCBWaitForBatAfter";
			this->fcgCBWaitForBatAfter->Size = System::Drawing::Size(176, 21);
			this->fcgCBWaitForBatAfter->TabIndex = 8;
			this->fcgCBWaitForBatAfter->Tag = L"chValue";
			this->fcgCBWaitForBatAfter->Text = L"バッチ処理の終了を待機する";
			this->fcgCBWaitForBatAfter->UseVisualStyleBackColor = true;
			// 
			// fcgCBRunBatAfter
			// 
			this->fcgCBRunBatAfter->AutoSize = true;
			this->fcgCBRunBatAfter->Location = System::Drawing::Point(20, 118);
			this->fcgCBRunBatAfter->Name = L"fcgCBRunBatAfter";
			this->fcgCBRunBatAfter->Size = System::Drawing::Size(210, 21);
			this->fcgCBRunBatAfter->TabIndex = 7;
			this->fcgCBRunBatAfter->Tag = L"chValue";
			this->fcgCBRunBatAfter->Text = L"エンコード終了後、バッチ処理を行う";
			this->fcgCBRunBatAfter->UseVisualStyleBackColor = true;
			// 
			// fcgPNSeparator
			// 
			this->fcgPNSeparator->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->fcgPNSeparator->Location = System::Drawing::Point(20, 106);
			this->fcgPNSeparator->Name = L"fcgPNSeparator";
			this->fcgPNSeparator->Size = System::Drawing::Size(384, 1);
			this->fcgPNSeparator->TabIndex = 6;
			// 
			// fcgBTBatBeforePath
			// 
			this->fcgBTBatBeforePath->Location = System::Drawing::Point(371, 69);
			this->fcgBTBatBeforePath->Name = L"fcgBTBatBeforePath";
			this->fcgBTBatBeforePath->Size = System::Drawing::Size(34, 26);
			this->fcgBTBatBeforePath->TabIndex = 5;
			this->fcgBTBatBeforePath->Tag = L"chValue";
			this->fcgBTBatBeforePath->Text = L"...";
			this->fcgBTBatBeforePath->UseVisualStyleBackColor = true;
			this->fcgBTBatBeforePath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatBeforePath_Click);
			// 
			// fcgTXBatBeforePath
			// 
			this->fcgTXBatBeforePath->AllowDrop = true;
			this->fcgTXBatBeforePath->Location = System::Drawing::Point(142, 70);
			this->fcgTXBatBeforePath->Name = L"fcgTXBatBeforePath";
			this->fcgTXBatBeforePath->Size = System::Drawing::Size(227, 23);
			this->fcgTXBatBeforePath->TabIndex = 4;
			this->fcgTXBatBeforePath->Tag = L"chValue";
			this->fcgTXBatBeforePath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
			this->fcgTXBatBeforePath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
			// 
			// fcgLBBatBeforePath
			// 
			this->fcgLBBatBeforePath->AutoSize = true;
			this->fcgLBBatBeforePath->Location = System::Drawing::Point(45, 73);
			this->fcgLBBatBeforePath->Name = L"fcgLBBatBeforePath";
			this->fcgLBBatBeforePath->Size = System::Drawing::Size(74, 17);
			this->fcgLBBatBeforePath->TabIndex = 2;
			this->fcgLBBatBeforePath->Text = L"バッチファイル";
			// 
			// fcgCBWaitForBatBefore
			// 
			this->fcgCBWaitForBatBefore->AutoSize = true;
			this->fcgCBWaitForBatBefore->Location = System::Drawing::Point(45, 40);
			this->fcgCBWaitForBatBefore->Name = L"fcgCBWaitForBatBefore";
			this->fcgCBWaitForBatBefore->Size = System::Drawing::Size(176, 21);
			this->fcgCBWaitForBatBefore->TabIndex = 1;
			this->fcgCBWaitForBatBefore->Tag = L"chValue";
			this->fcgCBWaitForBatBefore->Text = L"バッチ処理の終了を待機する";
			this->fcgCBWaitForBatBefore->UseVisualStyleBackColor = true;
			// 
			// fcgCBRunBatBefore
			// 
			this->fcgCBRunBatBefore->AutoSize = true;
			this->fcgCBRunBatBefore->Location = System::Drawing::Point(20, 14);
			this->fcgCBRunBatBefore->Name = L"fcgCBRunBatBefore";
			this->fcgCBRunBatBefore->Size = System::Drawing::Size(210, 21);
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
			this->fcgTXCmd->Location = System::Drawing::Point(10, 615);
			this->fcgTXCmd->Name = L"fcgTXCmd";
			this->fcgTXCmd->ReadOnly = true;
			this->fcgTXCmd->Size = System::Drawing::Size(1116, 23);
			this->fcgTXCmd->TabIndex = 4;
			this->fcgTXCmd->DoubleClick += gcnew System::EventHandler(this, &frmConfig::fcgTXCmd_DoubleClick);
			// 
			// fcgBTCancel
			// 
			this->fcgBTCancel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->fcgBTCancel->Location = System::Drawing::Point(867, 642);
			this->fcgBTCancel->Name = L"fcgBTCancel";
			this->fcgBTCancel->Size = System::Drawing::Size(94, 32);
			this->fcgBTCancel->TabIndex = 5;
			this->fcgBTCancel->Text = L"キャンセル";
			this->fcgBTCancel->UseVisualStyleBackColor = true;
			this->fcgBTCancel->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTCancel_Click);
			// 
			// fcgBTOK
			// 
			this->fcgBTOK->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->fcgBTOK->Location = System::Drawing::Point(1005, 642);
			this->fcgBTOK->Name = L"fcgBTOK";
			this->fcgBTOK->Size = System::Drawing::Size(94, 32);
			this->fcgBTOK->TabIndex = 6;
			this->fcgBTOK->Text = L"OK";
			this->fcgBTOK->UseVisualStyleBackColor = true;
			this->fcgBTOK->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTOK_Click);
			// 
			// fcgBTDefault
			// 
			this->fcgBTDefault->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->fcgBTDefault->Location = System::Drawing::Point(10, 645);
			this->fcgBTDefault->Name = L"fcgBTDefault";
			this->fcgBTDefault->Size = System::Drawing::Size(126, 32);
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
			this->fcgLBVersionDate->Location = System::Drawing::Point(468, 652);
			this->fcgLBVersionDate->Name = L"fcgLBVersionDate";
			this->fcgLBVersionDate->Size = System::Drawing::Size(59, 18);
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
			this->fcgLBVersion->Location = System::Drawing::Point(202, 652);
			this->fcgLBVersion->Name = L"fcgLBVersion";
			this->fcgLBVersion->Size = System::Drawing::Size(59, 18);
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
			this->fcgCSCQM->Size = System::Drawing::Size(127, 76);
			// 
			// fcgCSFlat
			// 
			this->fcgCSFlat->Name = L"fcgCSFlat";
			this->fcgCSFlat->Size = System::Drawing::Size(126, 22);
			this->fcgCSFlat->Tag = L"0";
			this->fcgCSFlat->Text = L"flat";
			this->fcgCSFlat->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::CSCqmCheckedChanged);
			this->fcgCSFlat->Click += gcnew System::EventHandler(this, &frmConfig::CSCqmClick);
			// 
			// fcgCSJvt
			// 
			this->fcgCSJvt->Name = L"fcgCSJvt";
			this->fcgCSJvt->Size = System::Drawing::Size(126, 22);
			this->fcgCSJvt->Tag = L"1";
			this->fcgCSJvt->Text = L"jvt";
			this->fcgCSJvt->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::CSCqmCheckedChanged);
			this->fcgCSJvt->Click += gcnew System::EventHandler(this, &frmConfig::CSCqmClick);
			// 
			// toolStripSeparator1
			// 
			this->toolStripSeparator1->Name = L"toolStripSeparator1";
			this->toolStripSeparator1->Size = System::Drawing::Size(123, 6);
			// 
			// fcgCSCqmFile
			// 
			this->fcgCSCqmFile->Name = L"fcgCSCqmFile";
			this->fcgCSCqmFile->Size = System::Drawing::Size(126, 22);
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
			this->fcgLBguiExBlog->Location = System::Drawing::Point(701, 652);
			this->fcgLBguiExBlog->Name = L"fcgLBguiExBlog";
			this->fcgLBguiExBlog->Size = System::Drawing::Size(114, 17);
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
			this->fcgtabControlAudio->Location = System::Drawing::Point(700, 28);
			this->fcgtabControlAudio->Name = L"fcgtabControlAudio";
			this->fcgtabControlAudio->SelectedIndex = 0;
			this->fcgtabControlAudio->Size = System::Drawing::Size(432, 333);
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
			this->fcgtabPageAudioMain->Location = System::Drawing::Point(4, 24);
			this->fcgtabPageAudioMain->Name = L"fcgtabPageAudioMain";
			this->fcgtabPageAudioMain->Padding = System::Windows::Forms::Padding(3);
			this->fcgtabPageAudioMain->Size = System::Drawing::Size(424, 305);
			this->fcgtabPageAudioMain->TabIndex = 0;
			this->fcgtabPageAudioMain->Text = L"音声";
			this->fcgtabPageAudioMain->UseVisualStyleBackColor = true;
			this->fcgtabPageAudioMain->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTAudioEncoderPath_Click);
			// 
			// fcgCXAudioDelayCut
			// 
			this->fcgCXAudioDelayCut->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXAudioDelayCut->FormattingEnabled = true;
			this->fcgCXAudioDelayCut->Location = System::Drawing::Point(327, 150);
			this->fcgCXAudioDelayCut->Name = L"fcgCXAudioDelayCut";
			this->fcgCXAudioDelayCut->Size = System::Drawing::Size(78, 23);
			this->fcgCXAudioDelayCut->TabIndex = 43;
			this->fcgCXAudioDelayCut->Tag = L"chValue";
			// 
			// fcgLBAudioDelayCut
			// 
			this->fcgLBAudioDelayCut->AutoSize = true;
			this->fcgLBAudioDelayCut->Location = System::Drawing::Point(252, 153);
			this->fcgLBAudioDelayCut->Name = L"fcgLBAudioDelayCut";
			this->fcgLBAudioDelayCut->Size = System::Drawing::Size(71, 17);
			this->fcgLBAudioDelayCut->TabIndex = 54;
			this->fcgLBAudioDelayCut->Text = L"ディレイカット";
			// 
			// fcgCBAudioEncTiming
			// 
			this->fcgCBAudioEncTiming->AutoSize = true;
			this->fcgCBAudioEncTiming->Location = System::Drawing::Point(272, 61);
			this->fcgCBAudioEncTiming->Name = L"fcgCBAudioEncTiming";
			this->fcgCBAudioEncTiming->Size = System::Drawing::Size(47, 17);
			this->fcgCBAudioEncTiming->TabIndex = 53;
			this->fcgCBAudioEncTiming->Text = L"処理順";
			// 
			// fcgCXAudioEncTiming
			// 
			this->fcgCXAudioEncTiming->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXAudioEncTiming->FormattingEnabled = true;
			this->fcgCXAudioEncTiming->Location = System::Drawing::Point(322, 57);
			this->fcgCXAudioEncTiming->Name = L"fcgCXAudioEncTiming";
			this->fcgCXAudioEncTiming->Size = System::Drawing::Size(76, 23);
			this->fcgCXAudioEncTiming->TabIndex = 52;
			this->fcgCXAudioEncTiming->Tag = L"chValue";
			// 
			// fcgCXAudioTempDir
			// 
			this->fcgCXAudioTempDir->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXAudioTempDir->FormattingEnabled = true;
			this->fcgCXAudioTempDir->Location = System::Drawing::Point(152, 234);
			this->fcgCXAudioTempDir->Name = L"fcgCXAudioTempDir";
			this->fcgCXAudioTempDir->Size = System::Drawing::Size(168, 23);
			this->fcgCXAudioTempDir->TabIndex = 46;
			this->fcgCXAudioTempDir->Tag = L"chValue";
			// 
			// fcgTXCustomAudioTempDir
			// 
			this->fcgTXCustomAudioTempDir->Location = System::Drawing::Point(72, 265);
			this->fcgTXCustomAudioTempDir->Name = L"fcgTXCustomAudioTempDir";
			this->fcgTXCustomAudioTempDir->Size = System::Drawing::Size(275, 23);
			this->fcgTXCustomAudioTempDir->TabIndex = 47;
			this->fcgTXCustomAudioTempDir->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXCustomAudioTempDir_TextChanged);
			// 
			// fcgBTCustomAudioTempDir
			// 
			this->fcgBTCustomAudioTempDir->Location = System::Drawing::Point(355, 263);
			this->fcgBTCustomAudioTempDir->Name = L"fcgBTCustomAudioTempDir";
			this->fcgBTCustomAudioTempDir->Size = System::Drawing::Size(33, 26);
			this->fcgBTCustomAudioTempDir->TabIndex = 49;
			this->fcgBTCustomAudioTempDir->Text = L"...";
			this->fcgBTCustomAudioTempDir->UseVisualStyleBackColor = true;
			this->fcgBTCustomAudioTempDir->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTCustomAudioTempDir_Click);
			// 
			// fcgCBAudioUsePipe
			// 
			this->fcgCBAudioUsePipe->AutoSize = true;
			this->fcgCBAudioUsePipe->Location = System::Drawing::Point(146, 151);
			this->fcgCBAudioUsePipe->Name = L"fcgCBAudioUsePipe";
			this->fcgCBAudioUsePipe->Size = System::Drawing::Size(83, 21);
			this->fcgCBAudioUsePipe->TabIndex = 42;
			this->fcgCBAudioUsePipe->Tag = L"chValue";
			this->fcgCBAudioUsePipe->Text = L"パイプ処理";
			this->fcgCBAudioUsePipe->UseVisualStyleBackColor = true;
			// 
			// fcgLBAudioBitrate
			// 
			this->fcgLBAudioBitrate->AutoSize = true;
			this->fcgLBAudioBitrate->Location = System::Drawing::Point(319, 181);
			this->fcgLBAudioBitrate->Name = L"fcgLBAudioBitrate";
			this->fcgLBAudioBitrate->Size = System::Drawing::Size(37, 17);
			this->fcgLBAudioBitrate->TabIndex = 50;
			this->fcgLBAudioBitrate->Text = L"kbps";
			// 
			// fcgNUAudioBitrate
			// 
			this->fcgNUAudioBitrate->Location = System::Drawing::Point(239, 177);
			this->fcgNUAudioBitrate->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1536, 0, 0, 0 });
			this->fcgNUAudioBitrate->Name = L"fcgNUAudioBitrate";
			this->fcgNUAudioBitrate->Size = System::Drawing::Size(73, 23);
			this->fcgNUAudioBitrate->TabIndex = 40;
			this->fcgNUAudioBitrate->Tag = L"chValue";
			this->fcgNUAudioBitrate->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			// 
			// fcgCBAudio2pass
			// 
			this->fcgCBAudio2pass->AutoSize = true;
			this->fcgCBAudio2pass->Location = System::Drawing::Point(66, 151);
			this->fcgCBAudio2pass->Name = L"fcgCBAudio2pass";
			this->fcgCBAudio2pass->Size = System::Drawing::Size(63, 21);
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
			this->fcgCXAudioEncMode->Location = System::Drawing::Point(18, 175);
			this->fcgCXAudioEncMode->Name = L"fcgCXAudioEncMode";
			this->fcgCXAudioEncMode->Size = System::Drawing::Size(212, 23);
			this->fcgCXAudioEncMode->TabIndex = 39;
			this->fcgCXAudioEncMode->Tag = L"chValue";
			this->fcgCXAudioEncMode->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXAudioEncMode_SelectedIndexChanged);
			// 
			// fcgLBAudioEncMode
			// 
			this->fcgLBAudioEncMode->AutoSize = true;
			this->fcgLBAudioEncMode->Location = System::Drawing::Point(5, 153);
			this->fcgLBAudioEncMode->Name = L"fcgLBAudioEncMode";
			this->fcgLBAudioEncMode->Size = System::Drawing::Size(38, 17);
			this->fcgLBAudioEncMode->TabIndex = 48;
			this->fcgLBAudioEncMode->Text = L"モード";
			// 
			// fcgBTAudioEncoderPath
			// 
			this->fcgBTAudioEncoderPath->Location = System::Drawing::Point(365, 101);
			this->fcgBTAudioEncoderPath->Name = L"fcgBTAudioEncoderPath";
			this->fcgBTAudioEncoderPath->Size = System::Drawing::Size(34, 26);
			this->fcgBTAudioEncoderPath->TabIndex = 38;
			this->fcgBTAudioEncoderPath->Text = L"...";
			this->fcgBTAudioEncoderPath->UseVisualStyleBackColor = true;
			this->fcgBTAudioEncoderPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTAudioEncoderPath_Click);
			// 
			// fcgTXAudioEncoderPath
			// 
			this->fcgTXAudioEncoderPath->AllowDrop = true;
			this->fcgTXAudioEncoderPath->Location = System::Drawing::Point(18, 103);
			this->fcgTXAudioEncoderPath->Name = L"fcgTXAudioEncoderPath";
			this->fcgTXAudioEncoderPath->Size = System::Drawing::Size(340, 23);
			this->fcgTXAudioEncoderPath->TabIndex = 37;
			this->fcgTXAudioEncoderPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXAudioEncoderPath_TextChanged);
			this->fcgTXAudioEncoderPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
			this->fcgTXAudioEncoderPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
			// 
			// fcgLBAudioEncoderPath
			// 
			this->fcgLBAudioEncoderPath->AutoSize = true;
			this->fcgLBAudioEncoderPath->Location = System::Drawing::Point(14, 84);
			this->fcgLBAudioEncoderPath->Name = L"fcgLBAudioEncoderPath";
			this->fcgLBAudioEncoderPath->Size = System::Drawing::Size(58, 17);
			this->fcgLBAudioEncoderPath->TabIndex = 44;
			this->fcgLBAudioEncoderPath->Text = L"～の指定";
			// 
			// fcgCBAudioOnly
			// 
			this->fcgCBAudioOnly->AutoSize = true;
			this->fcgCBAudioOnly->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
			this->fcgCBAudioOnly->Location = System::Drawing::Point(284, 6);
			this->fcgCBAudioOnly->Name = L"fcgCBAudioOnly";
			this->fcgCBAudioOnly->Size = System::Drawing::Size(102, 21);
			this->fcgCBAudioOnly->TabIndex = 34;
			this->fcgCBAudioOnly->Tag = L"chValue";
			this->fcgCBAudioOnly->Text = L"音声のみ出力";
			this->fcgCBAudioOnly->UseVisualStyleBackColor = true;
			// 
			// fcgCBFAWCheck
			// 
			this->fcgCBFAWCheck->AutoSize = true;
			this->fcgCBFAWCheck->Location = System::Drawing::Point(284, 31);
			this->fcgCBFAWCheck->Name = L"fcgCBFAWCheck";
			this->fcgCBFAWCheck->Size = System::Drawing::Size(94, 21);
			this->fcgCBFAWCheck->TabIndex = 36;
			this->fcgCBFAWCheck->Tag = L"chValue";
			this->fcgCBFAWCheck->Text = L"FAWCheck";
			this->fcgCBFAWCheck->UseVisualStyleBackColor = true;
			// 
			// fcgCXAudioEncoder
			// 
			this->fcgCXAudioEncoder->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXAudioEncoder->FormattingEnabled = true;
			this->fcgCXAudioEncoder->Location = System::Drawing::Point(19, 38);
			this->fcgCXAudioEncoder->Name = L"fcgCXAudioEncoder";
			this->fcgCXAudioEncoder->Size = System::Drawing::Size(193, 23);
			this->fcgCXAudioEncoder->TabIndex = 32;
			this->fcgCXAudioEncoder->Tag = L"chValue";
			this->fcgCXAudioEncoder->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXAudioEncoder_SelectedIndexChanged);
			// 
			// fcgLBAudioEncoder
			// 
			this->fcgLBAudioEncoder->AutoSize = true;
			this->fcgLBAudioEncoder->Location = System::Drawing::Point(6, 16);
			this->fcgLBAudioEncoder->Name = L"fcgLBAudioEncoder";
			this->fcgLBAudioEncoder->Size = System::Drawing::Size(57, 17);
			this->fcgLBAudioEncoder->TabIndex = 33;
			this->fcgLBAudioEncoder->Text = L"エンコーダ";
			// 
			// fcgLBAudioTemp
			// 
			this->fcgLBAudioTemp->AutoSize = true;
			this->fcgLBAudioTemp->Location = System::Drawing::Point(8, 237);
			this->fcgLBAudioTemp->Name = L"fcgLBAudioTemp";
			this->fcgLBAudioTemp->Size = System::Drawing::Size(136, 17);
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
			this->fcgtabPageAudioOther->Location = System::Drawing::Point(4, 24);
			this->fcgtabPageAudioOther->Name = L"fcgtabPageAudioOther";
			this->fcgtabPageAudioOther->Padding = System::Windows::Forms::Padding(3);
			this->fcgtabPageAudioOther->Size = System::Drawing::Size(424, 305);
			this->fcgtabPageAudioOther->TabIndex = 1;
			this->fcgtabPageAudioOther->Text = L"その他";
			this->fcgtabPageAudioOther->UseVisualStyleBackColor = true;
			// 
			// panel2
			// 
			this->panel2->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->panel2->Location = System::Drawing::Point(20, 142);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(384, 1);
			this->panel2->TabIndex = 61;
			// 
			// fcgLBBatAfterAudioString
			// 
			this->fcgLBBatAfterAudioString->AutoSize = true;
			this->fcgLBBatAfterAudioString->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Italic | System::Drawing::FontStyle::Underline)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(128)));
			this->fcgLBBatAfterAudioString->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
			this->fcgLBBatAfterAudioString->Location = System::Drawing::Point(342, 234);
			this->fcgLBBatAfterAudioString->Name = L"fcgLBBatAfterAudioString";
			this->fcgLBBatAfterAudioString->Size = System::Drawing::Size(32, 18);
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
			this->fcgLBBatBeforeAudioString->Location = System::Drawing::Point(342, 156);
			this->fcgLBBatBeforeAudioString->Name = L"fcgLBBatBeforeAudioString";
			this->fcgLBBatBeforeAudioString->Size = System::Drawing::Size(32, 18);
			this->fcgLBBatBeforeAudioString->TabIndex = 51;
			this->fcgLBBatBeforeAudioString->Text = L" 前& ";
			this->fcgLBBatBeforeAudioString->TextAlign = System::Drawing::ContentAlignment::TopCenter;
			// 
			// fcgBTBatAfterAudioPath
			// 
			this->fcgBTBatAfterAudioPath->Location = System::Drawing::Point(371, 260);
			this->fcgBTBatAfterAudioPath->Name = L"fcgBTBatAfterAudioPath";
			this->fcgBTBatAfterAudioPath->Size = System::Drawing::Size(34, 26);
			this->fcgBTBatAfterAudioPath->TabIndex = 59;
			this->fcgBTBatAfterAudioPath->Tag = L"chValue";
			this->fcgBTBatAfterAudioPath->Text = L"...";
			this->fcgBTBatAfterAudioPath->UseVisualStyleBackColor = true;
			this->fcgBTBatAfterAudioPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatAfterAudioPath_Click);
			// 
			// fcgTXBatAfterAudioPath
			// 
			this->fcgTXBatAfterAudioPath->AllowDrop = true;
			this->fcgTXBatAfterAudioPath->Location = System::Drawing::Point(142, 261);
			this->fcgTXBatAfterAudioPath->Name = L"fcgTXBatAfterAudioPath";
			this->fcgTXBatAfterAudioPath->Size = System::Drawing::Size(227, 23);
			this->fcgTXBatAfterAudioPath->TabIndex = 58;
			this->fcgTXBatAfterAudioPath->Tag = L"chValue";
			this->fcgTXBatAfterAudioPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
			this->fcgTXBatAfterAudioPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
			// 
			// fcgLBBatAfterAudioPath
			// 
			this->fcgLBBatAfterAudioPath->AutoSize = true;
			this->fcgLBBatAfterAudioPath->Location = System::Drawing::Point(45, 265);
			this->fcgLBBatAfterAudioPath->Name = L"fcgLBBatAfterAudioPath";
			this->fcgLBBatAfterAudioPath->Size = System::Drawing::Size(74, 17);
			this->fcgLBBatAfterAudioPath->TabIndex = 57;
			this->fcgLBBatAfterAudioPath->Text = L"バッチファイル";
			// 
			// fcgCBRunBatAfterAudio
			// 
			this->fcgCBRunBatAfterAudio->AutoSize = true;
			this->fcgCBRunBatAfterAudio->Location = System::Drawing::Point(20, 233);
			this->fcgCBRunBatAfterAudio->Name = L"fcgCBRunBatAfterAudio";
			this->fcgCBRunBatAfterAudio->Size = System::Drawing::Size(236, 21);
			this->fcgCBRunBatAfterAudio->TabIndex = 55;
			this->fcgCBRunBatAfterAudio->Tag = L"chValue";
			this->fcgCBRunBatAfterAudio->Text = L"音声エンコード終了後、バッチ処理を行う";
			this->fcgCBRunBatAfterAudio->UseVisualStyleBackColor = true;
			// 
			// panel1
			// 
			this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->panel1->Location = System::Drawing::Point(20, 221);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(384, 1);
			this->panel1->TabIndex = 54;
			// 
			// fcgBTBatBeforeAudioPath
			// 
			this->fcgBTBatBeforeAudioPath->Location = System::Drawing::Point(371, 184);
			this->fcgBTBatBeforeAudioPath->Name = L"fcgBTBatBeforeAudioPath";
			this->fcgBTBatBeforeAudioPath->Size = System::Drawing::Size(34, 26);
			this->fcgBTBatBeforeAudioPath->TabIndex = 53;
			this->fcgBTBatBeforeAudioPath->Tag = L"chValue";
			this->fcgBTBatBeforeAudioPath->Text = L"...";
			this->fcgBTBatBeforeAudioPath->UseVisualStyleBackColor = true;
			this->fcgBTBatBeforeAudioPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatBeforeAudioPath_Click);
			// 
			// fcgTXBatBeforeAudioPath
			// 
			this->fcgTXBatBeforeAudioPath->AllowDrop = true;
			this->fcgTXBatBeforeAudioPath->Location = System::Drawing::Point(142, 185);
			this->fcgTXBatBeforeAudioPath->Name = L"fcgTXBatBeforeAudioPath";
			this->fcgTXBatBeforeAudioPath->Size = System::Drawing::Size(227, 23);
			this->fcgTXBatBeforeAudioPath->TabIndex = 52;
			this->fcgTXBatBeforeAudioPath->Tag = L"chValue";
			this->fcgTXBatBeforeAudioPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
			this->fcgTXBatBeforeAudioPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
			// 
			// fcgLBBatBeforeAudioPath
			// 
			this->fcgLBBatBeforeAudioPath->AutoSize = true;
			this->fcgLBBatBeforeAudioPath->Location = System::Drawing::Point(45, 188);
			this->fcgLBBatBeforeAudioPath->Name = L"fcgLBBatBeforeAudioPath";
			this->fcgLBBatBeforeAudioPath->Size = System::Drawing::Size(74, 17);
			this->fcgLBBatBeforeAudioPath->TabIndex = 50;
			this->fcgLBBatBeforeAudioPath->Text = L"バッチファイル";
			// 
			// fcgCBRunBatBeforeAudio
			// 
			this->fcgCBRunBatBeforeAudio->AutoSize = true;
			this->fcgCBRunBatBeforeAudio->Location = System::Drawing::Point(20, 156);
			this->fcgCBRunBatBeforeAudio->Name = L"fcgCBRunBatBeforeAudio";
			this->fcgCBRunBatBeforeAudio->Size = System::Drawing::Size(236, 21);
			this->fcgCBRunBatBeforeAudio->TabIndex = 48;
			this->fcgCBRunBatBeforeAudio->Tag = L"chValue";
			this->fcgCBRunBatBeforeAudio->Text = L"音声エンコード開始前、バッチ処理を行う";
			this->fcgCBRunBatBeforeAudio->UseVisualStyleBackColor = true;
			// 
			// fcgCXAudioPriority
			// 
			this->fcgCXAudioPriority->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->fcgCXAudioPriority->FormattingEnabled = true;
			this->fcgCXAudioPriority->Location = System::Drawing::Point(176, 23);
			this->fcgCXAudioPriority->Name = L"fcgCXAudioPriority";
			this->fcgCXAudioPriority->Size = System::Drawing::Size(152, 23);
			this->fcgCXAudioPriority->TabIndex = 47;
			this->fcgCXAudioPriority->Tag = L"chValue";
			// 
			// fcgLBAudioPriority
			// 
			this->fcgLBAudioPriority->AutoSize = true;
			this->fcgLBAudioPriority->Location = System::Drawing::Point(33, 26);
			this->fcgLBAudioPriority->Name = L"fcgLBAudioPriority";
			this->fcgLBAudioPriority->Size = System::Drawing::Size(73, 17);
			this->fcgLBAudioPriority->TabIndex = 46;
			this->fcgLBAudioPriority->Text = L"音声優先度";
			// 
			// frmConfig
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(108, 108);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;
			this->ClientSize = System::Drawing::Size(1134, 677);
			this->Controls->Add(this->fcgtabControlAudio);
			this->Controls->Add(this->fcgLBguiExBlog);
			this->Controls->Add(this->fcgLBVersion);
			this->Controls->Add(this->fcgLBVersionDate);
			this->Controls->Add(this->fcgBTDefault);
			this->Controls->Add(this->fcgBTOK);
			this->Controls->Add(this->fcgBTCancel);
			this->Controls->Add(this->fcgTXCmd);
			this->Controls->Add(this->fcgtabControlMux);
			this->Controls->Add(this->fcgtoolStripSettings);
			this->Controls->Add(this->fcgtabControlVideo);
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
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAMPLimitBitrate))->EndInit();
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
			this->fcggroupBoxAepectRatio->ResumeLayout(false);
			this->fcggroupBoxAepectRatio->PerformLayout();
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
			this->fcgtoolStripSettings->ResumeLayout(false);
			this->fcgtoolStripSettings->PerformLayout();
			this->fcgtabControlMux->ResumeLayout(false);
			this->fcgtabPageMP4->ResumeLayout(false);
			this->fcgtabPageMP4->PerformLayout();
			this->fcgtabPageMKV->ResumeLayout(false);
			this->fcgtabPageMKV->PerformLayout();
			this->fcgtabPageMPG->ResumeLayout(false);
			this->fcgtabPageMPG->PerformLayout();
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
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private:
		CONF_X264 *cnf_fcgTemp;
		const SYSTEM_DATA *sys_dat;
		CONF_GUIEX *conf;
		LocalSettings LocalStg;
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
		System::Int32 GetCurrentAudioDefaultBitrate();
		delegate System::Void qualityTimerChangeDelegate();
		System::Void qualityTimerChange(Object^ state);
		System::Void fcgTBQualityChange();
		System::Void InitTimer();
		System::Void InitComboBox();
		System::Void setAudioDisplay();
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
		System::Void GetfcgTSLSettingsNotes(char *notes, int nSize);
		System::Void SetfcgTSLSettingsNotes(const char *notes);
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
		System::Void SetHelpToolTips();
		System::Void SetHelpToolTipsColorMatrix(Control^ control, const char *type);
		System::Void SetX264VersionToolTip(String^ x264Path, bool ashighbit);
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
			fcgLBAMPAutoBitrate->Text = L"※目標ビットレートを自動にするには\n　　上限設定が必要です。";
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
		System::Void setComboBox(ComboBox^ CX, const X264_OPTION_STR * list) {
			CX->BeginUpdate();
			CX->Items->Clear();
			for (int i = 0; list[i].desc; i++)
				CX->Items->Add(String(list[i].desc).ToString());
			CX->EndUpdate();
		}
	private:
		System::Void setComboBox(ComboBox^ CX, const char * const * list) {
			CX->BeginUpdate();
			CX->Items->Clear();
			for (int i = 0; list[i]; i++)
				CX->Items->Add(String(list[i]).ToString());
			CX->EndUpdate();
		}
	private:
		System::Void setComboBox(ComboBox^ CX, const WCHAR * const * list) {
			CX->BeginUpdate();
			CX->Items->Clear();
			for (int i = 0; list[i]; i++)
				CX->Items->Add(String(list[i]).ToString());
			CX->EndUpdate();
		}
	private:
		System::Void setPriorityList(ComboBox^ CX) {
			CX->BeginUpdate();
			CX->Items->Clear();
			for (int i = 0; priority_table[i].text; i++)
				CX->Items->Add(String(priority_table[i].text).ToString());
			CX->EndUpdate();
		}
	private:
		System::Void setMuxerCmdExNames(ComboBox^ CX, int muxer_index) {
			CX->BeginUpdate();
			CX->Items->Clear();
			MUXER_SETTINGS *mstg = &sys_dat->exstg->s_mux[muxer_index];
			for (int i = 0; i < mstg->ex_count; i++)
				CX->Items->Add(String(mstg->ex_cmd[i].name).ToString());
			CX->EndUpdate();
		}
	private:
		System::Void setAudioEncoderNames() {
			fcgCXAudioEncoder->BeginUpdate();
			fcgCXAudioEncoder->Items->Clear();
			//fcgCXAudioEncoder->Items->AddRange(reinterpret_cast<array<String^>^>(LocalStg.audEncName->ToArray(String::typeid)));
			fcgCXAudioEncoder->Items->AddRange(LocalStg.audEncName->ToArray());
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
				MessageBox::Show(this, L"入力された文字数が多すぎます。減らしてください。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
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
			bool ret = (ofd.ShowDialog() == Windows::Forms::DialogResult::OK);
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
				SetX264VersionToolTip(fcgTXX264Path->Text, fcgCBUsehighbit->Checked);
		}
	private: 
		System::Void fcgBTX264PathSub_Click(System::Object^  sender, System::EventArgs^  e) {
			if (openExeFile(fcgTXX264PathSub, LocalStg.x264ExeName))
				SetX264VersionToolTip(fcgTXX264PathSub->Text, false);
		}
	private: 
		System::Void fcgBTX264PathSubhighbit_Click(System::Object^  sender, System::EventArgs^  e) {
			if (openExeFile(fcgTXX264PathSubhighbit, LocalStg.x264ExeName))
				SetX264VersionToolTip(fcgTXX264PathSubhighbit->Text, true);
		}
	private: 
		System::Void fcgTXX264Path_Leave(System::Object^  sender, System::EventArgs^  e) {
			SetX264VersionToolTip(fcgTXX264Path->Text, fcgCBUsehighbit->Checked);
		}
	private: 
		System::Void fcgTXX264PathSub_Leave(System::Object^  sender, System::EventArgs^  e) {
			SetX264VersionToolTip(fcgTXX264PathSub->Text, false);
		}
	private: 
		System::Void fcgTXX264PathSubhighbit_Leave(System::Object^  sender, System::EventArgs^  e) {
			SetX264VersionToolTip(fcgTXX264PathSubhighbit->Text, true);
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
		System::Void fcgBTMPGMuxerPath_Click(System::Object^  sender, System::EventArgs^  e) {
			openExeFile(fcgTXMPGMuxerPath, LocalStg.MPGMuxerExeName);
		}
	private:
		System::Void openTempFolder(TextBox^ TX) {
			FolderBrowserDialog^ fbd = fcgfolderBrowserTemp;
			if (Directory::Exists(TX->Text))
				fbd->SelectedPath = TX->Text;
			if (fbd->ShowDialog() == Windows::Forms::DialogResult::OK) {
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
			bool ret = (ofd->ShowDialog() == Windows::Forms::DialogResult::OK);
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
			openAndSetFilePath(fcgTXStatusFile, L"ステータスファイル");
		}
	private: 
		System::Void fcgBTTCIN_Click(System::Object^  sender, System::EventArgs^  e) {
			openAndSetFilePath(fcgTXTCIN, L"タイムコードファイル");
		}
	private:
		System::Void fcgBTBatBeforePath_Click(System::Object^  sender, System::EventArgs^  e) {
			if (openAndSetFilePath(fcgTXBatBeforePath, L"バッチファイル", ".bat", LocalStg.LastBatDir))
				LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatBeforePath->Text);
		}
	private:
		System::Void fcgBTBatAfterPath_Click(System::Object^  sender, System::EventArgs^  e) {
			if (openAndSetFilePath(fcgTXBatAfterPath, L"バッチファイル", ".bat", LocalStg.LastBatDir))
				LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatAfterPath->Text);
		}
	private:
		System::Void fcgBTBatBeforeAudioPath_Click(System::Object^  sender, System::EventArgs^  e) {
			if (openAndSetFilePath(fcgTXBatBeforeAudioPath, L"バッチファイル", ".bat", LocalStg.LastBatDir))
				LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatBeforeAudioPath->Text);
		}
	private:
		System::Void fcgBTBatAfterAudioPath_Click(System::Object^  sender, System::EventArgs^  e) {
			if (openAndSetFilePath(fcgTXBatAfterAudioPath, L"バッチファイル", ".bat", LocalStg.LastBatDir))
				LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatAfterAudioPath->Text);
		}
	private:
		System::Void SetCXIndex(ComboBox^ CX, int index) {
			CX->SelectedIndex = clamp(index, 0, CX->Items->Count - 1);
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
				if (!(openAndSetFilePath(fcgTXCQM, L"マトリックスファイル")))
					return;
			fcgTXCQM->SelectionStart = fcgTXCQM->Text->Length;
			fcgCSFlat->Checked = false;
			fcgCSJvt->Checked = false;
			fcgCSCqmFile->Checked = false;
			item->Checked = true;
			fcgRebuildCmd(sender, e);
		}
	private:
		System::Void SetCQM(int index, const char *cqmfile) {
			for (int i = 0; i < fcgCSCQM->Items->Count; i++) {
				ToolStripMenuItem^ TSItem = dynamic_cast<ToolStripMenuItem^>(fcgCSCQM->Items[i]);
				if (TSItem != nullptr)
					TSItem->Checked = (index == Convert::ToInt32(TSItem->Tag));
			}
			if (index == 2)
				fcgTXCQM->Text = String(cqmfile).ToString();
		}
	private:
		System::Int32 GetCQMIndex(char *cqmfile, int nSize) {
			for (int i = 0; i < fcgCSCQM->Items->Count; i++) {
				ToolStripMenuItem^ TSItem = dynamic_cast<ToolStripMenuItem^>(fcgCSCQM->Items[i]);
				if (TSItem != nullptr && TSItem->Checked) {
					int index = Convert::ToInt32(TSItem->Tag);
					if (index == 2)
						GetCHARfromString(cqmfile, nSize, fcgTXCQM->Text);
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
			cnf.x264.preset = fcgCXPreset->SelectedIndex;
			cnf.x264.tune = fcgCXTune->SelectedIndex;
			cnf.x264.profile = fcgCXProfile->SelectedIndex;
			apply_presets(&cnf.x264);
			ConfToFrm(&cnf, false);
		}
	private: 
		System::Void fcgBTCmdEx_Click(System::Object^  sender, System::EventArgs^  e) {
			CONF_GUIEX cnf;
			init_CONF_GUIEX(&cnf, fcgCBUsehighbit->Checked);
			FrmToConf(&cnf);
			char cmdex[2048] = { 0 };
			GetCHARfromString(cmdex, sizeof(cmdex), fcgTXCmdEx->Text);
			set_cmd_to_conf(cmdex, &cnf.x264);
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
			char re_cmd[MAX_CMD_LEN] = { 0 };
			CONF_GUIEX rebuild;
			init_CONF_GUIEX(&rebuild, fcgCBUsehighbit->Checked);
			FrmToConf(&rebuild);
			if (!rebuild.oth.disable_guicmd)
				build_cmd_from_conf(re_cmd, sizeof(re_cmd), &rebuild.x264, &rebuild.vid, FALSE);
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
			int c = fcgTXX264Path->SelectionStart;
			if (fcgCBUsehighbit->Checked) {
				LocalStg.x264Pathhighbit = fcgTXX264Path->Text;
				fcgTXX264PathSubhighbit->Text = LocalStg.x264Pathhighbit;
				fcgTXX264PathSubhighbit->SelectionStart = fcgTXX264PathSubhighbit->Text->Length;
			} else {
				LocalStg.x264Path      = fcgTXX264Path->Text;
				fcgTXX264PathSub->Text = LocalStg.x264Path;
				fcgTXX264PathSub->SelectionStart = fcgTXX264PathSub->Text->Length;
			}
			fcgTXX264Path->SelectionStart = c;
			fcgBTX264Path->ContextMenuStrip = (File::Exists(fcgTXX264Path->Text)) ? fcgCSExeFiles : nullptr;
		}
	private: 
		System::Void fcgTXX264PathSub_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			LocalStg.x264Path   = fcgTXX264PathSub->Text;
			int c = fcgTXX264PathSub->SelectionStart;
			if (!fcgCBUsehighbit->Checked) {
				fcgTXX264Path->Text = LocalStg.x264Path;
				fcgTXX264Path->SelectionStart = fcgTXX264Path->Text->Length;
			}
			fcgTXX264PathSub->SelectionStart = c;
			fcgBTX264PathSub->ContextMenuStrip = (File::Exists(fcgTXX264PathSub->Text)) ? fcgCSExeFiles : nullptr;
		}
	private: 
		System::Void fcgTXX264PathSubhighbit_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			LocalStg.x264Pathhighbit   = fcgTXX264PathSubhighbit->Text;
			int c = fcgTXX264PathSubhighbit->SelectionStart;
			if (fcgCBUsehighbit->Checked) {
				fcgTXX264Path->Text = LocalStg.x264Pathhighbit;
				fcgTXX264Path->SelectionStart = fcgTXX264Path->Text->Length;
			}
			fcgTXX264PathSubhighbit->SelectionStart = c;
			fcgBTX264PathSubhighbit->ContextMenuStrip = (File::Exists(fcgTXX264PathSubhighbit->Text)) ? fcgCSExeFiles : nullptr;
		}
	private: 
		System::Void fcgTXAudioEncoderPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			LocalStg.audEncPath[fcgCXAudioEncoder->SelectedIndex] = fcgTXAudioEncoderPath->Text;
			fcgBTAudioEncoderPath->ContextMenuStrip = (File::Exists(fcgTXAudioEncoderPath->Text)) ? fcgCSExeFiles : nullptr;
		}
	private: 
		System::Void fcgTXMP4MuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			LocalStg.MP4MuxerPath = fcgTXMP4MuxerPath->Text;
			fcgBTMP4MuxerPath->ContextMenuStrip = (File::Exists(fcgTXMP4MuxerPath->Text)) ? fcgCSExeFiles : nullptr;
		}
	private: 
		System::Void fcgTXTC2MP4Path_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			LocalStg.TC2MP4Path = fcgTXTC2MP4Path->Text;
			fcgBTTC2MP4Path->ContextMenuStrip = (File::Exists(fcgTXTC2MP4Path->Text)) ? fcgCSExeFiles : nullptr;
		}
	private:
		System::Void fcgTXMP4RawMuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			LocalStg.MP4RawPath = fcgTXMP4RawPath->Text;
			fcgBTMP4RawPath->ContextMenuStrip = (File::Exists(fcgTXMP4RawPath->Text)) ? fcgCSExeFiles : nullptr;
		}
	private: 
		System::Void fcgTXMKVMuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			LocalStg.MKVMuxerPath = fcgTXMKVMuxerPath->Text;
			fcgBTMKVMuxerPath->ContextMenuStrip = (File::Exists(fcgTXMKVMuxerPath->Text)) ? fcgCSExeFiles : nullptr;
		}
	private:
		System::Void fcgTXMPGMuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			LocalStg.MPGMuxerPath = fcgTXMPGMuxerPath->Text;
			fcgBTMPGMuxerPath->ContextMenuStrip = (File::Exists(fcgTXMPGMuxerPath->Text)) ? fcgCSExeFiles : nullptr;
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
				{ fcgBTX264Path->Name,           fcgTXX264Path->Text,           sys_dat->exstg->s_x264.help_cmd },
				{ fcgBTX264PathSub->Name,        fcgTXX264PathSub->Text,        sys_dat->exstg->s_x264.help_cmd },
				{ fcgBTX264PathSubhighbit->Name, fcgTXX264PathSubhighbit->Text, sys_dat->exstg->s_x264.help_cmd },
				{ fcgBTAudioEncoderPath->Name,   fcgTXAudioEncoderPath->Text,   sys_dat->exstg->s_aud[fcgCXAudioEncoder->SelectedIndex].cmd_help },
				{ fcgBTMP4MuxerPath->Name,       fcgTXMP4MuxerPath->Text,       sys_dat->exstg->s_mux[MUXER_MP4].help_cmd },
				{ fcgBTTC2MP4Path->Name,         fcgTXTC2MP4Path->Text,         sys_dat->exstg->s_mux[MUXER_TC2MP4].help_cmd },
				{ fcgBTMP4RawPath->Name,         fcgTXMP4RawPath->Text,         sys_dat->exstg->s_mux[MUXER_MP4_RAW].help_cmd },
				{ fcgBTMKVMuxerPath->Name,       fcgTXMKVMuxerPath->Text,       sys_dat->exstg->s_mux[MUXER_MKV].help_cmd },
				{ fcgBTMPGMuxerPath->Name,       fcgTXMPGMuxerPath->Text,       sys_dat->exstg->s_mux[MUXER_MPG].help_cmd }
			};
			for (int i = 0; i < ControlList->Length; i++) {
				if (NULL == String::Compare(CS->SourceControl->Name, ControlList[i].Name)) {
					ShowExehelp(ControlList[i].Path, String(ControlList[i].args).ToString());
					return;
				}
			}
			MessageBox::Show(L"ヘルプ表示用のコマンドが設定されていません。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
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
