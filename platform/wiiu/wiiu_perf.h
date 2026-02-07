#pragma once
// Lightweight Wii U performance instrumentation.
// Drop-in timing helpers with near-zero overhead when disabled.

#include <cstdint>

namespace wiiu_perf {

/// Initialise the profiling subsystem. Call once at startup.
void initialize();

/// Enable or disable profiling at runtime.
void setEnabled(bool enable);

/// Return true if profiling is currently enabled.
bool isEnabled();

/// Begin a named timing section. Sections can be nested.
void beginSection(const char* name);

/// End the most-recent timing section.
void endSection();

/// Print a summary of all measured sections to the log.
/// Typically called once per N frames.
void reportAndReset();

/// Get the total frame time of the last full frame (microseconds).
uint64_t lastFrameTimeUs();

/// RAII scope timer.
struct ScopedSection {
    explicit ScopedSection(const char* name) { beginSection(name); }
    ~ScopedSection() { endSection(); }
};

} // namespace wiiu_perf

#define WIIU_PERF_SCOPE(name) ::wiiu_perf::ScopedSection _perfScope_##__LINE__(name)
#define WIIU_PERF_BEGIN(name) ::wiiu_perf::beginSection(name)
#define WIIU_PERF_END()       ::wiiu_perf::endSection()
