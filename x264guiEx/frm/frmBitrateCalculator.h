//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#pragma once

#include "auo_frm.h"
#include "auo_util.h"
#include "auo_settings.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

//以下部分的にwarning C4100を黙らせる
//C4100 : 引数は関数の本体部で 1 度も参照されません。
#pragma warning( push )
#pragma warning( disable: 4100 )

namespace x264guiEx {

	/// <summary>
	/// frmBitrateCalculator の概要
	///
	/// 警告: このクラスの名前を変更する場合、このクラスが依存するすべての .resx ファイルに関連付けられた
	///          マネージ リソース コンパイラ ツールに対して 'Resource File Name' プロパティを
	///          変更する必要があります。この変更を行わないと、
	///          デザイナと、このフォームに関連付けられたローカライズ済みリソースとが、
	///          正しく相互に利用できなくなります。
	/// </summary>
	public ref class frmBitrateCalculator : public System::Windows::Forms::Form
	{
	public:
		frmBitrateCalculator(void)
		{
			InitializeComponent();
			//
			//TODO: ここにコンストラクタ コードを追加します
			//
		}

	protected:
		/// <summary>
		/// 使用中のリソースをすべてクリーンアップします。
		/// </summary>
		~frmBitrateCalculator()
		{
			if (components)
			{
				delete components;
			}
		}
	private:
		static frmBitrateCalculator^ _instance;
	public:
		static property frmBitrateCalculator^ Instance {
			frmBitrateCalculator^ get() {
				if (_instance == nullptr || _instance->IsDisposed)
					_instance = gcnew frmBitrateCalculator();
				return _instance;
			}
		}
	private: System::Windows::Forms::NumericUpDown^  fbcNULengthHour;
	protected: 
	private: System::Windows::Forms::NumericUpDown^  fbcNULengthMin;
	private: System::Windows::Forms::NumericUpDown^  fbcNULengthSec;

	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::Label^  label5;
	private: System::Windows::Forms::Label^  label6;
	private: System::Windows::Forms::Label^  label7;
	private: System::Windows::Forms::NumericUpDown^  fbcNUBitrateAudio;


	private: System::Windows::Forms::NumericUpDown^  fbcNUBitrateVideo;


	private: System::Windows::Forms::Label^  label8;
	private: System::Windows::Forms::Label^  label9;
	private: System::Windows::Forms::Label^  label10;





	private: System::Windows::Forms::Button^  fbcBTVBApply;
	private: System::Windows::Forms::Button^  fbcBTABApply;
	private: System::Windows::Forms::TextBox^  fbcTXSize;



	private: System::Windows::Forms::Label^  label11;
	private: System::Windows::Forms::NumericUpDown^  fbcNUBitrateSum;

	private: System::Windows::Forms::Label^  label12;
	private: System::Windows::Forms::RadioButton^  fbcRBCalcRate;
	private: System::Windows::Forms::RadioButton^  fbcRBCalcSize;




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
			this->fbcNULengthHour = (gcnew System::Windows::Forms::NumericUpDown());
			this->fbcNULengthMin = (gcnew System::Windows::Forms::NumericUpDown());
			this->fbcNULengthSec = (gcnew System::Windows::Forms::NumericUpDown());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->fbcNUBitrateVideo = (gcnew System::Windows::Forms::NumericUpDown());
			this->fbcNUBitrateAudio = (gcnew System::Windows::Forms::NumericUpDown());
			this->label8 = (gcnew System::Windows::Forms::Label());
			this->label9 = (gcnew System::Windows::Forms::Label());
			this->label10 = (gcnew System::Windows::Forms::Label());
			this->fbcBTVBApply = (gcnew System::Windows::Forms::Button());
			this->fbcBTABApply = (gcnew System::Windows::Forms::Button());
			this->fbcTXSize = (gcnew System::Windows::Forms::TextBox());
			this->label11 = (gcnew System::Windows::Forms::Label());
			this->fbcNUBitrateSum = (gcnew System::Windows::Forms::NumericUpDown());
			this->label12 = (gcnew System::Windows::Forms::Label());
			this->fbcRBCalcRate = (gcnew System::Windows::Forms::RadioButton());
			this->fbcRBCalcSize = (gcnew System::Windows::Forms::RadioButton());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fbcNULengthHour))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fbcNULengthMin))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fbcNULengthSec))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fbcNUBitrateVideo))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fbcNUBitrateAudio))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fbcNUBitrateSum))->BeginInit();
			this->SuspendLayout();
			// 
			// fbcNULengthHour
			// 
			this->fbcNULengthHour->Location = System::Drawing::Point(88, 49);
			this->fbcNULengthHour->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {120, 0, 0, 0});
			this->fbcNULengthHour->Name = L"fbcNULengthHour";
			this->fbcNULengthHour->Size = System::Drawing::Size(55, 21);
			this->fbcNULengthHour->TabIndex = 0;
			this->fbcNULengthHour->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->fbcNULengthHour->TextChanged += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcNULength_TextChanged);
			// 
			// fbcNULengthMin
			// 
			this->fbcNULengthMin->Location = System::Drawing::Point(179, 49);
			this->fbcNULengthMin->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {59, 0, 0, 0});
			this->fbcNULengthMin->Name = L"fbcNULengthMin";
			this->fbcNULengthMin->Size = System::Drawing::Size(55, 21);
			this->fbcNULengthMin->TabIndex = 1;
			this->fbcNULengthMin->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->fbcNULengthMin->TextChanged += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcNULength_TextChanged);
			// 
			// fbcNULengthSec
			// 
			this->fbcNULengthSec->Location = System::Drawing::Point(261, 49);
			this->fbcNULengthSec->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {59, 0, 0, 0});
			this->fbcNULengthSec->Name = L"fbcNULengthSec";
			this->fbcNULengthSec->Size = System::Drawing::Size(55, 21);
			this->fbcNULengthSec->TabIndex = 2;
			this->fbcNULengthSec->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->fbcNULengthSec->TextChanged += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcNULength_TextChanged);
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(32, 51);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(48, 14);
			this->label1->TabIndex = 3;
			this->label1->Text = L"動画長さ";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(146, 51);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(29, 14);
			this->label2->TabIndex = 4;
			this->label2->Text = L"時間";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(236, 51);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(18, 14);
			this->label3->TabIndex = 5;
			this->label3->Text = L"分";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(318, 51);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(18, 14);
			this->label4->TabIndex = 6;
			this->label4->Text = L"秒";
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Location = System::Drawing::Point(31, 94);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(76, 14);
			this->label5->TabIndex = 7;
			this->label5->Text = L"映像ビットレート";
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->Location = System::Drawing::Point(31, 127);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(76, 14);
			this->label6->TabIndex = 8;
			this->label6->Text = L"音声ビットレート";
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Location = System::Drawing::Point(31, 188);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(55, 14);
			this->label7->TabIndex = 9;
			this->label7->Text = L"動画サイズ";
			// 
			// fbcNUBitrateVideo
			// 
			this->fbcNUBitrateVideo->Location = System::Drawing::Point(132, 92);
			this->fbcNUBitrateVideo->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {128000, 0, 0, 0});
			this->fbcNUBitrateVideo->Name = L"fbcNUBitrateVideo";
			this->fbcNUBitrateVideo->Size = System::Drawing::Size(65, 21);
			this->fbcNUBitrateVideo->TabIndex = 4;
			this->fbcNUBitrateVideo->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->fbcNUBitrateVideo->TextChanged += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcNUVideoBitrate_TextChanged);
			// 
			// fbcNUBitrateAudio
			// 
			this->fbcNUBitrateAudio->Location = System::Drawing::Point(132, 125);
			this->fbcNUBitrateAudio->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1536, 0, 0, 0});
			this->fbcNUBitrateAudio->Name = L"fbcNUBitrateAudio";
			this->fbcNUBitrateAudio->Size = System::Drawing::Size(65, 21);
			this->fbcNUBitrateAudio->TabIndex = 5;
			this->fbcNUBitrateAudio->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->fbcNUBitrateAudio->TextChanged += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcNUAudioBitrate_TextChanged);
			// 
			// label8
			// 
			this->label8->AutoSize = true;
			this->label8->Location = System::Drawing::Point(203, 94);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(32, 14);
			this->label8->TabIndex = 13;
			this->label8->Text = L"kbps";
			// 
			// label9
			// 
			this->label9->AutoSize = true;
			this->label9->Location = System::Drawing::Point(203, 127);
			this->label9->Name = L"label9";
			this->label9->Size = System::Drawing::Size(32, 14);
			this->label9->TabIndex = 14;
			this->label9->Text = L"kbps";
			// 
			// label10
			// 
			this->label10->AutoSize = true;
			this->label10->Location = System::Drawing::Point(203, 188);
			this->label10->Name = L"label10";
			this->label10->Size = System::Drawing::Size(23, 14);
			this->label10->TabIndex = 15;
			this->label10->Text = L"MB";
			// 
			// fbcBTVBApply
			// 
			this->fbcBTVBApply->Location = System::Drawing::Point(245, 90);
			this->fbcBTVBApply->Name = L"fbcBTVBApply";
			this->fbcBTVBApply->Size = System::Drawing::Size(129, 25);
			this->fbcBTVBApply->TabIndex = 8;
			this->fbcBTVBApply->Text = L"映像ビットレートを反映";
			this->fbcBTVBApply->UseVisualStyleBackColor = true;
			this->fbcBTVBApply->Click += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcBTVBApply_Click);
			// 
			// fbcBTABApply
			// 
			this->fbcBTABApply->Location = System::Drawing::Point(245, 123);
			this->fbcBTABApply->Name = L"fbcBTABApply";
			this->fbcBTABApply->Size = System::Drawing::Size(129, 25);
			this->fbcBTABApply->TabIndex = 9;
			this->fbcBTABApply->Text = L"音声ビットレートを反映";
			this->fbcBTABApply->UseVisualStyleBackColor = true;
			this->fbcBTABApply->Click += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcBTABApply_Click);
			// 
			// fbcTXSize
			// 
			this->fbcTXSize->Location = System::Drawing::Point(132, 185);
			this->fbcTXSize->Name = L"fbcTXSize";
			this->fbcTXSize->Size = System::Drawing::Size(65, 21);
			this->fbcTXSize->TabIndex = 7;
			this->fbcTXSize->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->fbcTXSize->TextChanged += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcTXSize_TextChanged);
			// 
			// label11
			// 
			this->label11->AutoSize = true;
			this->label11->Location = System::Drawing::Point(203, 158);
			this->label11->Name = L"label11";
			this->label11->Size = System::Drawing::Size(32, 14);
			this->label11->TabIndex = 24;
			this->label11->Text = L"kbps";
			// 
			// fbcNUBitrateSum
			// 
			this->fbcNUBitrateSum->Location = System::Drawing::Point(132, 156);
			this->fbcNUBitrateSum->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {128000, 0, 0, 0});
			this->fbcNUBitrateSum->Name = L"fbcNUBitrateSum";
			this->fbcNUBitrateSum->Size = System::Drawing::Size(65, 21);
			this->fbcNUBitrateSum->TabIndex = 6;
			this->fbcNUBitrateSum->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->fbcNUBitrateSum->TextChanged += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcNUBitrateSum_TextChanged);
			// 
			// label12
			// 
			this->label12->AutoSize = true;
			this->label12->Location = System::Drawing::Point(31, 158);
			this->label12->Name = L"label12";
			this->label12->Size = System::Drawing::Size(76, 14);
			this->label12->TabIndex = 22;
			this->label12->Text = L"合計ビットレート";
			// 
			// fbcRBCalcRate
			// 
			this->fbcRBCalcRate->AutoSize = true;
			this->fbcRBCalcRate->Checked = true;
			this->fbcRBCalcRate->Location = System::Drawing::Point(25, 12);
			this->fbcRBCalcRate->Name = L"fbcRBCalcRate";
			this->fbcRBCalcRate->Size = System::Drawing::Size(151, 18);
			this->fbcRBCalcRate->TabIndex = 25;
			this->fbcRBCalcRate->TabStop = true;
			this->fbcRBCalcRate->Text = L"サイズからビットレートを求める";
			this->fbcRBCalcRate->UseVisualStyleBackColor = true;
			this->fbcRBCalcRate->CheckedChanged += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcRBCalcRate_CheckedChanged);
			// 
			// fbcRBCalcSize
			// 
			this->fbcRBCalcSize->AutoSize = true;
			this->fbcRBCalcSize->Location = System::Drawing::Point(206, 12);
			this->fbcRBCalcSize->Name = L"fbcRBCalcSize";
			this->fbcRBCalcSize->Size = System::Drawing::Size(151, 18);
			this->fbcRBCalcSize->TabIndex = 26;
			this->fbcRBCalcSize->Text = L"ビットレートからサイズを求める";
			this->fbcRBCalcSize->UseVisualStyleBackColor = true;
			this->fbcRBCalcSize->CheckedChanged += gcnew System::EventHandler(this, &frmBitrateCalculator::fbcRBCalcSize_CheckedChanged);
			// 
			// frmBitrateCalculator
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;
			this->ClientSize = System::Drawing::Size(386, 220);
			this->Controls->Add(this->fbcRBCalcSize);
			this->Controls->Add(this->fbcRBCalcRate);
			this->Controls->Add(this->label11);
			this->Controls->Add(this->fbcNUBitrateSum);
			this->Controls->Add(this->label12);
			this->Controls->Add(this->fbcTXSize);
			this->Controls->Add(this->fbcBTABApply);
			this->Controls->Add(this->fbcBTVBApply);
			this->Controls->Add(this->label10);
			this->Controls->Add(this->label9);
			this->Controls->Add(this->label8);
			this->Controls->Add(this->fbcNUBitrateAudio);
			this->Controls->Add(this->fbcNUBitrateVideo);
			this->Controls->Add(this->label7);
			this->Controls->Add(this->label6);
			this->Controls->Add(this->label5);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->fbcNULengthSec);
			this->Controls->Add(this->fbcNULengthMin);
			this->Controls->Add(this->fbcNULengthHour);
			this->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->KeyPreview = true;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"frmBitrateCalculator";
			this->ShowIcon = false;
			this->Text = L"簡易ビットレート計算機";
			this->Load += gcnew System::EventHandler(this, &frmBitrateCalculator::frmBitrateCalculator_Load);
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &frmBitrateCalculator::frmBitrateCalculator_FormClosing);
			this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &frmBitrateCalculator::frmBitrateCalculator_KeyDown);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fbcNULengthHour))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fbcNULengthMin))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fbcNULengthSec))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fbcNUBitrateVideo))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fbcNUBitrateAudio))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fbcNUBitrateSum))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private:
		String^ LastStr;
		bool enable_events;
		System::Void fbcBTVBApply_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void fbcBTABApply_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void frmBitrateCalculator_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);
		System::Void fbcRBCalcRate_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
	public:
		System::Void Init(int VideoBitrate, int AudioBitrate, bool BTVBEnable, bool BTABEnable, int ab_max);
	private: 
		System::Void frmBitrateCalculator_Load(System::Object^  sender, System::EventArgs^  e) {
			SetAllCheckChangedEvents(this);
			enable_events = true;
			LastStr = L"";
			SetNUValue(fbcNUBitrateSum, fbcNUBitrateAudio->Value + fbcNUBitrateVideo->Value);
			fbcNULength_TextChanged(nullptr, nullptr);
			//フォントの設定
			guiEx_settings exstg;
			exstg.load_encode_stg();
			if (char_has_length(exstg.s_local.conf_font.name))
				SetFontFamilyToForm(this, gcnew FontFamily(String(exstg.s_local.conf_font.name).ToString()), this->Font->FontFamily);
		}
	private: 
		System::Void fbcRBCalcSize_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			fbcTXSize->ReadOnly = fbcRBCalcSize->Checked;
		}
	private:
		System::Void SetAllCheckChangedEvents(Control ^top) {
			for (int i = 0; i < top->Controls->Count; i++) {
				System::Type^ type = top->Controls[i]->GetType();
				if (type == NumericUpDown::typeid)
					top->Controls[i]->Enter += gcnew System::EventHandler(this, &frmBitrateCalculator::NUSelectAll);
				else
					SetAllCheckChangedEvents(top->Controls[i]);
			}
		}
	private:
		System::Void NUSelectAll(System::Object^  sender, System::EventArgs^  e) {
			 NumericUpDown^ NU = dynamic_cast<NumericUpDown^>(sender);
			 NU->Select(0, NU->Text->Length);
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
		System::Void SetNUValue(NumericUpDown^ NU, float f) {
			NU->Value = clamp(Convert::ToDecimal(f), NU->Minimum, NU->Maximum);
		}
	private:
		System::Void SetNUValue(NumericUpDown^ NU, double d) {
			NU->Value = clamp(Convert::ToDecimal(d), NU->Minimum, NU->Maximum);
		}
	private:
		System::Int32 GetDurationSec() {
			return  (int)fbcNULengthHour->Value * 3600 + (int)fbcNULengthMin->Value * 60 + (int)fbcNULengthSec->Value;
		}
	public:
		System::Void SetBTVBEnabled(bool enable) {
			fbcBTVBApply->Enabled = enable;
		}
	public:
		System::Void SetBTABEnabled(bool enable, int ab_max) {
			fbcBTABApply->Enabled = enable;
			fbcNUBitrateAudio->Maximum = ab_max;
		}
	public:
		System::Void SetNUVideoBitrate(int bitrate) {
			SetNUValue(fbcNUBitrateVideo, bitrate);
		}
	public:
		System::Void SetNUAudioBitrate(int bitrate) {
			SetNUValue(fbcNUBitrateAudio, bitrate);
		}
	private: 
		System::Void fbcNUVideoBitrate_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			if (enable_events) {
				enable_events = false;
				if (fbcRBCalcRate->Checked) {
					SetNUValue(fbcNUBitrateAudio, fbcNUBitrateSum->Value - fbcNUBitrateVideo->Value);
					SetNUValue(fbcNUBitrateVideo, fbcNUBitrateSum->Value - fbcNUBitrateAudio->Value);
				} else {
					SetNUValue(fbcNUBitrateSum, fbcNUBitrateAudio->Value + fbcNUBitrateVideo->Value);
					fbcTXSize->Text = CalcSize((int)fbcNUBitrateSum->Value, GetDurationSec()).ToString("F2");
				}
				enable_events = true;
			}
		}
	private: 
		System::Void fbcNUAudioBitrate_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			if (enable_events) {
				enable_events = false;
				if (fbcRBCalcRate->Checked) {
					SetNUValue(fbcNUBitrateVideo, fbcNUBitrateSum->Value - fbcNUBitrateAudio->Value);
					SetNUValue(fbcNUBitrateAudio, fbcNUBitrateSum->Value - fbcNUBitrateVideo->Value);
				} else {
					SetNUValue(fbcNUBitrateSum, fbcNUBitrateAudio->Value + fbcNUBitrateVideo->Value);
					fbcTXSize->Text = CalcSize((int)fbcNUBitrateSum->Value, GetDurationSec()).ToString("F2");
				}
				enable_events = true;
			}
		}
	private: 
		System::Void fbcNUBitrateSum_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			if (enable_events) {
				enable_events = false;
				if (fbcRBCalcRate->Checked) {
					SetNUValue(fbcNUBitrateVideo, fbcNUBitrateSum->Value - fbcNUBitrateAudio->Value);
					SetNUValue(fbcNUBitrateAudio, fbcNUBitrateSum->Value - fbcNUBitrateVideo->Value);
				} else {
					SetNUValue(fbcNUBitrateVideo, fbcNUBitrateSum->Value - fbcNUBitrateAudio->Value);
					SetNUValue(fbcNUBitrateAudio, fbcNUBitrateSum->Value - fbcNUBitrateVideo->Value);
					fbcTXSize->Text = CalcSize((int)fbcNUBitrateSum->Value, GetDurationSec()).ToString("F2");
				}
				enable_events = true;
			}
		}
	private: 
		System::Void fbcTXSize_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			if (fbcTXSize->Text->Length == 0)
				return;
			int c = fbcTXSize->SelectionStart;
			double d;
			bool restore = false;
			if (!Double::TryParse(fbcTXSize->Text, d)) {
				fbcTXSize->Text = LastStr;
				restore = true;
			} else if (enable_events) {
				enable_events = false;
				if (fbcRBCalcRate->Checked) {
					if (GetSize() > 2*1024*1024) fbcTXSize->Text = (2*1024*1024).ToString();
					if (GetDurationSec()) {
						int va = CalcBitate(GetSize(), GetDurationSec());
						SetNUValue(fbcNUBitrateSum, va);
						SetNUValue(fbcNUBitrateVideo, fbcNUBitrateSum->Value - fbcNUBitrateAudio->Value);
						SetNUValue(fbcNUBitrateAudio, fbcNUBitrateSum->Value - fbcNUBitrateVideo->Value);
					}
				} else {
					//計算しない
				}
				enable_events = true;
			}
			//カーソルの位置調整
			fbcTXSize->SelectionStart = clamp(c - Convert::ToInt32(restore), 0, fbcTXSize->Text->Length);
			fbcTXSize->SelectionLength = 0;
			LastStr = fbcTXSize->Text;
		}
	private:
		System::Int32 CalcBitate(double size_MB, int dur_sec) {
			return (dur_sec) ? (int)(size_MB * (1024.0 * 1024.0 / 1000.0) * 8.0 / dur_sec) : 0;
		}
	private:
		double CalcSize(int bitrate_kbps, int dur_sec) {
			return bitrate_kbps * (1000.0 / (1024.0 * 1024.0 * 8.0)) * dur_sec;
		}
	private:
		double GetSize() {
			return Convert::ToDouble(fbcTXSize->Text);
		}
	private:
		System::Void fbcNULength_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			if (enable_events) {
				enable_events = false;
				if (fbcRBCalcRate->Checked) {
					if (GetDurationSec()) {
						int va = CalcBitate(GetSize(), GetDurationSec());
						SetNUValue(fbcNUBitrateSum, va);
						SetNUValue(fbcNUBitrateVideo, fbcNUBitrateSum->Value - fbcNUBitrateAudio->Value);
						SetNUValue(fbcNUBitrateAudio, fbcNUBitrateSum->Value - fbcNUBitrateVideo->Value);
					}
				} else {
					fbcTXSize->Text = CalcSize((int)fbcNUBitrateSum->Value, GetDurationSec()).ToString("F2");
				}
				enable_events = true;
			}
		}
	private:
		System::Void frmBitrateCalculator_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
			if (e->KeyCode == Keys::Escape)
				this->Close();
		}
};
}

#pragma warning( pop )
