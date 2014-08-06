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

#include <string>

using namespace System;

static void GetCHARfromString(char *chr, DWORD nSize, System::String^ str) {
	System::IntPtr ptr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(str);
	strcpy_s(chr, nSize, (char *)ptr.ToPointer());
	System::Runtime::InteropServices::Marshal::FreeHGlobal(ptr);
}

static int CountStringBytes(System::String^ str) {
	return System::Text::Encoding::GetEncoding("Shift_JIS")->GetByteCount(str);
}

static String^ MakeExeFilter(System::String^ fileExeName) {
	String^ fileName = System::IO::Path::GetFileNameWithoutExtension(fileExeName);
	String^ filter = fileName + L"*" + System::IO::Path::GetExtension(fileExeName);
	return fileName + L" (" + filter + L")|" + filter;
}

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

#endif //_X264GUIEX_CLRUTIL_H_