#include "resource/config_manager.h"

#include "core/log/log_macros.h"
#include "platform/filesystem/platform.h"
#include "resource/config_serializer.h"

#include <filesystem>

namespace RealmEngine
{
    void ConfigManager::initialize()
    {
        std::filesystem::path exe_path = Platform::getExecutablePath();
        m_general_config.root_folder   = exe_path.parent_path();

        m_general_config.asset_folder  = m_general_config.root_folder / "assets";
        m_general_config.shader_folder = m_general_config.root_folder / "shaders";

        if (!std::filesystem::exists(m_general_config.asset_folder))
            RE_LOG_FATAL("Assets folder not found: " + m_general_config.asset_folder.string());
        if (!std::filesystem::exists(m_general_config.shader_folder))
            RE_LOG_FATAL("Shaders folder not found: " + m_general_config.shader_folder.string());

        std::filesystem::path config_file = m_general_config.root_folder / "config.json";
        if (ConfigSerializer::loadFromFile(*this, config_file.string()))
            RE_LOG_INFO("Config loaded from: " + config_file.string());
        else
            RE_LOG_INFO("Using default config.");

        RE_LOG_INFO("Config manager initialized.");
    }

    void ConfigManager::disposal() const
    {
        std::filesystem::path config_file = m_general_config.root_folder / "config.json";
        if (ConfigSerializer::saveToFile(*this, config_file.string()))
            RE_LOG_INFO("Config saved to: " + config_file.string());
        else
            RE_LOG_ERROR("Failed to save config file.");

        RE_LOG_INFO("Config manager disposed all resources.");
    }

    const GeneralConfig& ConfigManager::getGeneralConfig() const { return m_general_config; }

    void ConfigManager::setGeneralConfig(const GeneralConfig& config) { m_general_config = config; }

    const std::filesystem::path& ConfigManager::getRootFolder() const { return m_general_config.root_folder; }

    const std::filesystem::path& ConfigManager::getAssetFolder() const { return m_general_config.asset_folder; }

    const std::filesystem::path& ConfigManager::getShaderFolder() const { return m_general_config.shader_folder; }

    const WindowConfig& ConfigManager::getWindowConfig() const { return m_window_config; }

    void ConfigManager::setWindowConfig(const WindowConfig& config) { m_window_config = config; }

    const RendererConfig& ConfigManager::getRendererConfig() const { return m_renderer_config; }

    void ConfigManager::setRendererConfig(const RendererConfig& config) { m_renderer_config = config; }

    const GamePlayConfig& ConfigManager::getGamePlayConfig() const { return m_gameplay_config; }

    void ConfigManager::setGamePlayConfig(const GamePlayConfig& config) { m_gameplay_config = config; }

} // namespace RealmEngine
