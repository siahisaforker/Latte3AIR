#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace rmx {

class WiiUSaveData
{
public:
    static bool initialize();
    static void shutdown();

    // Save data operations
    static bool saveData(const std::string& filename, const std::vector<uint8_t>& data);
    static std::vector<uint8_t> loadData(const std::string& filename);
    static bool deleteSave(const std::string& filename);
    static bool saveExists(const std::string& filename);

    // Atomic write operations (temp file + rename)
    static bool atomicSave(const std::string& filename, const std::vector<uint8_t>& data);

    // Get save directory
    static std::string getSaveDirectory();

private:
    static std::string getTempFilePath(const std::string& filename);

    static bool mInitialized;
};

} // namespace rmx
