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
#include <intrin.h>
#include <algorithm>

#include "auo.h"
#include "auo_version.h"

DWORD cpu_core_count() {
	SYSTEM_INFO si;
    GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

//mmx    CPUInfo[3] & 0x00800000
//sse    CPUInfo[3] & 0x02000000
//sse2   CPUInfo[3] & 0x04000000
//sse3   CPUInfo[2] & 0x00000001
//ssse3  CPUInfo[2] & 0x00000200
//sse4.1 CPUInfo[2] & 0x00080000
//sse4.2 CPUInfo[2] & 0x00100000
//avx    CPUInfo[2] & 0x18000000 == 0x18000000 + OSチェック
BOOL check_sse2() {
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	return (CPUInfo[3] & 0x04000000) != 0;
}

BOOL check_ssse3() {
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	return (CPUInfo[2] & 0x00000200) != 0;
}

BOOL check_sse3() {
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	return (CPUInfo[2] & 0x00000001) != 0;
}

BOOL check_sse4_1() {
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	return (CPUInfo[2] & 0x00080000) != 0;
}

BOOL check_OS_Win7orLater() {
	OSVERSIONINFO osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	return ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && ((osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 1) || osvi.dwMajorVersion > 6));
}

void get_auo_path(char *auo_path, size_t nSize) {
	GetModuleFileName(GetModuleHandle(AUO_NAME), auo_path, nSize);
}
//最後に"\"なしで戻る
void get_aviutl_dir(char *aviutl_dir, size_t nSize) {
	GetModuleFileName(NULL, aviutl_dir, nSize);
	PathRemoveFileSpec(aviutl_dir);
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
//文字列の置換 str内で置き換える
void replace(char *str, size_t nSize, const char *old_str, const char *new_str) {
	char *c = str;
	char *p = NULL;
	char *fin = str + strlen(str) + 1;//null文字まで
	char * const limit = str + nSize;
	const size_t old_len = strlen(old_str);
	const size_t new_len = strlen(new_str);
	const int move_len = new_len - old_len;
	if (old_len) {
		while ((p = strstr(c, old_str)) != NULL) {
			if (move_len) {
				if (fin + move_len > limit)
					return;
				memmove((c = p + new_len), p + old_len, fin - (p + old_len));
				fin += move_len;
			}
			memcpy(p, new_str, new_len);
		}
	}
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

//フォルダがあればOK、なければ作成する
BOOL DirectoryExistsOrCreate(const char *dir) {
	if (PathIsDirectory(dir))
		return TRUE;
	return (PathRootExists(dir) && CreateDirectory(dir, NULL) != NULL) ? TRUE : FALSE;
}

//ファイルの存在と0byteより大きいかを確認
BOOL FileExistsAndHasSize(const char *path) {
	HANDLE h_file = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	BOOL ret = ((DWORD)h_file != INVALID_FILE_ATTRIBUTES && GetFileSize(h_file, NULL) > 0) ? TRUE : FALSE;
	CloseHandle(h_file);
	return ret;
}

void PathGetDirectory(char *dir, size_t nSize, const char *path) {
	strcpy_s(dir, nSize, path);
	PathRemoveFileSpec(dir);
}

BOOL GetFileSizeInt(const char *filepath, DWORD *filesize) {
	HANDLE h_file = CreateFile(filepath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h_file == INVALID_HANDLE_VALUE)
		return FALSE;
	BOOL ret = ((int)(*filesize = GetFileSize(h_file, NULL)) != -1) ? TRUE : FALSE;
	CloseHandle(h_file);
	return ret;
}

BOOL GetFileSizeInt64(const char *filepath, __int64 *filesize) {
	HANDLE h_file = CreateFile(filepath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h_file == INVALID_HANDLE_VALUE)
		return FALSE;
	LARGE_INTEGER size;
	BOOL ret = (GetFileSizeEx(h_file, &size)) ? TRUE : FALSE;
	CloseHandle(h_file);
	*filesize = (ret) ? size.QuadPart : 0;
	return ret;
}

__int64 GetFileLastUpdate(const char *filename) {
	HANDLE h_file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h_file == INVALID_HANDLE_VALUE)
		return 0;
	FILETIME ft = { 0 };
	GetFileTime(h_file, NULL, NULL, &ft);
	CloseHandle(h_file);
	return ((__int64)ft.dwHighDateTime << 32) + (__int64)ft.dwLowDateTime;
}
int append_str(char **dst, size_t *nSize, const char *append) {
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
	int len = strlen(dir);
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

BOOL GetPathRootFreeSpace(const char *path, __int64 *freespace) {
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
	int len = strlen(path);
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