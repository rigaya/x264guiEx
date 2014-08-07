//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#pragma once

#include <Windows.h>
#include <mmsystem.h>

#include "auo.h"
#include "auo_frm.h"
#include "auo_util.h"
#include "auo_version.h"
#include "auo_settings.h"
#include "auo_win7_taskbar.h"


//以下部分的にwarning C4100を黙らせる
//C4100 : 引数は関数の本体部で 1 度も参照されません。
#pragma warning( push )
#pragma warning( disable: 4100 )

#include "frmAutoSaveLogSettings.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Text;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::IO;

namespace x264guiEx {

	/// <summary>
	/// frmLog の概要
	///
	/// 警告: このクラスの名前を変更する場合、このクラスが依存するすべての .resx ファイルに関連付けられた
	///          マネージ リソース コンパイラ ツールに対して 'Resource File Name' プロパティを
	///          変更する必要があります。この変更を行わないと、
	///          デザイナと、このフォームに関連付けられたローカライズ済みリソースとが、
	///          正しく相互に利用できなくなります。
	/// </summary>
	public ref class frmLog : public System::Windows::Forms::Form
	{
	public:
		frmLog(void)
		{
			//これがfalseだとイベントで設定保存をするので、とりあえずtrue
			prevent_log_closing = true;

			//設定から情報を取得
			guiEx_settings exstg;
			exstg.load_log_win();
			_x264_priority = NULL;
			_enc_pause = NULL;
			LogTitle = String(AUO_FULL_NAME).ToString();

			InitializeComponent();
			//
			//TODO: ここにコンストラクタ コードを追加します
			//
			this->log_type = gcnew array<String^>(3) { L"info", L"warning", L"error" };
			this->richTextLog->LanguageOption = System::Windows::Forms::RichTextBoxLanguageOptions::UIFonts;

			//x264優先度メニューを動的生成
			for (int i = 0; priority_table[i].text; i++) {
				if (wcscmp(priority_table[i].text, L"") != NULL) {
					System::Windows::Forms::ToolStripMenuItem^ x264_priority = gcnew System::Windows::Forms::ToolStripMenuItem();
					x264_priority->Name = L"ToolStripItem" + String(priority_table[i].text).ToString();
					x264_priority->Text = String(priority_table[i].text).ToString();
					x264_priority->Checked = (_x264_priority != NULL && *_x264_priority == priority_table[i].value) ? true : false;
					this->ToolStripMenuItemx264Priority->DropDownItems->Add(x264_priority);
				} else {
					this->ToolStripMenuItemx264Priority->DropDownItems->Add(gcnew System::Windows::Forms::ToolStripSeparator());
				}
			}
			this->ToolStripMenuItemx264Priority->Enabled = false;
			this->ToolStripMenuItemEncPause->Enabled = false;
			this->ToolStripMenuItemTransparent->Checked = exstg.s_log.transparent != 0;
			this->toolStripMenuItemAutoSave->Checked = exstg.s_log.auto_save_log != 0;
			this->toolStripMenuItemShowStatus->Checked = exstg.s_log.show_status_bar != 0;
			this->ToolStripMenuItemStartMinimized->Checked = exstg.s_log.minimized != 0;
			this->toolStripMenuItemSaveLogSize->Checked = exstg.s_log.save_log_size != 0;
			bool check_win7later = check_OS_Win7orLater() != 0;
			this->toolStripMenuItemTaskBarProgress->Enabled = check_win7later;
			this->toolStripMenuItemTaskBarProgress->Checked = (exstg.s_log.taskbar_progress != 0 && check_win7later);
			//ウィンドウサイズ調整等(サイズ設定->最小化の設定の順に行うこと)
			if (exstg.s_log.save_log_size)
				SetWindowSize(exstg.s_log.log_width, exstg.s_log.log_height);
			if (exstg.s_log.minimized)
				this->WindowState = FormWindowState::Minimized;
			lastWindowState = this->WindowState;
			//プログレスバーの初期化
			taskbar_progress_enable(exstg.s_log.taskbar_progress);
			hWnd = (HWND)this->Handle.ToInt32();
			taskbar_progress_init();
			//通常のステータスに戻す(false) -> 設定保存イベントで設定保存される
			prevent_log_closing = false;
			closed = true;
		}
	protected:
		/// <summary>
		/// 使用中のリソースをすべてクリーンアップします。
		/// </summary>
		~frmLog()
		{
			if (components)
			{
				delete components;
			}
			delete log_type;
			frmAutoSaveLogSettings::Instance::get()->Close();
		}
	//Instanceを介し、ひとつだけ生成
	private:
		static frmLog^ _instance;
	public:
		static property frmLog^ Instance {
			frmLog^ get() {
				if (_instance == nullptr || _instance->IsDisposed)
					_instance = gcnew frmLog();
				return _instance;
			}
		}
	private:
		HWND hWnd; //このウィンドウのハンドル
		DWORD *_x264_priority; //x264優先度へのポインタ
		BOOL *_enc_pause;      //エンコ一時停止へのポインタ
		DWORD _x264_start_time; //x264エンコ開始時間
		bool closed; //このウィンドウが閉じているか、開いているか
		bool prevent_log_closing; //ログウィンドウを閉じるを無効化するか・設定保存イベントのフラグでもある
		bool add_progress;
		array<String^>^ log_type;
		int LastLogLen;  //ひとつ前のエンコードブロックの終わり
		bool using_afs; //afs使用時にオン
		int total_frame; //エンコ総フレーム数
		DWORD pause_start; //一時停止を開始した時間
		String^ LogTitle; //ログウィンドウのタイトル表示
		FormWindowState lastWindowState; //最終ウィンドウステータス(normal/最大化/最小化)

	private: System::Windows::Forms::RichTextBox^  richTextLog;
	private: System::Windows::Forms::ContextMenuStrip^  contextMenuStripLog;

	private: System::Windows::Forms::ToolStripMenuItem^  ToolStripMenuItemx264Priority;
	private: System::Windows::Forms::ToolStripMenuItem^  ToolStripMenuItemTransparent;
	private: System::Windows::Forms::ToolStripMenuItem^  ToolStripMenuItemStartMinimized;
	private: System::Windows::Forms::ToolStripMenuItem^  ToolStripMenuItemEncPause;
	private: System::Windows::Forms::ToolStripMenuItem^  toolStripMenuItemAutoSave;
	private: System::Windows::Forms::StatusStrip^  statusStripLog;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusCurrentTask;
	private: System::Windows::Forms::ToolStripProgressBar^  toolStripCurrentProgress;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusCurrentProgress;
	private: System::Windows::Forms::ToolStripMenuItem^  toolStripMenuItemShowStatus;
	private: System::Windows::Forms::ToolStripMenuItem^  toolStripMenuItemTaskBarProgress;
	private: System::Windows::Forms::ToolStripMenuItem^  toolStripMenuItemAutoSaveSettings;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusElapsedTime;
	private: System::Windows::Forms::ToolStripMenuItem^  toolStripMenuItemSaveLogSize;


	private: System::ComponentModel::IContainer^  components;
	protected: 

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
			this->richTextLog = (gcnew System::Windows::Forms::RichTextBox());
			this->contextMenuStripLog = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->ToolStripMenuItemx264Priority = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->ToolStripMenuItemEncPause = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->ToolStripMenuItemTransparent = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->ToolStripMenuItemStartMinimized = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripMenuItemSaveLogSize = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripMenuItemAutoSave = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripMenuItemAutoSaveSettings = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripMenuItemShowStatus = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripMenuItemTaskBarProgress = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->statusStripLog = (gcnew System::Windows::Forms::StatusStrip());
			this->toolStripStatusCurrentTask = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->toolStripStatusElapsedTime = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->toolStripCurrentProgress = (gcnew System::Windows::Forms::ToolStripProgressBar());
			this->toolStripStatusCurrentProgress = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->contextMenuStripLog->SuspendLayout();
			this->statusStripLog->SuspendLayout();
			this->SuspendLayout();
			// 
			// richTextLog
			// 
			this->richTextLog->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->richTextLog->BackColor = System::Drawing::Color::Black;
			this->richTextLog->ContextMenuStrip = this->contextMenuStripLog;
			this->richTextLog->Font = (gcnew System::Drawing::Font(L"ＭＳ ゴシック", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(128)));
			this->richTextLog->Location = System::Drawing::Point(0, 0);
			this->richTextLog->Name = L"richTextLog";
			this->richTextLog->ReadOnly = true;
			this->richTextLog->Size = System::Drawing::Size(684, 251);
			this->richTextLog->TabIndex = 0;
			this->richTextLog->Text = L"";
			this->richTextLog->WordWrap = false;
			// 
			// contextMenuStripLog
			// 
			this->contextMenuStripLog->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(9) {this->ToolStripMenuItemx264Priority, 
				this->ToolStripMenuItemEncPause, this->ToolStripMenuItemTransparent, this->ToolStripMenuItemStartMinimized, this->toolStripMenuItemSaveLogSize, 
				this->toolStripMenuItemAutoSave, this->toolStripMenuItemAutoSaveSettings, this->toolStripMenuItemShowStatus, this->toolStripMenuItemTaskBarProgress});
			this->contextMenuStripLog->Name = L"contextMenuStrip1";
			this->contextMenuStripLog->Size = System::Drawing::Size(248, 202);
			// 
			// ToolStripMenuItemx264Priority
			// 
			this->ToolStripMenuItemx264Priority->Name = L"ToolStripMenuItemx264Priority";
			this->ToolStripMenuItemx264Priority->Size = System::Drawing::Size(247, 22);
			this->ToolStripMenuItemx264Priority->Text = L"x264優先度";
			this->ToolStripMenuItemx264Priority->DropDownItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &frmLog::ToolStripMenuItemx264Priority_DropDownItemClicked);
			// 
			// ToolStripMenuItemEncPause
			// 
			this->ToolStripMenuItemEncPause->CheckOnClick = true;
			this->ToolStripMenuItemEncPause->Name = L"ToolStripMenuItemEncPause";
			this->ToolStripMenuItemEncPause->Size = System::Drawing::Size(247, 22);
			this->ToolStripMenuItemEncPause->Text = L"エンコード一時停止";
			this->ToolStripMenuItemEncPause->CheckedChanged += gcnew System::EventHandler(this, &frmLog::ToolStripMenuItemEncPause_CheckedChanged);
			// 
			// ToolStripMenuItemTransparent
			// 
			this->ToolStripMenuItemTransparent->CheckOnClick = true;
			this->ToolStripMenuItemTransparent->Name = L"ToolStripMenuItemTransparent";
			this->ToolStripMenuItemTransparent->Size = System::Drawing::Size(247, 22);
			this->ToolStripMenuItemTransparent->Text = L"ちょっと透過";
			this->ToolStripMenuItemTransparent->CheckedChanged += gcnew System::EventHandler(this, &frmLog::ToolStripMenuItemTransparent_CheckedChanged);
			// 
			// ToolStripMenuItemStartMinimized
			// 
			this->ToolStripMenuItemStartMinimized->CheckOnClick = true;
			this->ToolStripMenuItemStartMinimized->Name = L"ToolStripMenuItemStartMinimized";
			this->ToolStripMenuItemStartMinimized->Size = System::Drawing::Size(247, 22);
			this->ToolStripMenuItemStartMinimized->Text = L"このウィンドウを最小化で開始";
			this->ToolStripMenuItemStartMinimized->CheckedChanged += gcnew System::EventHandler(this, &frmLog::ToolStripCheckItem_CheckedChanged);
			// 
			// toolStripMenuItemSaveLogSize
			// 
			this->toolStripMenuItemSaveLogSize->CheckOnClick = true;
			this->toolStripMenuItemSaveLogSize->Name = L"toolStripMenuItemSaveLogSize";
			this->toolStripMenuItemSaveLogSize->Size = System::Drawing::Size(247, 22);
			this->toolStripMenuItemSaveLogSize->Text = L"このウィンドウのサイズを保存";
			this->toolStripMenuItemSaveLogSize->CheckedChanged += gcnew System::EventHandler(this, &frmLog::toolStripMenuItemSaveLogSize_CheckedChanged);
			// 
			// toolStripMenuItemAutoSave
			// 
			this->toolStripMenuItemAutoSave->CheckOnClick = true;
			this->toolStripMenuItemAutoSave->Name = L"toolStripMenuItemAutoSave";
			this->toolStripMenuItemAutoSave->Size = System::Drawing::Size(247, 22);
			this->toolStripMenuItemAutoSave->Text = L"ログ自動保存を行う";
			this->toolStripMenuItemAutoSave->CheckedChanged += gcnew System::EventHandler(this, &frmLog::ToolStripCheckItem_CheckedChanged);
			// 
			// toolStripMenuItemAutoSaveSettings
			// 
			this->toolStripMenuItemAutoSaveSettings->Name = L"toolStripMenuItemAutoSaveSettings";
			this->toolStripMenuItemAutoSaveSettings->Size = System::Drawing::Size(247, 22);
			this->toolStripMenuItemAutoSaveSettings->Text = L"ログ自動保存の設定...";
			this->toolStripMenuItemAutoSaveSettings->Click += gcnew System::EventHandler(this, &frmLog::toolStripMenuItemAutoSaveSettings_Click);
			// 
			// toolStripMenuItemShowStatus
			// 
			this->toolStripMenuItemShowStatus->Checked = true;
			this->toolStripMenuItemShowStatus->CheckOnClick = true;
			this->toolStripMenuItemShowStatus->CheckState = System::Windows::Forms::CheckState::Checked;
			this->toolStripMenuItemShowStatus->Name = L"toolStripMenuItemShowStatus";
			this->toolStripMenuItemShowStatus->Size = System::Drawing::Size(247, 22);
			this->toolStripMenuItemShowStatus->Text = L"ステータスバーの表示";
			this->toolStripMenuItemShowStatus->CheckedChanged += gcnew System::EventHandler(this, &frmLog::toolStripMenuItemShowStatus_CheckedChanged);
			// 
			// toolStripMenuItemTaskBarProgress
			// 
			this->toolStripMenuItemTaskBarProgress->CheckOnClick = true;
			this->toolStripMenuItemTaskBarProgress->Name = L"toolStripMenuItemTaskBarProgress";
			this->toolStripMenuItemTaskBarProgress->Size = System::Drawing::Size(247, 22);
			this->toolStripMenuItemTaskBarProgress->Text = L"タスクバーに進捗を表示(Win7)";
			this->toolStripMenuItemTaskBarProgress->CheckedChanged += gcnew System::EventHandler(this, &frmLog::toolStripMenuItemTaskBarProgress_CheckedChanged);
			// 
			// statusStripLog
			// 
			this->statusStripLog->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->toolStripStatusCurrentTask, 
				this->toolStripStatusElapsedTime, this->toolStripCurrentProgress, this->toolStripStatusCurrentProgress});
			this->statusStripLog->Location = System::Drawing::Point(0, 254);
			this->statusStripLog->Name = L"statusStripLog";
			this->statusStripLog->Size = System::Drawing::Size(684, 23);
			this->statusStripLog->TabIndex = 1;
			this->statusStripLog->Text = L"statusStrip1";
			// 
			// toolStripStatusCurrentTask
			// 
			this->toolStripStatusCurrentTask->Name = L"toolStripStatusCurrentTask";
			this->toolStripStatusCurrentTask->Size = System::Drawing::Size(35, 18);
			this->toolStripStatusCurrentTask->Text = L"Task";
			this->toolStripStatusCurrentTask->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// toolStripStatusElapsedTime
			// 
			this->toolStripStatusElapsedTime->Font = (gcnew System::Drawing::Font(L"メイリオ", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(128)));
			this->toolStripStatusElapsedTime->Margin = System::Windows::Forms::Padding(6, 3, 0, 1);
			this->toolStripStatusElapsedTime->Name = L"toolStripStatusElapsedTime";
			this->toolStripStatusElapsedTime->Size = System::Drawing::Size(628, 19);
			this->toolStripStatusElapsedTime->Spring = true;
			this->toolStripStatusElapsedTime->Text = L"ElapsedTime";
			this->toolStripStatusElapsedTime->TextAlign = System::Drawing::ContentAlignment::BottomLeft;
			// 
			// toolStripCurrentProgress
			// 
			this->toolStripCurrentProgress->Maximum = 1000;
			this->toolStripCurrentProgress->Name = L"toolStripCurrentProgress";
			this->toolStripCurrentProgress->Size = System::Drawing::Size(180, 17);
			this->toolStripCurrentProgress->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
			this->toolStripCurrentProgress->Visible = false;
			// 
			// toolStripStatusCurrentProgress
			// 
			this->toolStripStatusCurrentProgress->AutoSize = false;
			this->toolStripStatusCurrentProgress->Name = L"toolStripStatusCurrentProgress";
			this->toolStripStatusCurrentProgress->Size = System::Drawing::Size(60, 18);
			this->toolStripStatusCurrentProgress->Text = L"Progress";
			this->toolStripStatusCurrentProgress->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->toolStripStatusCurrentProgress->Visible = false;
			// 
			// frmLog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;
			this->ClientSize = System::Drawing::Size(684, 277);
			this->Controls->Add(this->statusStripLog);
			this->Controls->Add(this->richTextLog);
			this->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->KeyPreview = true;
			this->Name = L"frmLog";
			this->ShowIcon = false;
			this->Text = L"x264guiEx Log";
			this->Load += gcnew System::EventHandler(this, &frmLog::frmLog_Load);
			this->ClientSizeChanged += gcnew System::EventHandler(this, &frmLog::frmLog_ClientSizeChanged);
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &frmLog::frmLog_FormClosing);
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &frmLog::frmLog_KeyDown);
			this->contextMenuStripLog->ResumeLayout(false);
			this->statusStripLog->ResumeLayout(false);
			this->statusStripLog->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: 
		System::Void frmLog_Load(System::Object^  sender, System::EventArgs^  e) {
			closed = false;
			pause_start = NULL;
			taskbar_progress_init();
		}
	private:
		System::Void SetWindowSize(int width, int height) {
			//デフォルトのままにする
			if (width <= 0 || height <= 0)
				return;

			//デスクトップ領域(タスクバー等除く)
			System::Drawing::Rectangle screen = System::Windows::Forms::Screen::GetWorkingArea(this);
			this->ClientSize = System::Drawing::Size(min(width, screen.Width), min(height, screen.Height));
		}
	public:
		System::Void ReloadLogWindowSettings() {
			guiEx_settings exstg;
			exstg.load_log_win();
			ToolStripMenuItemTransparent->Checked    = exstg.s_log.transparent != 0;
			toolStripMenuItemAutoSave->Checked       = exstg.s_log.auto_save_log != 0;
			toolStripMenuItemShowStatus->Checked     = exstg.s_log.show_status_bar != 0;
			ToolStripMenuItemStartMinimized->Checked = exstg.s_log.minimized != 0;
			toolStripMenuItemSaveLogSize->Checked    = exstg.s_log.save_log_size != 0;
		}
	public:
		System::Void SetWindowTitle(const char *chr, int progress_mode) {
			LogTitle = String(chr).ToString();
			this->Text = LogTitle;
			bool show_progress = (progress_mode != PROGRESSBAR_DISABLED);
			toolStripCurrentProgress->Visible = show_progress;
			toolStripStatusCurrentProgress->Visible = show_progress;
			toolStripStatusElapsedTime->Visible = show_progress;
			toolStripCurrentProgress->Style = (progress_mode == PROGRESSBAR_MARQUEE) ? ProgressBarStyle::Marquee : ProgressBarStyle::Continuous;
			toolStripStatusCurrentProgress->Text = L"";
			toolStripStatusElapsedTime->Text = L"";
			toolStripStatusCurrentTask->Text = (show_progress) ? LogTitle : L"";
			if (!show_progress)
				toolStripCurrentProgress->Value = 0;
			taskbar_progress_start(hWnd, progress_mode);
		}
	public:
		System::Void SetTaskName(const char *chr) {
			toolStripStatusCurrentTask->Text = String(chr).ToString();
		}
	public:
		System::Void SetProgress(double progress) {
			toolStripCurrentProgress->Value = clamp((int)(progress * toolStripCurrentProgress->Maximum + 0.5), toolStripCurrentProgress->Minimum, toolStripCurrentProgress->Maximum);
			toolStripStatusCurrentProgress->Text = (progress).ToString("P1");
			toolStripStatusElapsedTime->Text = L"";
			this->Text = L"[" + toolStripStatusCurrentProgress->Text + L"] " + LogTitle;
			taskbar_setprogress(hWnd, progress);
		}
	public:
		System::Void SetWindowTitleX264Mes(const char *chr, int total_drop, int frame_n) {
			String^ title = String(chr).ToString();
			double progress = frame_n / (double)total_frame;
			String^ ProgressPercent = (progress).ToString("P1");
			DWORD time_elapsed = timeGetTime() - _x264_start_time;
			int t;
			if (using_afs) {
				StringBuilder^ SB = gcnew StringBuilder();
				SB->Append(title);
				SB->Append(L", current afs ");
				SB->Append(total_drop);
				SB->Append(L"/");
				SB->Append(frame_n);
				if (add_progress) {
					DWORD time_remain = (DWORD)(time_elapsed * ((double)(total_frame - frame_n) / (double)frame_n)) / 1000;
					SB->Insert(0, L"[" + ProgressPercent + "] ");

					t = (int)(time_remain / 3600);
					SB->Append(", eta ");
					SB->Append(t.ToString("D2"));
					SB->Append(L":");
					time_remain -= t * 3600;
					t = (int)(time_remain / 60);
					SB->Append(t.ToString("D2"));
					SB->Append(L":");
					time_remain -= t * 60;
					SB->Append(time_remain.ToString("D2"));
				}
				title = SB->ToString();
			}
			toolStripCurrentProgress->Value = clamp((int)(progress * toolStripCurrentProgress->Maximum + 0.5), toolStripCurrentProgress->Minimum, toolStripCurrentProgress->Maximum);
			toolStripStatusCurrentProgress->Text = ProgressPercent;
			taskbar_setprogress(hWnd, progress);

			time_elapsed /= 1000;
			t = (int)(time_elapsed / 3600);
			StringBuilder^ SBE = gcnew StringBuilder();
			//SBE->Append(L"経過時間 ");
			SBE->Append(t.ToString("D2"));
			SBE->Append(L":");
			time_elapsed -= t * 3600;
			t = (int)(time_elapsed / 60);
			SBE->Append(t.ToString("D2"));
			SBE->Append(L":");
			time_elapsed -= t * 60;
			SBE->Append(time_elapsed.ToString("D2"));

			toolStripStatusElapsedTime->Text = SBE->ToString();

			this->Text = title;
		}
	public:
		System::Int32 GetLogStringLen(int current_pass) {
			if (current_pass == 1) {
				LastLogLen = (closed) ? 0 : this->richTextLog->Text->Length;
				return LastLogLen;
			} else {
				return (closed) ? 0 : this->richTextLog->Text->Length;
			}
		}
	public:
		System::Void WriteLogAuoLine(const char *chr, int log_type_index) {
			log_type_index = clamp(log_type_index, LOG_INFO, LOG_ERROR);
			richTextLog->SuspendLayout();
			richTextLog->SelectionStart = richTextLog->Text->Length;
			richTextLog->SelectionLength = richTextLog->Text->Length;
			richTextLog->SelectionColor = Color::FromArgb(LOG_COLOR[log_type_index][0], LOG_COLOR[log_type_index][1], LOG_COLOR[log_type_index][2]);
			richTextLog->AppendText(L"auo [" + log_type[log_type_index] + L"]: " + String(chr).ToString() + L"\n");
			richTextLog->SelectionStart = richTextLog->Text->Length;
			richTextLog->ScrollToCaret();
			richTextLog->ResumeLayout();
		}
	public:
		System::Void WriteLogLine(const char *chr, int log_type_index) {
			log_type_index = clamp(log_type_index, LOG_INFO, LOG_ERROR);
			richTextLog->SuspendLayout();
			richTextLog->SelectionStart = richTextLog->Text->Length;
			richTextLog->SelectionLength = richTextLog->Text->Length;
			richTextLog->SelectionColor = Color::FromArgb(LOG_COLOR[log_type_index][0], LOG_COLOR[log_type_index][1], LOG_COLOR[log_type_index][2]);
			richTextLog->AppendText(String(chr).ToString() + L"\n");
			richTextLog->SelectionStart = richTextLog->Text->Length;
			richTextLog->ScrollToCaret();
			richTextLog->ResumeLayout();
		}
	private:
		System::Void SaveLog(String^ SaveLogName) {
			StreamWriter^ sw;
			try {
				sw = gcnew StreamWriter(SaveLogName, true, System::Text::Encoding::GetEncoding("shift_jis"));
				System::Text::StringBuilder^ sb = gcnew System::Text::StringBuilder(richTextLog->Text->Substring(LastLogLen));
				sb->Replace(L"\n", L"\r\n");//改行コード変換
				sw->WriteLine(sb->ToString());
				sw->WriteLine(DateTime::Now.ToString("yyyy年M月d日 H時mm分 エンコード終了"));
				sw->WriteLine(L"-------------------------------------------------------------------------------------");
				sw->WriteLine();
			} finally {
				if (sw != nullptr) {
					sw->Close();
				}
			}
		}
	public:
		System::Void SetPreventLogWindowClosing(BOOL prevent) {
			prevent_log_closing = (prevent != 0);
			if (!prevent_log_closing) {
				SaveLogSettings();
				this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &frmLog::frmLog_KeyDown);
			} else {
				this->KeyDown -= gcnew System::Windows::Forms::KeyEventHandler(this, &frmLog::frmLog_KeyDown);
			}
		}
	public:
		System::Void AutoSaveLogFile(const char *log_filename) {
			if (toolStripMenuItemAutoSave->Checked && !prevent_log_closing && log_filename != NULL)
				SaveLog(String(log_filename).ToString());
		}
	public:
		System::Void Enablex264Control(DWORD *priority, BOOL *enc_pause, BOOL afs, BOOL _add_progress, DWORD start_time, int _total_frame) {
			int i, j;
			_x264_priority = priority;
			_enc_pause = enc_pause;
			add_progress = _add_progress != 0;
			using_afs = afs != 0;
			_x264_start_time = start_time;
			total_frame = _total_frame;

			if (_x264_priority) {
				for (i = 0; priority_table[i].text; i++)
					if (*_x264_priority == priority_table[i].value)
						break;

				for (j = 0; j < this->ToolStripMenuItemx264Priority->DropDownItems->Count; j++)
					if (wcscmp(priority_table[j].text, L"") != NULL)
						((System::Windows::Forms::ToolStripMenuItem^)this->ToolStripMenuItemx264Priority->DropDownItems[j])->Checked = false;
				((System::Windows::Forms::ToolStripMenuItem^)this->ToolStripMenuItemx264Priority->DropDownItems[i])->Checked = true;
				this->ToolStripMenuItemx264Priority->Enabled = true;
			}

			if (_enc_pause) {
				this->ToolStripMenuItemEncPause->Checked = *_enc_pause != 0;
				this->ToolStripMenuItemEncPause->Enabled = true;
			}
		}
	public:
		System::Void Disablex264Control() {
			this->ToolStripMenuItemx264Priority->Enabled = false;
			this->ToolStripMenuItemEncPause->Enabled = false;
			_x264_priority = NULL;
			_enc_pause = NULL;
		}
	private:
		System::Void frmLog_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
			if (prevent_log_closing && e->CloseReason == System::Windows::Forms::CloseReason::UserClosing) {
				e->Cancel = true;
				this->WindowState = FormWindowState::Minimized;
			} else
				closed = true;
		}
	private:
		System::Void SaveLogSettings() {
			guiEx_settings exstg(true);
			exstg.load_log_win();
			exstg.s_log.transparent      = ToolStripMenuItemTransparent->Checked;
			exstg.s_log.minimized        = ToolStripMenuItemStartMinimized->Checked;
			exstg.s_log.auto_save_log    = toolStripMenuItemAutoSave->Checked;
			exstg.s_log.show_status_bar  = toolStripMenuItemShowStatus->Checked;
			exstg.s_log.taskbar_progress = toolStripMenuItemTaskBarProgress->Checked;
			exstg.s_log.save_log_size    = toolStripMenuItemSaveLogSize->Checked;
			//最大化・最小化中なら保存しない
			if (this->WindowState == FormWindowState::Normal) {
				if (exstg.s_log.save_log_size) {
					exstg.s_log.log_width    = this->ClientSize.Width;
					exstg.s_log.log_height   = this->ClientSize.Height;
				} else {
					//デフォルト
					exstg.s_log.log_width    = 0;
					exstg.s_log.log_height   = 0;
				}
			}
			exstg.save_log_win();
		}
	private:
		System::Void ToolStripCheckItem_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			if (!prevent_log_closing)
				SaveLogSettings();
		}
	private: 
		System::Void ToolStripMenuItemTransparent_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			this->Opacity = LOG_TRANSPARENT_RATIO[Convert::ToInt32(ToolStripMenuItemTransparent->Checked)];
			ToolStripCheckItem_CheckedChanged(sender, e);
		}
	private: 
		System::Void ToolStripMenuItemEncPause_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			if (_enc_pause) {
				*_enc_pause = Convert::ToInt32(ToolStripMenuItemEncPause->Checked);
				if (*_enc_pause)
					taskbar_progress_paused(hWnd);
				//if (*_enc_pause) {
				//	pause_start = timeGetTime(); //一時停止を開始した時間
				//} else {
				//	if (pause_start)
				//		*_x264_start_time += timeGetTime() - pause_start; //開始時間を修正し、一時停止後も正しい時間情報を維持
				//	pause_start = NULL;
				//}
			}
		}
	private: 
		System::Void toolStripMenuItemShowStatus_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			statusStripLog->Visible = toolStripMenuItemShowStatus->Checked;
			if (statusStripLog->Visible)
				richTextLog->Height -= statusStripLog->Height;
			else
				richTextLog->Height += statusStripLog->Height;
			ToolStripCheckItem_CheckedChanged(sender, e);
		}
	private: 
		System::Void toolStripMenuItemTaskBarProgress_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			taskbar_progress_enable(Convert::ToInt32(toolStripMenuItemTaskBarProgress->Checked));
			ToolStripCheckItem_CheckedChanged(sender, e);
		 }
	private:
		System::Void frmLog_ClientSizeChanged(System::Object^  sender, System::EventArgs^  e) {
			//通常->通常でのサイズ変更以外、保存しないようにする
			//最小化/最大化->通常とか通常->最小化/最大化には興味がない
			if (this->WindowState == FormWindowState::Normal &&
				  lastWindowState == FormWindowState::Normal)
				SaveLogSettings();
			lastWindowState = this->WindowState;
		}
	private:
		System::Void toolStripMenuItemSaveLogSize_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			ToolStripCheckItem_CheckedChanged(sender, e);
		}
	private: 
		System::Void ToolStripMenuItemx264Priority_DropDownItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e) {
			int i, j;
			if (_x264_priority != NULL && e->ClickedItem->GetType() == this->ToolStripMenuItemx264Priority->GetType()) {
				System::Windows::Forms::ToolStripMenuItem^ item = (System::Windows::Forms::ToolStripMenuItem^)e->ClickedItem;
				//一度全部のチェックを外す
				for (i = 0; i < this->ToolStripMenuItemx264Priority->DropDownItems->Count; i++) {
					if (wcscmp(priority_table[i].text, L"") != NULL) {
						((System::Windows::Forms::ToolStripMenuItem^)this->ToolStripMenuItemx264Priority->DropDownItems[i])->Checked = false;
					}
				}

				//自分だけチェックする
				item->Checked = true;
				//自分のインデックスを取得
				for (j = 0; j < this->ToolStripMenuItemx264Priority->DropDownItems->Count; j++) {
					if (wcscmp(priority_table[j].text, L"") != NULL) {
						if (String::Compare(String(priority_table[j].text).ToString(), item->Text) == 0) {
							break;
						}
					}
				}
				*_x264_priority = priority_table[j].value;
			}
		}
	private: 
		System::Void toolStripMenuItemAutoSaveSettings_Click(System::Object^  sender, System::EventArgs^  e) {
			frmAutoSaveLogSettings::Instance::get()->Owner = this;
			frmAutoSaveLogSettings::Instance::get()->Show();
		}
	private:
		System::Void frmLog_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
			if (e->KeyCode == Keys::Escape)
				this->Close();
		}
};
}

#pragma warning( pop ) //( disable: 4100 ) 終了