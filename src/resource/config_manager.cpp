#include "config_manager.h"

#include "plateform/plateform.h"
#include "utils.h"

#include <filesystem>
#include <iostream>

namespace RealmEngine
{
    void ConfigManager::initialize()
    {
        std::filesystem::path exe_path = PlateForm::getExecutablePath();
        std::filesystem::path exe_dir  = exe_path.parent_path();

        m_root_folder = exe_dir.parent_path();

        m_asset_folder  = m_root_folder / "assets";
        m_shader_folder = m_root_folder / "shaders";

        if (!std::filesystem::exists(m_asset_folder))
            fatal("Assets folder not found: " + m_asset_folder.string());
        if (!std::filesystem::exists(m_shader_folder))
            fatal("Shaders folder not found: " + m_shader_folder.string());
    }

    void ConfigManager::disposal() {}

    const std::filesystem::path& ConfigManager::getRootFolder() const { return m_root_folder; }

    const std::filesystem::path& ConfigManager::getAssetFolder() const { return m_asset_folder; }

    const std::filesystem::path& ConfigManager::getShaderFolder() const { return m_shader_folder; }

} // namespace RealmEngine