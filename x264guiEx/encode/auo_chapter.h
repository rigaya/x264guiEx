//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _AUO_CHAPTER_H_
#define _AUO_CHAPTER_H_

#include <Windows.h>

enum AuoChapStatus {
	AUO_CHAP_ERR_NONE = 0,
	AUO_CHAP_ERR_FILE_OPEN,
	AUO_CHAP_ERR_FILE_WRITE,
	AUO_CHAP_ERR_FILE_READ,
	AUO_CHAP_ERR_FILE_SWAP,
	AUO_CHAP_ERR_NULL_PTR,
	AUO_CHAP_ERR_INIT_IMUL2,
	AUO_CHAP_ERR_CONVERTION,
	AUO_CHAP_ERR_CP_DETECT,
	AUO_CHAP_ERR_INVALID_FMT,
	AUO_CHAP_ERR_INIT_XML_PARSER,
	AUO_CHAP_ERR_INIT_READ_STREAM,
	AUO_CHAP_ERR_FAIL_SET_STREAM,
	AUO_CHAP_ERR_PARSE_XML
};

//チャプターファイルの変換を行う
//基本的にはorig_nero_filename(nero形式) から new_apple_filename(apple形式) へ
//orig_fileがapple形式の場合、nero形式を出力してファイル名をスワップする
AuoChapStatus convert_chapter(const char *new_apple_filename, const char *orig_nero_filename, DWORD orig_code_page, DWORD duration_ms);

#endif //_AUO_CHAPTER_H_