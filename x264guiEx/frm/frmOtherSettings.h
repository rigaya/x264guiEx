//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#pragma once

#include "auo_version.h"
#include "auo_settings.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace x264guiEx {

	/// <summary>
	/// frmOtherSettings の概要
	///
	/// 警告: このクラスの名前を変更する場合、このクラスが依存するすべての .resx ファイルに関連付けられた
	///          マネージ リソース コンパイラ ツールに対して 'Resource File Name' プロパティを
	///          変更する必要があります。この変更を行わないと、
	///          デザイナと、このフォームに関連付けられたローカライズ済みリソースとが、
	///          正しく相互に利用できなくなります。
	/// </summary>
	public ref class frmOtherSettings : public System::Windows::Forms::Form
	{
	public:
		frmOtherSettings(void)
		{
			fos_ex_stg = new guiEx_settings(TRUE);
			InitializeComponent();
			//
			//TODO: ここにコンストラクタ コードを追加します
			//
		}

	protected:
		/// <summary>
		/// 使用中のリソースをすべてクリーンアップします。
		/// </summary>
		~frmOtherSettings()
		{
			if (components)
			{
				delete components;
			}
			delete fos_ex_stg;
		}
	private:
		guiEx_settings *fos_ex_stg;
		static frmOtherSettings^ _instance;

	protected: 
	private: System::Windows::Forms::Button^  fosCBCancel;
	private: System::Windows::Forms::Button^  fosCBOK;
	private: System::Windows::Forms::TextBox^  fosTXStgDir;
	private: System::Windows::Forms::Label^  fosLBStgDir;
	private: System::Windows::Forms::Button^  fosBTStgDir;
	public:
		static String^ stgDir;
		static int useLastExt;
		static bool DisableToolTipHelp;
	private: System::Windows::Forms::CheckBox^  fosCBAutoAFSDisable;
	private: System::Windows::Forms::CheckBox^  fosCBAutoDelStats;
	private: System::Windows::Forms::CheckBox^  fosCBDisableToolTip;
	private: System::Windows::Forms::CheckBox^  fosCBDisableVisualStyles;
	private: System::Windows::Forms::Label^  fosLBDisableVisualStyles;
	private: System::Windows::Forms::CheckBox^  fosCBLogStartMinimized;
	private: System::Windows::Forms::CheckBox^  fosCBLogDisableTransparency;
	private: System::Windows::Forms::CheckBox^  fosCBAutoDelChap;


	public: 

	public:
		static property frmOtherSettings^ Instance {
			frmOtherSettings^ get() {
				if (_instance == nullptr || _instance->IsDisposed)
					_instance = gcnew frmOtherSettings();
				return _instance;
			}
		}


	private:
		/// <summary>
		/// 必要なデザイナ変数です。
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// デザイナ サポートに必要なメソッドです。このメソッドの内容を
		/// コード エディタで変更しないでください。
		/// </summary>
		void InitializeComponent(void)
		{
			this->fosCBCancel = (gcnew System::Windows::Forms::Button());
			this->fosCBOK = (gcnew System::Windows::Forms::Button());
			this->fosTXStgDir = (gcnew System::Windows::Forms::TextBox());
			this->fosLBStgDir = (gcnew System::Windows::Forms::Label());
			this->fosBTStgDir = (gcnew System::Windows::Forms::Button());
			this->fosCBAutoAFSDisable = (gcnew System::Windows::Forms::CheckBox());
			this->fosCBAutoDelStats = (gcnew System::Windows::Forms::CheckBox());
			this->fosCBDisableToolTip = (gcnew System::Windows::Forms::CheckBox());
			this->fosCBDisableVisualStyles = (gcnew System::Windows::Forms::CheckBox());
			this->fosLBDisableVisualStyles = (gcnew System::Windows::Forms::Label());
			this->fosCBLogStartMinimized = (gcnew System::Windows::Forms::CheckBox());
			this->fosCBLogDisableTransparency = (gcnew System::Windows::Forms::CheckBox());
			this->fosCBAutoDelChap = (gcnew System::Windows::Forms::CheckBox());
			this->SuspendLayout();
			// 
			// fosCBCancel
			// 
			this->fosCBCancel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->fosCBCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->fosCBCancel->Location = System::Drawing::Point(171, 355);
			this->fosCBCancel->Name = L"fosCBCancel";
			this->fosCBCancel->Size = System::Drawing::Size(84, 29);
			this->fosCBCancel->TabIndex = 1;
			this->fosCBCancel->Text = L"キャンセル";
			this->fosCBCancel->UseVisualStyleBackColor = true;
			this->fosCBCancel->Click += gcnew System::EventHandler(this, &frmOtherSettings::fosCBCancel_Click);
			// 
			// fosCBOK
			// 
			this->fosCBOK->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->fosCBOK->Location = System::Drawing::Point(283, 355);
			this->fosCBOK->Name = L"fosCBOK";
			this->fosCBOK->Size = System::Drawing::Size(84, 29);
			this->fosCBOK->TabIndex = 2;
			this->fosCBOK->Text = L"OK";
			this->fosCBOK->UseVisualStyleBackColor = true;
			this->fosCBOK->Click += gcnew System::EventHandler(this, &frmOtherSettings::fosCBOK_Click);
			// 
			// fosTXStgDir
			// 
			this->fosTXStgDir->Location = System::Drawing::Point(48, 42);
			this->fosTXStgDir->Name = L"fosTXStgDir";
			this->fosTXStgDir->Size = System::Drawing::Size(294, 23);
			this->fosTXStgDir->TabIndex = 3;
			// 
			// fosLBStgDir
			// 
			this->fosLBStgDir->AutoSize = true;
			this->fosLBStgDir->Location = System::Drawing::Point(21, 22);
			this->fosLBStgDir->Name = L"fosLBStgDir";
			this->fosLBStgDir->Size = System::Drawing::Size(123, 15);
			this->fosLBStgDir->TabIndex = 4;
			this->fosLBStgDir->Text = L"設定ファイルの保存場所";
			// 
			// fosBTStgDir
			// 
			this->fosBTStgDir->Location = System::Drawing::Point(348, 42);
			this->fosBTStgDir->Name = L"fosBTStgDir";
			this->fosBTStgDir->Size = System::Drawing::Size(35, 23);
			this->fosBTStgDir->TabIndex = 5;
			this->fosBTStgDir->Text = L"...";
			this->fosBTStgDir->UseVisualStyleBackColor = true;
			this->fosBTStgDir->Click += gcnew System::EventHandler(this, &frmOtherSettings::fosBTStgDir_Click);
			// 
			// fosCBAutoAFSDisable
			// 
			this->fosCBAutoAFSDisable->Location = System::Drawing::Point(24, 84);
			this->fosCBAutoAFSDisable->Name = L"fosCBAutoAFSDisable";
			this->fosCBAutoAFSDisable->Size = System::Drawing::Size(308, 53);
			this->fosCBAutoAFSDisable->TabIndex = 6;
			this->fosCBAutoAFSDisable->Text = L"自動フィールドシフト(afs)オンで初期化に失敗した場合、afsをオフにしてエンコード続行を試みる";
			this->fosCBAutoAFSDisable->UseVisualStyleBackColor = true;
			// 
			// fosCBAutoDelStats
			// 
			this->fosCBAutoDelStats->AutoSize = true;
			this->fosCBAutoDelStats->Location = System::Drawing::Point(24, 144);
			this->fosCBAutoDelStats->Name = L"fosCBAutoDelStats";
			this->fosCBAutoDelStats->Size = System::Drawing::Size(293, 19);
			this->fosCBAutoDelStats->TabIndex = 7;
			this->fosCBAutoDelStats->Text = L"自動マルチパス時、ステータスファイルも自動的に削除する";
			this->fosCBAutoDelStats->UseVisualStyleBackColor = true;
			// 
			// fosCBDisableToolTip
			// 
			this->fosCBDisableToolTip->AutoSize = true;
			this->fosCBDisableToolTip->Location = System::Drawing::Point(24, 206);
			this->fosCBDisableToolTip->Name = L"fosCBDisableToolTip";
			this->fosCBDisableToolTip->Size = System::Drawing::Size(158, 19);
			this->fosCBDisableToolTip->TabIndex = 8;
			this->fosCBDisableToolTip->Text = L"ポップアップヘルプを抑制する";
			this->fosCBDisableToolTip->UseVisualStyleBackColor = true;
			// 
			// fosCBDisableVisualStyles
			// 
			this->fosCBDisableVisualStyles->AutoSize = true;
			this->fosCBDisableVisualStyles->Location = System::Drawing::Point(24, 236);
			this->fosCBDisableVisualStyles->Name = L"fosCBDisableVisualStyles";
			this->fosCBDisableVisualStyles->Size = System::Drawing::Size(128, 19);
			this->fosCBDisableVisualStyles->TabIndex = 9;
			this->fosCBDisableVisualStyles->Text = L"視覚効果をオフにする";
			this->fosCBDisableVisualStyles->UseVisualStyleBackColor = true;
			// 
			// fosLBDisableVisualStyles
			// 
			this->fosLBDisableVisualStyles->AutoSize = true;
			this->fosLBDisableVisualStyles->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(128)));
			this->fosLBDisableVisualStyles->ForeColor = System::Drawing::Color::OrangeRed;
			this->fosLBDisableVisualStyles->Location = System::Drawing::Point(45, 255);
			this->fosLBDisableVisualStyles->Name = L"fosLBDisableVisualStyles";
			this->fosLBDisableVisualStyles->Size = System::Drawing::Size(161, 14);
			this->fosLBDisableVisualStyles->TabIndex = 10;
			this->fosLBDisableVisualStyles->Text = L"※反映にはAviutlの再起動が必要";
			// 
			// fosCBLogStartMinimized
			// 
			this->fosCBLogStartMinimized->AutoSize = true;
			this->fosCBLogStartMinimized->Location = System::Drawing::Point(24, 282);
			this->fosCBLogStartMinimized->Name = L"fosCBLogStartMinimized";
			this->fosCBLogStartMinimized->Size = System::Drawing::Size(184, 19);
			this->fosCBLogStartMinimized->TabIndex = 11;
			this->fosCBLogStartMinimized->Text = L"ログウィンドウを最小化で開始する";
			this->fosCBLogStartMinimized->UseVisualStyleBackColor = true;
			// 
			// fosCBLogDisableTransparency
			// 
			this->fosCBLogDisableTransparency->AutoSize = true;
			this->fosCBLogDisableTransparency->Location = System::Drawing::Point(24, 312);
			this->fosCBLogDisableTransparency->Name = L"fosCBLogDisableTransparency";
			this->fosCBLogDisableTransparency->Size = System::Drawing::Size(174, 19);
			this->fosCBLogDisableTransparency->TabIndex = 12;
			this->fosCBLogDisableTransparency->Text = L"ログウィンドウの透過をオフにする";
			this->fosCBLogDisableTransparency->UseVisualStyleBackColor = true;
			// 
			// fosCBAutoDelChap
			// 
			this->fosCBAutoDelChap->AutoSize = true;
			this->fosCBAutoDelChap->Location = System::Drawing::Point(24, 173);
			this->fosCBAutoDelChap->Name = L"fosCBAutoDelChap";
			this->fosCBAutoDelChap->Size = System::Drawing::Size(295, 19);
			this->fosCBAutoDelChap->TabIndex = 13;
			this->fosCBAutoDelChap->Text = L"mux正常終了後、チャプターファイルを自動的に削除する";
			this->fosCBAutoDelChap->UseVisualStyleBackColor = true;
			// 
			// frmOtherSettings
			// 
			this->AcceptButton = this->fosCBOK;
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Inherit;
			this->CancelButton = this->fosCBCancel;
			this->ClientSize = System::Drawing::Size(392, 394);
			this->Controls->Add(this->fosCBAutoDelChap);
			this->Controls->Add(this->fosCBLogDisableTransparency);
			this->Controls->Add(this->fosCBLogStartMinimized);
			this->Controls->Add(this->fosLBDisableVisualStyles);
			this->Controls->Add(this->fosCBDisableVisualStyles);
			this->Controls->Add(this->fosCBDisableToolTip);
			this->Controls->Add(this->fosCBAutoDelStats);
			this->Controls->Add(this->fosCBAutoAFSDisable);
			this->Controls->Add(this->fosBTStgDir);
			this->Controls->Add(this->fosLBStgDir);
			this->Controls->Add(this->fosTXStgDir);
			this->Controls->Add(this->fosCBOK);
			this->Controls->Add(this->fosCBCancel);
			this->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;
			this->Name = L"frmOtherSettings";
			this->ShowIcon = false;
			this->Text = L"frmOtherSettings";
			this->Load += gcnew System::EventHandler(this, &frmOtherSettings::frmOtherSettings_Load);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: 
		System::Void fosCBOK_Click(System::Object^  sender, System::EventArgs^  e) {
			DisableToolTipHelp = fosCBDisableToolTip->Checked;

			stgDir = fosTXStgDir->Text;
			fos_ex_stg->load_encode_stg();
			fos_ex_stg->load_log_win();
			fos_ex_stg->s_local.auto_afs_disable      = fosCBAutoAFSDisable->Checked;
			fos_ex_stg->s_local.auto_del_stats        = fosCBAutoDelStats->Checked;
			fos_ex_stg->s_local.auto_del_chap         = fosCBAutoDelChap->Checked;
			fos_ex_stg->s_local.disable_tooltip_help  = fosCBDisableToolTip->Checked;
			fos_ex_stg->s_local.disable_visual_styles = fosCBDisableVisualStyles->Checked;
			fos_ex_stg->s_log.minimized               = fosCBLogStartMinimized->Checked;
			fos_ex_stg->s_log.transparent             = !fosCBLogDisableTransparency->Checked;
			fos_ex_stg->save_local();
			fos_ex_stg->save_log_win();
			this->Close();
		}
	private: 
		System::Void frmOtherSettings_Load(System::Object^  sender, System::EventArgs^  e) {
			this->Text = String(AUO_FULL_NAME).ToString();
			fosTXStgDir->Text = stgDir;
			fos_ex_stg->load_encode_stg();
			fos_ex_stg->load_log_win();
			fosCBAutoAFSDisable->Checked         = fos_ex_stg->s_local.auto_afs_disable != 0;
			fosCBAutoDelStats->Checked           = fos_ex_stg->s_local.auto_del_stats != 0;
			fosCBAutoDelChap->Checked            = fos_ex_stg->s_local.auto_del_chap != 0;
			fosCBDisableToolTip->Checked         = fos_ex_stg->s_local.disable_tooltip_help != 0;
			fosCBDisableVisualStyles->Checked    = fos_ex_stg->s_local.disable_visual_styles != 0;
			fosCBLogStartMinimized->Checked      = fos_ex_stg->s_log.minimized != 0;
			fosCBLogDisableTransparency->Checked = fos_ex_stg->s_log.transparent == 0;
		}
	private: 
		System::Void fosBTStgDir_Click(System::Object^  sender, System::EventArgs^  e) {
			FolderBrowserDialog^ fbd = gcnew FolderBrowserDialog();
			if (System::IO::Directory::Exists(fosTXStgDir->Text)) {
				fbd->SelectedPath = fosTXStgDir->Text;
			}
			if (fbd->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
				fosTXStgDir->Text = fbd->SelectedPath;
			}
		}
	private: 
		System::Void fosCBCancel_Click(System::Object^  sender, System::EventArgs^  e) {
			this->Close();
		}
};
}
