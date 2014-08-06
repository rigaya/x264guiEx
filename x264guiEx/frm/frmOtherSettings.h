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
	private: System::Windows::Forms::CheckBox^  fosCBAutoAFSDisable;
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
			this->SuspendLayout();
			// 
			// fosCBCancel
			// 
			this->fosCBCancel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->fosCBCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->fosCBCancel->Location = System::Drawing::Point(171, 147);
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
			this->fosCBOK->Location = System::Drawing::Point(283, 147);
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
			// frmOtherSettings
			// 
			this->AcceptButton = this->fosCBOK;
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Inherit;
			this->CancelButton = this->fosCBCancel;
			this->ClientSize = System::Drawing::Size(392, 186);
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
			stgDir = fosTXStgDir->Text;
			fos_ex_stg->load_encode_stg();
			fos_ex_stg->s_local.auto_afs_disable = fosCBAutoAFSDisable->Checked;
			fos_ex_stg->save_local();
			this->Close();
		}
	private: 
		System::Void frmOtherSettings_Load(System::Object^  sender, System::EventArgs^  e) {
			this->Text = String(AUO_FULL_NAME).ToString();
			fosTXStgDir->Text = stgDir;
			fos_ex_stg->load_encode_stg();
			fosCBAutoAFSDisable->Checked = fos_ex_stg->s_local.auto_afs_disable != 0;
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
