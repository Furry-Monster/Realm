#include "panels/asset_browser_widget.h"

#include "bridge/editor_engine_bridge.h"
#include "core/log/log_macros.h"

#include <imgui.h>
#include <algorithm>

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

        // Split: directory tree (left) + file list (right)
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

        ImGui::End();
    }

    void AssetBrowserWidget::renderToolbar()
    {
        if (ImGui::Button("Refresh"))
        {
            refreshDirectory();
        }
        ImGui::SameLine();

        ImGui::Checkbox("Assets only", &m_show_only_assets);

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();

        std::string path_str = m_current_path.string();
        if (path_str.length() > 80)
            path_str = "..." + path_str.substr(path_str.length() - 77);
        ImGui::TextUnformatted(path_str.c_str());
    }

    void AssetBrowserWidget::renderDirectoryTree(const std::filesystem::path& path, int depth)
    {
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
            return;

        std::string name = path.filename().string();
        if (name.empty())
            name = path.string();

        ImGuiTreeNodeFlags flags      = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool               is_current = (std::filesystem::equivalent(path, m_current_path));
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
                for (const auto& entry : std::filesystem::directory_iterator(path))
                {
                    if (entry.is_directory())
                        subdirs.push_back(entry.path());
                }
                std::sort(
                    subdirs.begin(), subdirs.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
                        return a.filename().string() < b.filename().string();
                    });

                for (const auto& subdir : subdirs)
                {
                    renderDirectoryTree(subdir, depth + 1);
                }
            }
            catch (const std::filesystem::filesystem_error&)
            {
                // Ignore permission errors
            }
            ImGui::TreePop();
        }
    }

    void AssetBrowserWidget::renderFileList()
    {
        // Parent directory
        if (m_current_path.has_parent_path())
        {
            if (ImGui::Selectable("../", false))
            {
                m_current_path = m_current_path.parent_path();
                refreshDirectory();
            }
        }

        for (const auto& entry : m_directory_entries)
        {
            std::string filename    = entry.filename().string();
            bool        is_dir      = std::filesystem::is_directory(entry);
            bool        is_selected = (m_selected_path == entry);

            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (is_selected)
                flags |= ImGuiTreeNodeFlags_Selected;

            if (is_dir)
            {
                if (ImGui::TreeNodeEx(("📁 " + filename).c_str(), flags))
                {
                    if (ImGui::IsItemClicked())
                    {
                        m_current_path = entry;
                        refreshDirectory();
                    }
                }
            }
            else
            {
                const char* icon = "📄";
                if (isModelFile(entry))
                    icon = "🎲";
                else if (isTextureFile(entry) || isHdrFile(entry))
                    icon = "🖼";

                if (ImGui::TreeNodeEx((std::string(icon) + " " + filename).c_str(), flags))
                {
                    if (ImGui::IsItemClicked())
                        m_selected_path = entry;

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        if (isModelFile(entry))
                            m_bridge->addModelToScene(entry);
                    }
                }
            }
        }

        if (!m_selected_path.empty() && std::filesystem::is_regular_file(m_selected_path) &&
            isModelFile(m_selected_path))
        {
            ImGui::Separator();
            if (ImGui::Button("Add to Scene"))
                m_bridge->addModelToScene(m_selected_path);
        }
    }

    void AssetBrowserWidget::refreshDirectory()
    {
        m_directory_entries.clear();

        try
        {
            if (std::filesystem::exists(m_current_path) && std::filesystem::is_directory(m_current_path))
            {
                for (const auto& entry : std::filesystem::directory_iterator(m_current_path))
                {
                    m_directory_entries.push_back(entry.path());
                }

                std::sort(m_directory_entries.begin(),
                          m_directory_entries.end(),
                          [](const std::filesystem::path& a, const std::filesystem::path& b) {
                              bool a_is_dir = std::filesystem::is_directory(a);
                              bool b_is_dir = std::filesystem::is_directory(b);
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

    bool AssetBrowserWidget::isModelFile(const std::filesystem::path& path) const
    {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == ".gltf" || ext == ".glb" || ext == ".fbx" || ext == ".obj";
    }

    bool AssetBrowserWidget::isTextureFile(const std::filesystem::path& path) const
    {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp";
    }

    bool AssetBrowserWidget::isHdrFile(const std::filesystem::path& path) const
    {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == ".hdr";
    }

} // namespace RealmEngine
