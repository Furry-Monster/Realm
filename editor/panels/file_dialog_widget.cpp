#include "panels/file_dialog_widget.h"

#include <imgui.h>
#include <algorithm>

namespace RealmEngine
{
    FileDialogWidget::FileDialogWidget() : Widget("FileDialog") {}

    void FileDialogWidget::open(Mode                         mode,
                                const std::string&           title,
                                const std::string&           filter,
                                const std::filesystem::path& initial_path)
    {
        m_mode        = mode;
        m_title       = title;
        m_filter      = filter;
        m_dialog_open = true;
        m_open        = true;

        if (initial_path.empty())
        {
            m_current_path = std::filesystem::current_path();
        }
        else if (std::filesystem::is_directory(initial_path))
        {
            m_current_path = initial_path;
        }
        else
        {
            m_current_path = initial_path.parent_path();
            strncpy(m_filename_buffer, initial_path.filename().string().c_str(), sizeof(m_filename_buffer) - 1);
            m_filename_buffer[sizeof(m_filename_buffer) - 1] = '\0';
        }

        m_selected_path.clear();
        updateCurrentPath();
    }

    void FileDialogWidget::close()
    {
        m_dialog_open        = false;
        m_open               = false;
        m_filename_buffer[0] = '\0';
    }

    void FileDialogWidget::render()
    {
        if (!m_dialog_open)
            return;

        renderDialog();
    }

    void FileDialogWidget::renderDialog()
    {
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(m_title.c_str(), &m_dialog_open, ImGuiWindowFlags_NoDocking))
        {
            // Current path display
            std::string path_str = m_current_path.string();
            ImGui::Text("Path: %s", path_str.c_str());

            ImGui::Separator();

            // File list
            if (ImGui::BeginChild("FileList", ImVec2(0, -100), true))
            {
                // Parent directory
                if (m_current_path.has_parent_path() && m_current_path != m_current_path.root_path())
                {
                    if (ImGui::Selectable("../", false))
                    {
                        navigateTo(m_current_path.parent_path());
                    }
                }

                // Directory entries
                for (const auto& entry : m_current_directory_entries)
                {
                    bool        is_selected = (m_selected_path == entry);
                    std::string name        = entry.filename().string();

                    if (std::filesystem::is_directory(entry))
                    {
                        name += "/";
                    }

                    if (ImGui::Selectable(name.c_str(), is_selected))
                    {
                        m_selected_path = entry;
                        if (std::filesystem::is_directory(entry))
                        {
                            navigateTo(entry);
                        }
                        else
                        {
                            strncpy(
                                m_filename_buffer, entry.filename().string().c_str(), sizeof(m_filename_buffer) - 1);
                            m_filename_buffer[sizeof(m_filename_buffer) - 1] = '\0';
                        }
                    }

                    if (ImGui::IsItemClicked())
                    {
                        if (std::filesystem::is_directory(entry))
                        {
                            navigateTo(entry);
                        }
                        else if (m_mode == Mode::Open)
                        {
                            m_selected_path = entry;
                            if (m_callback)
                            {
                                m_callback(m_selected_path);
                            }
                            close();
                        }
                    }
                }
            }
            ImGui::EndChild();

            ImGui::Separator();

            // Filename input
            ImGui::Text("File name:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 120);
            ImGui::InputText("##filename", m_filename_buffer, sizeof(m_filename_buffer));

            // Filter display
            if (!m_filter.empty())
            {
                ImGui::Text("Filter: %s", m_filter.c_str());
            }

            // Buttons
            ImGui::Separator();
            if (ImGui::Button("Cancel"))
            {
                close();
            }
            ImGui::SameLine();

            std::string button_text = (m_mode == Mode::Open) ? "Open" : "Save";
            if (ImGui::Button(button_text.c_str()))
            {
                if (m_mode == Mode::Open)
                {
                    if (!m_selected_path.empty() && std::filesystem::is_regular_file(m_selected_path))
                    {
                        if (m_callback)
                        {
                            m_callback(m_selected_path);
                        }
                        close();
                    }
                }
                else // Save
                {
                    if (strlen(m_filename_buffer) > 0)
                    {
                        std::filesystem::path file_path = m_current_path / m_filename_buffer;
                        if (m_callback)
                        {
                            m_callback(file_path);
                        }
                        close();
                    }
                }
            }

            if (!m_dialog_open)
            {
                m_open = false;
            }
        }
        ImGui::End();
    }

    void FileDialogWidget::navigateTo(const std::filesystem::path& path)
    {
        if (std::filesystem::is_directory(path))
        {
            m_current_path = path;
            updateCurrentPath();
            m_selected_path.clear();
        }
    }

    void FileDialogWidget::updateCurrentPath()
    {
        m_current_directory_entries.clear();

        try
        {
            if (std::filesystem::exists(m_current_path) && std::filesystem::is_directory(m_current_path))
            {
                for (const auto& entry : std::filesystem::directory_iterator(m_current_path))
                {
                    // Filter by extension if filter is specified (exact extension match)
                    if (!m_filter.empty() && std::filesystem::is_regular_file(entry))
                    {
                        std::string ext = entry.path().extension().string();
                        if (ext.empty() || ext != m_filter)
                        {
                            continue;
                        }
                    }

                    m_current_directory_entries.push_back(entry.path());
                }

                // Sort: directories first, then files, both alphabetically
                std::sort(m_current_directory_entries.begin(),
                          m_current_directory_entries.end(),
                          [](const std::filesystem::path& a, const std::filesystem::path& b) {
                              bool a_is_dir = std::filesystem::is_directory(a);
                              bool b_is_dir = std::filesystem::is_directory(b);
                              if (a_is_dir != b_is_dir)
                                  return a_is_dir;
                              return a.filename().string() < b.filename().string();
                          });
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            // Handle error silently or log it
        }
    }

} // namespace RealmEngine
