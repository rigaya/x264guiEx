//  -----------------------------------------------------------------------------------------
//    x264guiEx/x265guiEx by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _CPU_INFO_H_
#define _CPU_INFO_H_

#include <stdint.h>
#include <tchar.h>

static const int MAX_CACHE_LEVEL = 4;
static const int MAX_CORE_COUNT = 512;
static const int MAX_NODE_COUNT = 8;

enum class RGYCacheLevel {
    L0,
    L1,
    L2,
    L3,
    L4
};

enum class RGYCacheType {
    Unified,
    Instruction,
    Data,
    Trace
};

enum class RGYUnitType {
    Core,
    Cache,
    Node
};

enum class RGYCoreType {
    Physical,
    Logical
};

typedef struct node_info_t {
    size_t mask;
} node_info_t;

typedef struct cache_info_t {
    RGYCacheType type;
    RGYCacheLevel level;
    int associativity;
    int linesize;
    int size;
    size_t mask;
} cache_info_t;

typedef struct {
    int processor_id;   // プロセッサID
    int core_id;        // コアID
    int socket_id;      // ソケットID
    int logical_cores;  // 論理コア数
    size_t mask;        // 対応する物理コアのマスク
} processor_info_t;     // 物理コアの情報

typedef struct {
    int node_count;           // ノード数
    node_info_t nodes[MAX_NODE_COUNT];
    int physical_cores;  // 物理コア数
    int physical_cores_p; // 物理コア数
    int physical_cores_e; // 物理コア数
    int logical_cores;   // 論理コア数
    int max_cache_level; // キャッシュの最大レベル
    int cache_count[MAX_CACHE_LEVEL];       // 各階層のキャッシュの数
    cache_info_t caches[MAX_CACHE_LEVEL][MAX_CORE_COUNT]; // 各階層のキャッシュの情報
    processor_info_t proc_list[MAX_CORE_COUNT]; // 物理コアの情報
    size_t maskCoreP;  // Performanceコアのマスク
    size_t maskCoreE;  // Efficiencyコアのマスク
    size_t maskSystem; // システム全体のマスク
} cpu_info_t;


int getCPUName(char *buffer, size_t nSize);
bool get_cpu_info(cpu_info_t *cpu_info);
cpu_info_t get_cpu_info();
uint64_t get_mask(const cpu_info_t *cpu_info, RGYUnitType unit_type, int level, int id);

int getCPUInfo(TCHAR *buffer, size_t nSize);

template <size_t size>
int inline getCPUInfo(TCHAR(&buffer)[size]) {
    return getCPUInfo(buffer, size);
}

double getCPUDefaultClock();
double getCPUMaxTurboClock();

typedef struct PROCESS_TIME {
    uint64_t creation, exit, kernel, user;
} PROCESS_TIME;

BOOL GetProcessTime(PROCESS_TIME *time);
BOOL GetProcessTime(HANDLE hProcess, PROCESS_TIME *time);
double GetProcessAvgCPUUsage(HANDLE hProcess, PROCESS_TIME *start = nullptr);
double GetProcessAvgCPUUsage(PROCESS_TIME *start = nullptr);

#endif //_CPU_INFO_H_
