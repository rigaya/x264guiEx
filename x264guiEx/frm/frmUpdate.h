//  -----------------------------------------------------------------------------------------
//    拡張 x264/x265 出力(GUI) Ex  v1.xx/2.xx/3.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#pragma once

#include "frmConfig_helper.h"

namespace x264guiEx {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;
    using namespace System::Threading;

    /// <summary>
    /// frmUpdate の概要
    /// </summary>
    public ref class frmUpdate : public System::Windows::Forms::Form
    {
    public:
        frmUpdate(void)
        {

            InitializeComponent();
            //
            //TODO: ここにコンストラクター コードを追加します
            //
        }

    protected:
        /// <summary>
        /// 使用中のリソースをすべてクリーンアップします。
        /// </summary>
        ~frmUpdate()
        {
            //ウィンドウに関連づいているDelegateなどが存在する可能性のあるauoSetupCtrlをまずdelete
            if (nullptr != auoSetupCtrl) {
                delete auoSetupCtrl;
            }
            if (components)
            {
                delete components;
            }
        }
    private: System::Windows::Forms::DataGridView^  fruDGExeFiles;
    private: System::Windows::Forms::ProgressBar^  fruPBUpdate;
    private: System::Windows::Forms::Label^  fruLBUpdate;
    protected:



    private: System::Windows::Forms::Button^  fruBTUpdate;

    private: System::Windows::Forms::TextBox^  fruTXUpdate;

    protected:

    private:
        /// <summary>
        /// 必要なデザイナー変数です。
        /// </summary>
        System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
        /// <summary>
        /// デザイナー サポートに必要なメソッドです。このメソッドの内容を
        /// コード エディターで変更しないでください。
        /// </summary>
        void InitializeComponent(void)
        {
            this->fruDGExeFiles = (gcnew System::Windows::Forms::DataGridView());
            this->fruPBUpdate = (gcnew System::Windows::Forms::ProgressBar());
            this->fruLBUpdate = (gcnew System::Windows::Forms::Label());
            this->fruBTUpdate = (gcnew System::Windows::Forms::Button());
            this->fruTXUpdate = (gcnew System::Windows::Forms::TextBox());
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fruDGExeFiles))->BeginInit();
            this->SuspendLayout();
            // 
            // fruDGExeFiles
            // 
            this->fruDGExeFiles->AllowUserToAddRows = false;
            this->fruDGExeFiles->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
                | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->fruDGExeFiles->AutoSizeColumnsMode = System::Windows::Forms::DataGridViewAutoSizeColumnsMode::Fill;
            this->fruDGExeFiles->BackgroundColor = System::Drawing::SystemColors::Control;
            this->fruDGExeFiles->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
            this->fruDGExeFiles->Location = System::Drawing::Point(14, 52);
            this->fruDGExeFiles->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->fruDGExeFiles->Name = L"fruDGExeFiles";
            this->fruDGExeFiles->ReadOnly = true;
            this->fruDGExeFiles->RowTemplate->Height = 21;
            this->fruDGExeFiles->Size = System::Drawing::Size(415, 127);
            this->fruDGExeFiles->TabIndex = 0;
            // 
            // fruPBUpdate
            // 
            this->fruPBUpdate->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->fruPBUpdate->Location = System::Drawing::Point(14, 373);
            this->fruPBUpdate->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->fruPBUpdate->Name = L"fruPBUpdate";
            this->fruPBUpdate->Size = System::Drawing::Size(260, 29);
            this->fruPBUpdate->TabIndex = 30;
            this->fruPBUpdate->Visible = false;
            // 
            // fruLBUpdate
            // 
            this->fruLBUpdate->AutoSize = true;
            this->fruLBUpdate->Location = System::Drawing::Point(14, 18);
            this->fruLBUpdate->Name = L"fruLBUpdate";
            this->fruLBUpdate->Size = System::Drawing::Size(224, 15);
            this->fruLBUpdate->TabIndex = 29;
            this->fruLBUpdate->Text = L"各種実行ファイルの更新を行うことができます。";
            // 
            // fruBTUpdate
            // 
            this->fruBTUpdate->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
            this->fruBTUpdate->Location = System::Drawing::Point(284, 363);
            this->fruBTUpdate->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->fruBTUpdate->Name = L"fruBTUpdate";
            this->fruBTUpdate->Size = System::Drawing::Size(145, 45);
            this->fruBTUpdate->TabIndex = 28;
            this->fruBTUpdate->Text = L"更新チェック";
            this->fruBTUpdate->UseVisualStyleBackColor = true;
            this->fruBTUpdate->Click += gcnew System::EventHandler(this, &frmUpdate::fruBTUpdate_Click);
            // 
            // fruTXUpdate
            // 
            this->fruTXUpdate->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->fruTXUpdate->Location = System::Drawing::Point(14, 187);
            this->fruTXUpdate->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->fruTXUpdate->Multiline = true;
            this->fruTXUpdate->Name = L"fruTXUpdate";
            this->fruTXUpdate->ReadOnly = true;
            this->fruTXUpdate->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
            this->fruTXUpdate->Size = System::Drawing::Size(413, 168);
            this->fruTXUpdate->TabIndex = 31;
            // 
            // frmUpdate
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;
            this->ClientSize = System::Drawing::Size(442, 423);
            this->Controls->Add(this->fruTXUpdate);
            this->Controls->Add(this->fruPBUpdate);
            this->Controls->Add(this->fruLBUpdate);
            this->Controls->Add(this->fruBTUpdate);
            this->Controls->Add(this->fruDGExeFiles);
            this->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
            this->Name = L"frmUpdate";
            this->Text = L"実行ファイルのアップデート";
            this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &frmUpdate::frmUpdate_FormClosing);
            this->Load += gcnew System::EventHandler(this, &frmUpdate::frmUpdate_Load);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fruDGExeFiles))->EndInit();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
        bool updateFinWaiting;
        System::Object^ updateFinLockObject;
        ManualResetEvent^ updateFinEvent;
        auoSetupControl^ auoSetupCtrl;
        DataTable^ versionInfo;
        DelegateProcessFin^ checkFin;  //更新チェック終了時のfrmConfig側へのコールバック
        DelegateProcessFin^ updateFin; //更新終了時のfrmConfig側へのコールバック
        System::Void frmUpdate_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);
    public:
        System::Boolean waitingForUpdateFinishes() {
            if (updateFinWaiting)
                this->Show();
            return updateFinWaiting;
        }
    public:
        //frmConfig側へのコールバックを受け取っておく
        System::Void init(DelegateProcessFin^ progCheckFin, DelegateProcessFin^ progUpdateFin) {
            this->checkFin = progCheckFin;
            this->updateFin = progUpdateFin;
            updateFinEvent = nullptr;
            updateFinLockObject = gcnew System::Object();
            updateFinWaiting = false;

            if (nullptr == auoSetupCtrl) {
                auoSetupCtrl = gcnew auoSetupControl();
                auoSetupCtrl->init();
                versionInfo = auoSetupCtrl->getVersionInfoDataTable();
                fruDGExeFiles->DataSource = versionInfo;
            }
        }
    public:
        //更新チェックの開始
        System::Void startCheck() {
            //frmConfig側へのコールバックと自分(frmUpdate)へのコールバックを両方渡す
            auoSetupCtrl->checkAsync(checkFin + gcnew x264guiEx::DelegateProcessFin(this, &frmUpdate::frmCheckFinished));

            fruTXUpdate->Text = L"更新情報を取得しています...";
            fruPBUpdate->Minimum = 0;
            fruPBUpdate->Maximum = 100;
            fruPBUpdate->Value = 0;
            fruPBUpdate->Style = ProgressBarStyle::Marquee;
            fruPBUpdate->Visible = true;
            fruBTUpdate->Enabled = false;
        }
    private:
        System::Void frmUpdate_Load(System::Object^  sender, System::EventArgs^  e) {
            if (nullptr == auoSetupCtrl) {
                auoSetupCtrl = gcnew auoSetupControl();
                auoSetupCtrl->init();
                versionInfo = auoSetupCtrl->getVersionInfoDataTable();
                fruDGExeFiles->DataSource = versionInfo;
            }
            if (0 == String::Compare(fruBTUpdate->Text, L"更新チェック")) {
                startCheck();
            }
        }
    private:
        System::Void frmMessage(String^ message) {
            if (this->InvokeRequired) {
                this->Invoke(gcnew x264guiEx::DelegateMessage(this, &frmUpdate::frmMessage), message);
                return;
            }
            fruTXUpdate->AppendText(message);
            if (this->Visible) {
                fruTXUpdate->ScrollToCaret();
            }
        }
    private:
        //更新チェック終了のコールバック
        System::Void frmCheckFinished(String^ message) {
            if (this->InvokeRequired) {
                this->Invoke(gcnew x264guiEx::DelegateMessage(this, &frmUpdate::frmCheckFinished), message);
                return;
            }
            fruPBUpdate->Style = ProgressBarStyle::Continuous;
            fruPBUpdate->Visible = false;

            fruTXUpdate->Clear();
            fruTXUpdate->Text = message;

            fruBTUpdate->Text = L"更新";
            fruBTUpdate->Enabled = auoSetupControl::checkIfUpdateAvailable(message);
        }
    private:
        //更新終了のコールバック
        //要更新のエラーチェック
        System::Void frmUpdateFinished(String^ message) {
            if (this->InvokeRequired) {
                this->Invoke(gcnew x264guiEx::DelegateMessage(this, &frmUpdate::frmUpdateFinished), message);
                return;
            }
            switch (auoSetupControl::strToResult(message)) {
            case auoSetupControl::auoSetupResult::success:
                fruTXUpdate->AppendText("更新終了\r\n");

                //プログレスバーに更新終了を表示
                fruPBUpdate->Style = ProgressBarStyle::Continuous;
                fruPBUpdate->Value = 100;

                fruBTUpdate->Text = L"終了";
                fruBTUpdate->Enabled = true;
                break;
            case auoSetupControl::auoSetupResult::abort:
                fruPBUpdate->Style = ProgressBarStyle::Continuous;
                fruPBUpdate->Visible = false;

                fruBTUpdate->Text = L"更新";
                fruBTUpdate->Enabled = true;
                break;
            case auoSetupControl::auoSetupResult::error:
            default:
                fruPBUpdate->Visible = false;

                fruBTUpdate->Text = L"終了";
                fruBTUpdate->Enabled = true;
                break;
            }
            updateFinWaiting = false;
        }
    private:
        //更新ボタンのチェック
        System::Void fruBTUpdate_Click(System::Object^  sender, System::EventArgs^  e) {
            if (0 == String::Compare(fruBTUpdate->Text, L"終了")) {
                this->Close();
            } else if (0 == String::Compare(fruBTUpdate->Text, L"中止")) {
                auoSetupCtrl->abortAuoSetup();
            } else if (0 == String::Compare(fruBTUpdate->Text, L"更新")) {
                fruTXUpdate->Clear();
                fruPBUpdate->Minimum = 0;
                fruPBUpdate->Maximum = 100;
                fruPBUpdate->Value = 0;
                fruPBUpdate->Style = ProgressBarStyle::Marquee;
                fruPBUpdate->Visible = true;

                updateFinWaiting = true;

                //frmConfig側へのコールバックと自分(frmUpdate)へのコールバックを両方渡す
                auoSetupCtrl->runAsync(
                    gcnew x264guiEx::DelegateMessage(this, &frmUpdate::frmMessage),
                    updateFin + gcnew x264guiEx::DelegateProcessFin(this, &frmUpdate::frmUpdateFinished));
                fruBTUpdate->Text = L"中止";
            } else {
                startCheck();
            }
        }
};
}
