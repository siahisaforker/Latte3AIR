#include "WiiUFileSystem.h"
#include <coreinit/filesystem.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

namespace rmx {

// Static member definitions
std::string WiiUFileSystem::mBasePath;
std::string WiiUFileSystem::mRomPath;
std::string WiiUFileSystem::mModsPath;
std::string WiiUFileSystem::mSavePath;
std::string WiiUFileSystem::mConfigPath;
std::string WiiUFileSystem::mScriptsPath;
std::string WiiUFileSystem::mCachePath;
bool WiiUFileSystem::mInitialized = false;

bool WiiUFileSystem::initialize()
{
    if (mInitialized)
        return true;

    // Set up Wii U paths — must match PlatformFunctions::onEngineStartup()
    mBasePath    = "/vol/external01/S3AIR/";
    mRomPath     = mBasePath + "roms/";
    mModsPath    = mBasePath + "mods/";
    mSavePath    = mBasePath + "saves/";
    mConfigPath  = mBasePath + "config/";
    mScriptsPath = mBasePath + "scripts/";
    mCachePath   = mBasePath + "cache/";

    // Ensure all required directories exist
    if (!ensureDirectoriesExist())
    {
        return false;
    }

    mInitialized = true;
    return true;
}

void WiiUFileSystem::shutdown()
{
    mInitialized = false;
}

std::string WiiUFileSystem::getWiiUPath(const std::string& relativePath)
{
    if (!mInitialized)
        return relativePath;

    // Already an absolute Wii U path
    if (relativePath.size() > 0 && relativePath[0] == '/')
        return relativePath;

    // Handle common path prefixes
    if (relativePath.find("data/audio/") == 0)
    {
        return mRomPath + "audio/" + relativePath.substr(11);
    }
    else if (relativePath.find("data/") == 0)
    {
        return mRomPath + relativePath.substr(5);
    }
    else if (relativePath.find("scripts/") == 0)
    {
        return mScriptsPath + relativePath.substr(8);
    }
    else if (relativePath.find("mods/") == 0)
    {
        return mModsPath + relativePath.substr(5);
    }
    else if (relativePath.find("save/") == 0 || relativePath.find("saves/") == 0)
    {
        size_t skip = (relativePath[4] == '/') ? 5 : 6;
        return mSavePath + relativePath.substr(skip);
    }
    else if (relativePath.find("config/") == 0)
    {
        return mConfigPath + relativePath.substr(7);
    }
    else if (relativePath.find("cache/") == 0)
    {
        return mCachePath + relativePath.substr(6);
    }

    // If no special prefix, treat as relative to base path
    return mBasePath + relativePath;
}

std::wstring WiiUFileSystem::getWiiUPath(const std::wstring& relativePath)
{
    std::string utf8Path(relativePath.begin(), relativePath.end());
    std::string wiiUPath = getWiiUPath(utf8Path);
    return std::wstring(wiiUPath.begin(), wiiUPath.end());
}

bool WiiUFileSystem::createDirectory(const std::string& path)
{
    if (path.empty())
        return false;

    int result = mkdir(path.c_str(), 0755);
    return (result == 0 || errno == EEXIST);
}

bool WiiUFileSystem::createDirectory(const std::wstring& path)
{
    std::string utf8Path(path.begin(), path.end());
    return createDirectory(utf8Path);
}

bool WiiUFileSystem::createDirectoryRecursive(const std::string& path)
{
    if (path.empty()) return false;
    if (directoryExists(path)) return true;

    // Find parent and create recursively
    size_t pos = path.find_last_of('/');
    if (pos != std::string::npos && pos > 0)
    {
        std::string parent = path.substr(0, pos);
        if (!createDirectoryRecursive(parent))
            return false;
    }
    return createDirectory(path);
}

bool WiiUFileSystem::directoryExists(const std::string& path)
{
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0)
        return false;
    
    return S_ISDIR(statbuf.st_mode);
}

bool WiiUFileSystem::directoryExists(const std::wstring& path)
{
    std::string utf8Path(path.begin(), path.end());
    return directoryExists(utf8Path);
}

bool WiiUFileSystem::fileExists(const std::string& path)
{
    struct stat statbuf;
    return (stat(path.c_str(), &statbuf) == 0 && !S_ISDIR(statbuf.st_mode));
}

bool WiiUFileSystem::fileExists(const std::wstring& path)
{
    std::string utf8Path(path.begin(), path.end());
    return fileExists(utf8Path);
}

bool WiiUFileSystem::deleteFile(const std::string& path)
{
    return (::remove(path.c_str()) == 0);
}

std::vector<std::string> WiiUFileSystem::listDirectory(const std::string& path)
{
    std::vector<std::string> entries;
    DIR* dir = opendir(path.c_str());
    if (!dir) return entries;
    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr)
    {
        if (ent->d_name[0] == '.' && (ent->d_name[1] == '\0' || (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
            continue;
        entries.emplace_back(ent->d_name);
    }
    closedir(dir);
    return entries;
}

const std::string& WiiUFileSystem::getBasePath()    { return mBasePath; }
const std::string& WiiUFileSystem::getRomPath()      { return mRomPath; }
const std::string& WiiUFileSystem::getModsPath()     { return mModsPath; }
const std::string& WiiUFileSystem::getSavePath()     { return mSavePath; }
const std::string& WiiUFileSystem::getConfigPath()   { return mConfigPath; }
const std::string& WiiUFileSystem::getScriptsPath()  { return mScriptsPath; }
const std::string& WiiUFileSystem::getCachePath()    { return mCachePath; }

bool WiiUFileSystem::ensureDirectoriesExist()
{
    const std::string dirs[] = {
        mBasePath, mRomPath, mModsPath, mSavePath, mConfigPath,
        mScriptsPath, mCachePath,
        mRomPath + "audio/",
        mRomPath + "audio/original/",
        mRomPath + "audio/remastered/"
    };
    for (const auto& d : dirs)
    {
        if (!createDirectory(d))
            return false;
    }
    return true;
}

} // namespace rmx
