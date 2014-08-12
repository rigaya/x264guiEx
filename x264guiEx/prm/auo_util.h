//  -----------------------------------------------------------------------------------------
//    拡張 x264 出力(GUI) Ex  v1.xx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _AUO_UTIL_H_
#define _AUO_UTIL_H_

#include <Windows.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <algorithm>
#include <intrin.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "auo.h"
#include "auo_version.h"

#if (_MSC_VER >= 1600)
#include <immintrin.h>
#endif

//日本語環境の一般的なコードページ一覧
static const DWORD CODE_PAGE_SJIS        = 932; //Shift-JIS
static const DWORD CODE_PAGE_JIS         = 50220;
static const DWORD CODE_PAGE_EUC_JP      = 51932;
static const DWORD CODE_PAGE_UTF8        = CP_UTF8;
static const DWORD CODE_PAGE_UTF16_LE    = CP_WINUNICODE; //WindowsのUnicode WCHAR のコードページ
static const DWORD CODE_PAGE_UTF16_BE    = 1201;
static const DWORD CODE_PAGE_US_ASCII    = 20127;
static const DWORD CODE_PAGE_WEST_EUROPE = 1252;  //厄介な西ヨーロッパ言語
static const DWORD CODE_PAGE_UNSET       = 0xffffffff;

//BOM文字リスト
static const BYTE UTF8_BOM[]     = { 0xEF, 0xBB, 0xBF };
static const BYTE UTF16_LE_BOM[] = { 0xFF, 0xFE };
static const BYTE UTF16_BE_BOM[] = { 0xFE, 0xFF };

//SIMD
static const DWORD AUO_SIMD_NONE  = 0x0000;
static const DWORD AUO_SIMD_SSE2  = 0x0001;
static const DWORD AUO_SIMD_SSE3  = 0x0002; //使用していない
static const DWORD AUO_SIMD_SSSE3 = 0x0004;
static const DWORD AUO_SIMD_SSE41 = 0x0008;
static const DWORD AUO_SIMD_SSE42 = 0x0010; //使用していない
static const DWORD AUO_SIMD_AVX   = 0x0020;
static const DWORD AUO_SIMD_AVX2  = 0x0040; //使用していない

//関数マクロ
#define clamp(x, low, high) (((x) <= (high)) ? (((x) >= (low)) ? (x) : (low)) : (high))
#define foreach(type,it,a) \
    for (type::iterator (it)=(a)->begin();(it)!=(a)->end();(it)++)
#define const_foreach(type,it,a) \
    for (type::const_iterator (it)=(a)->begin();(it)!=(a)->end();(it)++)

//基本的な関数
static inline double pow2(double a) {
	return a * a;
}
static inline int pow2(int a) {
	return a * a;
}
static inline int ceil_div_int(int i, int div) {
	return (i + (div-1)) / div;
}
static inline DWORD ceil_div_int(DWORD i, int div) {
	return (i + (div-1)) / div;
}
static inline __int64 ceil_div_int64(__int64 i, int div) {
	return (i + (div-1)) / div;
}
static inline UINT64 ceil_div_int64(UINT64 i, int div) {
	return (i + (div-1)) / div;
}

//大文字小文字を無視して、1文字検索
static inline const char *strichr(const char *str, int c) {
	c = tolower(c);
	for (; *str; str++)
		if (c == tolower(*str))
			return str;
	return NULL;
}
static inline char *strichr(char *str, int c) {
	c = tolower(c);
	for (; *str; str++)
		if (c == tolower(*str))
			return str;
	return NULL;
}

//大文字小文字を無視して、文字列を検索
static inline const char *stristr(const char *str, const char *substr) {
	size_t len = 0;
	if (substr && (len = strlen(substr)) != NULL)
		for (; (str = strichr(str, substr[0])) != NULL; str++)
			if (_strnicmp(str, substr, len) == NULL)
				return str;
	return NULL;
}
static inline char *stristr(char *str, const char *substr) {
	size_t len = 0;
	if (substr && (len = strlen(substr)) != NULL)
		for (; (str = strichr(str, substr[0])) != NULL; str++)
			if (_strnicmp(str, substr, len) == NULL)
				return str;
	return NULL;
}

//指定した場所から後ろ向きに1文字検索
static inline const char *strrchr(const char *str, int c, int start_index) {
	if (start_index < 0) return NULL;
	const char *result = str + start_index;
	str--;
	for (; result - str; result--)
		if (*result == c)
			return result;
	return NULL;
}
static inline char *strrchr(char *str, int c, int start_index) {
	if (start_index < 0) return NULL;
	char *result = str + start_index;
	str--;
	for (; result - str; result--)
		if (*result == c)
			return result;
	return NULL;
}

//strのcount byteを検索し、substrとの一致を返す
static inline const char * strnstr(const char *str, const char *substr, int count) {
	const char *ptr = strstr(str, substr);
	if (ptr && ptr - str >= count)
		ptr = NULL;
	return ptr;
}
static inline char * strnstr(char *str, const char *substr, int count) {
	char *ptr = strstr(str, substr);
	if (ptr && ptr - str >= count)
		ptr = NULL;
	return ptr;
}

//strのsubstrとの最後の一致を返す
static inline const char * strrstr(const char *str, const char *substr) {
	const char *last_ptr = NULL;
	for (const char *ptr = str; *ptr && (ptr = strstr(ptr, substr)) != NULL; ptr++ )
		last_ptr = ptr;
	return last_ptr;
}
static inline char * strrstr(char *str, const char *substr) {
	char *last_ptr = NULL;
	for (char *ptr = str; *ptr && (ptr = strstr(ptr, substr)) != NULL; ptr++ )
		last_ptr = ptr;
	return last_ptr;
}

//strのcount byteを検索し、substrとの最後の一致を返す
static inline const char * strnrstr(const char *str, const char *substr, int count) {
	const char *last_ptr = NULL;
	if (count > 0)
		for (const char *ptr = str; *ptr && (ptr = strnstr(ptr, substr, count - (ptr - str))) != NULL; ptr++)
			last_ptr = ptr;
	return last_ptr;
}
static inline char * strnrstr(char *str, const char *substr, int count) {
	char *last_ptr = NULL;
	if (count > 0)
		for (char *ptr = str; *ptr && (ptr = strnstr(ptr, substr, count - (ptr - str))) != NULL; ptr++)
			last_ptr = ptr;
	return last_ptr;
}

//文字列中の文字「ch」の数を数える
static inline int countchr(const char *str, int ch) {
	int i = 0;
	for (; *str; str++)
		if (*str == ch)
			i++;
	return i;
}

//文字列の末尾についている '\r' '\n' ' ' を削除する
static inline size_t deleteCRLFSpace_at_End(WCHAR *str) {
	WCHAR *pw = str + wcslen(str) - 1;
	WCHAR * const qw = pw;
	while ((*pw == L'\n' || *pw == L'\r' || *pw == L' ') && pw >= str) {
		*pw = L'\0';
		pw--;
	}
	return qw - pw;
}

static inline size_t deleteCRLFSpace_at_End(char *str) {
	char *pw = str + strlen(str) - 1;
	char *qw = pw;
	while ((*pw == '\n' || *pw == '\r' || *pw == ' ') && pw >= str) {
		*pw = '\0';
		pw--;
	}
	return qw - pw;
}

static inline BOOL str_has_char(const char *str) {
	BOOL ret = FALSE;
	for (; !ret && *str != '\0'; str++)
		ret = (*str != ' ');
	return ret;
}

static DWORD cpu_core_count() {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

//mmx    cpuid_1 CPUInfo[3] & 0x00800000
//sse    cpuid_1 CPUInfo[3] & 0x02000000
//sse2   cpuid_1 CPUInfo[3] & 0x04000000
//sse3   cpuid_1 CPUInfo[2] & 0x00000001
//ssse3  cpuid_1 CPUInfo[2] & 0x00000200
//sse4.1 cpuid_1 CPUInfo[2] & 0x00080000
//sse4.2 cpuid_1 CPUInfo[2] & 0x00100000
//avx    cpuid_1 CPUInfo[2] & 0x18000000 == 0x18000000 + OSチェック
//avx2   cpuid_7 CPUInfo[1] & 0x00000020 + OSチェック
static BOOL check_sse2() {
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	return (CPUInfo[3] & 0x04000000) != 0;
}

static BOOL check_ssse3() {
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	return (CPUInfo[2] & 0x00000200) != 0;
}

static BOOL check_sse3() {
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	return (CPUInfo[2] & 0x00000001) != 0;
}

static BOOL check_sse4_1() {
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	return (CPUInfo[2] & 0x00080000) != 0;
}
#if (_MSC_VER >= 1600)
static BOOL check_avx() {
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	if ((CPUInfo[2] & 0x18000000) == 0x18000000) {
		UINT64 XGETBV = _xgetbv(0);
		if ((XGETBV & 0x06) == 0x06)
			return TRUE;
	}
	return FALSE;
}
#endif

static DWORD get_availableSIMD() {
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	DWORD simd = AUO_SIMD_NONE;
	if  (CPUInfo[3] & 0x04000000)
		simd |= AUO_SIMD_SSE2;
	if  (CPUInfo[2] & 0x00000001)
		simd |= AUO_SIMD_SSE3;
	if  (CPUInfo[2] & 0x00000200)
		simd |= AUO_SIMD_SSSE3;
	if  (CPUInfo[2] & 0x00080000)
		simd |= AUO_SIMD_SSE41;
	if  (CPUInfo[2] & 0x00100000)
		simd |= AUO_SIMD_SSE42;
#if (_MSC_VER >= 1600)
	UINT64 XGETBV = 0;
	if ((CPUInfo[2] & 0x18000000) == 0x18000000) {
		XGETBV = _xgetbv(0);
		if ((XGETBV & 0x06) == 0x06)
			simd |= AUO_SIMD_AVX;
	}
	__cpuid(CPUInfo, 7);
	if ((simd & AUO_SIMD_AVX) && (CPUInfo[1] & 0x00000020))
		simd |= AUO_SIMD_AVX2;
#endif
	return simd;
}

static BOOL check_OS_Win7orLater() {
	OSVERSIONINFO osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	return ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && ((osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 1) || osvi.dwMajorVersion > 6));
}

static const char *GetFullPath(const char *path, char *buffer, size_t nSize) {
	if (PathIsRelative(path) == FALSE)
		return path;

	_fullpath(buffer, path, nSize);
	return buffer;
}
//文字列の置換に必要な領域を計算する
static size_t calc_replace_mem_required(char *str, const char *old_str, const char *new_str) {
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
static int replace(char *str, size_t nSize, const char *old_str, const char *new_str) {
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
				memmove((c = p + new_len), p + old_len, (fin - (p + old_len)) * sizeof(str[0]));
				fin += move_len;
			}
			memcpy(p, new_str, new_len * sizeof(str[0]));
			count++;
		}
	}
	return count;
}

//ファイル名(拡張子除く)の後ろに文字列を追加する
static void apply_appendix(char *new_filename, size_t new_filename_size, const char *orig_filename, const char *appendix) {
	if (new_filename != orig_filename)
		strcpy_s(new_filename, new_filename_size, orig_filename);
	strcpy_s(PathFindExtension(new_filename), new_filename_size - (PathFindExtension(new_filename) - new_filename), appendix);
}

//拡張子が一致するか確認する
static BOOL check_ext(const char *filename, const char *ext) {
	return (_stricmp(PathFindExtension(filename), ext) == NULL) ? TRUE : FALSE;
}

//ルートディレクトリを取得
static BOOL PathGetRoot(const char *path, char *root, size_t nSize) {
	if (PathIsRelative(path) == FALSE)
		strcpy_s(root, nSize, path);
	else
		_fullpath(root, path, nSize);
	return PathStripToRoot(root);
}

//パスのルートが存在するかどうか
static BOOL PathRootExists(const char *path) {
	if (path == NULL)
		return FALSE;
	char root[MAX_PATH_LEN];
	return (PathGetRoot(path, root, _countof(root)) && PathIsDirectory(root));
}

//PathRemoveFileSpecFixedがVistaでは5C問題を発生させるため、その回避策
static BOOL PathRemoveFileSpecFixed(char *path) {
	char *ptr = PathFindFileName(path);
	if (path == ptr)
		return FALSE;
	*(ptr - 1) = '\0';
	return TRUE;
}

//フォルダがあればOK、なければ作成する
static BOOL DirectoryExistsOrCreate(const char *dir) {
	if (PathIsDirectory(dir))
		return TRUE;
	return (PathRootExists(dir) && CreateDirectory(dir, NULL) != NULL) ? TRUE : FALSE;
}

//ファイルの存在と0byteより大きいかを確認
static BOOL FileExistsAndHasSize(const char *path) {
	WIN32_FILE_ATTRIBUTE_DATA fd = { 0 };
	return GetFileAttributesEx(path, GetFileExInfoStandard, &fd) && ((((UINT64)fd.nFileSizeHigh) << 32) + (UINT64)fd.nFileSizeLow) > 0;
}

static void PathGetDirectory(char *dir, size_t nSize, const char *path) {
	strcpy_s(dir, nSize, path);
	PathRemoveFileSpecFixed(dir);
}

static BOOL GetFileSizeDWORD(const char *filepath, DWORD *filesize) {
	WIN32_FILE_ATTRIBUTE_DATA fd = { 0 };
	BOOL ret = (GetFileAttributesEx(filepath, GetFileExInfoStandard, &fd)) ? TRUE : FALSE;
	*filesize = (ret) ? fd.nFileSizeLow : 0;
	return ret;
}

//64bitでファイルサイズを取得,TRUEで成功
static BOOL GetFileSizeUInt64(const char *filepath, UINT64 *filesize) {
	WIN32_FILE_ATTRIBUTE_DATA fd = { 0 };
	BOOL ret = (GetFileAttributesEx(filepath, GetFileExInfoStandard, &fd)) ? TRUE : FALSE;
	*filesize = (ret) ? (((UINT64)fd.nFileSizeHigh) << 32) + (UINT64)fd.nFileSizeLow : NULL;
	return ret;
}

static UINT64 GetFileLastUpdate(const char *filepath) {
	WIN32_FILE_ATTRIBUTE_DATA fd = { 0 };
	GetFileAttributesEx(filepath, GetFileExInfoStandard, &fd);
	return ((UINT64)fd.ftLastWriteTime.dwHighDateTime << 32) + (UINT64)fd.ftLastWriteTime.dwLowDateTime;
}

static size_t append_str(char **dst, size_t *nSize, const char *append) {
	size_t len = strlen(append);
	if (*nSize - 1 <= len)
		return 0;
	memcpy(*dst, append, (len + 1) * sizeof(dst[0][0]));
	*dst += len;
	*nSize -= len;
	return len;
}

//多くのPath～関数はMAX_LEN(260)以上でもOKだが、一部は不可
//これもそのひとつ
static BOOL PathAddBackSlashLong(char *dir) {
	size_t len = strlen(dir);
	if (dir[len-1] != '\\') {
		dir[len] = '\\';
		dir[len+1] = '\0';
		return TRUE;
	}
	return FALSE;
}
//PathCombineもMAX_LEN(260)以上不可
static BOOL PathCombineLong(char *path, size_t nSize, const char *dir, const char *filename) {
	size_t dir_len;
	if (path == dir) {
		dir_len = strlen(path);
	} else {
		dir_len = strlen(dir);
		if (nSize <= dir_len)
			return FALSE;

		memcpy(path, dir, (dir_len+1) * sizeof(path[0]));
	}
	dir_len += PathAddBackSlashLong(path);

	size_t filename_len = strlen(filename);
	if (nSize - dir_len <= filename_len)
		return FALSE;
	memcpy(path + dir_len, filename, (filename_len+1) * sizeof(path[0]));
	return TRUE;
}

static BOOL GetPathRootFreeSpace(const char *path, UINT64 *freespace) {
	//指定されたドライブが存在するかどうか
	char temp_root[MAX_PATH_LEN];
	strcpy_s(temp_root, _countof(temp_root), path);
	PathStripToRoot(temp_root);
	//ドライブの空き容量取得
	ULARGE_INTEGER drive_avail_space = { 0 };
	if (GetDiskFreeSpaceEx(temp_root, &drive_avail_space, NULL, NULL)) {
		*freespace = drive_avail_space.QuadPart;
		return TRUE;
	}
	return FALSE;
}

static BOOL PathForceRemoveBackSlash(char *path) {
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

static BOOL check_process_exitcode(PROCESS_INFORMATION *pi) {
	DWORD exit_code;
	if (!GetExitCodeProcess(pi->hProcess, &exit_code))
		return TRUE;
	return exit_code != 0;
}

static BOOL swap_file(const char *fileA, const char *fileB) {
	if (!PathFileExists(fileA) || !PathFileExists(fileB))
		return FALSE;

	char filetemp[MAX_PATH_LEN];
	char appendix[MAX_APPENDIX_LEN];
	for (int i = 0; i && PathFileExists(filetemp); i++) {
		sprintf_s(appendix, _countof(appendix), ".swap%d.tmp", i);
		apply_appendix(filetemp, _countof(filetemp), fileA, appendix);
	}
	if (rename(fileA, filetemp))
		return FALSE;
	if (rename(fileB, fileA))
		return FALSE;
	if (rename(filetemp, fileB))
		return FALSE;
	return TRUE;
}
//最後に"\"なしで戻る
static void get_aviutl_dir(char *aviutl_dir, size_t nSize) {
	GetModuleFileName(NULL, aviutl_dir, (DWORD)nSize);
	PathRemoveFileSpecFixed(aviutl_dir);
}
static void get_auo_path(char *auo_path, size_t nSize) {
	GetModuleFileName(GetModuleHandle(AUO_NAME), auo_path, (DWORD)nSize);
}

static int replace_cmd_CRLF_to_Space(char *cmd, size_t nSize) {
	int ret = 0;
	ret += replace(cmd, nSize, "\r\n", " ");
	ret += replace(cmd, nSize, "\r",   " ");
	ret += replace(cmd, nSize, "\n",   " ");
	return ret;
}

//文字列先頭がBOM文字でないか確認する
DWORD check_bom(const void* chr);

//与えられた文字列から主に日本語について文字コード判定を行う
DWORD get_code_page(const void *str, DWORD size_in_byte);

//IMultipleLanguge2 の DetectInoutCodePageがたまに的外れな「西ヨーロッパ言語」を返すので
//西ヨーロッパ言語 なら Shift-JIS にしてしまう
BOOL fix_ImulL_WesternEurope(UINT *code_page);

//cmd中のtarget_argを抜き出し削除する
//del_valueが+1ならその後の値を削除する、-1ならその前の値を削除する
//値を削除できたらTRUEを返す
BOOL del_arg(char *cmd, char *target_arg, int del_arg_delta);

#endif //_AUO_UTIL_H_
