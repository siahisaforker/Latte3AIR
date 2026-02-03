#pragma once

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <cstdint>
#include <cstdio>

namespace platform_filesystem {

// Filesystem initialization
bool initializeFilesystem();
void shutdownFilesystem();

// Directory operations
bool ensureDirectoryExists(const char* path);
bool directoryExists(const char* path);

// File operations
bool fileExists(const char* path);
long getFileSize(const char* path);
bool deleteFile(const char* path);

// File I/O operations
FILE* openFile(const char* path, const char* mode);
bool readFile(const char* path, void* buffer, size_t size);
bool writeFile(const char* path, const void* buffer, size_t size);

// Atomic write (temp → rename)
bool atomicWriteFile(const char* path, const void* buffer, size_t size);

// Path utilities
const char* getBasePath();
const char* getRomPath();
const char* getSavePath();
const char* getConfigPath();
const char* getModsPath();

// ROM specific functions
bool romExists(const char* romName);
long getRomSize(const char* romName);
bool loadRom(const char* romName, void* buffer, size_t bufferSize);

// Save data functions
bool saveDataExists(const char* saveName);
bool loadSaveData(const char* saveName, void* buffer, size_t bufferSize);
bool saveSaveData(const char* saveName, const void* buffer, size_t size);

// Config functions
bool configExists(const char* configName);
bool loadConfig(const char* configName, void* buffer, size_t bufferSize);
bool saveConfig(const char* configName, const void* buffer, size_t size);

} // namespace platform_filesystem

#endif // PLATFORM_WIIU
