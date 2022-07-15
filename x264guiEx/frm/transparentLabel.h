// -----------------------------------------------------------------------------------------
// x264guiEx/x265guiEx/svtAV1guiEx by rigaya
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

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::IO;

// 透過なLabelを作成する
// コントロールの背景を透過・透明にする - IYouryellable様
//   http://youryella.wankuma.com/Library/Extensions/Control/Transparent.aspx
// 上記を参考にC++/Cli用に改変したもの + アルファ(マウスメッセージの透過機能の追加)

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
	public ref class TransparentLabel : public System::Windows::Forms::Label
	{
		public:
			TransparentLabel() {
				// 適用されていない場合は SupportsTransparentBackColor を true にする
				this->SetStyle(ControlStyles::SupportsTransparentBackColor, true);
				AllowTransparency = true;
				AllowMouseEventTransparency = true;
			}

	private:
		bool _AllowTransparency;
		/// <summary>
		/// 背景が透明なとき背面のコントロールを描画するかどうかを示す値取得または設定します。
		/// </summary>
		//[Category("表示")]
		//[DefaultValue(false)]
		//[Description("背景が透明なとき背面のコントロールが表示されるどうかを示します。")]
	public:
		property bool AllowTransparency {
			bool get(void) { return _AllowTransparency; }
			void set(bool value) {
				if (_AllowTransparency == value) {
					return;
				}

				_AllowTransparency = value;

				this->Invalidate();
			}
		}

	private:
		bool _AllowMouseEventTransparency;
		/// <summary>
		/// 背景が透明なとき背面のコントロールを描画するかどうかを示す値取得または設定します。
		/// </summary>
		//[Category("表示")]
		//[DefaultValue(false)]
		//[Description("背景が透明なとき背面のコントロールが表示されるどうかを示します。")]
	public:
		property bool AllowMouseEventTransparency {
			bool get(void) { return _AllowMouseEventTransparency; }
			void set(bool value) {
				if (_AllowMouseEventTransparency == value) {
					return;
				}

				_AllowMouseEventTransparency = value;

				this->Invalidate();
			}
		}

	protected:
		//マウスメッセージの透過
		virtual void WndProc(System::Windows::Forms::Message %m) override {

			if (AllowMouseEventTransparency && m.Msg == 0x0084) //WM_NCHITTEST
			{
				m.Result = (IntPtr)(-1); //HTTRANSPARENT
			}
			else
			{
				__super::WndProc(m);
			}
		}

	protected:
		virtual void OnPaintBackground(System::Windows::Forms::PaintEventArgs^ pevent) override {
			// 背面のコントロールを描画しない Or 背景色が不透明なので背面のコントロールを描画する必要なし
			if ((this->AllowTransparency == false) || (this->BackColor.A == 255)) {
				__super::OnPaintBackground(pevent);
				return;
			}

			// 背面のコントロールを描画
			this->DrawParentWithBackControl(pevent);

			// 背景を描画
			this->DrawBackground(pevent);
		}

		/// <summary>
		/// コントロールの背景を描画します。
		/// </summary>
		/// <param name="pevent">描画先のコントロールに関連する情報</param>
	private:
		void DrawBackground(System::Windows::Forms::PaintEventArgs^ pevent) {
			// 背景色
			SolidBrush^ sb = gcnew SolidBrush(this->BackColor);
			try {
				pevent->Graphics->FillRectangle(sb, this->ClientRectangle);
			} finally {
				if (sb != nullptr)
					delete sb;
			}

			// 背景画像
			if (this->BackgroundImage != nullptr) {
				this->DrawBackgroundImage(pevent->Graphics, this->BackgroundImage, this->BackgroundImageLayout);
			}
		}

		/// <summary>
		/// コントロールの背景画像を描画します。
		/// </summary>
		/// <param name="g">描画に使用するグラフィックス オブジェクト</param>
		/// <param name="img">描画する画像</param>
		/// <param name="layout">画像のレイアウト</param>
	private:
		void DrawBackgroundImage(Graphics^ g, System::Drawing::Image^ img, ImageLayout layout) {
			System::Drawing::Size imgSize = img->Size;

			switch (layout) {
				case ImageLayout::None:
					g->DrawImage(img, 0, 0, imgSize.Width, imgSize.Height);

					break;
				case ImageLayout::Tile:
					{
						int xCount = Convert::ToInt32(Math::Ceiling((double)this->ClientRectangle.Width / (double)imgSize.Width));
						int yCount = Convert::ToInt32(Math::Ceiling((double)this->ClientRectangle.Height / (double)imgSize.Height));
						for (int x = 0; x <= xCount - 1; x++) {
							for (int y = 0; y <= yCount - 1; y++) {
								g->DrawImage(img, imgSize.Width * x, imgSize.Height * y, imgSize.Width, imgSize.Height);
							}
						}

						break;
					}
				case ImageLayout::Center:
					{
						int x = 0;
						if (this->ClientRectangle.Width > imgSize.Width) {
							x = (int)Math::Floor((double)(this->ClientRectangle.Width - imgSize.Width) / 2.0);
						}
						int y = 0;
						if (this->ClientRectangle.Height > imgSize.Height) {
							y = (int)Math::Floor((double)(this->ClientRectangle.Height - imgSize.Height) / 2.0);
						}
						g->DrawImage(img, x, y, imgSize.Width, imgSize.Height);

						break;
					}
				case ImageLayout::Stretch:
					g->DrawImage(img, 0, 0, this->ClientRectangle.Width, this->ClientRectangle.Height);

					break;
				case ImageLayout::Zoom:
					{
						double xRatio = (double)this->ClientRectangle.Width / (double)imgSize.Width;
						double yRatio = (double)this->ClientRectangle.Height / (double)imgSize.Height;
						double minRatio = Math::Min(xRatio, yRatio);

						System::Drawing::Size zoomSize = System::Drawing::Size(Convert::ToInt32(Math::Ceiling(imgSize.Width * minRatio)), Convert::ToInt32(Math::Ceiling(imgSize.Height * minRatio)));

						int x = 0;
						if (this->ClientRectangle.Width > zoomSize.Width) {
							x = (int)Math::Floor((double)(this->ClientRectangle.Width - zoomSize.Width) / 2.0);
						}
						int y = 0;
						if (this->ClientRectangle.Height > zoomSize.Height) {
							y = (int)Math::Floor((double)(this->ClientRectangle.Height - zoomSize.Height) / 2.0);
						}
						g->DrawImage(img, x, y, zoomSize.Width, zoomSize.Height);

						break;
					}
			}
		}

		/// <summary>
		/// 親コントロールと背面にあるコントロールを描画します。
		/// </summary>
		/// <param name="pevent">描画先のコントロールに関連する情報</param>
	private:
		void DrawParentWithBackControl(System::Windows::Forms::PaintEventArgs^ pevent) {
			// 親コントロールを描画
			this->DrawParentControl(this->Parent, pevent);

			// 親コントロールとの間のコントロールを親側から描画
			for (int i = this->Parent->Controls->Count - 1; i >= 0; i--) {
				Control^ c = this->Parent->Controls[i];
				if (c == this) {
					break;
				}
				if (this->Bounds.IntersectsWith(c->Bounds) == false) {
					continue;
				}

				this->DrawBackControl(c, pevent);
			}
		}

		/// <summary>
		/// 親コントロールを描画します。
		/// </summary>
		/// <param name="c">親コントロール</param>
		/// <param name="pevent">描画先のコントロールに関連する情報</param>
	private:
		void DrawParentControl(Control^ c, System::Windows::Forms::PaintEventArgs^ pevent) {
			Bitmap^ bmp = gcnew Bitmap(c->Width, c->Height, System::Drawing::Imaging::PixelFormat::Format32bppArgb);
			try {
				Graphics^ g = Graphics::FromImage(bmp);
				try {
					PaintEventArgs^ p = gcnew PaintEventArgs(g, c->ClientRectangle);
					try {
						this->InvokePaintBackground(c, p);
						this->InvokePaint(c, p);
					} finally {
						if (p != nullptr)
							delete p;
					}
				} finally {
					if (g != nullptr)
						delete g;
				}

				int offsetX = this->Left + (int)Math::Floor((double)(this->Bounds.Width - this->ClientRectangle.Width) / 2.0);
				int offsetY = this->Top + (int)Math::Floor((double)(this->Bounds.Height - this->ClientRectangle.Height) / 2.0);
				pevent->Graphics->DrawImage(bmp, this->ClientRectangle, System::Drawing::Rectangle(offsetX, offsetY, this->ClientRectangle.Width, this->ClientRectangle.Height), GraphicsUnit::Pixel);
			} finally {
				if (bmp != nullptr)
					delete bmp;
			}
		}

		/// <summary>
		/// 背面のコントロールを描画します。
		/// </summary>
		/// <param name="c">背面のコントロール</param>
		/// <param name="pevent">描画先のコントロールに関連する情報</param>
	private:
		void DrawBackControl(Control^ c, System::Windows::Forms::PaintEventArgs^ pevent) {
			Bitmap^ bmp = gcnew Bitmap(c->Width, c->Height, System::Drawing::Imaging::PixelFormat::Format32bppArgb);
			try {
				c->DrawToBitmap(bmp, System::Drawing::Rectangle(0, 0, c->Width, c->Height));

				int offsetX = (c->Left - this->Left) - (int)Math::Floor((double)(this->Bounds.Width - this->ClientRectangle.Width) / 2.0);
				int offsetY = (c->Top - this->Top) - (int)Math::Floor((double)(this->Bounds.Height - this->ClientRectangle.Height) / 2.0);
				pevent->Graphics->DrawImage(bmp, offsetX, offsetY, c->Width, c->Height);
			} finally {
				if (bmp != nullptr)
					delete bmp;
			}
		}
	};
}
