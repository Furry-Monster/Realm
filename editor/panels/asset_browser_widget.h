#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "widget.h"

namespace RealmEngine
{
    class EditorEngineBridge;

    class AssetBrowserWidget : public Widget
    {
    public:
        static constexpr const char* DRAG_DROP_PAYLOAD_TYPE = "ASSET_PATH";

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
        void renderFileItem(const std::filesystem::path& entry, bool is_dir);
        void renderTexturePreviewPopup();
        void renderAssetContextMenu();
        void openTexturePreview(const std::filesystem::path& path);
        bool passesFilter(const std::filesystem::path& entry) const;

        static bool isModelFile(const std::filesystem::path& path);
        static bool isTextureFile(const std::filesystem::path& path);
        static bool isHdrFile(const std::filesystem::path& path);
        static bool isPbrTextureFile(const std::filesystem::path& path);
        static bool filenameHasNormalHint(const std::filesystem::path& path);

        EditorEngineBridge* m_bridge;

        std::filesystem::path              m_current_path;
        std::vector<std::filesystem::path> m_directory_entries;
        std::filesystem::path              m_selected_path;
        bool                               m_show_only_assets {false};
        bool                               m_show_thumbnails {true};
        float                              m_thumbnail_size {64.0f};
        std::string                        m_search_filter;
        std::filesystem::path              m_texture_preview_path;
        bool                               m_texture_preview_open {false};
        std::filesystem::path              m_context_menu_path;

        /// Deferred navigation to avoid iterator invalidation during renderFileList
        bool                  m_pending_navigate {false};
        std::filesystem::path m_pending_navigate_path;
    };

} // namespace RealmEngine
