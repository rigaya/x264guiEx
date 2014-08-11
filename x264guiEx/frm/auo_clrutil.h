///  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _X264GUIEX_CLRUTIL_H_
#define _X264GUIEX_CLRUTIL_H_

#include <Windows.h>
#include "auo.h"
#include "auo_util.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

static size_t GetCHARfromString(char *chr, DWORD nSize, System::String^ str) {
	DWORD str_len;
	System::IntPtr ptr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(str);
	char *ch_ptr = (char *)ptr.ToPointer();
	if ((str_len = (DWORD)strlen(ch_ptr)) >= nSize)
		return 0; //バッファサイズを超えていたら何もしない
	memcpy(chr, ch_ptr, str_len+1);
	System::Runtime::InteropServices::Marshal::FreeHGlobal(ptr);
	return str_len;
}

static int CountStringBytes(System::String^ str) {
	return System::Text::Encoding::GetEncoding(L"Shift_JIS")->GetByteCount(str);
}

static String^ MakeExeFilter(System::String^ fileExeName) {
	String^ fileName = System::IO::Path::GetFileNameWithoutExtension(fileExeName);
	String^ filter = fileName + L"*" + System::IO::Path::GetExtension(fileExeName);
	return fileName + L" (" + filter + L")|" + filter;
}

//使用できないファイル名かどうか確認する
static bool ValidiateFileName(System::String^ fileName) {
	array<wchar_t>^ InvalidChars = {L'\\', L'/', L':', L'*', L'?', L'\"', L'<', L'>', L'|'};
	array<String^>^ InvalidString = { L"CON", L"PRN", L"AUX", L"CLOCK$", L"NUL",
    L"COM0", L"COM1", L"COM2", L"COM3", L"COM4", L"COM5", L"COM6", L"COM7", L"COM8", L"COM9",
    L"LPT0", L"LPT1", L"LPT2", L"LPT3", L"LPT4", L"LPT5", L"LPT6", L"LPT7", L"LPT8", L"LPT9" };
	if (fileName->IndexOfAny(InvalidChars) >= 0)
		return false;
	for (int i = 0; i < InvalidString->Length; i++)
		if (String::Compare(fileName, InvalidString[i], true) == NULL)
			return false;
	return true;
}

//AUO_FONT_INFOからフォントを作成する
//情報がない場合、baseFontのものを使用する
static System::Drawing::Font^ GetFontFrom_AUO_FONT_INFO(const AUO_FONT_INFO *info, System::Drawing::Font^ baseFont) {
	if (info && (str_has_char(info->name) || info->size > 0.0)) {
		return gcnew System::Drawing::Font(
			(str_has_char(info->name)) ? String(info->name).ToString() : baseFont->FontFamily->ToString(),
			(info->size > 0.0) ? (float)info->size : baseFont->Size, 
			(System::Drawing::FontStyle)info->style);
	}
	return baseFont;
}

//DefaultFontと比較して、異なっていたらAUO_FONT_INFOに保存する
static void Set_AUO_FONT_INFO(AUO_FONT_INFO *info, System::Drawing::Font^ Font, System::Drawing::Font^ DefaultFont) {				
	if (String::Compare(DefaultFont->FontFamily->Name, Font->FontFamily->Name))
		GetCHARfromString(info->name, sizeof(info->name), Font->FontFamily->Name);
	if (DefaultFont->Size != Font->Size)
		info->size = Font->Size;
	info->style = (int)Font->Style;
}

//ToolStripへのフォントの適用
static void SetFontFamilyToToolStrip(ToolStrip^ TS, FontFamily^ NewFontFamily, FontFamily^ BaseFontFamily) {
	for (int i = 0; i < TS->Items->Count; i++) {
		TS->Items[i]->Font = gcnew Font(NewFontFamily, TS->Items[i]->Font->Size, TS->Items[i]->Font->Style);
	}
}

//再帰を用いて全コントロールにフォントを適用する
static void SetFontFamilyToControl(Control^ top, FontFamily^ NewFontFamily, FontFamily^ BaseFontFamily) {
	for (int i = 0; i < top->Controls->Count; i++) {
		System::Type^ type = top->Controls[i]->GetType();
		if (type == ToolStrip::typeid)
			SetFontFamilyToToolStrip((ToolStrip^)top->Controls[i], NewFontFamily, BaseFontFamily);
		else
			SetFontFamilyToControl(top->Controls[i], NewFontFamily, BaseFontFamily);
	}
	top->Font = gcnew Font(NewFontFamily, top->Font->Size, top->Font->Style);
}

//フォントが存在するかチェックする
static bool CheckFontFamilyExists(FontFamily^ targetFontFamily) {
	//新しくフォントを作ってみて、設定しようとしたフォントと違ったら諦める
	Font^ NewFont = gcnew Font(targetFontFamily, 12); //12は適当
	return (String::Compare(NewFont->FontFamily->Name, targetFontFamily->Name)) ? false : true;
}

//フォーム全体にフォントを適用する
static void SetFontFamilyToForm(Form^ form, FontFamily^ NewFontFamily, FontFamily^ BaseFontFamily) {
	if (!CheckFontFamilyExists(NewFontFamily))
		return;
	::SendMessage((HWND)form->Handle.ToPointer(), WM_SETREDRAW, false, 0); //描画を停止
	SetFontFamilyToControl(form, NewFontFamily, BaseFontFamily);
	::SendMessage((HWND)form->Handle.ToPointer(), WM_SETREDRAW, true, 0); //描画再開
	form->Refresh(); //強制再描画
}

#endif //_X264GUIEX_CLRUTIL_H_
