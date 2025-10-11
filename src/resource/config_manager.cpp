#include "config_manager.h"
#include <filesystem>

namespace RealmEngine
{
    void ConfigManager::initialize() {}

    void ConfigManager::disposal() {}

    const std::filesystem::path& ConfigManager::getRootFolder() const { return m_root_folder; }

    const std::filesystem::path& ConfigManager::getAssetFolder() const { return m_asset_folder; }

    const std::filesystem::path& ConfigManager::getShaderFolder() const { return m_shader_folder; }
} // namespace RealmEngine