#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include "widget.h"

namespace RealmEngine
{
    class ConfigManager;
    class SceneManager;
    class AssetManager;
    class RHIDevice;

    class AssetBrowserWidget : public Widget
    {
    public:
        AssetBrowserWidget(ConfigManager& config, SceneManager& scene_mgr, AssetManager& asset_mgr, RHIDevice& device);
        ~AssetBrowserWidget() override = default;

        AssetBrowserWidget(const AssetBrowserWidget&)            = delete;
        AssetBrowserWidget& operator=(const AssetBrowserWidget&) = delete;
        AssetBrowserWidget(AssetBrowserWidget&&)                 = delete;
        AssetBrowserWidget& operator=(AssetBrowserWidget&&)      = delete;

        void render() override;

    private:
        void refreshDirectory();
        void renderToolbar();
        void renderDirectoryTree(const std::filesystem::path& path, int depth = 0);
        void renderFileList();
        bool isModelFile(const std::filesystem::path& path) const;
        bool isTextureFile(const std::filesystem::path& path) const;
        bool isHdrFile(const std::filesystem::path& path) const;
        void addModelToScene(const std::filesystem::path& model_path);

        ConfigManager& m_config;
        SceneManager&  m_scene_mgr;
        AssetManager&  m_asset_mgr;
        RHIDevice&     m_device;

        std::filesystem::path              m_current_path;
        std::vector<std::filesystem::path> m_directory_entries;
        std::filesystem::path              m_selected_path;
        bool                               m_show_only_assets {false};
    };

} // namespace RealmEngine
