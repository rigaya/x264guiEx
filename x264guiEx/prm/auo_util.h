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
#include <intrin.h>

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
static const DWORD AUO_SIMD_NONE  = 0x00;
static const DWORD AUO_SIMD_SSE2  = 0x01;
static const DWORD AUO_SIMD_SSE3  = 0x02; //使用していない
static const DWORD AUO_SIMD_SSSE3 = 0x04;
static const DWORD AUO_SIMD_SSE41 = 0x08;
static const DWORD AUO_SIMD_SSE42 = 0x10; //使用していない
static const DWORD AUO_SIMD_AVX   = 0x20;
static const DWORD AUO_SIMD_AVX2  = 0x40; //使用していない

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
static inline __int64 ceil_div_int64(__int64 i, int div) {
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
static inline const char *strrchr(const char *str, int c, size_t start_index) {
	if (start_index < 0) return NULL;
	const char *result = str + start_index;
	str--;
	for (; result - str; result--)
		if (*result == c)
			return result;
	return NULL;
}
static inline char *strrchr(char *str, int c, size_t start_index) {
	if (start_index < 0) return NULL;
	char *result = str + start_index;
	str--;
	for (; result - str; result--)
		if (*result == c)
			return result;
	return NULL;
}

//strのcount byteを検索し、substrとの一致を返す
static inline const char * strnstr(const char *str, const char *substr, size_t count) {
	const char *ptr = strstr(str, substr);
	if (ptr && (size_t)(ptr - str) >= count)
		ptr = NULL;
	return ptr;
}
static inline char * strnstr(char *str, const char *substr, size_t count) {
	char *ptr = strstr(str, substr);
	if (ptr && (size_t)(ptr - str) >= count)
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
static inline const char * strnrstr(const char *str, const char *substr, size_t count) {
	const char *last_ptr = NULL;
	if (count > 0)
		for (const char *ptr = str; *ptr && (ptr = strnstr(ptr, substr, count - (ptr - str))) != NULL; ptr++)
			last_ptr = ptr;
	return last_ptr;
}
static inline char * strnrstr(char *str, const char *substr, size_t count) {
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

static inline BOOL char_has_length(const char *str) {
	return str[0] != '\0';
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
	return simd;
}

static BOOL check_OS_Win7orLater() {
	OSVERSIONINFO osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	return ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && ((osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 1) || osvi.dwMajorVersion > 6));
}

void get_auo_path(char *auo_path, size_t nSize);
void get_aviutl_dir(char *aviutl_dir, size_t nSize);
size_t calc_replace_mem_required(char *str, const char *old_str, const char *new_str);
int replace(char *str, size_t nSize, const char *old_str, const char *new_str);
void apply_appendix(char *new_filename, size_t new_filename_size, const char *orig_filename, const char *appendix);
BOOL check_ext(const char *filename, const char *ext);
BOOL PathGetRoot(const char *path, char *root, size_t nSize);
BOOL PathRootExists(const char *path);
BOOL PathRemoveFileSpecFixed(char *path);
BOOL DirectoryExistsOrCreate(const char *dir);
BOOL FileExistsAndHasSize(const char *path);
void PathGetDirectory(char *dir, size_t nSize, const char *path);
BOOL GetFileSizeInt(const char *filepath, DWORD *filesize);
BOOL GetFileSizeInt64(const char *filepath, __int64 *filesize);
__int64 GetFileLastUpdate(const char *filename);
size_t append_str(char **dst, size_t *nSize, const char *append);
BOOL PathAddBackSlashLong(char *dir);
BOOL PathCombineLong(char *path, size_t nSize, const char *dir, const char *filename);
BOOL GetPathRootFreeSpace(const char *path, __int64 *freespace);
BOOL PathForceRemoveBackSlash(char *path);
BOOL check_process_exitcode(PROCESS_INFORMATION *pi);
int replace_cmd_CRLF_to_Space(char *cmd, size_t nSize);

//ファイル名をスワップする
BOOL swap_file(const char *fileA, const char *fileB);

//文字列先頭がBOM文字でないか確認する
DWORD check_bom(const void* chr);

//与えられた文字列から主に日本語について文字コード判定を行う
DWORD get_code_page(const void *str, DWORD size_in_byte);

//IMultipleLanguge2 の DetectInoutCodePageがたまに的外れな「西ヨーロッパ言語」を返すので
//西ヨーロッパ言語 なら Shift-JIS にしてしまう
BOOL fix_ImulL_WesternEurope(UINT *code_page);

//cmd中のtarget_argを抜き出し削除する
//del_valueがTRUEならその値削除する
BOOL del_arg(char *cmd, char *target_arg, BOOL del_value);

#endif //_AUO_UTIL_H_