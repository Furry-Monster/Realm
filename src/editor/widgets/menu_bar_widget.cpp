#include "editor/widgets/menu_bar_widget.h"

#include "editor/widgets/file_dialog_widget.h"
#include "gameplay/scene/scene_manager.h"
#include "global_context.h"
#include "resource/config_manager.h"
#include "utils.h"
#include "window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <filesystem>

namespace RealmEngine
{
    MenuBarWidget::MenuBarWidget() : Widget("MenuBar") {}

    void MenuBarWidget::render()
    {
        if (ImGui::BeginMainMenuBar())
        {
            renderFileMenu();
            renderEditMenu();
            renderViewMenu();

            ImGui::EndMainMenuBar();
        }
    }

    void MenuBarWidget::renderFileMenu()
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene"))
            {
                auto new_scene = g_context.m_scene->createDefaultScene();
                if (new_scene)
                {
                    g_context.m_scene->setCurrentScene(new_scene);
                    RE_LOG_INFO("New scene created");
                }
            }

            if (ImGui::MenuItem("Open Scene..."))
            {
                if (m_file_dialog)
                {
                    std::filesystem::path initial_path = g_context.m_config->getRootFolder();
                    m_file_dialog->open(FileDialogWidget::Mode::Open, "Open Scene", ".json", initial_path);
                }
            }

            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
                if (g_context.m_scene->getCurrentScene())
                {
                    std::filesystem::path scene_file =
                        g_context.m_config->getRootFolder() / g_context.m_config->getGamePlayConfig().scene_file;

                    if (g_context.m_scene->saveCurrentScene(scene_file.string()))
                    {
                        RE_LOG_INFO("Scene saved to: " + scene_file.string());
                    }
                    else
                    {
                        RE_LOG_ERROR("Failed to save scene to: " + scene_file.string());
                    }
                }
                else
                {
                    RE_LOG_INFO("No scene to save");
                }
            }

            if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
            {
                // Use default path for now (file dialog can be added later)
                if (g_context.m_scene->getCurrentScene())
                {
                    std::filesystem::path scene_file =
                        g_context.m_config->getRootFolder() / g_context.m_config->getGamePlayConfig().scene_file;

                    if (g_context.m_scene->saveCurrentScene(scene_file.string()))
                    {
                        RE_LOG_INFO("Scene saved to: " + scene_file.string());
                    }
                    else
                    {
                        RE_LOG_ERROR("Failed to save scene to: " + scene_file.string());
                    }
                }
                else
                {
                    RE_LOG_INFO("No scene to save");
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                glfwSetWindowShouldClose(g_context.m_window->getGLFWWindow(), GLFW_TRUE);
            }

            ImGui::EndMenu();
        }
    }

    void MenuBarWidget::renderEditMenu()
    {
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false))
            {
                // TODO: Implement undo
            }

            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false))
            {
                // TODO: Implement redo
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Cut", "Ctrl+X", false, false))
            {
                // TODO: Implement cut
            }

            if (ImGui::MenuItem("Copy", "Ctrl+C", false, false))
            {
                // TODO: Implement copy
            }

            if (ImGui::MenuItem("Paste", "Ctrl+V", false, false))
            {
                // TODO: Implement paste
            }

            ImGui::EndMenu();
        }
    }

    void MenuBarWidget::renderViewMenu()
    {
        if (ImGui::BeginMenu("View"))
        {
            if (m_widgets)
            {
                for (size_t i = 1; i < m_widgets->size(); ++i) // Skip menu bar itself
                {
                    auto widget = m_widgets->at(i);
                    if (widget)
                    {
                        bool is_open = widget->isOpen();
                        if (ImGui::MenuItem(widget->getName().c_str(), nullptr, &is_open))
                        {
                            widget->setOpen(is_open);
                        }
                    }
                }
            }

            ImGui::EndMenu();
        }
    }

} // namespace RealmEngine
