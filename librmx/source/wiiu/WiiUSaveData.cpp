#include "WiiUSaveData.h"
#include "WiiUFileSystem.h"
#include <fstream>
#include <algorithm>
#include <cstring>

namespace rmx {

bool WiiUSaveData::mInitialized = false;

bool WiiUSaveData::initialize()
{
    if (mInitialized)
        return true;

    // Ensure filesystem is initialized
    if (!WiiUFileSystem::initialize())
        return false;

    // Ensure save directory exists
    if (!WiiUFileSystem::createDirectory(getSaveDirectory()))
        return false;

    mInitialized = true;
    return true;
}

void WiiUSaveData::shutdown()
{
    mInitialized = false;
}

bool WiiUSaveData::saveData(const std::string& filename, const std::vector<uint8_t>& data)
{
    if (!mInitialized || filename.empty())
        return false;

    std::string fullPath = getSaveDirectory() + filename;
    std::ofstream file(fullPath, std::ios::binary);
    if (!file.is_open())
        return false;

    return file.write(reinterpret_cast<const char*>(data.data()), data.size()).good();
}

std::vector<uint8_t> WiiUSaveData::loadData(const std::string& filename)
{
    std::vector<uint8_t> data;

    if (!mInitialized || filename.empty())
        return data;

    std::string fullPath = getSaveDirectory() + filename;
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return data;

    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    data.resize(fileSize);
    if (!file.read(reinterpret_cast<char*>(data.data()), fileSize))
    {
        data.clear();
    }

    return data;
}

bool WiiUSaveData::deleteSave(const std::string& filename)
{
    if (!mInitialized || filename.empty())
        return false;

    std::string fullPath = getSaveDirectory() + filename;
    return (std::remove(fullPath.c_str()) == 0);
}

bool WiiUSaveData::saveExists(const std::string& filename)
{
    if (!mInitialized || filename.empty())
        return false;

    std::string fullPath = getSaveDirectory() + filename;
    std::ifstream file(fullPath, std::ios::binary);
    return file.good();
}

bool WiiUSaveData::atomicSave(const std::string& filename, const std::vector<uint8_t>& data)
{
    if (!mInitialized || filename.empty())
        return false;

    // Create temporary file
    std::string tempPath = getTempFilePath(filename);
    if (!saveData(tempPath, data))
        return false;

    // Rename temp file to final destination (atomic operation)
    std::string finalPath = getSaveDirectory() + filename;
    if (std::rename(tempPath.c_str(), finalPath.c_str()) != 0)
    {
        // Clean up temp file if rename failed
        std::remove(tempPath.c_str());
        return false;
    }

    return true;
}

std::string WiiUSaveData::getSaveDirectory()
{
    return WiiUFileSystem::getSavePath();
}

std::string WiiUSaveData::getTempFilePath(const std::string& filename)
{
    // Add .tmp extension to filename for temporary file
    return getSaveDirectory() + filename + ".tmp";
}

} // namespace rmx
