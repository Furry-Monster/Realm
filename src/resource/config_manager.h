#pragma once

#include "json.hpp"
#include <filesystem>

namespace RealmEngine
{
    class ConfigManager
    {
    public:
        ConfigManager()           = default;
        ~ConfigManager() noexcept = default;

        ConfigManager(const ConfigManager&)                = delete;
        ConfigManager(ConfigManager&&) noexcept            = default;
        ConfigManager& operator=(const ConfigManager&)     = delete;
        ConfigManager& operator=(ConfigManager&&) noexcept = default;

        void initialize();
        void disposal();

        const std::filesystem::path& getRootFolder() const;
        const std::filesystem::path& getAssetFolder() const;
        const std::filesystem::path& getShaderFolder() const;

    private:
        std::filesystem::path m_root_folder;
        std::filesystem::path m_asset_folder;
        std::filesystem::path m_shader_folder;

        nlohmann::json m_config;
    };
} // namespace RealmEngine