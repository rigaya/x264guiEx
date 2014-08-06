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
static const char *strichr(const char *str, int c) {
	c = tolower(c);
	for (; *str; str++)
		if (c == tolower(*str))
			return str;
	return NULL;
}
static char *strichr(char *str, int c) {
	c = tolower(c);
	for (; *str; str++)
		if (c == tolower(*str))
			return str;
	return NULL;
}

//大文字小文字を無視して、文字列を検索
static const char *stristr(const char *str, const char *substr) {
	int len = 0;
	if (substr && (len = strlen(substr)) != NULL)
		for (; (str = strichr(str, substr[0])) != NULL; str++)
			if (_strnicmp(str, substr, len) == NULL)
				return str;
	return NULL;
}
static char *stristr(char *str, const char *substr) {
	int len = 0;
	if (substr && (len = strlen(substr)) != NULL)
		for (; (str = strichr(str, substr[0])) != NULL; str++)
			if (_strnicmp(str, substr, len) == NULL)
				return str;
	return NULL;
}

//指定した場所から後ろ向きに1文字検索
static const char *strrchr(const char *str, int c, int start_index) {
	if (start_index < 0) return NULL;
	const char *result = str + start_index;
	str--;
	for (; result - str; result--)
		if (*result == c)
			return result;
	return NULL;
}
static char *strrchr(char *str, int c, int start_index) {
	if (start_index < 0) return NULL;
	char *result = str + start_index;
	str--;
	for (; result - str; result--)
		if (*result == c)
			return result;
	return NULL;
}

static int countchr(const char *str, int ch) {
	int i = 0;
	for (; *str; str++)
		if (*str == ch)
			i++;
	return i;
}

DWORD cpu_core_count();
BOOL check_sse2();
BOOL check_ssse3();
BOOL check_sse4_1();
BOOL check_OS_Win7orLater();
void get_auo_path(char *auo_path, size_t nSize);
void get_aviutl_dir(char *aviutl_dir, size_t nSize);
size_t calc_replace_mem_required(char *str, const char *old_str, const char *new_str);
void replace(char *str, size_t nSize, const char *old_str, const char *new_str);
void apply_appendix(char *new_filename, size_t new_filename_size, const char *orig_filename, const char *appendix);
BOOL check_ext(const char *filename, const char *ext);
BOOL PathGetRoot(const char *path, char *root, size_t nSize);
BOOL PathRootExists(const char *path);
BOOL DirectoryExistsOrCreate(const char *dir);
BOOL FileExistsAndHasSize(const char *path);
void PathGetDirectory(char *dir, size_t nSize, const char *path);
BOOL GetFileSizeInt(const char *filepath, DWORD *filesize);
BOOL GetFileSizeInt64(const char *filepath, __int64 *filesize);
__int64 GetFileLastUpdate(const char *filename);
int append_str(char **dst, size_t *nSize, const char *append);
BOOL PathAddBackSlashLong(char *dir);
BOOL PathCombineLong(char *path, size_t nSize, const char *dir, const char *filename);
BOOL GetPathRootFreeSpace(const char *path, __int64 *freespace);
BOOL PathForceRemoveBackSlash(char *path);
BOOL check_process_exitcode(PROCESS_INFORMATION *pi);

#endif //_AUO_UTIL_H_