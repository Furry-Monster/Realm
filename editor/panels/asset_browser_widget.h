#pragma once

#include <filesystem>
#include <memory>
#include <vector>
#include "widget.h"

namespace RealmEngine
{
    class EditorEngineBridge;

    class AssetBrowserWidget : public Widget
    {
    public:
        explicit AssetBrowserWidget(EditorEngineBridge& bridge);
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

        EditorEngineBridge* m_bridge;

        std::filesystem::path              m_current_path;
        std::vector<std::filesystem::path> m_directory_entries;
        std::filesystem::path              m_selected_path;
        bool                               m_show_only_assets {false};
    };

} // namespace RealmEngine
