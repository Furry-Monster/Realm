#include "panels/asset_browser_widget.h"

#include "bridge/editor_engine_bridge.h"
#include "module/render/rhi/rhi_texture.h"

#include <imgui.h>
#include <algorithm>
#include <cstring>

#include "core/base/macros.h"

namespace RealmEngine
{
    AssetBrowserWidget::AssetBrowserWidget(EditorEngineBridge& bridge) : Widget("Asset Browser"), m_bridge(&bridge)
    {
        m_current_path = m_bridge->getAssetFolder();
        refreshDirectory();
    }

    void AssetBrowserWidget::render()
    {
        ImGui::Begin(m_name.c_str(), &m_open);

        renderToolbar();

        ImGui::Separator();

        float tree_width = ImGui::GetContentRegionAvail().x * 0.3f;
        if (tree_width < 100.0f)
            tree_width = 100.0f;

        if (ImGui::BeginChild("AssetTree", ImVec2(tree_width, -1), true))
        {
            renderDirectoryTree(m_bridge->getAssetFolder());
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("AssetList", ImVec2(0, -1), true))
        {
            renderFileList();
        }
        ImGui::EndChild();

        renderTexturePreviewPopup();

        ImGui::End();

        if (m_pending_navigate)
        {
            m_pending_navigate = false;
            m_current_path     = m_pending_navigate_path;
            m_pending_navigate_path.clear();
            refreshDirectory();
        }
    }

    void AssetBrowserWidget::renderToolbar()
    {
        if (ImGui::Button("Refresh"))
            refreshDirectory();
        ImGui::SameLine();

        ImGui::Checkbox("Assets only", &m_show_only_assets);
        ImGui::SameLine();
        ImGui::Checkbox("Thumbnails", &m_show_thumbnails);
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        char filter_buf[128];
        std::strncpy(filter_buf, m_search_filter.c_str(), sizeof(filter_buf) - 1);
        filter_buf[sizeof(filter_buf) - 1] = '\0';
        if (ImGui::InputTextWithHint("##Search", "Search...", filter_buf, sizeof(filter_buf)))
            m_search_filter = filter_buf;

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        std::string path_str = m_current_path.string();
        if (path_str.length() > 80)
            path_str = "..." + path_str.substr(path_str.length() - 77);
        ImGui::TextUnformatted(path_str.c_str());
    }

    void AssetBrowserWidget::renderDirectoryTree(const std::filesystem::path& path, const int depth)
    {
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
            return;

        std::string name = path.filename().string();
        if (name.empty())
            name = path.string();

        ImGuiTreeNodeFlags flags      = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        const bool         is_current = (std::filesystem::equivalent(path, m_current_path));
        if (is_current)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool is_open = ImGui::TreeNodeEx(name.c_str(), flags);

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            m_current_path = path;
            refreshDirectory();
        }

        if (is_open)
        {
            try
            {
                std::vector<std::filesystem::path> subdirs;
                std::vector<std::filesystem::path> files;
                for (const auto& entry : std::filesystem::directory_iterator(path))
                {
                    if (entry.is_directory())
                        subdirs.push_back(entry.path());
                    else if (entry.is_regular_file())
                        files.push_back(entry.path());
                }

                auto cmp = [](const std::filesystem::path& a, const std::filesystem::path& b) {
                    return a.filename().string() < b.filename().string();
                };
                std::sort(subdirs.begin(), subdirs.end(), cmp);
                std::sort(files.begin(), files.end(), cmp);

                // Directories first (recursive)
                for (const auto& subdir : subdirs)
                    renderDirectoryTree(subdir, depth + 1);

                // Then files as leaf nodes
                for (const auto& file : files)
                {
                    if (m_show_only_assets && !isModelFile(file) && !isTextureFile(file) && !isHdrFile(file))
                        continue;

                    std::string file_name = file.filename().string();
                    const char* icon      = "[F] ";
                    if (isModelFile(file))
                        icon = "[M] ";
                    else if (isTextureFile(file) || isHdrFile(file))
                        icon = "[T] ";

                    std::string        file_label  = std::string(icon) + file_name;
                    const bool         is_selected = (m_selected_path == file);
                    ImGuiTreeNodeFlags file_flags  = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                                                    ImGuiTreeNodeFlags_SpanAvailWidth;
                    if (is_selected)
                        file_flags |= ImGuiTreeNodeFlags_Selected;

                    ImGui::TreeNodeEx(file_label.c_str(), file_flags);
                    if (ImGui::IsItemClicked())
                    {
                        m_selected_path = file;
                        // Also switch the right panel to this file's parent directory
                        if (m_current_path != path)
                        {
                            m_pending_navigate      = true;
                            m_pending_navigate_path = path;
                        }
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        if (isModelFile(file))
                            m_bridge->addModelToScene(file);
                        else if (isTextureFile(file) || isHdrFile(file))
                            openTexturePreview(file);
                    }
                }
            }
            catch (const std::filesystem::filesystem_error&)
            {}
            ImGui::TreePop();
        }
    }

    void AssetBrowserWidget::renderFileList()
    {
        if (m_show_thumbnails)
        {
            const float cell_size = m_thumbnail_size + ImGui::GetStyle().ItemSpacing.y + ImGui::GetTextLineHeight();
            const float panel_w   = ImGui::GetContentRegionAvail().x;
            const int   cols      = std::max(1, static_cast<int>(panel_w / cell_size));

            int col = 0;
            if (m_current_path.has_parent_path())
            {
                ImGui::BeginGroup();
                const ImVec2 thumb_size(m_thumbnail_size, m_thumbnail_size);
                if (ImGui::Button("##thumb_parent", thumb_size))
                {
                    m_pending_navigate      = true;
                    m_pending_navigate_path = m_current_path.parent_path();
                }
                const ImVec2 pos = ImGui::GetItemRectMin();
                ImGui::GetWindowDrawList()->AddText(
                    ImVec2(pos.x + m_thumbnail_size * 0.5f - ImGui::CalcTextSize("../").x * 0.5f,
                           pos.y + m_thumbnail_size * 0.5f - ImGui::GetTextLineHeight() * 0.5f),
                    ImGui::GetColorU32(ImGuiCol_TextDisabled),
                    "../");
                ImGui::TextWrapped("../");
                ImGui::EndGroup();
                col = 1;
            }
            for (const auto& entry : m_directory_entries)
            {
                if (m_pending_navigate)
                    break;
                if (!passesFilter(entry))
                    continue;

                if (col > 0)
                    ImGui::SameLine();
                renderFileItem(entry, std::filesystem::is_directory(entry));
                if (++col >= cols)
                {
                    col = 0;
                }
            }
        }
        else
        {
            if (m_current_path.has_parent_path())
            {
                if (ImGui::Selectable("../", false))
                {
                    m_pending_navigate      = true;
                    m_pending_navigate_path = m_current_path.parent_path();
                }
            }
            for (const auto& entry : m_directory_entries)
            {
                if (m_pending_navigate)
                    break;
                if (!passesFilter(entry))
                    continue;
                renderFileItem(entry, std::filesystem::is_directory(entry));
            }
        }

        if (!m_selected_path.empty() && std::filesystem::is_regular_file(m_selected_path) &&
            isModelFile(m_selected_path))
        {
            ImGui::Separator();
            if (ImGui::Button("Add to Scene"))
                m_bridge->addModelToScene(m_selected_path);
        }

        renderAssetContextMenu();
    }

    void AssetBrowserWidget::renderFileItem(const std::filesystem::path& entry, const bool is_dir)
    {
        std::string  filename    = entry.filename().string();
        const bool   is_selected = (m_selected_path == entry);
        const ImVec2 thumb_size(m_thumbnail_size, m_thumbnail_size);

        if (m_show_thumbnails)
        {
            ImGui::BeginGroup();
            const bool has_texture = !is_dir && (isTextureFile(entry) || isHdrFile(entry));
            const auto tex         = has_texture ? m_bridge->getTextureForPreview(entry) : nullptr;

            if (tex)
            {
                const ImTextureID tid = static_cast<ImTextureID>(static_cast<intptr_t>(tex->getNativeHandle()));
                if (ImGui::ImageButton(
                        ("##thumb_" + entry.string()).c_str(), tid, thumb_size, ImVec2(0, 1), ImVec2(1, 0)))
                    m_selected_path = entry;
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                    m_selected_path = entry;
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                {
                    m_context_menu_path = entry;
                    ImGui::OpenPopupOnItemClick("AssetContextMenu");
                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    openTexturePreview(entry);
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    const std::string path_str = entry.generic_string();
                    ImGui::SetDragDropPayload(
                        DRAG_DROP_PAYLOAD_TYPE, path_str.c_str(), path_str.size() + 1, ImGuiCond_Once);
                    ImGui::Image(tid, ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0));
                    ImGui::EndDragDropSource();
                }
            }
            else
            {
                const char* hint = is_dir ? "[D]" : (isModelFile(entry) ? "[M]" : "[F]");
                if (ImGui::Button(("##thumb_" + entry.string()).c_str(), thumb_size))
                    m_selected_path = entry;
                const ImVec2 pos = ImGui::GetItemRectMin();
                ImGui::GetWindowDrawList()->AddText(
                    ImVec2(pos.x + m_thumbnail_size * 0.5f - ImGui::CalcTextSize(hint).x * 0.5f,
                           pos.y + m_thumbnail_size * 0.5f - ImGui::GetTextLineHeight() * 0.5f),
                    ImGui::GetColorU32(ImGuiCol_TextDisabled),
                    hint);
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                    m_selected_path = entry;
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                {
                    m_context_menu_path = entry;
                    ImGui::OpenPopupOnItemClick("AssetContextMenu");
                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (is_dir)
                    {
                        m_pending_navigate      = true;
                        m_pending_navigate_path = entry;
                    }
                    else if (isModelFile(entry))
                        m_bridge->addModelToScene(entry);
                }
                if (!is_dir && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    const std::string path_str = entry.generic_string();
                    ImGui::SetDragDropPayload(
                        DRAG_DROP_PAYLOAD_TYPE, path_str.c_str(), path_str.size() + 1, ImGuiCond_Once);
                    ImGui::TextUnformatted(filename.c_str());
                    ImGui::EndDragDropSource();
                }
            }

            const float text_w = ImGui::CalcTextSize(filename.c_str()).x;
            if (text_w > m_thumbnail_size)
                filename = filename.substr(0, static_cast<size_t>(m_thumbnail_size / 8)) + "..";
            ImGui::TextWrapped("%s", filename.c_str());
            ImGui::EndGroup();
        }
        else
        {
            const char* icon = is_dir ? "[D] " : "[F] ";
            if (!is_dir && isModelFile(entry))
                icon = "[M] ";
            else if (!is_dir && (isTextureFile(entry) || isHdrFile(entry)))
                icon = "[T] ";

            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (is_selected)
                flags |= ImGuiTreeNodeFlags_Selected;

            const std::string label = std::string(icon) + filename;
            if (ImGui::TreeNodeEx(label.c_str(), flags))
            {
                if (ImGui::IsItemClicked())
                    m_selected_path = entry;
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                {
                    m_context_menu_path = entry;
                    ImGui::OpenPopupOnItemClick("AssetContextMenu");
                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (is_dir)
                    {
                        m_pending_navigate      = true;
                        m_pending_navigate_path = entry;
                    }
                    else if (isModelFile(entry))
                        m_bridge->addModelToScene(entry);
                    else if (isTextureFile(entry) || isHdrFile(entry))
                        openTexturePreview(entry);
                }

                if (!is_dir && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    const std::string path_str = entry.generic_string();
                    ImGui::SetDragDropPayload(
                        DRAG_DROP_PAYLOAD_TYPE, path_str.c_str(), path_str.size() + 1, ImGuiCond_Once);
                    ImGui::TextUnformatted(filename.c_str());
                    ImGui::EndDragDropSource();
                }
            }
        }
    }

    void AssetBrowserWidget::renderAssetContextMenu()
    {
        if (!ImGui::BeginPopup("AssetContextMenu"))
            return;
        if (!m_context_menu_path.empty())
        {
            if (isModelFile(m_context_menu_path) && ImGui::MenuItem("Add to Scene"))
                m_bridge->addModelToScene(m_context_menu_path);
            if ((isTextureFile(m_context_menu_path) || isHdrFile(m_context_menu_path)) && ImGui::MenuItem("Preview"))
                openTexturePreview(m_context_menu_path);
        }
        if (ImGui::MenuItem("Refresh"))
            refreshDirectory();
        ImGui::EndPopup();
    }

    void AssetBrowserWidget::refreshDirectory()
    {
        m_directory_entries.clear();

        try
        {
            if (std::filesystem::exists(m_current_path) && std::filesystem::is_directory(m_current_path))
            {
                for (const auto& entry : std::filesystem::directory_iterator(m_current_path))
                    m_directory_entries.push_back(entry.path());

                std::sort(m_directory_entries.begin(),
                          m_directory_entries.end(),
                          [](const std::filesystem::path& a, const std::filesystem::path& b) {
                              const bool a_is_dir = std::filesystem::is_directory(a);
                              const bool b_is_dir = std::filesystem::is_directory(b);
                              if (a_is_dir != b_is_dir)
                                  return a_is_dir;
                              return a.filename().string() < b.filename().string();
                          });

                if (m_show_only_assets)
                {
                    m_directory_entries.erase(std::remove_if(m_directory_entries.begin(),
                                                             m_directory_entries.end(),
                                                             [this](const std::filesystem::path& p) {
                                                                 if (std::filesystem::is_directory(p))
                                                                     return false;
                                                                 return !isModelFile(p) && !isTextureFile(p) &&
                                                                        !isHdrFile(p);
                                                             }),
                                              m_directory_entries.end());
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            RE_LOG_WARN("Asset Browser: " + std::string(e.what()));
        }
    }

    void AssetBrowserWidget::renderTexturePreviewPopup()
    {
        if (!m_texture_preview_open || m_texture_preview_path.empty())
            return;

        const std::string title = "Texture Preview: " + m_texture_preview_path.filename().string();
        ImGui::SetNextWindowSize(ImVec2(512, 512), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title.c_str(), &m_texture_preview_open))
        {
            ImGui::End();
            return;
        }

        const auto tex = m_bridge->getTextureForPreview(m_texture_preview_path);
        if (tex)
        {
            const float     w      = static_cast<float>(tex->getWidth());
            const float     h      = static_cast<float>(tex->getHeight());
            constexpr float max_sz = 480.0f;
            const float     scale  = (w > 0.0f && h > 0.0f) ? std::min(1.0f, std::min(max_sz / w, max_sz / h)) : 1.0f;
            const ImVec2    display_size(w * scale, h * scale);

            const ImTextureID tid = static_cast<ImTextureID>(static_cast<intptr_t>(tex->getNativeHandle()));
            ImGui::Image(tid, display_size, ImVec2(0, 1), ImVec2(1, 0));

            ImGui::Separator();
            ImGui::Text("Size: %d x %d", tex->getWidth(), tex->getHeight());
            if (isHdrFile(m_texture_preview_path))
                ImGui::TextDisabled("HDR (tonemapped for display)");
            else if (isPbrTextureFile(m_texture_preview_path))
                ImGui::TextDisabled("PBR texture");
            else if (filenameHasNormalHint(m_texture_preview_path))
                ImGui::TextDisabled("Normal map");
        }
        else
        {
            ImGui::Text("Failed to load texture");
        }
        ImGui::End();
    }

    void AssetBrowserWidget::openTexturePreview(const std::filesystem::path& path)
    {
        m_texture_preview_path = path;
        m_texture_preview_open = true;
    }

    bool AssetBrowserWidget::passesFilter(const std::filesystem::path& entry) const
    {
        if (m_search_filter.empty())
            return true;
        const std::string name         = entry.filename().string();
        std::string       lower_name   = name;
        std::string       lower_filter = m_search_filter;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        std::transform(lower_filter.begin(), lower_filter.end(), lower_filter.begin(), ::tolower);
        return lower_name.find(lower_filter) != std::string::npos;
    }

    bool AssetBrowserWidget::isModelFile(const std::filesystem::path& path)
    {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == ".gltf" || ext == ".glb" || ext == ".fbx" || ext == ".obj";
    }

    bool AssetBrowserWidget::isTextureFile(const std::filesystem::path& path)
    {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp";
    }

    bool AssetBrowserWidget::isHdrFile(const std::filesystem::path& path)
    {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == ".hdr";
    }

    bool AssetBrowserWidget::isPbrTextureFile(const std::filesystem::path& path)
    {
        std::string stem = path.stem().string();
        std::transform(stem.begin(), stem.end(), stem.begin(), ::tolower);
        return stem.find("metallic") != std::string::npos || stem.find("roughness") != std::string::npos ||
               stem.find("ao") != std::string::npos || stem.find("albedo") != std::string::npos ||
               stem.find("basecolor") != std::string::npos;
    }

    bool AssetBrowserWidget::filenameHasNormalHint(const std::filesystem::path& path)
    {
        std::string stem = path.stem().string();
        std::transform(stem.begin(), stem.end(), stem.begin(), ::tolower);
        return stem.find("normal") != std::string::npos || stem.find("norm") != std::string::npos ||
               stem.find("nrm") != std::string::npos;
    }

} // namespace RealmEngine
