#include "panels/menu_bar_widget.h"

#include "core/log/log_macros.h"
#include "editor_context.h"
#include "engine.h"
#include "panels/file_dialog_widget.h"
#include "platform/window/window.h"
#include "renderer/renderer.h"
#include "resource/config_manager.h"
#include "rhi/rhi_device.h"
#include "scene/components/camera_controller.h"
#include "scene/scene.h"
#include "scene/scene_manager.h"

#include <imgui.h>
#include <filesystem>

namespace RealmEngine
{
    MenuBarWidget::MenuBarWidget(Engine& engine) : Widget("MenuBar"), m_engine(engine) {}

    void MenuBarWidget::registerShortcuts()
    {
        if (!m_context || m_shortcuts_registered)
            return;

        auto& shortcuts = m_context->getShortcutSystem();

        shortcuts.registerShortcut(ImGuiMod_Ctrl | ImGuiKey_N, [this] {
            auto new_scene = m_engine.getSceneManager().createDefaultScene(m_engine.getRenderer().getDevice());
            if (new_scene)
            {
                m_engine.getSceneManager().setCurrentScene(new_scene);
                RE_LOG_INFO("New scene created");
            }
        });

        shortcuts.registerShortcut(ImGuiMod_Ctrl | ImGuiKey_O, [this] {
            if (m_file_dialog)
            {
                std::filesystem::path initial_path = m_engine.getConfig().getRootFolder();
                m_file_dialog->open(FileDialogWidget::Mode::Open, "Open Scene", ".json", initial_path);
            }
        });

        shortcuts.registerShortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S, [this] {
            if (m_file_dialog && m_engine.getSceneManager().getCurrentScene())
            {
                std::filesystem::path initial_path = m_engine.getConfig().getRootFolder();
                m_file_dialog->open(FileDialogWidget::Mode::Save, "Save Scene As", ".json", initial_path);
            }
        });

        shortcuts.registerShortcut(ImGuiMod_Ctrl | ImGuiKey_S, [this] {
            if (!m_engine.getSceneManager().getCurrentScene())
            {
                RE_LOG_INFO("No scene to save");
                return;
            }
            ConfigManager& config = m_engine.getConfig();
            std::filesystem::path scene_file = config.getRootFolder() / config.getGamePlayConfig().scene_file;
            if (m_engine.getSceneManager().saveCurrentScene(scene_file.string()))
                RE_LOG_INFO("Scene saved to: " + scene_file.string());
            else
                RE_LOG_ERROR("Failed to save scene to: " + scene_file.string());
        });

        shortcuts.registerShortcut(ImGuiMod_Alt | ImGuiKey_F4, [this] { m_engine.getWindow().requestClose(); });

        if (m_widgets && m_widgets->size() > 1)
        {
            auto toggle = [this](size_t idx) {
                if (idx < m_widgets->size())
                {
                    auto w = m_widgets->at(idx);
                    if (w)
                        w->setOpen(!w->isOpen());
                }
            };
            shortcuts.registerShortcut(ImGuiKey_F1, [toggle] { toggle(1); });
            shortcuts.registerShortcut(ImGuiKey_F2, [toggle] { toggle(2); });
            shortcuts.registerShortcut(ImGuiKey_F3, [toggle] { toggle(3); });
            shortcuts.registerShortcut(ImGuiKey_F4, [toggle] { toggle(4); });
            shortcuts.registerShortcut(ImGuiKey_F5, [toggle] { toggle(5); });
            shortcuts.registerShortcut(ImGuiKey_F6, [toggle] { toggle(6); });
        }

        m_shortcuts_registered = true;
    }

    void MenuBarWidget::render()
    {
        if (m_context)
            m_context->getShortcutSystem().process();

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

            if (ImGui::MenuItem("Reload Scene"))
            {
                if (scene_mgr.getCurrentScene())
                {
                    std::filesystem::path scene_file = config.getRootFolder() / config.getGamePlayConfig().scene_file;
                    if (std::filesystem::exists(scene_file))
                    {
                        auto loaded = scene_mgr.loadScene(scene_file.string(), m_engine.getRenderer().getDevice());
                        if (loaded)
                        {
                            scene_mgr.setCurrentScene(loaded);
                            const GamePlayConfig& gp = config.getGamePlayConfig();
                            loaded->getCameraController()->initialize(m_engine.getRenderer().getCamera(),
                                                                      m_engine.getInput(),
                                                                      gp.camera_mouse_sensitivity,
                                                                      gp.camera_move_speed,
                                                                      gp.camera_sprint_multiplier);
                            RE_LOG_INFO("Scene reloaded from: " + scene_file.string());
                        }
                        else
                            RE_LOG_ERROR("Failed to reload scene from: " + scene_file.string());
                    }
                    else
                        RE_LOG_WARN("Scene file not found: " + scene_file.string());
                }
            }

            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
                if (scene_mgr.getCurrentScene())
                {
                    std::filesystem::path scene_file = config.getRootFolder() / config.getGamePlayConfig().scene_file;

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
                if (m_file_dialog && scene_mgr.getCurrentScene())
                {
                    std::filesystem::path initial_path = config.getRootFolder();
                    m_file_dialog->open(FileDialogWidget::Mode::Save, "Save Scene As", ".json", initial_path);
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

    namespace
    {
        const char* getPanelShortcut(size_t idx)
        {
            switch (idx)
            {
                case 1: return "F1";
                case 2: return "F2";
                case 3: return "F3";
                case 4: return "F4";
                case 5: return "F5";
                case 6: return "F6";
                default: return nullptr;
            }
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
                        const char* shortcut = getPanelShortcut(i);
                        if (ImGui::MenuItem(widget->getName().c_str(), shortcut, &is_open))
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
