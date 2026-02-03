#include "platform_filesystem.h"

#if defined(PLATFORM_WIIU) || defined(__WIIU__)

#include <coreinit/filesystem.h>
#include <nsysnet/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <vector>
#include <string>
#include <dirent.h>

// Complete filesystem implementation for Wii U SD card
namespace platform_filesystem {

static bool sFilesystemInitialized = false;
static char sBasePath[256] = "/vol/external01/sonic3air/";
static char sRomPath[256] = "/vol/external01/sonic3air/rom/";
static char sSavePath[256] = "/vol/external01/sonic3air/save/";
static char sConfigPath[256] = "/vol/external01/sonic3air/config/";
static char sModsPath[256] = "/vol/external01/sonic3air/mods/";
static char sTempPath[256] = "/vol/external01/sonic3air/temp/";

// File operation logging
static FILE* sLogFile = nullptr;

void logFileOperation(const char* operation, const char* path, bool success) {
    if (!sLogFile) {
        sLogFile = fopen("/vol/external01/sonic3air/filesystem.log", "a");
    }
    
    if (sLogFile) {
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        
        fprintf(sLogFile, "[%04d-%02d-%02d %02d:%02d:%02d] %s: %s - %s\n",
                timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
                operation, path, success ? "SUCCESS" : "FAILED");
        fflush(sLogFile);
    }
}

bool initializeFilesystem() {
    if (sFilesystemInitialized) return true;
    
    // Initialize Wii U filesystem
    FSInit();
    
    logFileOperation("FSInit", "Wii U filesystem", true);
    
    // Ensure all required directories exist
    bool success = true;
    success &= ensureDirectoryExists(sBasePath);
    success &= ensureDirectoryExists(sRomPath);
    success &= ensureDirectoryExists(sSavePath);
    success &= ensureDirectoryExists(sConfigPath);
    success &= ensureDirectoryExists(sModsPath);
    success &= ensureDirectoryExists(sTempPath);
    
    if (!success) {
        logFileOperation("DirectoryCreation", "Required directories", false);
        return false;
    }
    
    logFileOperation("DirectoryCreation", "Required directories", true);
    
    sFilesystemInitialized = true;
    return true;
}

void shutdownFilesystem() {
    if (sLogFile) {
        fclose(sLogFile);
        sLogFile = nullptr;
    }
    
    sFilesystemInitialized = false;
}

// Directory operations
bool ensureDirectoryExists(const char* path) {
    if (!path) return false;
    
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    
    // Create directory with full permissions
    int result = mkdir(path, 0755);
    if (result == 0) {
        logFileOperation("CreateDirectory", path, true);
        return true;
    }
    
    logFileOperation("CreateDirectory", path, false);
    return false;
}

bool directoryExists(const char* path) {
    if (!path) return false;
    
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

// File operations
bool fileExists(const char* path) {
    if (!path) return false;
    
    struct stat st;
    bool exists = (stat(path, &st) == 0);
    
    if (exists) {
        logFileOperation("FileExists", path, true);
    } else {
        logFileOperation("FileExists", path, false);
    }
    
    return exists;
}

long getFileSize(const char* path) {
    if (!path) return -1;
    
    struct stat st;
    if (stat(path, &st) != 0) {
        logFileOperation("GetFileSize", path, false);
        return -1;
    }
    
    long size = st.st_size;
    logFileOperation("GetFileSize", path, true);
    return size;
}

bool deleteFile(const char* path) {
    if (!path) return false;
    
    int result = unlink(path);
    bool success = (result == 0);
    
    logFileOperation("DeleteFile", path, success);
    return success;
}

// File I/O operations
FILE* openFile(const char* path, const char* mode) {
    if (!path || !mode) return nullptr;
    
    FILE* file = fopen(path, mode);
    if (file) {
        logFileOperation("OpenFile", path, true);
    } else {
        logFileOperation("OpenFile", path, false);
    }
    
    return file;
}

bool readFile(const char* path, void* buffer, size_t size) {
    if (!path || !buffer || size == 0) return false;
    
    FILE* file = fopen(path, "rb");
    if (!file) {
        logFileOperation("ReadFile", path, false);
        return false;
    }
    
    size_t bytesRead = fread(buffer, 1, size, file);
    fclose(file);
    
    bool success = (bytesRead == size);
    logFileOperation("ReadFile", path, success);
    
    return success;
}

bool writeFile(const char* path, const void* buffer, size_t size) {
    if (!path || !buffer || size == 0) return false;
    
    FILE* file = fopen(path, "wb");
    if (!file) {
        logFileOperation("WriteFile", path, false);
        return false;
    }
    
    size_t bytesWritten = fwrite(buffer, 1, size, file);
    fclose(file);
    
    bool success = (bytesWritten == size);
    logFileOperation("WriteFile", path, success);
    
    return success;
}

// Atomic write (temp → rename) - prevents corruption
bool atomicWriteFile(const char* path, const void* buffer, size_t size) {
    if (!path || !buffer || size == 0) return false;
    
    // Create temp file path
    char tempPath[512];
    snprintf(tempPath, sizeof(tempPath), "%s.tmp", path);
    
    // Write to temp file first
    if (!writeFile(tempPath, buffer, size)) {
        deleteFile(tempPath);
        logFileOperation("AtomicWrite", tempPath, false);
        return false;
    }
    
    // Verify temp file integrity
    long tempSize = getFileSize(tempPath);
    if (tempSize != static_cast<long>(size)) {
        deleteFile(tempPath);
        logFileOperation("AtomicWrite", tempPath, false);
        return false;
    }
    
    // Rename temp file to target (atomic operation)
    int result = rename(tempPath, path);
    bool success = (result == 0);
    
    if (!success) {
        deleteFile(tempPath);
        logFileOperation("AtomicWrite", path, false);
    } else {
        logFileOperation("AtomicWrite", path, true);
    }
    
    return success;
}

// Path utilities
const char* getBasePath() {
    return sBasePath;
}

const char* getRomPath() {
    return sRomPath;
}

const char* getSavePath() {
    return sSavePath;
}

const char* getConfigPath() {
    return sConfigPath;
}

const char* getModsPath() {
    return sModsPath;
}

const char* getTempPath() {
    return sTempPath;
}

// ROM specific functions
bool romExists(const char* romName) {
    if (!romName) return false;
    
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s%s", sRomPath, romName);
    
    return fileExists(fullPath);
}

long getRomSize(const char* romName) {
    if (!romName) return -1;
    
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s%s", sRomPath, romName);
    
    return getFileSize(fullPath);
}

bool loadRom(const char* romName, void* buffer, size_t bufferSize) {
    if (!romName || !buffer || bufferSize == 0) return false;
    
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s%s", sRomPath, romName);
    
    if (!fileExists(fullPath)) {
        logFileOperation("LoadROM", fullPath, false);
        return false;
    }
    
    long romSize = getFileSize(fullPath);
    if (romSize <= 0 || romSize > bufferSize) {
        logFileOperation("LoadROM", fullPath, false);
        return false;
    }
    
    bool success = readFile(fullPath, buffer, romSize);
    logFileOperation("LoadROM", fullPath, success);
    
    return success;
}

// Save data functions
bool saveDataExists(const char* saveName) {
    if (!saveName) return false;
    
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s%s", sSavePath, saveName);
    
    return fileExists(fullPath);
}

bool loadSaveData(const char* saveName, void* buffer, size_t bufferSize) {
    if (!saveName || !buffer || bufferSize == 0) return false;
    
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s%s", sSavePath, saveName);
    
    if (!fileExists(fullPath)) {
        logFileOperation("LoadSaveData", fullPath, false);
        return false;
    }
    
    long saveSize = getFileSize(fullPath);
    if (saveSize <= 0 || saveSize > bufferSize) {
        logFileOperation("LoadSaveData", fullPath, false);
        return false;
    }
    
    bool success = readFile(fullPath, buffer, saveSize);
    logFileOperation("LoadSaveData", fullPath, success);
    
    return success;
}

bool saveSaveData(const char* saveName, const void* buffer, size_t size) {
    if (!saveName || !buffer || size == 0) return false;
    
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s%s", sSavePath, saveName);
    
    bool success = atomicWriteFile(fullPath, buffer, size);
    logFileOperation("SaveSaveData", fullPath, success);
    
    return success;
}

// Config functions
bool configExists(const char* configName) {
    if (!configName) return false;
    
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s%s", sConfigPath, configName);
    
    return fileExists(fullPath);
}

bool loadConfig(const char* configName, void* buffer, size_t bufferSize) {
    if (!configName || !buffer || bufferSize == 0) return false;
    
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s%s", sConfigPath, configName);
    
    if (!fileExists(fullPath)) {
        logFileOperation("LoadConfig", fullPath, false);
        return false;
    }
    
    long configSize = getFileSize(fullPath);
    if (configSize <= 0 || configSize > bufferSize) {
        logFileOperation("LoadConfig", fullPath, false);
        return false;
    }
    
    bool success = readFile(fullPath, buffer, configSize);
    logFileOperation("LoadConfig", fullPath, success);
    
    return success;
}

bool saveConfig(const char* configName, const void* buffer, size_t size) {
    if (!configName || !buffer || size == 0) return false;
    
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s%s", sConfigPath, configName);
    
    bool success = atomicWriteFile(fullPath, buffer, size);
    logFileOperation("SaveConfig", fullPath, success);
    
    return success;
}

// Mod functions
bool modExists(const char* modName) {
    if (!modName) return false;
    
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s%s", sModsPath, modName);
    
    return fileExists(fullPath);
}

bool loadMod(const char* modName, void* buffer, size_t bufferSize) {
    if (!modName || !buffer || bufferSize == 0) return false;
    
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s%s", sModsPath, modName);
    
    if (!fileExists(fullPath)) {
        logFileOperation("LoadMod", fullPath, false);
        return false;
    }
    
    long modSize = getFileSize(fullPath);
    if (modSize <= 0 || modSize > bufferSize) {
        logFileOperation("LoadMod", fullPath, false);
        return false;
    }
    
    bool success = readFile(fullPath, buffer, modSize);
    logFileOperation("LoadMod", fullPath, success);
    
    return success;
}

// Directory listing
std::vector<std::string> listDirectory(const char* path) {
    std::vector<std::string> files;
    
    if (!path || !directoryExists(path)) {
        return files;
    }
    
    DIR* dir = opendir(path);
    if (!dir) {
        return files;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) {
            files.push_back(std::string(entry->d_name));
        }
    }
    
    closedir(dir);
    
    logFileOperation("ListDirectory", path, true);
    return files;
}

std::vector<std::string> listFilesWithExtension(const char* path, const char* extension) {
    std::vector<std::string> files;
    
    if (!path || !extension) {
        return files;
    }
    
    DIR* dir = opendir(path);
    if (!dir) {
        return files;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) {
            const char* filename = entry->d_name;
            const char* ext = strrchr(filename, '.');
            
            if (ext && strcmp(ext, extension) == 0) {
                files.push_back(std::string(filename));
            }
        }
    }
    
    closedir(dir);
    
    logFileOperation("ListFilesWithExtension", path, true);
    return files;
}

// File copying
bool copyFile(const char* srcPath, const char* destPath) {
    if (!srcPath || !destPath) return false;
    
    if (!fileExists(srcPath)) {
        logFileOperation("CopyFile", srcPath, false);
        return false;
    }
    
    FILE* src = fopen(srcPath, "rb");
    if (!src) {
        logFileOperation("CopyFile", srcPath, false);
        return false;
    }
    
    FILE* dest = fopen(destPath, "wb");
    if (!dest) {
        fclose(src);
        logFileOperation("CopyFile", destPath, false);
        return false;
    }
    
    char buffer[4096];
    size_t bytesRead;
    bool success = true;
    
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytesRead, dest) != bytesRead) {
            success = false;
            break;
        }
    }
    
    fclose(src);
    fclose(dest);
    
    logFileOperation("CopyFile", destPath, success);
    return success;
}

// File backup
bool backupFile(const char* path) {
    if (!path) return false;
    
    char backupPath[512];
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    snprintf(backupPath, sizeof(backupPath), "%s.backup.%04d%02d%02d_%02d%02d%02d",
             path, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    
    return copyFile(path, backupPath);
}

} // namespace platform_filesystem

#endif // PLATFORM_WIIU
