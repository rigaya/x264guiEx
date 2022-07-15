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

#include <Windows.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <algorithm>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include <regex>
#include <filesystem>
#include <tchar.h>

#include "auo_util.h"
#include "auo_version.h"

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

DWORD jpn_check(const void *str, DWORD size_in_byte) {
    int score_sjis = 0;
    int score_euc = 0;
    int score_utf8 = 0;
    const BYTE * const str_fin = (const BYTE *)str + size_in_byte;
    for (const BYTE *chr = (const BYTE *)str; chr < str_fin - 1; chr++) {
        if (   ((0x81 <= chr[0] && chr[0] <= 0x9F) || (0xE0 <= chr[0] && chr[0] <= 0xFC))
            && ((0x40 <= chr[1] && chr[1] <= 0x7E) || (0x80 <= chr[1] && chr[1] <= 0xFC))) {
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
    int ret = CODE_PAGE_UNSET;
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

//ひとつのコードページの表すutf-8文字を返す
std::string cp_to_utf8(uint32_t codepoint) {
    char ptr[7] = { 0 };
    if (codepoint <= 0x7f) {
        ptr[0] = (char)codepoint & 0x7f;
    } else if (codepoint <= 0x7ff) {
        ptr[0] = (char)(0xc0 | (codepoint >> 6));
        ptr[1] = (char)(0x80 | (codepoint & 0x3f));
    } else if (codepoint <= 0xffff) {
        ptr[0] = (char)(0xe0 | (codepoint >> 12));
        ptr[1] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
        ptr[2] = (char)(0x80 |  (codepoint       & 0x3f));
    } else if (codepoint <= 0x1fffff) {
        ptr[0] = (char)(0xf0 |  (codepoint >> 18));
        ptr[1] = (char)(0x80 | ((codepoint >> 12) & 0x3f));
        ptr[2] = (char)(0x80 | ((codepoint >>  6) & 0x3f));
        ptr[3] = (char)(0x80 |  (codepoint        & 0x3f));
    } else if (codepoint <= 0x3fffff) {
        ptr[0] = (char)(0xf8 |  (codepoint >> 24));
        ptr[1] = (char)(0x80 | ((codepoint >> 18) & 0x3f));
        ptr[2] = (char)(0x80 | ((codepoint >> 12) & 0x3f));
        ptr[3] = (char)(0x80 | ((codepoint >>  6) & 0x3f));
        ptr[4] = (char)(0x80 |  (codepoint        & 0x3f));
    } else if (codepoint <= 0x7fffff) {
        ptr[0] = (char)(0xfc |  (codepoint >> 30));
        ptr[1] = (char)(0x80 | ((codepoint >> 24) & 0x3f));
        ptr[2] = (char)(0x80 | ((codepoint >> 18) & 0x3f));
        ptr[3] = (char)(0x80 | ((codepoint >> 12) & 0x3f));
        ptr[4] = (char)(0x80 | ((codepoint >>  6) & 0x3f));
        ptr[5] = (char)(0x80 |  (codepoint        & 0x3f));
    }
    return std::string(ptr);
}

//複数のU+xxxxU+xxxxのような文字列について、codepageのリストを作成する
std::vector<uint32_t> get_cp_list(const std::string& str_in) {
    std::string str = str_in;
    std::vector<uint32_t> cp_list;
    std::regex re(R"(^\s*U\+([0-9A-Fa-f]{2,})(.*))");
    std::smatch match;
    while (regex_match(str, match, re) && match.size() == 3) {
        cp_list.push_back(std::stoi(match[1], nullptr, 16));
        str = match[2];
    }
    return cp_list;
}

//code pageを記述している'U+xxxx'を含むUTF-8文字列をcode page部分を文字列に置換して返す
std::string conv_cp_part_to_utf8(const std::string& string_utf8_with_cp) {
    std::string string_utf8 = string_utf8_with_cp;
    //まず 'U+****'の部分を抽出
    std::regex re1(R"((.+)'(U\+[U\+0-9A-Fa-f\s]{2,}[0-9A-Fa-f]{2,})'(.+))");
    std::smatch match1;
    while (regex_match(string_utf8, match1, re1)) {
        if (match1.size() != 4) {
            break;
        }
        std::string str_next = match1[1];
        //'U+****'の部分をcodepageに変換し、さらにUTF-8に変換する
        for (auto cp : get_cp_list(match1[2])) {
            str_next += cp_to_utf8(cp);
        }
        str_next += match1[3];
        string_utf8 = str_next;
    }
    return string_utf8;
}

std::string GetFullPathFrom(const char *path, const char *baseDir) {
    if (auto p = std::filesystem::path(path); p.is_absolute()) {
        return path;
    }
    path = (path && strlen(path)) ? path : ".";
    const auto p = (baseDir) ? std::filesystem::path(baseDir).append(path) : std::filesystem::absolute(std::filesystem::path(path));
    return p.lexically_normal().string();
}

std::wstring GetFullPathFrom(const wchar_t *path, const wchar_t *baseDir) {
    if (auto p = std::filesystem::path(path); p.is_absolute()) {
        return path;
    }
    path = (path && wcslen(path)) ? path : L".";
    const auto p = (baseDir) ? std::filesystem::path(baseDir).append(path) : std::filesystem::absolute(std::filesystem::path(path));
    return p.lexically_normal().wstring();
}

static inline BOOL is_space_or_crlf(int c) {
    return (c == ' ' || c == '\r' || c == '\n');
}
BOOL del_arg(char *cmd, char *target_arg, int del_arg_delta) {
    char *p_start, *ptr;
    char * const cmd_fin = cmd + strlen(cmd);
    del_arg_delta = clamp(del_arg_delta, -1, 1);
    //指定された文字列を検索
    if ((p_start = strstr(cmd, target_arg)) == NULL)
        return FALSE;
    //指定された文字列の含まれる部分の先頭を検索
    for ( ; cmd < p_start; p_start--)
        if (is_space_or_crlf(*(p_start-1)))
            break;
    //指定された文字列の含まれる部分の最後尾を検索
    ptr = p_start;
    {
        BOOL dQB = FALSE;
        while (is_space_or_crlf(*ptr))
            ptr++;

        while (cmd < ptr && ptr < cmd_fin) {
            if (*ptr == '"') dQB = !dQB;
            if (!dQB && is_space_or_crlf(*ptr))
                break;
            ptr++;
        }
    }
    if (del_arg_delta < 0)
        std::swap(p_start, ptr);

    //次の値を検索
    if (del_arg_delta) {
        do {
            ptr += del_arg_delta;
        } while (is_space_or_crlf(*ptr));

        BOOL dQB = FALSE;
        while (cmd < ptr && ptr < cmd_fin) {
            if (*ptr == '"') dQB = !dQB;
            if (!dQB && is_space_or_crlf(*ptr))
                break;
            ptr += del_arg_delta;
        }
    }
    //文字列の移動
    if (del_arg_delta < 0)
        std::swap(p_start, ptr);
        
    memmove(p_start, ptr, (cmd_fin - ptr + 1) * sizeof(cmd[0]));
    return TRUE;
}

static const int ThreadQuerySetWin32StartAddress = 9;
typedef int (WINAPI *typeNtQueryInformationThread)(HANDLE, int, PVOID, ULONG, PULONG);

static void *GetThreadBeginAddress(DWORD TargetProcessId) {
    HMODULE hNtDll = NULL;
    typeNtQueryInformationThread NtQueryInformationThread = NULL;
    HANDLE hThread = NULL;
    ULONG length = 0;
    void *BeginAddress = NULL;

    if (   NULL != (hNtDll = LoadLibrary("ntdll.dll"))
        && NULL != (NtQueryInformationThread = (typeNtQueryInformationThread)GetProcAddress(hNtDll, "NtQueryInformationThread"))
        && NULL != (hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, TargetProcessId)) ) {
        NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &BeginAddress, sizeof(BeginAddress), &length );
    }
    if (hNtDll)
        FreeLibrary(hNtDll);
    if (hThread)
        CloseHandle(hThread);
    return BeginAddress;
}

static inline std::vector<DWORD> GetThreadList(DWORD TargetProcessId) {
    std::vector<DWORD> ThreadList;
    HANDLE hSnapshot;

    if (INVALID_HANDLE_VALUE != (hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0x00))) {
        THREADENTRY32 te32 = { 0 };
        te32.dwSize = sizeof(THREADENTRY32);

        if (Thread32First(hSnapshot, &te32)) {
            do {
                if (te32.th32OwnerProcessID == TargetProcessId)
                    ThreadList.push_back(te32.th32ThreadID);
            } while (Thread32Next(hSnapshot, &te32));
        }
        CloseHandle(hSnapshot);
    }
    return ThreadList;
}

static inline std::vector<MODULEENTRY32> GetModuleList(DWORD TargetProcessId) {
    std::vector<MODULEENTRY32> ModuleList;
    HANDLE hSnapshot;

    if (INVALID_HANDLE_VALUE != (hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, TargetProcessId))) {
        MODULEENTRY32 me32 = { 0 };
        me32.dwSize = sizeof(MODULEENTRY32);

        if (Module32First(hSnapshot, &me32)) {
            do {
                ModuleList.push_back(me32);
            } while (Module32Next(hSnapshot, &me32));
        }
        CloseHandle(hSnapshot);
    }
    return ModuleList;
}

static BOOL SetThreadPriorityFromThreadId(DWORD TargetThreadId, int ThreadPriority) {
    HANDLE hThread = OpenThread(THREAD_SET_INFORMATION, FALSE, TargetThreadId);
    if (hThread == NULL)
        return FALSE;
    BOOL ret = SetThreadPriority(hThread, ThreadPriority);
    CloseHandle(hThread);
    return ret;
}

BOOL SetThreadPriorityForModule(DWORD TargetProcessId, const char *TargetModule, int ThreadPriority) {
    BOOL ret = TRUE;
    std::vector<DWORD> thread_list = GetThreadList(TargetProcessId);
    std::vector<MODULEENTRY32> module_list = GetModuleList(TargetProcessId);
    for (auto thread_id : thread_list) {
        void *thread_address = GetThreadBeginAddress(thread_id);
        if (!thread_address) {
            ret = FALSE;
        } else {
            for (auto i_module : module_list) {
                if (   check_range(thread_address, i_module.modBaseAddr, i_module.modBaseAddr + i_module.modBaseSize - 1)
                    && (NULL == TargetModule || NULL == _strnicmp(TargetModule, i_module.szModule, strlen(TargetModule)))) {
                    ret &= !!SetThreadPriorityFromThreadId(thread_id, ThreadPriority);
                    break;
                }
            }
        }
    }
    return ret;
}

static BOOL SetThreadAffinityFromThreadId(DWORD TargetThreadId, DWORD_PTR ThreadAffinityMask) {
    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, TargetThreadId);
    if (hThread == NULL)
        return FALSE;
    DWORD_PTR ret = SetThreadAffinityMask(hThread, ThreadAffinityMask);
    CloseHandle(hThread);
    return (ret != 0);
}

BOOL SetThreadAffinityForModule(DWORD TargetProcessId, const char *TargetModule, DWORD_PTR ThreadAffinityMask) {
    BOOL ret = TRUE;
    std::vector<DWORD> thread_list = GetThreadList(TargetProcessId);
    std::vector<MODULEENTRY32> module_list = GetModuleList(TargetProcessId);
    for (auto thread_id : thread_list) {
        void *thread_address = GetThreadBeginAddress(thread_id);
        if (!thread_address) {
            ret = FALSE;
        } else {
            for (auto i_module : module_list) {
                if (   check_range(thread_address, i_module.modBaseAddr, i_module.modBaseAddr + i_module.modBaseSize - 1)
                    && (NULL == TargetModule || NULL == _strnicmp(TargetModule, i_module.szModule, strlen(TargetModule)))) {
                    ret &= !!SetThreadAffinityFromThreadId(thread_id, ThreadAffinityMask);
                    break;
                }
            }
        }
    }
    return ret;
}


typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

static DWORD CountSetBits(ULONG_PTR bitMask) {
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    for (ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT; bitTest; bitTest >>= 1)
        bitSetCount += ((bitMask & bitTest) != 0);

    return bitSetCount;
}

typedef void (WINAPI *RtlGetVersion_FUNC)(OSVERSIONINFOEXW*);

static int getRealWindowsVersion(DWORD *major, DWORD *minor, DWORD *build) {
    *major = 0;
    *minor = 0;
    OSVERSIONINFOEXW osver;
    HMODULE hModule = NULL;
    RtlGetVersion_FUNC func = NULL;
    int ret = 1;
    if (   NULL != (hModule = LoadLibrary(_T("ntdll.dll")))
        && NULL != (func = (RtlGetVersion_FUNC)GetProcAddress(hModule, "RtlGetVersion"))) {
        func(&osver);
        *major = osver.dwMajorVersion;
        *minor = osver.dwMinorVersion;
        *build = osver.dwBuildNumber;
        ret = 0;
    }
    if (hModule) {
        FreeLibrary(hModule);
    }
    return ret;
}

#pragma warning(push)
#pragma warning(disable:4996) // warning C4996: 'GetVersionExW': が古い形式として宣言されました。
const TCHAR *getOSVersion(DWORD *buildNumber) {
    const TCHAR *ptr = _T("Unknown");
    OSVERSIONINFOW info = { 0 };
    OSVERSIONINFOEXW infoex = { 0 };
    info.dwOSVersionInfoSize = sizeof(info);
    infoex.dwOSVersionInfoSize = sizeof(infoex);
    GetVersionExW(&info);
    switch (info.dwPlatformId) {
    case VER_PLATFORM_WIN32_WINDOWS:
        if (4 <= info.dwMajorVersion) {
            switch (info.dwMinorVersion) {
            case 0:  ptr = _T("Windows 95"); break;
            case 10: ptr = _T("Windows 98"); break;
            case 90: ptr = _T("Windows Me"); break;
            default: break;
            }
        }
        break;
    case VER_PLATFORM_WIN32_NT:
        if (info.dwMajorVersion >= 6 || (info.dwMajorVersion == 5 && info.dwMinorVersion >= 2)) {
            GetVersionExW((OSVERSIONINFOW *)&infoex);
        } else {
            memcpy(&infoex, &info, sizeof(info));
        }
        if (info.dwMajorVersion == 6) {
            getRealWindowsVersion(&infoex.dwMajorVersion, &infoex.dwMinorVersion, &infoex.dwBuildNumber);
        }
        switch (infoex.dwMajorVersion) {
        case 3:
            switch (infoex.dwMinorVersion) {
            case 0:  ptr = _T("Windows NT 3"); break;
            case 1:  ptr = _T("Windows NT 3.1"); break;
            case 5:  ptr = _T("Windows NT 3.5"); break;
            case 51: ptr = _T("Windows NT 3.51"); break;
            default: break;
            }
            break;
        case 4:
            if (0 == infoex.dwMinorVersion)
                ptr = _T("Windows NT 4.0");
            break;
        case 5:
            switch (infoex.dwMinorVersion) {
            case 0:  ptr = _T("Windows 2000"); break;
            case 1:  ptr = _T("Windows XP"); break;
            case 2:  ptr = _T("Windows Server 2003"); break;
            default: break;
            }
            break;
        case 6:
            switch (infoex.dwMinorVersion) {
            case 0:  ptr = (infoex.wProductType == VER_NT_WORKSTATION) ? _T("Windows Vista") : _T("Windows Server 2008");    break;
            case 1:  ptr = (infoex.wProductType == VER_NT_WORKSTATION) ? _T("Windows 7")     : _T("Windows Server 2008 R2"); break;
            case 2:  ptr = (infoex.wProductType == VER_NT_WORKSTATION) ? _T("Windows 8")     : _T("Windows Server 2012");    break;
            case 3:  ptr = (infoex.wProductType == VER_NT_WORKSTATION) ? _T("Windows 8.1")   : _T("Windows Server 2012 R2"); break;
            case 4:  ptr = (infoex.wProductType == VER_NT_WORKSTATION) ? _T("Windows 10")    : _T("Windows Server 2016");    break;
            default:
                if (5 <= infoex.dwMinorVersion) {
                    ptr = _T("Later than Windows 10");
                }
                break;
            }
            break;
        case 10:
            ptr = (infoex.wProductType == VER_NT_WORKSTATION) ? ((infoex.dwBuildNumber >= 21996) ? _T("Windows 11") : _T("Windows 10")) : _T("Windows Server 2016"); break;
        default:
            if (10 < infoex.dwMajorVersion) {
                ptr = _T("Windows 11 or later");
            }
            break;
        }
        break;
    default:
        break;
    }
    if (buildNumber) {
        *buildNumber = infoex.dwBuildNumber;
    }
    return ptr;
}
#pragma warning(pop)
