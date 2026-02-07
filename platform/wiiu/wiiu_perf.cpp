#include "wiiu_perf.h"

#if defined(PLATFORM_WIIU)
#include <coreinit/time.h>
#include <coreinit/debug.h>
#else
#include <chrono>
#endif

#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>

namespace {

static bool g_enabled = false;
static uint64_t g_lastFrameTimeUs = 0;

struct TimingEntry {
    std::string name;
    uint64_t totalUs = 0;
    uint32_t count = 0;
    uint64_t maxUs = 0;
};

static std::unordered_map<std::string, TimingEntry> g_entries;
static std::vector<std::pair<std::string, uint64_t>> g_stack; // name, start tick
static std::mutex g_mutex;

inline uint64_t nowUs()
{
#if defined(PLATFORM_WIIU)
    return OSTicksToMicroseconds(OSGetSystemTime());
#else
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<microseconds>(
        steady_clock::now().time_since_epoch()).count());
#endif
}

} // anonymous

namespace wiiu_perf {

void initialize()
{
    g_enabled = false;
    g_entries.clear();
    g_stack.clear();
    g_lastFrameTimeUs = 0;
}

void setEnabled(bool enable)
{
    g_enabled = enable;
}

bool isEnabled()
{
    return g_enabled;
}

void beginSection(const char* name)
{
    if (!g_enabled) return;
    std::lock_guard<std::mutex> lock(g_mutex);
    g_stack.push_back({name, nowUs()});
}

void endSection()
{
    if (!g_enabled) return;
    uint64_t end = nowUs();
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_stack.empty()) return;
    auto [name, start] = g_stack.back();
    g_stack.pop_back();
    uint64_t elapsed = end - start;
    auto& e = g_entries[name];
    if (e.name.empty()) e.name = name;
    e.totalUs += elapsed;
    e.count++;
    if (elapsed > e.maxUs) e.maxUs = elapsed;
}

void reportAndReset()
{
    if (!g_enabled) return;
    std::lock_guard<std::mutex> lock(g_mutex);
    char buf[256];
    for (auto& [name, e] : g_entries)
    {
        if (e.count == 0) continue;
        uint64_t avg = e.totalUs / e.count;
        snprintf(buf, sizeof(buf), "[perf] %-24s  calls=%4u  avg=%6llu us  max=%6llu us  total=%8llu us",
                 e.name.c_str(), e.count,
                 (unsigned long long)avg, (unsigned long long)e.maxUs,
                 (unsigned long long)e.totalUs);
#if defined(PLATFORM_WIIU)
        OSReport("%s\n", buf);
#else
        printf("%s\n", buf);
#endif
    }
    g_entries.clear();
}

uint64_t lastFrameTimeUs()
{
    return g_lastFrameTimeUs;
}

} // namespace wiiu_perf
