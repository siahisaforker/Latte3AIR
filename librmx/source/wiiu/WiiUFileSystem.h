#pragma once

#include <string>
#include <vector>

namespace rmx {

class WiiUFileSystem
{
public:
    static bool initialize();
    static void shutdown();

    // Path conversion functions
    static std::string getWiiUPath(const std::string& relativePath);
    static std::wstring getWiiUPath(const std::wstring& relativePath);

    // Directory operations
    static bool createDirectory(const std::string& path);
    static bool createDirectory(const std::wstring& path);
    static bool directoryExists(const std::string& path);
    static bool directoryExists(const std::wstring& path);

    // File operations
    static bool fileExists(const std::string& path);
    static bool fileExists(const std::wstring& path);

    // Wii U specific paths
    static const std::string& getBasePath();
    static const std::string& getRomPath();
    static const std::string& getModsPath();
    static const std::string& getSavePath();
    static const std::string& getConfigPath();

private:
    static bool ensureDirectoriesExist();

    static std::string mBasePath;
    static std::string mRomPath;
    static std::string mModsPath;
    static std::string mSavePath;
    static std::string mConfigPath;
    static bool mInitialized;
};

} // namespace rmx
