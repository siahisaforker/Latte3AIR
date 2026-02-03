#include "WiiUFileSystem.h"
#include <coreinit/filesystem.h>
#include <nsysnet/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

namespace rmx {

// Static member definitions
std::string WiiUFileSystem::mBasePath;
std::string WiiUFileSystem::mRomPath;
std::string WiiUFileSystem::mModsPath;
std::string WiiUFileSystem::mSavePath;
std::string WiiUFileSystem::mConfigPath;
bool WiiUFileSystem::mInitialized = false;

bool WiiUFileSystem::initialize()
{
    if (mInitialized)
        return true;

    // Set up Wii U paths
    mBasePath = "/vol/external01/sonic3air/";
    mRomPath = mBasePath + "rom/";
    mModsPath = mBasePath + "mods/";
    mSavePath = mBasePath + "save/";
    mConfigPath = mBasePath + "config/";

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

    // Handle common path prefixes
    if (relativePath.find("data/audio/") == 0)
    {
        return mRomPath + "audio/" + relativePath.substr(11); // Remove "data/audio/"
    }
    else if (relativePath.find("data/") == 0)
    {
        return mRomPath + relativePath.substr(5); // Remove "data/"
    }
    else if (relativePath.find("mods/") == 0)
    {
        return mModsPath + relativePath.substr(5); // Remove "mods/"
    }
    else if (relativePath.find("save/") == 0)
    {
        return mSavePath + relativePath.substr(5); // Remove "save/"
    }
    else if (relativePath.find("config/") == 0)
    {
        return mConfigPath + relativePath.substr(7); // Remove "config/"
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

    // Use mkdir with proper permissions
    int result = mkdir(path.c_str(), 0755);
    return (result == 0 || errno == EEXIST);
}

bool WiiUFileSystem::createDirectory(const std::wstring& path)
{
    std::string utf8Path(path.begin(), path.end());
    return createDirectory(utf8Path);
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
    return (stat(path.c_str(), &statbuf) == 0);
}

bool WiiUFileSystem::fileExists(const std::wstring& path)
{
    std::string utf8Path(path.begin(), path.end());
    return fileExists(utf8Path);
}

const std::string& WiiUFileSystem::getBasePath()
{
    return mBasePath;
}

const std::string& WiiUFileSystem::getRomPath()
{
    return mRomPath;
}

const std::string& WiiUFileSystem::getModsPath()
{
    return mModsPath;
}

const std::string& WiiUFileSystem::getSavePath()
{
    return mSavePath;
}

const std::string& WiiUFileSystem::getConfigPath()
{
    return mConfigPath;
}

bool WiiUFileSystem::ensureDirectoriesExist()
{
    // Create all required directories
    if (!createDirectory(mBasePath))
        return false;

    if (!createDirectory(mRomPath))
        return false;

    if (!createDirectory(mModsPath))
        return false;

    if (!createDirectory(mSavePath))
        return false;

    if (!createDirectory(mConfigPath))
        return false;

    // Create audio subdirectories
    if (!createDirectory(mRomPath + "audio/"))
        return false;

    if (!createDirectory(mRomPath + "audio/original/"))
        return false;

    if (!createDirectory(mRomPath + "audio/remastered/"))
        return false;

    return true;
}

} // namespace rmx
