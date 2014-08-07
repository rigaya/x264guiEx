//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <stdio.h>
#include <vector>

#include <objbase.h>
#pragma comment(lib, "ole32.lib")
#include <mlang.h>
#include <xmllite.h>
#pragma comment (lib, "xmllite.lib")
#include <shlwapi.h>
#pragma comment (lib, "shlwapi.lib")

#include "auo.h"
#include "auo_util.h"
#include "auo_chapter.h"

enum AuoChapType {
	CHAP_TYPE_UNKNOWN = 0,
	CHAP_TYPE_NERO    = 1,
	CHAP_TYPE_APPLE   = 2,
};
//読み込みバッファサイズ
static const int CHAP_CONVERT_BUF_LEN = 8192;

//チャプターの種類を大雑把に判別
static AuoChapType check_chap_type(const WCHAR *data) {
	if (data == NULL || wcslen(data) == 0)
		return CHAP_TYPE_UNKNOWN;
	const WCHAR *rw, *qw, *pw = wcsstr(data, L"CHAPTER");
	if (pw != NULL) {
		qw = wcschr(pw, L'=');
		rw = wcschr(qw, L'\n');
		if (rw == NULL)
			rw = wcschr(qw, L'\r');
		if (qw && rw && wcsncmp(rw+1, pw, qw - pw) == NULL)
			return CHAP_TYPE_NERO;
	}
	if (wcsstr(data, L"<TextStream") && wcsstr(data, L"<TextSample"))
		return CHAP_TYPE_APPLE;

	return CHAP_TYPE_UNKNOWN;
}

//bufのWCHAR文字列をutf-8に変換しfpに出力
static AuoChapStatus write_utf8(FILE *fp, IMultiLanguage2 *pImul, WCHAR *buf, UINT *buf_len) {
	if (fp == NULL || pImul == NULL || buf == NULL)
		return AUO_CHAP_ERR_NULL_PTR;
	DWORD encMode = 0;
	std::vector<char> dst_buf;
	dst_buf.resize(wcslen(buf) * 3, '\0');
	UINT i_dst_buf = (UINT)dst_buf.size();

	UINT buf_len_in_byte = (buf_len) ? *buf_len * sizeof(WCHAR) : -1;
	DWORD last_len = buf_len_in_byte;
	if (S_OK != pImul->ConvertString(&encMode, CODE_PAGE_UTF16_LE, CODE_PAGE_UTF8, (BYTE *)buf, &buf_len_in_byte, (BYTE *)&dst_buf[0], &i_dst_buf))
		return AUO_CHAP_ERR_CONVERTION;
	if (buf_len) {
		//変換されなかったものを先頭に移動(そんなことないと思うけど念のため)
		memmove(buf, buf + buf_len_in_byte, last_len - buf_len_in_byte + sizeof(WCHAR));
		buf_len_in_byte = last_len - buf_len_in_byte;
		*buf_len = buf_len_in_byte / sizeof(WCHAR);
	}
	return (fwrite(&dst_buf[0], 1, i_dst_buf, fp) == i_dst_buf) ? AUO_CHAP_ERR_NONE : AUO_CHAP_ERR_FILE_WRITE;
}

static AuoChapStatus write_chapter_apple_header(FILE *fp, IMultiLanguage2 *pImul) {
	if (fp == NULL || pImul == NULL)
		return AUO_CHAP_ERR_NULL_PTR;
	fwrite(UTF8_BOM, 1, sizeof(UTF8_BOM), fp);
	return write_utf8(fp, pImul, 
	L"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n" 
	L"<TextStream version=\"1.1\">\r\n" 
	L"<TextStreamHeader>\r\n"
	L"<TextSampleDescription>\r\n"
	L"</TextSampleDescription>\r\n" 
	L"</TextStreamHeader>\r\n",
	 NULL);
}

//終端 長さも指定する(しないとわけのわからんdurationになる)
static AuoChapStatus write_chapter_apple_foot(FILE *fp, IMultiLanguage2 *pImul, double duration) {
	if (fp == NULL)
		return AUO_CHAP_ERR_NULL_PTR;
	WCHAR chap_foot[256];
	DWORD duration_ms = (DWORD)(duration * 1000.0 + 0.5);
	swprintf_s(chap_foot, sizeof(chap_foot) / sizeof(WCHAR), 
		L"<TextSample sampleTime=\"%02d:%02d:%02d.%03d\" text=\"\" />\r\n</TextStream>",
		duration_ms / (60*60*1000),
		(duration_ms % (60*60*1000)) / (60*1000),
		(duration_ms % (60*1000)) / 1000,
		duration_ms % 1000
		);
	return write_utf8(fp, pImul, chap_foot, NULL);
}

//Nero形式は xx:xx:xx:xxx でもいいらしいが Apple形式は xx:xx:xx.xxx でないとダメっぽいので修正
static AuoChapStatus fix_time_to_apple_format(WCHAR *time_str) {
	AuoChapStatus sts = AUO_CHAP_ERR_NONE;
	WCHAR *wp = time_str;
	int i = 0;
	while ((wp = wcschr(wp+1, L':')) != NULL) i++;

	switch (i) {
		case 2:  break;
		case 3:  *wp = L'.'; break;
		default: sts = AUO_CHAP_ERR_INVALID_FMT; break;
	}
	return sts;
}

//nero -> Apple 変換
static AuoChapStatus write_apple_chap(FILE *fp, IMultiLanguage2 *pImul, WCHAR *wchar_buf, UINT *wchar_buf_len, BOOL read_to_end) {
	if (fp == NULL || pImul == NULL || wchar_buf == NULL)
		return AUO_CHAP_ERR_NULL_PTR;
	if (wchar_buf_len && *wchar_buf_len == 0)
		return AUO_CHAP_ERR_NONE;

	static const WCHAR * const CHAP_KEY = L"CHAPTER";
	static const WCHAR * const ELEM_LINE = L"<TextSample sampleTime=\"%s\">%s</TextSample>\r\n";
	AuoChapStatus sts = AUO_CHAP_ERR_NONE;
	WCHAR *qw, *pw_data[2];
	const WCHAR *pw_key[] = { CHAP_KEY, NULL, NULL }; // 時間行, 名前行, ダミー

	//書き出し用バッファ
	std::vector<WCHAR> wbuf; 
	wbuf.resize(128);

	//BOM文字は飛ばし、改行コードで分解
	WCHAR last_word = wchar_buf[*wchar_buf_len];
	wchar_buf[*wchar_buf_len] = L'\0'; //null文字を付けておかないとwcstokがうまく働かない
	std::vector<WCHAR*> pw_line;
	const WCHAR * const delim = (wcschr(wchar_buf, L'\n')) ? L"\n" : L"\r"; //適切な改行コードを見つける
	for (WCHAR *pw = wchar_buf + (check_bom(wchar_buf) != CODE_PAGE_UNSET); (pw = wcstok_s(pw, delim, &qw)) != NULL; ) {
		pw_line.push_back(pw);
		pw = NULL;
	}

	//終端まで読めていなかったら、最後のブロックの解析は次に回す
	const int n = (int)((read_to_end) ? pw_line.size() : ((pw_line.size() - 1) / 2) * 2);
	for (int i = 0; i < n && !sts; i++) {
		deleteCRLFSpace_at_End(pw_line[i]);
		if (wcsncmp(pw_line[i], pw_key[i&1], wcslen(pw_key[i&1])) != NULL)
			return AUO_CHAP_ERR_INVALID_FMT;
		pw_key[(i&1) + 1] = pw_line[i];//CHAPTER KEY名を保存
		pw_data[i&1] = wcschr(pw_line[i], L'='); 
		*pw_data[i&1] = L'\0'; //CHAPTER KEY名を一つの文字列として扱えるように
		pw_data[i&1]++; //データは'='の次から
		if (i&1) {
			//必要な分のバッファを確保
			size_t wbuf_necessary = wcslen(ELEM_LINE) + wcslen(pw_data[0]) + wcslen(pw_data[1]); //大雑把だがこんなもんでよかろう
			if (wbuf_necessary > wbuf.size())
				wbuf.resize(wbuf_necessary);
			if ((sts = fix_time_to_apple_format(pw_data[0])) != AUO_CHAP_ERR_NONE)
				return sts;
			swprintf_s(&wbuf[0], wbuf.size(), ELEM_LINE, pw_data[0], pw_data[1]);
			sts = write_utf8(fp, pImul, &wbuf[0], NULL);
		}
	}
	if (read_to_end == FALSE) {
		//処理していない部分を移動
		//wcstokで削除した改行コードを戻す
		for (size_t i = n+1; i < pw_line.size(); i++)
			*(pw_line[i] - 1) = *delim;
		wchar_buf[*wchar_buf_len] = last_word; //null文字を入れたところを戻す
		*wchar_buf_len = wchar_buf + *wchar_buf_len - pw_line[n];
		memmove(wchar_buf, pw_line[n], (*wchar_buf_len + 1) * sizeof(WCHAR));
	}
	return sts;
}

//Apple -> Nero変換
//fpはすでに開かれたnewfileへのファイルポインタ。
//orig_filenameは変換元となるApple形式のファイル名。
//こいつをXmlパーサで解析する
static AuoChapStatus write_nero_chap(FILE *fp, const char *orig_filename) {
	if (fp == NULL || orig_filename == NULL)
		return AUO_CHAP_ERR_NULL_PTR;

	AuoChapStatus sts = AUO_CHAP_ERR_NONE;
	static const WCHAR * const ELEMENT_NAME = L"TextSample";
	static const WCHAR * const ATTRIBUTE_NAME = L"sampleTime";
	static const char  * const KEY_BASE = "CHAPTER";
	static const char  * const KEY_NAME = "NAME";
	int key_num = 0;

	IXmlReader *pReader = NULL;
	IStream *pStream = NULL;

	//SJIS変換用
	std::vector<char> buf;
	buf.resize(128);

	CoInitialize(NULL);

	if (S_OK != CreateXmlReader(IID_PPV_ARGS(&pReader), NULL))
		sts = AUO_CHAP_ERR_INIT_XML_PARSER;
	else if (S_OK != SHCreateStreamOnFile(orig_filename, STGM_READ, &pStream))
		sts = AUO_CHAP_ERR_INIT_READ_STREAM;
	else if (S_OK != pReader->SetInput(pStream))
		sts = AUO_CHAP_ERR_FAIL_SET_STREAM;
	else {
		const WCHAR *pwLocalName = NULL, *pwValue = NULL;
		XmlNodeType nodeType;
		BOOL flag_next_line_is_time = TRUE; //次は時間を取得するべき
		size_t value_len;

		while (S_OK == pReader->Read(&nodeType)) {
			switch (nodeType) {
				case XmlNodeType_Element:
					if (S_OK != pReader->GetLocalName(&pwLocalName, NULL))
						return AUO_CHAP_ERR_PARSE_XML;
					if (wcscmp(ELEMENT_NAME, pwLocalName))
						break;
					if (S_OK != pReader->MoveToFirstAttribute())
						break;
					do {
						const WCHAR *pwAttributeName = NULL;
						const WCHAR *pwAttributeValue = NULL;
						if (S_OK != pReader->GetLocalName(&pwAttributeName, NULL))
							break;
						if (_wcsicmp(ATTRIBUTE_NAME, pwAttributeName))
							break;
						if (S_OK != pReader->GetValue(&pwAttributeValue, NULL))
							break;
						//必要ならバッファ拡張(想定される最大限必要なバッファに設定)
						value_len = wcslen(pwAttributeValue);
						if (value_len * sizeof(WCHAR) + 1 > buf.size())
							buf.resize(value_len * sizeof(WCHAR) + 1);
						ZeroMemory(&buf[0], sizeof(buf[0]) * buf.size());
						WideCharToMultiByte(CP_ACP, NULL, pwAttributeValue, (int)value_len, &buf[0], (int)buf.size(), NULL, NULL);
						key_num++;
						fprintf(fp, "%s%02d=%s\r\n", KEY_BASE, key_num, &buf[0]);
						flag_next_line_is_time = FALSE;
					} while (S_OK == pReader->MoveToNextAttribute());
					break;
				case XmlNodeType_Text:
					if (S_OK != pReader->GetValue(&pwValue, NULL))
						break;
					if (pwLocalName == NULL || wcscmp(pwLocalName, ELEMENT_NAME))
						break;
					if (flag_next_line_is_time)
						break;
					//必要ならバッファ拡張(想定される最大限必要なバッファに設定)
					value_len = wcslen(pwValue);
					if (value_len * sizeof(WCHAR) + 1 > buf.size())
						buf.resize(value_len * sizeof(WCHAR) + 1);
					//変換
					ZeroMemory(&buf[0], sizeof(buf[0]) * buf.size());
					WideCharToMultiByte(CP_ACP, NULL, pwValue, (int)value_len, &buf[0], (int)buf.size(), NULL, NULL);
					fprintf(fp, "%s%02d%s=%s\r\n", KEY_BASE, key_num, KEY_NAME, &buf[0]);
					flag_next_line_is_time = TRUE;
					break;
				default:
					break;
			}
		}
	}
	
	//リソース解放
	if (pReader)
		pReader->Release();
	if (pStream)
		pStream->Release();
	CoUninitialize();

	return sts;
}

AuoChapStatus convert_chapter(const char *new_filename, const char *orig_filename, DWORD orig_code_page, double duration) {
	if (new_filename == NULL || orig_filename == NULL || new_filename == orig_filename)
		return AUO_CHAP_ERR_NULL_PTR;

	AuoChapStatus sts = AUO_CHAP_ERR_NONE;
	FILE *fp_new = NULL;  //変換したファイル
	FILE *fp_orig = NULL; //オリジナルのファイル
	IMultiLanguage2 *pIMulLang = NULL;
	DWORD encMode = 0;
	DetectEncodingInfo dEnc = { 0 };
	int   denc_count = 1;

	char  src_buf[CHAP_CONVERT_BUF_LEN] = { 0 };
	UINT  src_buf_len   = 0;

	WCHAR wchar_buf[CHAP_CONVERT_BUF_LEN] = { 0 };
	UINT  wchar_buf_len = 0;

	WCHAR *wchar_ptr = NULL;

	AuoChapType chap_type = CHAP_TYPE_UNKNOWN;

	//COM用初期化
	CoInitialize(NULL);

	if        (fopen_s(&fp_orig, orig_filename, "rb") != NULL || fp_orig == NULL) {
		sts = AUO_CHAP_ERR_FILE_OPEN;
	} else if (fopen_s(&fp_new,  new_filename,  "wb") != NULL || fp_new == NULL) {
		sts = AUO_CHAP_ERR_FILE_OPEN;
	} else if (S_OK != CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, (void**)&pIMulLang) || pIMulLang == NULL) {
		sts = AUO_CHAP_ERR_INIT_IMUL2;
	} else if ((src_buf_len = (UINT)fread(src_buf, 1, sizeof(src_buf) - sizeof(WCHAR), fp_orig)) == 0) {
		sts = AUO_CHAP_ERR_FILE_READ;
	} else if (
		//多段階でコードページ判定を行う
		orig_code_page == CODE_PAGE_UNSET && //指定があればスキップ
		(orig_code_page = get_code_page(src_buf, src_buf_len)) == CODE_PAGE_UNSET && //まず主に日本語をチェック
		S_OK != pIMulLang->DetectInputCodepage(MLDETECTCP_NONE, 0, src_buf, (int *)&src_buf_len, &dEnc, &denc_count) && //IMultiLanguage2で判定してみる
		fix_ImulL_WesternEurope(&dEnc.nCodePage) != TRUE) { //日本語に対してはよく西ヨーロッパ言語を返すので修正(この関数がfalseを返すことはない)
		sts = AUO_CHAP_ERR_CP_DETECT;
	} else {
		if (orig_code_page == CODE_PAGE_UNSET) orig_code_page = dEnc.nCodePage;

		for (;;) {
			BOOL read_to_end = (src_buf_len < sizeof(src_buf) - sizeof(WCHAR)); //最後まで読めているかどうか
			//一度 Unicodeに変換する
			DWORD last_len = src_buf_len;
			UINT wchar_buf_remain = sizeof(wchar_buf) / sizeof(WCHAR) - wchar_buf_len;
			if (orig_code_page != CODE_PAGE_UTF16_LE) {
				if (S_OK != pIMulLang->ConvertStringToUnicode(&encMode, orig_code_page, 
					src_buf,                   &src_buf_len, 
					wchar_buf + wchar_buf_len, &wchar_buf_remain)) {
					sts = AUO_CHAP_ERR_CONVERTION; break;
				}
				//変換し終わったところを削除
				memmove(src_buf, src_buf + src_buf_len, last_len - src_buf_len + sizeof(WCHAR));
				src_buf_len = last_len - src_buf_len;

				wchar_buf_len = wchar_buf_remain;
				wchar_ptr = wchar_buf;
			} else {
				//変換不要
				wchar_buf_len = src_buf_len / sizeof(WCHAR);
				wchar_ptr = (WCHAR *)src_buf;
			}

			//初回はチャプターの種類を判定
			if (chap_type == CHAP_TYPE_UNKNOWN) {
				chap_type = check_chap_type(wchar_ptr);
				//Apple形式だった場合はXmlパーサで処理する
				if (chap_type == CHAP_TYPE_APPLE) {
					fclose(fp_orig); fp_orig = NULL;
					sts = write_nero_chap(fp_new, orig_filename);
					break; //ループから抜ける
				}
				//Nero形式だった場合、Apple形式に変換
				//まずはヘッダ出力
				if ((sts = write_chapter_apple_header(fp_new, pIMulLang)) != AUO_CHAP_ERR_NONE)
					break;
			}
			//nero形式 -> apple形式 変換
			if ((sts = write_apple_chap(fp_new, pIMulLang, wchar_ptr, &wchar_buf_len, read_to_end)) != AUO_CHAP_ERR_NONE)
				break;
			//UNICODEへ変換不要の場合、src_bufをそのまま使っているのでsrc_buf_lenを修正する
			if (orig_code_page == CODE_PAGE_UTF16_LE)
				src_buf_len = wchar_buf_len * sizeof(WCHAR) + (src_buf_len % 2);

			//直前に最後まで読み切っていたらここで終了
			if (read_to_end) {
				sts = write_chapter_apple_foot(fp_new, pIMulLang, duration);
				break;
			}

			//追加読み込み
			ZeroMemory(src_buf + src_buf_len, sizeof(src_buf) - src_buf_len * sizeof(src_buf[0]));
			UINT read_bytes = (UINT)fread(src_buf + src_buf_len, 1, sizeof(src_buf) - src_buf_len - sizeof(WCHAR), fp_orig);
			if (read_bytes == 0) {
				sts = AUO_CHAP_ERR_FILE_READ; break;
			}
			src_buf_len += read_bytes;
		}
	}

	//開放処理
	if (pIMulLang)
		pIMulLang->Release();
	if (fp_new)
		fclose(fp_new);
	if (fp_orig)
		fclose(fp_orig);

	//Apple -> Nero変換をしたのなら、ファイル名を入れ替える
	if (chap_type == CHAP_TYPE_APPLE)
		if (!swap_file(orig_filename, new_filename))
			sts = AUO_CHAP_ERR_FILE_SWAP;

	CoUninitialize();

	return sts;
}