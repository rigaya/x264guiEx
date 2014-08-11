//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <algorithm>

#include "auo.h"
#include "auo_util.h"
#include "auo_version.h"

void get_auo_path(char *auo_path, size_t nSize) {
	GetModuleFileName(GetModuleHandle(AUO_NAME), auo_path, (DWORD)nSize);
}
//最後に"\"なしで戻る
void get_aviutl_dir(char *aviutl_dir, size_t nSize) {
	GetModuleFileName(NULL, aviutl_dir, (DWORD)nSize);
	PathRemoveFileSpecFixed(aviutl_dir);
}
//文字列の置換に必要な領域を計算する
size_t calc_replace_mem_required(char *str, const char *old_str, const char *new_str) {
	size_t size = strlen(str) + 1;
	const int move_len = strlen(new_str) - strlen(old_str);
	if (move_len <= 0)
		return size;
	char *p = str;
	while ((p == strstr(p, old_str)) != NULL)
		size += move_len;
	return size;
}
//文字列の置換 str内で置き換える 置換を実行した回数を返す
int replace(char *str, size_t nSize, const char *old_str, const char *new_str) {
	char *c = str;
	char *p = NULL;
	char *fin = str + strlen(str) + 1;//null文字まで
	char * const limit = str + nSize;
	int count = 0;
	const size_t old_len = strlen(old_str);
	const size_t new_len = strlen(new_str);
	const int move_len = (int)(new_len - old_len);
	if (old_len) {
		while ((p = strstr(c, old_str)) != NULL) {
			if (move_len) {
				if (fin + move_len > limit)
					break;
				memmove((c = p + new_len), p + old_len, fin - (p + old_len));
				fin += move_len;
			}
			memcpy(p, new_str, new_len);
			count++;
		}
	}
	return count;
}

//ファイル名(拡張子除く)の後ろに文字列を追加する
void apply_appendix(char *new_filename, size_t new_filename_size, const char *orig_filename, const char *appendix) {
	if (new_filename != orig_filename)
		strcpy_s(new_filename, new_filename_size, orig_filename);
	strcpy_s(PathFindExtension(new_filename), new_filename_size - (PathFindExtension(new_filename) - new_filename), appendix);
}

//拡張子が一致するか確認する
BOOL check_ext(const char *filename, const char *ext) {
	return (_stricmp(PathFindExtension(filename), ext) == NULL) ? TRUE : FALSE;
}

//ルートディレクトリを取得
BOOL PathGetRoot(const char *path, char *root, size_t nSize) {
	if (PathIsRelative(path) == FALSE)
		strcpy_s(root, nSize, path);
	else
		_fullpath(root, path, nSize);
	return PathStripToRoot(root);
}

//パスのルートが存在するかどうか
BOOL PathRootExists(const char *path) {
	if (path == NULL)
		return FALSE;
	char root[MAX_PATH_LEN];
	return (PathGetRoot(path, root, sizeof(root)) && PathIsDirectory(root));
}

//PathRemoveFileSpecFixedがVistaでは5C問題を発生させるため、その回避策
BOOL PathRemoveFileSpecFixed(char *path) {
	char *ptr = PathFindFileName(path);
	if (path == ptr)
		return FALSE;
	*(ptr - 1) = '\0';
	return TRUE;
}

//フォルダがあればOK、なければ作成する
BOOL DirectoryExistsOrCreate(const char *dir) {
	if (PathIsDirectory(dir))
		return TRUE;
	return (PathRootExists(dir) && CreateDirectory(dir, NULL) != NULL) ? TRUE : FALSE;
}

//ファイルの存在と0byteより大きいかを確認
BOOL FileExistsAndHasSize(const char *path) {
	WIN32_FILE_ATTRIBUTE_DATA fd = { 0 };
	return GetFileAttributesEx(path, GetFileExInfoStandard, &fd) && ((((UINT64)fd.nFileSizeHigh) << 32) + (UINT64)fd.nFileSizeLow) > 0;
}

void PathGetDirectory(char *dir, size_t nSize, const char *path) {
	strcpy_s(dir, nSize, path);
	PathRemoveFileSpecFixed(dir);
}

BOOL GetFileSizeDWORD(const char *filepath, DWORD *filesize) {
	WIN32_FILE_ATTRIBUTE_DATA fd = { 0 };
	BOOL ret = (GetFileAttributesEx(filepath, GetFileExInfoStandard, &fd)) ? TRUE : FALSE;
	*filesize = (ret) ? fd.nFileSizeLow : 0;
	return ret;
}

//64bitでファイルサイズを取得,TRUEで成功
BOOL GetFileSizeUInt64(const char *filepath, UINT64 *filesize) {
	WIN32_FILE_ATTRIBUTE_DATA fd = { 0 };
	BOOL ret = (GetFileAttributesEx(filepath, GetFileExInfoStandard, &fd)) ? TRUE : FALSE;
	*filesize = (ret) ? (((UINT64)fd.nFileSizeHigh) << 32) + (UINT64)fd.nFileSizeLow : NULL;
	return ret;
}

UINT64 GetFileLastUpdate(const char *filepath) {
	WIN32_FILE_ATTRIBUTE_DATA fd = { 0 };
	GetFileAttributesEx(filepath, GetFileExInfoStandard, &fd);
	return ((UINT64)fd.ftLastWriteTime.dwHighDateTime << 32) + (UINT64)fd.ftLastWriteTime.dwLowDateTime;
}

size_t append_str(char **dst, size_t *nSize, const char *append) {
	size_t len = strlen(append);
	if (*nSize - 1 <= len)
		return 0;
	memcpy(*dst, append, len + 1);
	*dst += len;
	*nSize -= len;
	return len;
}

//多くのPath～関数はMAX_LEN(260)以上でもOKだが、一部は不可
//これもそのひとつ
BOOL PathAddBackSlashLong(char *dir) {
	size_t len = strlen(dir);
	if (dir[len-1] != '\\') {
		dir[len] = '\\';
		dir[len+1] = '\0';
		return TRUE;
	}
	return FALSE;
}
//PathCombineもMAX_LEN(260)以上不可
BOOL PathCombineLong(char *path, size_t nSize, const char *dir, const char *filename) {
	size_t dir_len;
	if (path == dir) {
		dir_len = strlen(path);
	} else {
		dir_len = strlen(dir);
		if (nSize <= dir_len)
			return FALSE;

		memcpy(path, dir, dir_len+1);
	}
	dir_len += PathAddBackSlashLong(path);

	size_t filename_len = strlen(filename);
	if (nSize - dir_len <= filename_len)
		return FALSE;
	memcpy(path + dir_len, filename, filename_len+1);
	return TRUE;
}

BOOL GetPathRootFreeSpace(const char *path, UINT64 *freespace) {
	//指定されたドライブが存在するかどうか
	char temp_root[MAX_PATH_LEN];
	strcpy_s(temp_root, sizeof(temp_root), path);
	PathStripToRoot(temp_root);
	//ドライブの空き容量取得
	ULARGE_INTEGER drive_avail_space = { 0 };
	if (GetDiskFreeSpaceEx(temp_root, &drive_avail_space, NULL, NULL)) {
		*freespace = drive_avail_space.QuadPart;
		return TRUE;
	}
	return FALSE;
}

BOOL PathForceRemoveBackSlash(char *path) {
	size_t len = strlen(path);
	int ret = FALSE;
	if (path != NULL && len) {
		char *ptr = path + len - 1;
		if (*ptr == '\\') {
			*ptr = '\0';
			ret = TRUE;
		}
	}
	return ret;
}

BOOL check_process_exitcode(PROCESS_INFORMATION *pi) {
	DWORD exit_code;
	if (!GetExitCodeProcess(pi->hProcess, &exit_code))
		return TRUE;
	return exit_code != 0;
}

BOOL swap_file(const char *fileA, const char *fileB) {
	if (!PathFileExists(fileA) || !PathFileExists(fileB))
		return FALSE;

	char filetemp[MAX_PATH_LEN];
	char appendix[MAX_APPENDIX_LEN];
	strcpy_s(appendix, sizeof(appendix), ".swap.tmp");
	apply_appendix(filetemp, sizeof(filetemp), fileA, appendix);
	for (int i = 0; PathFileExists(filetemp); i++) {
		sprintf_s(appendix, sizeof(appendix), ".swap%d.tmp", i);
		apply_appendix(filetemp, sizeof(filetemp), fileA, appendix);
	}
	if (rename(fileA, filetemp))
		return FALSE;
	if (rename(fileB, fileA))
		return FALSE;
	if (rename(filetemp, fileB))
		return FALSE;
	return TRUE;
}

//ボム文字かどうか、コードページの判定
DWORD check_bom(const void* chr) {
	if (chr == NULL) return CODE_PAGE_UNSET;
	if (memcmp(chr, UTF16_LE_BOM, sizeof(UTF16_LE_BOM)) == NULL) return CODE_PAGE_UTF16_LE;
	if (memcmp(chr, UTF16_BE_BOM, sizeof(UTF16_BE_BOM)) == NULL) return CODE_PAGE_UTF16_BE;
	if (memcmp(chr, UTF8_BOM,     sizeof(UTF8_BOM))     == NULL) return CODE_PAGE_UTF8;
	return CODE_PAGE_UNSET;
}

static BOOL isJis(const void *str, DWORD size_in_byte) {
	static const BYTE ESCAPE[][7] = {
		//先頭に比較すべきバイト数
		{ 3, 0x1B, 0x28, 0x42, 0x00, 0x00, 0x00 },
		{ 3, 0x1B, 0x28, 0x4A, 0x00, 0x00, 0x00 },
		{ 3, 0x1B, 0x28, 0x49, 0x00, 0x00, 0x00 },
		{ 3, 0x1B, 0x24, 0x40, 0x00, 0x00, 0x00 },
		{ 3, 0x1B, 0x24, 0x42, 0x00, 0x00, 0x00 },
		{ 6, 0x1B, 0x26, 0x40, 0x1B, 0x24, 0x42 },
		{ 4, 0x1B, 0x24, 0x28, 0x44, 0x00, 0x00 },
		{ 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } //終了
	};
	const BYTE * const str_fin = (const BYTE *)str + size_in_byte;
	for (const BYTE *chr = (const BYTE *)str; chr < str_fin; chr++) {
		if (*chr > 0x7F)
			return FALSE;
		for (int i = 0; ESCAPE[i][0]; i++) {
			if (str_fin - chr > ESCAPE[i][0] && 
				memcmp(chr, &ESCAPE[i][1], ESCAPE[i][0]) == NULL)
				return TRUE;
		}
	}
	return FALSE;
}

static DWORD isUTF16(const void *str, DWORD size_in_byte) {
	const BYTE * const str_fin = (const BYTE *)str + size_in_byte;
	for (const BYTE *chr = (const BYTE *)str; chr < str_fin; chr++) {
		if (chr[0] == 0x00 && str_fin - chr > 1 && chr[1] <= 0x7F)
			return ((chr - (const BYTE *)str) % 2 == 1) ? CODE_PAGE_UTF16_LE : CODE_PAGE_UTF16_BE;
	}
	return CODE_PAGE_UNSET;
}

static BOOL isASCII(const void *str, DWORD size_in_byte) {
	const BYTE * const str_fin = (const BYTE *)str + size_in_byte;
	for (const BYTE *chr = (const BYTE *)str; chr < str_fin; chr++) {
		if (*chr == 0x1B || *chr >= 0x80)
			return FALSE;
	}
	return TRUE;
}

static DWORD jpn_check(const void *str, DWORD size_in_byte) {
	int score_sjis = 0;
	int score_euc = 0;
	int score_utf8 = 0;
	const BYTE * const str_fin = (const BYTE *)str + size_in_byte;
	for (const BYTE *chr = (const BYTE *)str; chr < str_fin - 1; chr++) {
		if ((0x81 <= chr[0] && chr[0] <= 0x9F) ||
			(0xE0 <= chr[0] && chr[0] <= 0xFC) ||
			(0x40 <= chr[1] && chr[1] <= 0x7E) ||
			(0x80 <= chr[1] && chr[1] <= 0xFC)) {
				score_sjis += 2; chr++;
		}
	}
	for (const BYTE *chr = (const BYTE *)str; chr < str_fin - 1; chr++) {
		if ((0xC0 <= chr[0] && chr[0] <= 0xDF) &&
			(0x80 <= chr[1] && chr[1] <= 0xBF)) {
				score_utf8 += 2; chr++;
		} else if (
			str_fin - chr > 2 &&
			(0xE0 <= chr[0] && chr[0] <= 0xEF) &&
			(0x80 <= chr[1] && chr[1] <= 0xBF) &&
			(0x80 <= chr[2] && chr[2] <= 0xBF)) {
				score_utf8 += 3; chr++;
		}
	}
	for (const BYTE *chr = (const BYTE *)str; chr < str_fin - 1; chr++) {
		if (((0xA1 <= chr[0] && chr[0] <= 0xFE) && (0xA1 <= chr[1] && chr[1] <= 0xFE)) ||
			(chr[0] == 0x8E                     && (0xA1 <= chr[1] && chr[1] <= 0xDF))) {
				score_euc += 2; chr++;
		} else if (
			str_fin - chr > 2 &&
			chr[0] == 0x8F && 
			(0xA1 <= chr[1] && chr[1] <= 0xFE) && 
			(0xA1 <= chr[2] && chr[2] <= 0xFE)) {
				score_euc += 3; chr += 2;
		}
	}
	if (score_sjis > score_euc && score_sjis > score_utf8)
		return CODE_PAGE_SJIS;
	if (score_utf8 > score_euc && score_utf8 > score_sjis)
		return CODE_PAGE_UTF8;
	if (score_euc > score_sjis && score_euc > score_utf8)
		return CODE_PAGE_EUC_JP;
	return CODE_PAGE_UNSET;
}

DWORD get_code_page(const void *str, DWORD size_in_byte) {
	DWORD ret = CODE_PAGE_UNSET;
	if ((ret = check_bom(str)) != CODE_PAGE_UNSET)
		return ret;

	if (isJis(str, size_in_byte))
		return CODE_PAGE_JIS;

	if ((ret = isUTF16(str, size_in_byte)) != CODE_PAGE_UNSET)
		return ret;

	if (isASCII(str, size_in_byte))
		return CODE_PAGE_US_ASCII;

	return jpn_check(str, size_in_byte);
}

BOOL fix_ImulL_WesternEurope(UINT *code_page) {
	//IMultiLanguage2 の DetectInputCodepage はよく西ヨーロッパ言語と誤判定しやがる
	if (*code_page == CODE_PAGE_WEST_EUROPE)
		*code_page = CODE_PAGE_SJIS;
	return TRUE;
}

//cmd中のtarget_argを抜き出し削除する
//del_valueがTRUEならその値削除する
//値を削除できたらTRUEを返す
BOOL del_arg(char *cmd, char *target_arg, BOOL del_value) {
	char *p_start, *ptr;
	char * const cmd_fin = cmd + strlen(cmd);
	if ((p_start = strstr(cmd, target_arg)) == NULL)
		return FALSE;
	ptr = p_start + strlen(target_arg);

	if (del_value) {
		while (*ptr == ' ' || *ptr == '\r' || *ptr == '\n')
			ptr++;

		BOOL dQB = FALSE;
		while (ptr < cmd_fin) {
			if (*ptr == '"') dQB = !dQB;
			if (!dQB && *ptr == ' ')
				break;
			ptr++;
		}
	}
	memmove(p_start, ptr, cmd_fin - ptr + 1);
	return TRUE;
}

int replace_cmd_CRLF_to_Space(char *cmd, size_t nSize) {
	int ret = 0;
	ret += replace(cmd, nSize, "\r\n", " ");
	ret += replace(cmd, nSize, "\r",   " ");
	ret += replace(cmd, nSize, "\n",   " ");
	return ret;
}
