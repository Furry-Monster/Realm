#include "asset_manager.h"
#include <filesystem>
#include <string>

namespace RealmEngine
{
    std::filesystem::path AssetManager::getFullPath(const std::string& relative_path) const
    {
        return std::filesystem::absolute(relative_path);
    }
} // namespace RealmEngine