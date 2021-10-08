//  -----------------------------------------------------------------------------------------
//    QSVEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <vector>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <intrin.h>
#include <tchar.h>
#include "cpu_info.h"

static int getCPUName(char *buffer, size_t nSize) {
    int CPUInfo[4] = {-1};
    __cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];
    if (nSize < 0x40)
        return 1;

    memset(buffer, 0, 0x40);
    for (unsigned int i = 0x80000000; i <= nExIds; i++) {
        __cpuid(CPUInfo, i);
        int offset = 0;
        switch (i) {
            case 0x80000002: offset =  0; break;
            case 0x80000003: offset = 16; break;
            case 0x80000004: offset = 32; break;
            default:
                continue;
        }
        memcpy(buffer + offset, CPUInfo, sizeof(CPUInfo));
    }
    auto remove_string =[](char *target_str, const char *remove_str) {
        char *ptr = strstr(target_str, remove_str);
        if (nullptr != ptr) {
            memmove(ptr, ptr + strlen(remove_str), (strlen(ptr) - strlen(remove_str) + 1) *  sizeof(target_str[0]));
        }
    };
    remove_string(buffer, "(R)");
    remove_string(buffer, "(TM)");
    remove_string(buffer, "CPU");
    //crop space beforce string
    for (int i = 0; buffer[i]; i++) {
        if (buffer[i] != ' ') {
            if (i)
                memmove(buffer, buffer + i, strlen(buffer + i) + 1);
            break;
        }
    }
    //remove space which continues.
    for (int i = 0; buffer[i]; i++) {
        if (buffer[i] == ' ') {
            int space_idx = i;
            while (buffer[i+1] == ' ')
                i++;
            if (i != space_idx)
                memmove(buffer + space_idx + 1, buffer + i + 1, strlen(buffer + i + 1) + 1);
        }
    }
    //delete last blank
    if (0 < strlen(buffer)) {
        char *last_ptr = buffer + strlen(buffer) - 1;
        if (' ' == *last_ptr)
            last_ptr = '\0';
    }
    return 0;
}
static int getCPUName(wchar_t *buffer, size_t nSize) {
    int ret = 0;
    char *buf = (char *)calloc(nSize, sizeof(char));
    if (NULL == buf) {
        buffer[0] = L'\0';
        ret = 1;
    } else {
        if (0 == (ret = getCPUName(buf, nSize)))
            MultiByteToWideChar(CP_ACP, 0, buf, -1, buffer, (DWORD)nSize);
        free(buf);
    }
    return ret;
}

double getCPUDefaultClockFromCPUName() {
    double defaultClock = 0.0;
    TCHAR buffer[1024] = { 0 };
    getCPUName(buffer, _countof(buffer));
    TCHAR *ptr_mhz = _tcsstr(buffer, _T("MHz"));
    TCHAR *ptr_ghz = _tcsstr(buffer, _T("GHz"));
    TCHAR *ptr = _tcschr(buffer, _T('@'));
    bool clockInfoAvailable = (NULL != ptr_mhz || ptr_ghz != NULL) && NULL != ptr;
    if (clockInfoAvailable && 1 == _stscanf_s(ptr+1, _T("%lf"), &defaultClock)) {
        return defaultClock * ((NULL == ptr_ghz) ? 1000.0 : 1.0);
    }
    return 0.0;
}

#include <Windows.h>
#include <process.h>

typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

static DWORD CountSetBits(ULONG_PTR bitMask) {
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    for (ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT; bitTest; bitTest >>= 1)
        bitSetCount += ((bitMask & bitTest) != 0);

    return bitSetCount;
}

bool get_cpu_info(cpu_info_t *cpu_info) {
    if (nullptr == cpu_info)
        return false;

    memset(cpu_info, 0, sizeof(cpu_info[0]));

    LPFN_GLPI glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(_T("kernel32")), "GetLogicalProcessorInformation");
    if (nullptr == glpi)
        return false;

    DWORD returnLength = 0;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr;
    while (FALSE == glpi(buffer, &returnLength)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            if (buffer)
                free(buffer);
            if (NULL == (buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength)))
                return FALSE;
        }
    }

    DWORD processorPackageCount = 0;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
    for (DWORD byteOffset = 0; byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength;
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)) {
        switch (ptr->Relationship) {
        case RelationNumaNode:
            // Non-NUMA systems report a single record of this type.
            cpu_info->nodes++;
            break;
        case RelationProcessorCore:
            cpu_info->physical_cores++;
            // A hyperthreaded core supplies more than one logical processor.
            cpu_info->logical_cores += CountSetBits(ptr->ProcessorMask);
            break;

        case RelationCache:
        {
            // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache.
            PCACHE_DESCRIPTOR Cache = &ptr->Cache;
            if (1 <= Cache->Level && Cache->Level <= _countof(cpu_info->caches)) {
                cache_info_t *cache = &cpu_info->caches[Cache->Level-1];
                cache->count++;
                cache->level = Cache->Level;
                cache->linesize = Cache->LineSize;
                cache->size += Cache->Size;
                cache->associativity = Cache->Associativity;
                cpu_info->max_cache_level = max(cpu_info->max_cache_level, cache->level);
            }
            break;
        }
        case RelationProcessorPackage:
            // Logical processors share a physical package.
            processorPackageCount++;
            break;

        default:
            //Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.
            break;
        }
        ptr++;
    }
    if (buffer)
        free(buffer);

    return true;
}

const int TEST_COUNT = 5000;
__declspec(noinline)
int64_t runl_por(int loop_count, int& dummy_dep) {
    unsigned int dummy;
    const auto ts = __rdtscp(&dummy);
    int i = loop_count;
#define ADD_XOR { i += loop_count; i ^= loop_count; }
#define ADD_XOR4 {ADD_XOR;ADD_XOR;ADD_XOR;ADD_XOR;}
#define ADD_XOR16 {ADD_XOR4;ADD_XOR4;ADD_XOR4;ADD_XOR4;}
    do {
        ADD_XOR16;
        ADD_XOR16;
        ADD_XOR16;
        ADD_XOR16;
        loop_count--;
    } while (loop_count > 0);
    const auto te = __rdtscp(&dummy);
    dummy_dep = i;
    return te - ts;
}

static double get_tick_per_clock() {
    const int outer_loop_count = 1000;
    const int inner_loop_count = TEST_COUNT;
    int dummy = 0;
    auto tick_min = runl_por(inner_loop_count, dummy);
    for (int i = 0; i < outer_loop_count; i++) {
        auto ret = runl_por(inner_loop_count, dummy);
        tick_min = min(tick_min, ret);
    }
    return tick_min / (128.0 * inner_loop_count);
}

static double get_tick_per_sec() {
    const int nMul = 100;
    const int outer_loop_count = TEST_COUNT * nMul;
    int dummy = 0;
    runl_por(outer_loop_count, dummy);
    auto start = std::chrono::high_resolution_clock::now();
    auto tick = runl_por(outer_loop_count, dummy);
    auto fin = std::chrono::high_resolution_clock::now();
    double second = std::chrono::duration_cast<std::chrono::microseconds>(fin - start).count() * 1e-6;
    return tick / second;
}

//rdtscpを使うと0xc0000096例外 (一般ソフトウェア例外)を発する場合があるらしい
//そこでそれを検出する
bool check_rdtscp_available() {
    __try {
        UINT dummy;
        __rdtscp(&dummy);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return true;
}

//__rdtscが定格クロックに基づいた値を返すのを利用して、実際の動作周波数を得る
//やや時間がかかるので注意
double getCPUMaxTurboClock() {
    double defaultClock = getCPUDefaultClock();
    if (0.0 >= defaultClock) {
        return 0.0;
    }

    //http://instlatx64.atw.hu/
    //によれば、Sandy/Ivy/Haswell/Silvermont
    //いずれでもサポートされているのでノーチェックでも良い気がするが...
    //固定クロックのタイマーを持つかチェック (Fn:8000_0007:EDX8)
    int CPUInfo[4] ={ -1 };
    __cpuid(CPUInfo, 0x80000007);
    if (0 == (CPUInfo[3] & (1<<8))) {
        return defaultClock;
    }
    //rdtscp命令のチェック (Fn:8000_0001:EDX27)
    __cpuid(CPUInfo, 0x80000001);
    if (0 == (CPUInfo[3] & (1<<27))) {
        return defaultClock;
    }
    //例外が発生するなら処理を中断する
    if (!check_rdtscp_available()) {
        return defaultClock;
    }

    const double tick_per_clock = get_tick_per_clock();
    const double tick_per_sec = get_tick_per_sec();
    return (tick_per_sec / tick_per_clock) * 1e-9;
}

#if ENABLE_OPENCL
#include "cl_func.h"
#endif

#pragma warning (push)
#pragma warning (disable: 4100)
double getCPUDefaultClockOpenCL() {
#if !ENABLE_OPENCL
    return 0.0;
#else
    int frequency = 0;
    char buffer[1024] = { 0 };
    getCPUName(buffer, _countof(buffer));
    const std::vector<const char*> vendorNameList = { "Intel", "NVIDIA", "AMD" };

    const char *vendorName = NULL;
    for (auto vendor : vendorNameList) {
        if (cl_check_vendor_name(buffer, vendor)) {
            vendorName = vendor;
        }
    }
    if (NULL != vendorName) {
        cl_func_t cl = { 0 };
        cl_data_t data = { 0 };
        if (CL_SUCCESS == cl_get_func(&cl)
            && CL_SUCCESS == cl_get_platform_and_device(vendorName, CL_DEVICE_TYPE_CPU, &data, &cl)) {
            frequency = cl_get_device_max_clock_frequency_mhz(&data, &cl);
        }
        cl_release(&data, &cl);
    }
    return frequency / 1000.0;
#endif // !ENABLE_OPENCL
}
#pragma warning (pop)

double getCPUDefaultClock() {
    double defautlClock = getCPUDefaultClockFromCPUName();
    if (0 >= defautlClock)
        defautlClock = getCPUDefaultClockOpenCL();
    return defautlClock;
}

int getCPUInfo(TCHAR *buffer, size_t nSize) {
    int ret = 0;
    buffer[0] = _T('\0');
    cpu_info_t cpu_info;
    if (getCPUName(buffer, nSize) || !get_cpu_info(&cpu_info)) {
        ret = 1;
    } else {
        const double defaultClock = getCPUDefaultClockFromCPUName();
        bool noDefaultClockInCPUName = (0.0 >= defaultClock);
        const double maxFrequency = getCPUMaxTurboClock();
        if (defaultClock > 0.0) {
            if (noDefaultClockInCPUName) {
                _stprintf_s(buffer + _tcslen(buffer), nSize - _tcslen(buffer), _T(" @ %.2fGHz"), defaultClock);
            }
            //大きな違いがなければ、TurboBoostはないものとして表示しない
            if (maxFrequency / defaultClock > 1.01) {
                _stprintf_s(buffer + _tcslen(buffer), nSize - _tcslen(buffer), _T(" [TB: %.2fGHz]"), maxFrequency);
            }
        } else if (maxFrequency > 0.0) {
            _stprintf_s(buffer + _tcslen(buffer), nSize - _tcslen(buffer), _T(" [%.2fGHz]"), maxFrequency);
        }
        _stprintf_s(buffer + _tcslen(buffer), nSize - _tcslen(buffer), _T(" (%dC/%dT)"), cpu_info.physical_cores, cpu_info.logical_cores);
    }
    return ret;
}

BOOL GetProcessTime(HANDLE hProcess, PROCESS_TIME *time) {
    SYSTEMTIME systime;
    GetSystemTime(&systime);
    return (NULL != hProcess
        && GetProcessTimes(hProcess, (FILETIME *)&time->creation, (FILETIME *)&time->exit, (FILETIME *)&time->kernel, (FILETIME *)&time->user)
        && (WAIT_OBJECT_0 == WaitForSingleObject(hProcess, 0) || SystemTimeToFileTime(&systime, (FILETIME *)&time->exit)));
}

double GetProcessAvgCPUUsage(HANDLE hProcess, PROCESS_TIME *start) {
    PROCESS_TIME current = { 0 };
    cpu_info_t cpu_info;
    double result = 0;
    if (NULL != hProcess
        && get_cpu_info(&cpu_info)
        && GetProcessTime(hProcess, &current)) {
        UINT64 current_total_time = current.kernel + current.user;
        UINT64 start_total_time = (nullptr == start) ? 0 : start->kernel + start->user;
        result = (current_total_time - start_total_time) * 100.0 / (double)(cpu_info.logical_cores * (current.exit - ((nullptr == start) ? current.creation : start->exit)));
    }
    return result;
}
