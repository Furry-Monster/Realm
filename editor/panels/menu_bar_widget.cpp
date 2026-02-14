#include "panels/menu_bar_widget.h"

#include "core/log/log_macros.h"
#include "engine.h"
#include "panels/file_dialog_widget.h"
#include "platform/window/window.h"
#include "renderer/renderer.h"
#include "resource/config_manager.h"
#include "rhi/rhi_device.h"
#include "scene/scene_manager.h"

#include <imgui.h>
#include <filesystem>

namespace RealmEngine
{
    MenuBarWidget::MenuBarWidget(Engine& engine) : Widget("MenuBar"), m_engine(engine) {}

    void MenuBarWidget::render()
    {
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

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
            SceneManager&  scene_mgr = m_engine.getSceneManager();
            ConfigManager& config    = m_engine.getConfig();

            if (ImGui::MenuItem("New Scene"))
            {
                auto new_scene = scene_mgr.createDefaultScene(m_engine.getRenderer().getDevice());
                if (new_scene)
                {
                    scene_mgr.setCurrentScene(new_scene);
                    RE_LOG_INFO("New scene created");
                }
            }

            if (ImGui::MenuItem("Open Scene..."))
            {
                if (m_file_dialog)
                {
                    std::filesystem::path initial_path = config.getRootFolder();
                    m_file_dialog->open(FileDialogWidget::Mode::Open, "Open Scene", ".json", initial_path);
                }
            }

            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
                if (scene_mgr.getCurrentScene())
                {
                    std::filesystem::path scene_file =
                        config.getRootFolder() / config.getGamePlayConfig().scene_file;

                    if (scene_mgr.saveCurrentScene(scene_file.string()))
                        RE_LOG_INFO("Scene saved to: " + scene_file.string());
                    else
                        RE_LOG_ERROR("Failed to save scene to: " + scene_file.string());
                }
                else
                {
                    RE_LOG_INFO("No scene to save");
                }
            }

            if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
            {
                if (scene_mgr.getCurrentScene())
                {
                    std::filesystem::path scene_file =
                        config.getRootFolder() / config.getGamePlayConfig().scene_file;

                    if (scene_mgr.saveCurrentScene(scene_file.string()))
                        RE_LOG_INFO("Scene saved to: " + scene_file.string());
                    else
                        RE_LOG_ERROR("Failed to save scene to: " + scene_file.string());
                }
                else
                {
                    RE_LOG_INFO("No scene to save");
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                m_engine.getWindow().requestClose();
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
